#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

import os
import re
import subprocess
import time

from common.common import get_host_sn, execute_cmd, execute_cmd_list, write_content_to_file
from common.common import invoke_rpc_tool_interface
from common.const import RpcParamKey, BackupTypeEnum
from common.file_common import delete_file
from common.util.cmd_utils import cmd_format
from mysql import log
from mysql.src.common.constant import MysqlPrivilege, MysqlBackupToolName, MySQLPreTableLockStatus, ExecCmdResult, \
    XtrbackupErrStr, SystemConstant, SQLCMD
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.protect_mysql_base_utils import get_lock_ddl_param
from mysql.src.service.backup.backup_param import BackupParam
from mysql.src.utils.common_func import ErrCodeException, SQLParam, exec_mysql_sql_cmd, get_version_from_sql, \
    source_sql_script


class BackupTaskConstant:
    REPORT_INTERVAL = 15


def get_last_copy_info(backup_param: BackupParam | None, job_id, last_copy_type, application=None):
    application = application if application else backup_param.protect_object
    input_param = {
        RpcParamKey.APPLICATION: application,
        RpcParamKey.TYPES: last_copy_type,
        RpcParamKey.COPY_ID: "",
        RpcParamKey.JOB_ID: job_id
    }
    try:
        result = invoke_rpc_tool_interface(job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
    except Exception as err_info:
        log.error(f"Get last copy info fail.{err_info}")
        return {}
    return result


def check_sn_equals(backup_param: BackupParam, job_id: str):
    local_host = get_host_sn()
    out_info = get_last_copy_info(backup_param, job_id, [RpcParamKey.FULL_COPY])
    if not out_info:
        log.error(f"get last copy info fail.")
        return False
    copy_host_sn = out_info.get("extendInfo", {}).get("backupHostSN")
    return local_host == copy_host_sn


def check_backup_sn_in_nodes_sn(backup_param: BackupParam, job_id: str):
    out_info = get_last_copy_info(backup_param, job_id, [RpcParamKey.FULL_COPY])
    if not out_info:
        log.error(f"get last copy info fail.")
        return False
    copy_host_sn = out_info.get("extendInfo", {}).get("backupHostSN")
    for node in backup_param.get_nodes():
        node_id = node.get("id", "")
        if node_id == copy_host_sn:
            log.info(f"copy_sn:{copy_host_sn},node_id:{node_id} matches")
            return True
    log.error(f"copy_sn:{copy_host_sn},nodes:{backup_param.get_nodes()} not matches")
    return False


def get_backup_tool_name(version: str):
    if MysqlPrivilege.is_mariadb(version):
        if version.startswith("5."):
            return MysqlBackupToolName.XTRBACKUP2
        return MysqlBackupToolName.MARIADBBACKUP
    if version.startswith("8.0"):
        return MysqlBackupToolName.XTRBACKUP8
    return MysqlBackupToolName.XTRBACKUP2


def get_lock_ddl_cmd(backup_tool):
    lock_ddl_param = get_lock_ddl_param()
    if lock_ddl_param == MySQLPreTableLockStatus.OFF:
        return ''
    log.warning("Backup in case of emergency!")
    # mariadb 不支持lock-ddl参数
    if MysqlBackupToolName.MARIADBBACKUP == backup_tool:
        return f"--lock-ddl-per-table={lock_ddl_param}"
    return f"--lock-ddl=0 --lock-ddl-per-table={lock_ddl_param}"


def get_apply_only(backup_tool):
    if backup_tool == MysqlBackupToolName.MARIADBBACKUP:
        return ""
    return "--apply-log-only"


def exec_xtrabackup_cmd(cmd_param_str, tool_name=MysqlBackupToolName.XTRBACKUP2):
    env_cmd_str = f"{tool_name} {cmd_param_str}"
    ret_code, std_out, std_err = execute_cmd(env_cmd_str)
    if ret_code == ExecCmdResult.SUCCESS:
        # 成功不再打印输出
        return True, std_err
    log.warning(f"output: {std_out}, std_err:{std_err}")
    # 兼容1.2 1.2是在备份完成后prepare的， 重复prepare会报错，但是可以忽略，不影响恢复
    if XtrbackupErrStr.DATABASE_COPY_IS_PREPARED in std_err:
        return True, std_err
    if XtrbackupErrStr.CORRUPT_DATABASE_PAGE in std_err and \
            tool_name in [MysqlBackupToolName.XTRBACKUP2, MysqlBackupToolName.XTRBACKUP8]:
        raise ErrCodeException(MySQLErrorCode.EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL,
                               ["xtrabackup", XtrbackupErrStr.CORRUPT_DATABASE_PAGE], message=std_err)
    return False, std_err


def time_str_convert_timestamp(time_str):
    time_array = time.strptime(time_str, "%Y%m%d %H:%M:%S")
    timestamp = int(time.mktime(time_array))
    return timestamp


def parse_backup_lock_detail(output, backup_tool):
    # output为空，1、备份失败 2、不是执行的备份命令
    if not output:
        log.error("Backup failed or not backup cmd")
        return "", ""
    output = re.sub(r"\s+", " ", output)
    # 是执行的全局锁还是非innoDB表锁
    lock_name = "database"
    if output.find('Executing FLUSH TABLES WITH READ LOCK') == -1:
        lock_name = "non-InnoDB tables"
    # 获取MySQL的锁的时间
    try:
        log.info(f"backup_tool:{backup_tool}")
        if backup_tool == MysqlBackupToolName.XTRBACKUP2:
            lock_time = _parse_xtrabackup2_lock_time(output)
        elif backup_tool == MysqlBackupToolName.XTRBACKUP8:
            lock_time = _parse_xtrabackup8_lock_time(output)
        elif backup_tool == MysqlBackupToolName.MARIADBBACKUP:
            lock_time = _parse_mariadb_lock_time(output)
        else:
            lock_time = SystemConstant.DEFAULT_LOCK_TIME
    except Exception as except_str:
        log.error(f"Parse backup lock time failed.error:{except_str}")
        lock_time = "00:00:00"
    log.info(f"lock_name:{lock_name},lock_time:{lock_time}")
    return lock_name, lock_time


def _parse_xtrabackup2_lock_time(output):
    start_pattern = r"Executing FLUSH TABLES WITH READ LOCK...(.*?)Starting to backup non-InnoDB tables and files"
    match_start = re.search(start_pattern, output)
    end_pattern = r"Executing UNLOCK TABLES(.*?)All tables unlocked"
    match_end = re.search(end_pattern, output)
    if match_start and match_end:
        lock_time_start = time_str_convert_timestamp(f"20{match_start.group(1).strip()}")
        lock_time_end = time_str_convert_timestamp(f"20{match_end.group(1).strip()}")
        log.info(f"lock_time_start:{lock_time_start},lock_time_end:{lock_time_end}")
        return time.strftime("%H:%M:%S", time.gmtime(lock_time_end - lock_time_start))
    return SystemConstant.DEFAULT_LOCK_TIME


def _parse_xtrabackup8_lock_time(output):
    def parse_timestamp(origin_str):
        pattern = r'\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}'
        time_str = re.findall(pattern, origin_str)[0]
        return time.mktime(time.strptime(time_str, '%Y-%m-%dT%H:%M:%S'))

    start_pattern = r"Starting to backup non-InnoDB tables and files(.*?)Note"
    match_start = re.search(start_pattern, output)
    end_pattern = r"All tables unlocked(.*?)Note"
    match_end = re.search(end_pattern, output)
    if match_start and match_end:
        lock_time_start = parse_timestamp(match_start.group(1))
        lock_time_end = parse_timestamp(match_end.group(1))
        log.info(f"lock_time_start:{lock_time_start},lock_time_end:{lock_time_end}")
        return time.strftime("%H:%M:%S", time.gmtime(lock_time_end - lock_time_start))
    return SystemConstant.DEFAULT_LOCK_TIME


def _parse_mariadb_lock_time(output):
    def parse_timestamp(origin_str):
        pattern = r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}'
        time_str = re.findall(pattern, origin_str)[0]
        return time.mktime(time.strptime(time_str, '%Y-%m-%d %H:%M:%S'))

    start_pattern = r"Executing FLUSH TABLES WITH READ LOCK...(.*?)Starting to backup non-InnoDB tables and files"
    match_start = re.search(start_pattern, output)
    end_pattern = r"Executing UNLOCK TABLES(.*?)All tables unlocked"
    match_end = re.search(end_pattern, output)
    if match_start and match_end:
        lock_time_start = parse_timestamp(match_start.group(1))
        lock_time_end = parse_timestamp(match_end.group(1))
        log.info(f"lock_time_start:{lock_time_start}, lock_time_end:{lock_time_end}")
        return time.strftime("%H:%M:%S", time.gmtime(lock_time_end - lock_time_start))
    return SystemConstant.DEFAULT_LOCK_TIME


def check_copy_complete(copy_path):
    check_point_file = os.path.join(copy_path, "xtrabackup_checkpoints")
    if os.access(check_point_file, os.F_OK):
        conf = ReadFile.read_conf(check_point_file)
        if conf.get("last_lsn") == conf.get("flushed_lsn") and conf.get("last_lsn"):
            return True
    return False


def query_backup_process_alive_log(job_id, pid):
    cmd_str = ["ps -ef", f"grep '{job_id}'", "grep python3 ", f"grep -v '{pid}'"]
    ret, output, _ = execute_cmd_list(cmd_str)
    if ret == ExecCmdResult.SUCCESS and output:
        return True, f"{output}"
    return False, ""


def query_backup_process_alive(backup_type, job_id, pid, backup_tool):
    if backup_type == BackupTypeEnum.LOG_BACKUP.value:
        ret, output = query_backup_process_alive_log(job_id, pid)
        return ret, output
    cmd_str_xtrabackup = [
        "ps -ef",
        cmd_format("grep '{}'", job_id),
        cmd_format("grep '{}'", backup_tool),
        "grep -v grep"
    ]
    ret_xtrabackup, output_xtrabackup, _ = execute_cmd_list(cmd_str_xtrabackup)
    if ret_xtrabackup == ExecCmdResult.SUCCESS and output_xtrabackup:
        return True, f"{output_xtrabackup}"
    return False, ""


def is_binlog_continuous(binlog_name: str, binlog_names: [str]):
    if binlog_name in binlog_names:
        return True
    binlog_index = int(re.sub(r'\D', "", binlog_name))
    min_binlog_index = int(re.sub(r'\D', "", binlog_names[0]))
    return binlog_index <= min_binlog_index <= binlog_index + 1


def optimize_tables(sql_param: SQLParam, mnt_path: str, version: str = ""):
    if not version:
        version = get_version_from_sql(sql_param)
    log.info(f"version:{version}")
    if str(version) not in SystemConstant.UNSUPPORTED_INSTANT_VERSION:
        return True, ""
    sql_param.sql = SQLCMD.select_instant
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret or not output:
        return True, ""
    optimize_table_sql = os.path.join(mnt_path, "optimize.sql")
    file_content = ""
    for i in output:
        table_full_name = i[0]
        dt_table = table_full_name.split("/")
        file_content += f"optimize table `{dt_table[0]}`.`{dt_table[1]}`"
    write_content_to_file(optimize_table_sql, file_content)
    ret, output = source_sql_script(sql_param, optimize_table_sql)
    log.info(f"optimize_tables ret:{ret},output:{output}")
    delete_file(optimize_table_sql)
    return ret, output


def support_parameter(cmd: str, parameter):
    try:
        ret, std_out, std_err = execute_cmd(f"{cmd} --help")
        if not ret:
            return False
        options_list = std_out.split("\n")
        for op in options_list:
            if parameter in op:
                return True
        return False
    except Exception as e:
        log.error("is_support_parameter error:{}".format(e))
        return False
