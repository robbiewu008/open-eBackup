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

import difflib
import os
import time

from common.cleaner import clear
from common.common import execute_cmd
from common.const import RepositoryDataTypeEnum
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file
from common.util.kmc_utils import Kmc
from mysql import log
from mysql.src.common.constant import RestoreType, \
    MySQLJsonConstant, MySQLClusterType, RoleType, MySQLRestoreStep, MysqlExecSqlError, \
    SystemConstant, SystemServiceType, ExecCmdResult, MySQLStrConstant, MysqlPrivilege
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import exec_sql, safe_get_environ, get_change_master_cmd, \
    get_cluster_type_from_target_env
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.utils.common_func import exec_mysql_sql_cmd, SQLParam, get_version_from_sql
from mysql.src.utils.mysql_utils import MysqlUtils
from mysql.src.utils.restore_func import parse_time_stamp, parse_xtrabackup_info, \
    convert_to_timestamp, convert_restore_binlog_files_str, get_bin_log_names, parse_log_meta, stop_slave, \
    reset_slave_all


# coding=utf-8


class MysqlRestoreBase(MysqlBase):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._restore_step = 0
        self._error_code = 0
        self._restore_time_stamp = json_param.get("job", {}).get("extendInfo", {}).get("restoreTimestamp", "")
        self.origin_log_path = os.path.dirname(self.get_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value))

    def pre_log_restore_param(self):
        try:
            connect_param_path = os.path.join(self._data_path, "connect_param.json")
            connect_param = ReadFile.read_param_file(connect_param_path)
        except Exception as exception_str:
            log.error(f"read connect param err:{exception_str}")
            connect_param = {}
        if connect_param:
            mysql_user = connect_param.get("user", self._mysql_user)
            encrypt_pwd = connect_param.get("passwd")
            try:
                mysql_passwd = Kmc().decrypt(encrypt_pwd)
            except Exception as err:
                log.warning(f"decrypt error,{err}")
                mysql_passwd = encrypt_pwd
        else:
            mysql_user = self._mysql_user
            mysql_passwd = safe_get_environ(self._mysql_pwd)
        return mysql_user, mysql_passwd

    def pre_log_restore(self, mysql_host, mysql_port, user, pass_str):
        max_allowed_packet = SystemConstant.MAX_ALLOWED_PACKET
        # 设置最大允许的数据包数量
        log.info(f"mysql_host {mysql_host}, mysql_port {mysql_port} user {user}")
        sql_param = SQLParam(mysql_host, mysql_port, user, pass_str)
        sql_param.sql = f"set global max_allowed_packet={max_allowed_packet};"
        ret, output = exec_mysql_sql_cmd(sql_param)
        # 设置最大等待超时时间
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")
        sql_param.sql = f"set global wait_timeout=288000;"
        ret, output = exec_mysql_sql_cmd(sql_param)
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")
        # 设置最大命令等待时间
        sql_param.sql = f"set global interactive_timeout=288000;"
        ret, output = exec_mysql_sql_cmd(sql_param)
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")

    def log_restore(self, full_copy_path, restore_type, database_name=''):
        if self._restore_step >= MySQLRestoreStep.LOG_RESTORE:
            log.debug(f"Step {MySQLRestoreStep.LOG_RESTORE} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        if restore_type == RestoreType.PXC_DB_COMMON_NODE:
            log.info(f"Get restore_type in step log restore: {restore_type} equal PXC DB COMMON NODE")
            self.write_restore_step_info(MySQLRestoreStep.LOG_RESTORE)
            return True
        mysql_user, pass_str = self.pre_log_restore_param()
        try:
            self.pre_log_restore(self._mysql_ip, self._mysql_port, mysql_user, pass_str)
            stop_datetime = parse_time_stamp(self._restore_time_stamp)
            xtrabackup_info_path = os.path.join(full_copy_path, 'xtrabackup_info')
            if not os.path.exists(xtrabackup_info_path):
                ret, origin_copy_path = self.get_restore_copy_path()
                xtrabackup_info_path = os.path.join(origin_copy_path, "xtrabackup_info")
            if not os.path.exists(xtrabackup_info_path):
                log.error(f"xtrabackup_info_path:{xtrabackup_info_path} not exits")
                return False
            xtrabackup_info = parse_xtrabackup_info(xtrabackup_info_path)
            start_position = xtrabackup_info.get("binlog_position")
            log_start_time = xtrabackup_info.get("end_time")
            binlog_file = xtrabackup_info.get("binlog_filename")
            log.info(f"start_position:{start_position},log_start_time:{log_start_time},binlog_filename:{binlog_file}")
            if not start_position:
                return False
            ret, restore_files = self.get_log_restore_files(convert_to_timestamp(log_start_time), binlog_file)
            log.info(f"ret:{ret}, restore_files:{restore_files}")
            if not ret:
                log.error(f"get restore_files error,{self.get_log_comm()}")
                return False
            database_cmd = f"--database={database_name}" if database_name else ""
            skip_gtids_cmd = self.get_skip_gtids_cmd()
            restore_cmd = f"mysqlbinlog --no-defaults {database_cmd} {skip_gtids_cmd} {restore_files} " \
                          f"--disable-log-bin " \
                          f"--start-position={start_position} " \
                          f"--stop-datetime='{stop_datetime}' | mysql -u{mysql_user} -P{self._mysql_port} " \
                          f"-h{self._mysql_ip} -p'{pass_str}'"
            restore_exec_cmd = f"/bin/bash -c \"{restore_cmd}\""
            ret, out_str, err_str = execute_cmd(restore_exec_cmd)
            log.info(f"ret:{ret}, out_str:{out_str}, err_str:{err_str}")
            if not ret:
                log.error(f"restore log failed,output:{out_str}, err_str:{err_str},{self.get_log_comm()}")
                return False
            log.info(f"restore log success,output:{out_str},err_str:{err_str},{self.get_log_comm()}")
            self.write_restore_step_info(MySQLRestoreStep.LOG_RESTORE)
            return True
        finally:
            clear(pass_str)

    def get_log_restore_files(self, copy_end_timestamp, start_binlog_name):
        restore_copy_id = self._json_param.get(MySQLJsonConstant.JOB, {}).get(
            MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.RESTORE_COPY_ID, "")
        meta_file = os.path.join(self.origin_log_path, f"{restore_copy_id}.meta")
        log.info(f"meta_file:{meta_file}")
        if not os.path.exists(meta_file):
            log.error(f"Get timestamp id dict is empty. {self.get_log_comm()}")
            return False, []
        log_meta_dict = parse_log_meta(meta_file)
        log.info(f"Get timestamp id dict is {log_meta_dict}.{self.get_log_comm()}")
        restore_end_time = int(self._restore_time_stamp)
        sub_dir_list = self.filter_related_binlog_dirs(log_meta_dict, restore_end_time, copy_end_timestamp)
        sub_dir_list.sort(key=lambda x: list(x.values())[0])
        log.info(f"sub_dir_list:{sub_dir_list}")
        sub_dir_list = sub_dir_list[::-1]
        restore_file_list = []
        restore_binlog_names = []
        for dir_dict in sub_dir_list:
            key, value = dir_dict.popitem()
            log.info(f"dir_name:{key},end_time:{value}")
            bin_log_copy_dir = self.get_bin_log_copy_dir(key)
            bin_log_names = get_bin_log_names(bin_log_copy_dir)
            log.info(f"bin_log_names:{bin_log_names}")
            for bin_log_name in bin_log_names:
                if bin_log_name in restore_binlog_names:
                    continue
                restore_binlog_names.append(bin_log_name)
                restore_file = os.path.join(bin_log_copy_dir, bin_log_name)
                restore_file_list.append(restore_file)
                if bin_log_name != start_binlog_name:
                    continue
                log.info(f"restore_files:{restore_file_list}")
                return True, convert_restore_binlog_files_str(restore_file_list)
        return True, convert_restore_binlog_files_str(restore_file_list)

    # 兼容老版本，多出 mysql_{copy_id}_{host_sn} 格式子文件
    def get_bin_log_copy_dir(self, log_copy_id):
        bin_log_copy_dir = os.path.join(self.origin_log_path, log_copy_id)
        for sub_file in os.listdir(bin_log_copy_dir):
            sub_file_path = os.path.join(bin_log_copy_dir, sub_file)
            if os.path.isdir(sub_file_path):
                return sub_file_path
        return bin_log_copy_dir

    def filter_related_binlog_dirs(self, log_meta_dict: dict, restore_end_time: int, copy_end_timestamp: int):
        log.info(f"restore_end_time:{restore_end_time},copy_end_timestamp:{copy_end_timestamp}")
        sub_dir_list = []
        for sub_dir in os.listdir(self.origin_log_path):
            log.info(f"sub_dir:{sub_dir}")
            sub_dir_path = os.path.join(self.origin_log_path, sub_dir)
            log.info(f"sub_dir_path:{sub_dir_path}")
            # 非文件、时间点恢复的开始时间大于等于时间戳结束，时间点恢复的结束时间小于等于时间戳开始，这三种情况不满足条件
            if not os.path.isdir(sub_dir_path) or sub_dir not in log_meta_dict:
                continue
            time_range = log_meta_dict[sub_dir]
            start_stamp = int(time_range.get("start_stamp"))
            end_stamp = int(time_range.get("end_stamp"))
            if start_stamp >= restore_end_time or end_stamp <= copy_end_timestamp:
                continue
            sub_dir_list.append({sub_dir: start_stamp})
        return sub_dir_list

    def get_skip_gtids_cmd(self):
        version = self.get_mysql_version()
        if MysqlPrivilege.is_mariadb(version):
            return ""
        return "--skip-gtids"

    def start_slave(self, channel=''):
        cmd = "start slave"
        if channel:
            cmd = cmd + cmd_format(" for channel '{}'", channel)
        try:
            exec_sql_param = self.generate_exec_sql_param()
            exec_sql_param.sql_str = cmd
            ret, output = exec_sql(exec_sql_param, self._data_path)
        except Exception as exception_str:
            output = str(exception_str)
            ret = False
        if not ret:
            if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
                self._error_code = MySQLErrorCode.RESTORE_CHECK_USER_FAILED
            log.error(f"Exec failed, sql:start slave. ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def exec_stop_slave(self, channel=''):
        cmd = "stop slave"
        if channel:
            cmd = cmd + cmd_format(" for channel '{}'", channel)
        try:
            exec_sql_param = self.generate_exec_sql_param()
            exec_sql_param.sql_str = cmd
            ret, output = exec_sql(exec_sql_param, self._data_path)
        except Exception as exception_str:
            output = str(exception_str)
            log.info(f"exec_stop_slave error:{exception_str}")
            ret = False
        if not ret:
            if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
                self._error_code = MySQLErrorCode.RESTORE_CHECK_USER_FAILED
            log.error(f"Exec_sql failed. sql:stop slave ret:%s, output:%s pid:%s jobId:%s",
                      ret, output, self._p_id, self._job_id)
            return False
        return True

    def restart_cluster(self, restore_type):
        ret, service_name = self.get_mysql_service_name()
        if not ret:
            log.error(f"Get mysql service name failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret, system_service_type = self.get_mysql_system_service_type()
        if self._restore_step >= MySQLRestoreStep.RESTART:
            log.debug(f"Step {MySQLRestoreStep.RESTART} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        if restore_type in [RestoreType.PXC_INSTANCE_CLUSTER_NODE, RestoreType.PXC_DB_CLUSTER_NODE]:
            log.info(f"Get restore_type: {restore_type} equal PXC INSTANCE CLUSTER NODE")
            if system_service_type == SystemServiceType.SYSTEMCTL:
                cmd_str = f"systemctl start {MySQLStrConstant.MYSQLPXCSERVICES}"
            else:
                cmd_str = f"service {MySQLStrConstant.MYSQLPXCSERVICES} start"
        elif system_service_type == "manual":
            _, my_cnf_path = self.find_mycnf_path(self.my_cnf_path)
            user_name = MysqlUtils.get_data_path_user_name(my_cnf_path)
            cmd_str = f"mysqld --defaults-file={my_cnf_path} --user={user_name} --port={self._mysql_port} &"
        else:
            if system_service_type == SystemServiceType.SYSTEMCTL:
                cmd_str = f"systemctl restart {service_name}"
            else:
                cmd_str = f"service {service_name} restart"
        log.info(f"cmd_str:{cmd_str} begin, pid:{self._p_id} jobId:{self._job_id}")
        # execute_cmd 在linux red hat 8.7及以上执行命令存在阻塞问题。
        ret = os.system(f"{cmd_str} 2>&1 > /dev/null")
        log.info(f"cmd_str:{cmd_str} end,ret:{ret},pid:{self._p_id} jobId:{self._job_id}")
        if str(ret) != ExecCmdResult.SUCCESS:
            log.error(f"Exec systemctl restart mysql failed.")
            return False
        log.info(f"Start {service_name} success.")
        self.write_restore_step_info(MySQLRestoreStep.RESTART)
        return True

    def read_master_status_info(self):
        """
        从cache仓读取主节点的master status信息
        :return:
        """
        self.set_cache_path()
        if not self._cache_path:
            return False, {}
        copy_info_path = os.path.join(self._cache_path, "master_info.json")
        json_dict = ReadFile.read_param_file(copy_info_path)
        if not json_dict:
            return False, {}
        return True, json_dict

    def clear_slave_info(self):
        mysql_user, pass_str = self.pre_log_restore_param()
        try:
            sql_param = SQLParam(host=self._mysql_ip, port=self._mysql_port, user=mysql_user, passwd=pass_str)
            ret = stop_slave(sql_param)
            if not ret:
                log.warning("stop slave false")
            ret = reset_slave_all(sql_param)
            if not ret:
                log.warning("reset slave false")
            return ret
        finally:
            clear(pass_str)

    def operate_slave_node_in_ap_cluster(self, restore_type):
        log.info(f"restore_type:{restore_type}")
        if restore_type not in [RestoreType.AP_INSTANCE_CLUSTER_SLAVE_NODE, RestoreType.AP_DB_CLUSTER_SLAVE_NODE]:
            log.info(f"Step {MySQLRestoreStep.STOP_SLAVE} has done. pid:{self._p_id} jobId:{self._job_id}")
            self.write_restore_step_info(MySQLRestoreStep.STOP_SLAVE)
            return True
        ret = self.clear_slave_info()
        if not ret:
            log.error(f"clear slave info failed,{self.get_log_comm()}")
            return False
        ret, master_info = self.read_master_status_info()
        if not ret:
            log.error(f"Exec read master status info. pid:{self._p_id} jobId{self._job_id}")
            return False
        cmd_str = get_change_master_cmd(master_info)
        log.info(f"cmd_str:{cmd_str}")
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = cmd_str
        ret, output = exec_sql(exec_sql_param, self._data_path)
        if not ret:
            log.error(f"Exec_sql failed. sql:change master. ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
                self._error_code = MySQLErrorCode.RESTORE_CHECK_USER_FAILED
            return False
        if not self.start_slave():
            log.error("Failed to start slave. jobId:%s", self._job_id)
            return False
        self.write_restore_step_info(MySQLRestoreStep.STOP_SLAVE)
        return True

    def write_master_info(self):
        """
        仅有AP集群写入master_info，兼容mysql 8.0 默认的master_info_repository=TABLE
        """
        cluster_type = get_cluster_type_from_target_env(self._json_param)
        log.info(f"cluster_type:{cluster_type}")
        if cluster_type != MySQLClusterType.AP:
            return True
        passwd_str = ""
        try:
            passwd_str = f"{safe_get_environ(self._mysql_pwd)}"
            master_info_parse_param = {
                "master_port": self._mysql_port,
                "master_user": self._mysql_user,
                "master_password": passwd_str
            }
            exec_sql_param = self.generate_exec_sql_param()
            exec_sql_param.sql_str = "show master status"
            ret, output = exec_sql(exec_sql_param)
            if not ret:
                log.error(f"Exec sql failed. sql:show master status \
                                                 ret:{ret}  pid:{self._p_id} jobId{self._job_id}")
                return False
            for i in output:
                temp_str = i[0]
                temp_list = [f"{temp_str}"]
                log_bin_val = 'mysql_bin'
                temp_log_bin_val = MysqlUtils.get_log_bin_by_config_file(self.my_cnf_path)
                if (temp_log_bin_val is not None) and len(temp_log_bin_val) != 0 and (not temp_log_bin_val.isspace()):
                    log_bin_val = temp_log_bin_val
                temp_matches = difflib.get_close_matches(log_bin_val, temp_list, 9, 0.5)
                if temp_matches:
                    master_info_parse_param.update({
                        "master_binlog_file": i[0],
                        "master_binlog_pos": i[1]
                    })
                    break
            node_list = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                get(MySQLJsonConstant.TARGETENV, {}).get(MySQLJsonConstant.NODES, [])
            master_host = ""
            for node in node_list:
                if node.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.ROLE, "") == RoleType.ACTIVE_NODE:
                    master_host = node.get("endpoint", "")
            master_info_parse_param.update({"master_host": master_host})
            self.set_cache_path()
            if not self._cache_path:
                return False
            master_info_path = os.path.join(self._cache_path, "master_info.json")
            log.info(f"master_info_parse_param:{master_info_parse_param}")
            exec_overwrite_file(master_info_path, master_info_parse_param)
            return True
        finally:
            clear(passwd_str)

    def operate_master_node_in_ap_cluster(self, restore_type):
        if restore_type not in [RestoreType.AP_INSTANCE_CLUSTER_MASTER_NODE, RestoreType.AP_DB_CLUSTER_MASTER_NODE]:
            log.info(f"not ap cluster,task done,pid:{self._p_id} jobId:{self._job_id}")
            self.write_restore_step_info(MySQLRestoreStep.OPERATE_MASTER_NODE)
            return True
        ret = self.clear_slave_info()
        if not ret:
            log.warning(f"clear slave info failed,{self.get_log_comm()}")
        ret = self.write_master_info()
        if not ret:
            log.error(f"Exec write master status info. pid:{self._p_id} jobId{self._job_id}")
            return False
        self.write_restore_step_info(MySQLRestoreStep.OPERATE_MASTER_NODE)
        return True

    def wait_mysql_ready(self):
        mysql_user, pass_str = self.pre_log_restore_param()
        try:
            sql_param = self.generate_sql_param()
            sql_param.user = mysql_user
            sql_param.passwd = pass_str
            for _ in range(30):
                version = get_version_from_sql(sql_param)
                if version:
                    return
                time.sleep(10)
            log.warning(f"wait mysql ready timeout. pid:{self._p_id} jobId{self._job_id}")
        finally:
            clear(pass_str)
