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
import signal
import time
import uuid
from concurrent.futures import ThreadPoolExecutor
from functools import wraps

from common.cleaner import clear_repository_dir
from common.common import get_host_sn, output_execution_result, report_job_details
from common.common_models import SubJobDetails, LogDetail
from common.const import BackupTypeEnum, RpcParamKey
from common.const import DBLogLevel, ReportDBLabel
from common.const import SubJobStatusEnum
from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import exec_mkdir_cmd, exec_overwrite_file, exec_cp_cmd
from common.util.kmc_utils import Kmc
from common.util.scanner_utils import scan_dir_size
from mysql import log
from mysql.src.common.constant import MySQLClusterType, MysqlLabel, MySQLParamType
from mysql.src.common.constant import MysqlPrivilege, MysqlBackupToolName
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.protect_mysql_base_utils import check_tool_exist
from mysql.src.service.backup.backup_func import get_backup_tool_name, \
    get_lock_ddl_cmd, exec_xtrabackup_cmd, get_apply_only, BackupTaskConstant, \
    parse_backup_lock_detail, check_copy_complete, query_backup_process_alive, get_last_copy_info, \
    is_binlog_continuous, optimize_tables, support_parameter
from mysql.src.service.backup.backup_param import BackupParam
from mysql.src.service.base_service import BaseService
from mysql.src.utils.common_func import get_instant_tables, get_version_from_sql, flush_logs, \
    validate_my_cnf, find_default_my_cnf_path, get_data_dir_from_sql, get_binlog_filenames, exec_cmd_spawn
from mysql.src.utils.restore_func import parse_xtrabackup_info, convert_to_timestamp


class BackupDecorator:

    @staticmethod
    def report_progress(start_label=ReportDBLabel.BACKUP_SUB_START_COPY, end_label=ReportDBLabel.SUB_JOB_SUCCESS,
                        error_label=ReportDBLabel.SUB_JOB_FALIED):
        def func_wrapper(func):
            @wraps(func)
            def wrapper(self, *args, **kwargs):
                report_job_details(self.pid, self.build_start_details(start_label))
                pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix="subtask thread")
                copy_feature = pool.submit(func, self, *args, **kwargs)
                while not copy_feature.done():
                    time.sleep(BackupTaskConstant.REPORT_INTERVAL)
                    report_job_details(self.pid, self.build_running_details())
                try:
                    copy_feature.result()
                except Exception as error:
                    if isinstance(error, ErrCodeException):
                        log.error(
                            f"code:{error.error_code},message:{error.error_message},params:{error.parameter_list}")
                        job_detail = self.build_error_details(error_label, code=int(error.error_code),
                                                              params=error.parameter_list)
                    else:
                        job_detail = self.build_error_details(error_label, code=MySQLErrorCode.SYSTEM_ERROR,
                                                              params=[])
                    report_job_details(self.pid, job_detail)
                    log.error(f"task execute err,{error}")
                    pool.shutdown()
                    return False
                log.info(f"end_label:{end_label} pid:{self.pid},func:{func}")
                report_job_details(self.pid, self.build_end_details(end_label))
                pool.shutdown()
                return True

            return wrapper

        return func_wrapper


class BackupService(BaseService):
    def __init__(self, job_id, sub_job_id, backup_param: BackupParam):
        super().__init__(job_id, sub_job_id, backup_param.pid)
        self.param: BackupParam = backup_param
        self.speed = 0
        self.size = 0

    def build_start_details(self, start_label):
        process = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=0,
                                taskStatus=SubJobStatusEnum.RUNNING)
        log_detail = LogDetail(logInfo=start_label, logInfoParam=[process.sub_task_id],
                               logLevel=DBLogLevel.INFO.value)
        process.log_detail = [log_detail]
        return process

    def build_error_details(self, error_label, code, params):
        process = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=100,
                                taskStatus=SubJobStatusEnum.FAILED)
        log_detail = LogDetail(logInfo=error_label, logInfoParam=[process.sub_task_id],
                               logLevel=DBLogLevel.ERROR.value, log_detail=code,
                               log_detail_param=params)
        process.log_detail = [log_detail]
        return process

    def build_end_details(self, end_label):
        process = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=100,
                                taskStatus=SubJobStatusEnum.COMPLETED)
        ret, size = scan_dir_size(self.job_id, self.param.get_copy_path(self.job_id))
        process.data_size = size
        log_detail = LogDetail(logInfo=end_label, logInfoParam=[process.sub_task_id],
                               logLevel=DBLogLevel.INFO.value)
        process.log_detail = [log_detail]
        return process

    def backup_prerequisite(self):
        version = get_version_from_sql(self.param.sql_param)
        log.info(f"backup_prerequisite version:{version},{self.get_log_comm()}")
        if not version:
            raise ErrCodeException(err_code=MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING)
        if self.param.is_log_backup:
            if not self.check_can_backup_log():
                raise ErrCodeException(MySQLErrorCode.BINLOG_CHECK_FAILED)
        if self.param.is_full_backup:
            clear_repository_dir(self.param.data_path)
        if not self.param.is_log_backup:
            instant_tables = get_instant_tables(self.param.sql_param, version)
            if instant_tables:
                log.error(f"unsupported instant tables:{instant_tables}")
                raise ErrCodeException(MySQLErrorCode.UNSUPPORTED_INSTANT_COLUMN, instant_tables)
        log.info(f"backup_prerequisite success,{self.get_log_comm()}")

    def find_valid_my_cnf(self):
        if self.param.my_cnf_path:
            ret = validate_my_cnf(self.param.my_cnf_path)
            if ret:
                return self.param.my_cnf_path
            return ""
        _, result = find_default_my_cnf_path()
        return result

    def allow_backup_in_local_node(self):
        self.check_cluster_allow()
        if not self.param.is_log_backup:
            self.allow_xtrabackup_backup_in_local()
        if not self.param.cluster_type == MySQLClusterType.EAPP:
            version = get_version_from_sql(self.param.sql_param)
            log.info(f"version:{version},{self.get_log_comm()}")
            if not version:
                log.error(f"mysql is not running,{self.get_log_comm()}")
                raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_NOT_RUNNING)
        if not check_tool_exist(MysqlBackupToolName.MYSQLBINLOG):
            log.error(f"mysqlbinlog not found,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_BIN_LOG_FAILED)
        if not check_tool_exist(MysqlBackupToolName.MYSQLDUMP):
            log.error(f"mysqldump not found,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.CHECK_BACKUP_TOOL_FAILED)
        log.info(f"check_allow_backup_in_local success,{self.get_log_comm()}")

    def check_cluster_allow(self):
        # 单机节点需要下发任务，epp各个节点都需要下发任务
        if not self.param.app_type == MySQLParamType.CLUSTER:
            return
        if not self.param.cluster_type or self.param.cluster_type == MySQLClusterType.EAPP:
            return
        if not self.param.is_active_node:
            raise ErrCodeException(MySQLErrorCode.ERR_DATABASE_STATUS)

    def allow_xtrabackup_backup_in_local(self):
        if not self.find_valid_my_cnf():
            log.error(f"validate my.cnf failed,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.CHECK_MYSQL_CONF_FAILED, message="validate my.cnf failed")
        backup_tool = get_backup_tool_name(self.param.get_version())
        if not check_tool_exist(backup_tool):
            log.error(f"backup tool not found,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.CHECK_BACKUP_TOOL_FAILED)

    def check_can_backup_log(self):
        binlog_filenames = get_binlog_filenames(self.param.sql_param)
        log.info(f"binlog_filenames:{binlog_filenames}")
        last_log_copy_info = get_last_copy_info(self.param, self.job_id, [RpcParamKey.LOG_COPY])
        last_full_copy_info = get_last_copy_info(self.param, self.job_id,
                                                 [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY,
                                                  RpcParamKey.DIFF_COPY])
        log.info(f"last_full_copy_info:{last_full_copy_info}")
        if not last_full_copy_info:
            log.info(f"last full copy is empty:{self.get_log_comm()}")
            return False
        if last_log_copy_info:
            full_copy_backup_time = int(last_full_copy_info.get("extendInfo", {}).get("backupTime", 0))
            last_log_copy_end_time = int(last_log_copy_info.get("extendInfo", {}).get("endTime", 0))
            # 如果上一次日志备份的时间大于全备/增备/差异 备份的时间，则说明上一次是日志备份，需要判断当前备份的binlog是否连续
            if last_log_copy_end_time >= full_copy_backup_time:
                last_binlog_name, _ = self.get_log_meta_info()
                log.info(f"last_binlog_name:{last_binlog_name},binlog_filenames:{binlog_filenames}")
                return is_binlog_continuous(last_binlog_name, binlog_filenames)
        backup_filename = last_full_copy_info.get("extendInfo", {}).get("backupBinlog")
        log.info(f"backup_filename:{backup_filename},binlog_filenames:{binlog_filenames}")
        return is_binlog_continuous(backup_filename, binlog_filenames)

    def get_log_meta_info(self):
        sn_file_path = self.param.get_meta_host_sn_file_path()
        try:
            read_json = ReadFile.read_param_file(sn_file_path)
        except Exception as err_exception:
            log.error(f"err_exception:{err_exception}")
            return "", ""
        last_start_time = read_json.get("logFlagStartTime", "")
        last_binlog_name = read_json.get("logFlag", "")
        return last_binlog_name, last_start_time

    def prepare_backup(self, backup_tool, copy_path):
        if not os.access(copy_path, os.F_OK):
            return False
        # 杀掉同一job_id备份进程
        self.kill_backup_process(copy_path, backup_tool)
        if not self.param.is_full_backup:
            return False
        ret = check_copy_complete(copy_path)
        if ret:
            log.info(f"Backup copy is complete.{self.get_log_comm()}")
            return True
        return False

    def kill_backup_process(self, copy_path, backup_tool):
        old_size = 0
        start_time = 0
        while True:
            ret, output = query_backup_process_alive(self.param.backup_type, self.job_id, self.pid, backup_tool)
            if not ret:
                return
            ret, size = scan_dir_size(self.job_id, copy_path)
            if size != old_size:
                start_time = time.clock()
            # 2分钟目标目录没有发生变化，认为备份进程假死kill掉重做
            elif time.clock() - start_time >= 2 * 60 * 1000:
                log.error(f"Backup progress timeout. {self.get_log_comm()}")
                for line in output.splitlines():
                    pid = int(line.split()[1])
                    os.kill(pid, signal.SIGKILL)
                    log.info(f"os.kill {pid} pid:{self.get_log_comm()}")
            old_size = size
            time.sleep(0.05)

    @BackupDecorator.report_progress(start_label=ReportDBLabel.BACKUP_SUB_START_COPY,
                                     end_label=ReportDBLabel.SUB_JOB_SUCCESS,
                                     error_label=ReportDBLabel.BACKUP_SUB_FAILED)
    def exec_backup(self):
        backup_tool = get_backup_tool_name(self.param.get_version())
        ret = self.prepare_backup(backup_tool, self.param.get_copy_path(self.job_id))
        copy_path = self.param.get_copy_path(self.job_id)
        if ret:
            log.warning(f"exec prepare_sub_job success.no need to re-backup.{self.get_log_comm()}")
            self.write_copy_info(copy_path)
            return
        log.info(f"exec backup start,{self.get_log_comm()}")
        self.flush_log()
        my_cnf_path = self.find_valid_my_cnf()
        extra_param = self.extra_xtrabackup_param(backup_tool)
        log.info(f"exec backup param,tool:{backup_tool},my_cnf_path:{my_cnf_path},extra_param:{extra_param}")
        if self.param.backup_type == BackupTypeEnum.FULL_BACKUP.value:
            self.exec_full_backup(backup_tool, my_cnf_path, extra_param, copy_path)
            self.backup_connect_param(my_cnf_path)
            self.write_copy_info(copy_path)
        elif self.param.backup_type in [BackupTypeEnum.INCRE_BACKUP.value, BackupTypeEnum.DIFF_BACKUP.value]:
            self.exec_incr_backup(backup_tool, my_cnf_path, extra_param, copy_path)
            self.backup_connect_param(my_cnf_path)
            self.write_copy_info(copy_path)
        else:
            self.exec_log_backup(my_cnf_path)

    def backup_connect_param(self, my_cnf_path):
        connect_param_path = os.path.join(self.param.data_path, 'connect_param.json')
        encrypt_pwd = Kmc().encrypt(self.param.pass_wd)
        connect_param = {
            "user": self.param.sql_param.user,
            "passwd": encrypt_pwd
        }
        exec_overwrite_file(connect_param_path, connect_param)
        origin_backup_cnf = os.path.join(self.param.data_path, 'backup.cnf')
        log.info(f"meta_path_my_cnf:{origin_backup_cnf}")
        exec_cp_cmd(my_cnf_path, origin_backup_cnf, is_check_white_list=False)

    def write_copy_info(self, copy_path):
        log.info(f"write_copy_info start:{self.get_log_comm()}")
        copy_info_path = os.path.join(self.param.cache_path, f"copy_info_{self.job_id}")
        copy_json = self.param.get_copy()
        xtrabackup_info_path = os.path.join(copy_path, "xtrabackup_info")
        xtrabackup_info = parse_xtrabackup_info(xtrabackup_info_path)
        backup_time = convert_to_timestamp(xtrabackup_info.get("end_time"))
        copy_json.update({
            "extendInfo": {
                "backupTime": backup_time,
                "backupHostSN": get_host_sn(),
                "backupBinlog": xtrabackup_info.get("binlog_filename")
            }
        })
        output_execution_result(copy_info_path, copy_json)
        log.info(f"write_copy_info success:{self.get_log_comm()}")

    def extra_xtrabackup_param(self, backup_tool):
        return ""

    def exec_full_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        optimize_tables(self.param.sql_param, copy_path, self.param.get_version())
        lock_ddl_cmd = get_lock_ddl_cmd(backup_tool)
        backup_cmd = f"--defaults-file={my_cnf_path} {extra_param} --backup " \
                     f"--parallel={self.param.get_channel_number()} {lock_ddl_cmd} --host={self.param.mysql_ip} " \
                     f"--port={self.param.port}  --user={self.param.user} --password='{self.param.pass_wd}' " \
                     f"--target-dir={copy_path}"
        ret, backup_output = exec_xtrabackup_cmd(backup_cmd, backup_tool)
        if not ret:
            log.error(f"full backup cmd execute error,output:{backup_output},{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

        prepare_cmd = (f"--prepare {get_apply_only(backup_tool)} --parallel={self.param.get_channel_number()} "
                       f"--target-dir={copy_path}")
        ret, output = exec_xtrabackup_cmd(prepare_cmd, backup_tool)
        if not ret:
            log.error(f"full prepare cmd execute error,output:{output},{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        log.info(f"exec full backup success,{self.get_log_comm()}")
        self.report_lock_detail(backup_output, backup_tool)

    def exec_incr_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        optimize_tables(self.param.sql_param, copy_path, self.param.get_version())
        lock_ddl_cmd = get_lock_ddl_cmd(backup_tool)
        increment_dir = os.path.join(self.param.cache_path, str(uuid.uuid4()))
        backup_cmd = f"--defaults-file={my_cnf_path} {extra_param} --backup " \
                     f"--parallel={self.param.get_channel_number()} {lock_ddl_cmd} --host={self.param.mysql_ip} " \
                     f"--port={self.param.port}  --user={self.param.user} --password='{self.param.pass_wd}' " \
                     f"--target-dir={increment_dir} --incremental-basedir={copy_path}"
        ret, backup_output = exec_xtrabackup_cmd(backup_cmd, backup_tool)
        if not ret:
            log.error(f"incr backup cmd execute error,output:{backup_output},{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        prepare_cmd = (f"--prepare {get_apply_only(backup_tool)} --parallel={self.param.get_channel_number()} "
                       f"--target-dir={copy_path} --incremental-dir={increment_dir}")
        ret, output = exec_xtrabackup_cmd(prepare_cmd, backup_tool)
        if not ret:
            log.error(f"incr prepare cmd execute error,output:{output},{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        log.info(f"exec incr backup success,{self.get_log_comm()}")
        self.report_lock_detail(backup_output, backup_tool)

    def report_lock_detail(self, output, backup_tool):
        sub_job_detail = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=50,
                                       taskStatus=SubJobStatusEnum.RUNNING)
        lock_name, lock_time = parse_backup_lock_detail(output, backup_tool)
        log.info(f"lock_name:{lock_name}, lock_time:{lock_time},{self.get_log_comm()}")
        if not lock_name or not lock_time:
            return
        log_detail = LogDetail(logInfo=MysqlLabel.BACKUP_LOCK_TABLE_DETAIL, logInfoParam=[lock_name, lock_time],
                               logLevel=DBLogLevel.WARN.value)
        sub_job_detail.log_detail = [log_detail]
        report_job_details(self.job_id, sub_job_detail)

    def prepare_log_backup(self, copy_path):
        self.flush_log()
        self.kill_backup_process(copy_path, "")
        if os.path.exists(copy_path):
            clear_repository_dir(copy_path)
        else:
            return exec_mkdir_cmd(copy_path)
        return True

    def exec_log_backup(self, my_cnf_path):
        pass

    def backup_post(self):
        backup_result = self.param.get_backup_job_result()
        if backup_result != 0:
            log.info(f"backup result is {backup_result},clear data dir")
            clear_repository_dir(self.param.get_copy_path(self.job_id))
        clear_repository_dir(self.param.cache_path)

    def flush_log(self):
        result = flush_logs(self.param.sql_param)
        if not result:
            log.error("flush logs error")
            raise ErrCodeException(MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING)

    def query_backup_copy(self):
        copy_info_path = os.path.join(self.param.cache_path, f"copy_info_{self.job_id}")
        return ReadFile.read_param_file(copy_info_path)


class SingleDatabaseService(BackupService):

    def backup_prerequisite(self):
        super().backup_prerequisite()
        self.check_backup_version()
        data_dir = get_data_dir_from_sql(self.param.sql_param)
        database_name = self.param.get_backup_database()
        for path in os.listdir(data_dir):
            if os.path.isfile(path):
                continue
            if path == database_name:
                return True
        raise ErrCodeException(MySQLErrorCode.EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL,
                               ["backup", f"The database {database_name} has been destroyed."],
                               message=f"The database {database_name} has been destroyed.")

    def check_backup_version(self):
        version = get_version_from_sql(self.param.sql_param)
        log.info(f"version:{version}")
        if MysqlPrivilege.is_mariadb(version):
            return
        try:
            major, _, minor = version.split('.')
            log.info(f"major:{major},minor:{minor}")
            if int(major) != 8:
                return
            if not minor:
                return
            minor = re.findall(r'\d+', minor)
            if not minor:
                return
            if int(minor[0]) <= 17:
                log.info(f"mysql version {version} don't use database backup,{self.get_log_comm()}")
                raise ErrCodeException(MySQLErrorCode.NOT_SUPPORT_DATABASE_BACKUP, ["can't use database backup"])
        except Exception as exception_str:
            if not isinstance(exception_str, ErrCodeException):
                log.warning(f"check_backup_version error,{exception_str},{self.get_log_comm()}")
                return
            raise exception_str

    def exec_full_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        super().exec_full_backup(backup_tool, my_cnf_path, extra_param, copy_path)
        self.backup_table_struct()
        log.info(f"full backup table struct success. {self.get_log_comm()}")

    def backup_table_struct(self):
        set_gtid_purged = "--set-gtid-purged=OFF"
        if MysqlPrivilege.is_mariadb(self.param.get_version()):
            set_gtid_purged = ""
        if not support_parameter("mysqldump", "--set-gtid-purged"):
            set_gtid_purged = ""
        database_name = self.param.get_backup_database()
        sql_file = os.path.join(self.param.get_copy_path(self.job_id), f"{database_name}.sql")
        backup_cmd = f"{MysqlBackupToolName.MYSQLDUMP} --user={self.param.user} -h{self.param.mysql_ip} " \
                     f"-P{self.param.port} --password --no-data --compact --add-drop-table --skip_add_locks " \
                     f"--skip-lock-tables --routines --triggers --events " \
                     f"{set_gtid_purged} {database_name} --result-file={sql_file}"
        ret, output = exec_cmd_spawn(backup_cmd, self.param.pass_wd)
        log.info(f"ret:{ret},output:{output}")
        if ret != 0:
            log.error(f"Backup table struct failed,output:{output} ,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        log.info(f"Backup table struct success. {self.get_log_comm()}")

    def extra_xtrabackup_param(self, backup_tool):
        return f"--databases={self.param.get_backup_database()}"

    def exec_incr_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        super().exec_incr_backup(backup_tool, my_cnf_path, extra_param, copy_path)
        self.backup_table_struct()
        log.info(f"incr backup table struct success. {self.get_log_comm()}")
