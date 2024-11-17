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

from common.exception.common_exception import ErrCodeException
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file
from mysql import log
from mysql.src.common.constant import MysqlExecSqlError
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import get_change_master_cmd
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.service.base_service import BaseService
from mysql.src.service.restore.restore_param import RestoreParam
from mysql.src.utils.common_func import reset_slave_all, exec_mysql_sql_cmd, find_log_bin_path_dir_from_cnf
from mysql.src.utils.restore_func import stop_slave, show_master_status


class InstanceRestore(BaseService):
    def __init__(self, job_id, sub_job_id, restore_param: RestoreParam):
        super().__init__(job_id, sub_job_id, restore_param.pid)
        self.param: RestoreParam = restore_param

    def exec_restore(self):
        pass

    def read_master_status_info(self):
        copy_info_path = os.path.join(self.param.cache_path, "master_info.json")
        json_dict = ReadFile.read_param_file(copy_info_path)
        if not json_dict:
            return False, {}
        return True, json_dict

    def clear_slave_info(self):
        self.param.reset_sql_param()
        ret = stop_slave(self.param.sql_param)
        if not ret:
            log.warning("stop slave false")
        ret = reset_slave_all(self.param.sql_param)
        if not ret:
            log.warning("reset slave false")
        return ret

    def start_slave(self, channel=''):
        cmd = "start slave"
        if channel:
            cmd = cmd + cmd_format(" for channel '{}'", channel)
        self.param.sql_param.sql = cmd
        ret, output = exec_mysql_sql_cmd(self.param.sql_param)
        if not ret:
            log.error(f"Exec failed, sql:start slave. {self.get_log_comm()}")
            if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
                raise ErrCodeException(MySQLErrorCode.RESTORE_CHECK_USER_FAILED)
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def operate_slave_node_in_ap_cluster(self):
        self.operate_slave_node_in_ap_cluster_before()
        self.operate_slave_node_in_ap_cluster_after()

    def operate_slave_node_in_ap_cluster_before(self):
        if not self.param.is_ap_node or self.param.is_active_node:
            return
        log.info(f"operate slave node in ap cluster:{self.get_log_comm()}")
        ret = self.clear_slave_info()
        if not ret:
            log.error(f"clear slave info failed,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)

    def operate_slave_node_in_ap_cluster_after(self):
        if not self.param.is_ap_node or self.param.is_active_node:
            return
        ret, master_info = self.read_master_status_info()
        if not ret:
            log.error(f"Exec read master status info. {self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        cmd_str = get_change_master_cmd(master_info)
        log.info(f"cmd_str:{cmd_str}")
        self.param.sql_param.sql = cmd_str
        ret, output = exec_mysql_sql_cmd(self.param.sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:change master. ret:{ret} {self.get_log_comm()}")
            if MysqlExecSqlError.ERROR_ACCESS_DENINED in output:
                raise ErrCodeException(MySQLErrorCode.RESTORE_CHECK_USER_FAILED)
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        self.start_slave()

    def operate_master_node_in_ap_cluster(self):
        if not self.param.is_ap_node or not self.param.is_active_node:
            return
        ret = self.clear_slave_info()
        if not ret:
            log.warning(f"clear slave info failed,{self.get_log_comm()}")
        master_binlog_file, master_binlog_pos = show_master_status(self.param.sql_param)
        if not master_binlog_file or not master_binlog_pos:
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        master_info_parse_param = {
            "master_port": self.param.port,
            "master_user": self.param.user,
            "master_password": self.param.pass_wd,
            "master_host": self.param.endpoint,
            "master_binlog_file": master_binlog_file,
            "master_binlog_pos": master_binlog_pos
        }
        master_info_path = os.path.join(self.param.cache_path, "master_info.json")
        exec_overwrite_file(master_info_path, master_info_parse_param)
        log.info(f"Exec write master status info. {self.get_log_comm()}")

    def get_restore_binlog_dir(self):
        ret, mysql_log_bin_dir = self.param.log_bin_path
        if os.path.isabs(mysql_log_bin_dir):
            if os.path.isdir(mysql_log_bin_dir):
                return str(mysql_log_bin_dir)
            return str(os.path.dirname(mysql_log_bin_dir))
        else:
            return find_log_bin_path_dir_from_cnf(self.param.my_cnf_path)
