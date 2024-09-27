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
import time
from threading import Thread

from common.util.exec_utils import exec_cp_cmd, exec_mkdir_cmd
from mysql import log
from common.cleaner import clear
from common.common import exter_attack
from common.const import SubJobStatusEnum
from common.util.check_utils import check_repo_path
from common.util.cmd_utils import cmd_format
from common.util.scanner_utils import scan_dir_size
from mysql.src.common.constant import MySQLProgressFileType, MysqlBackupToolName
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import safe_get_environ
from mysql.src.protect_mysql_base import MysqlBase


class MysqlBackupFull(MysqlBase):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._report_progress_thread_start = True
        self._data_dir = ""
        self._backup_time = 0
        self._data_size = 0
        self._error_code = 0
        self._log_detail_params = []

    def calc_progress(self):
        """
        计算备份进度
        :return 
        """
        return True, "5"

    def check_database_dir(self, database_name):
        if not database_name:
            log.info("Get database name failed.")
            return False
        data_dir = self.find_data_dir()
        if not data_dir:
            return False
        for path in os.listdir(data_dir):
            if os.path.isfile(path):
                continue
            if path == database_name:
                return True
        self._error_code = MySQLErrorCode.EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL
        self._log_detail_params = ["backup", f"The database {database_name} has been destroyed."]
        log.error(f"The database {database_name} has been destroyed.")
        return False

    def call_xtrabackup(self, my_cnf_path, copy_path):
        """
        执行备份命令
        :return 
        """
        database_cmd = ""
        passwd_str = ""
        app_log_only = "--apply-log-only"
        try:
            ret, database_name = self.get_backup_database()
            if ret:
                database_cmd = "--databases="
                if not self.check_database_dir(database_name):
                    return False
            channel_number = self.get_channel_number()
            lock_ddl_cmd = self.get_lock_ddl_cmd()
            tool_name = self.get_backup_tool_name()
            if tool_name == MysqlBackupToolName.MARIADBBACKUP:
                # mariaDB 就在命令码中拼接密码
                passwd_str = f"={safe_get_environ(self._mysql_pwd)}"
                # mariaDB 没有app_log_only 参数
                app_log_only = ""
            backup_cmd_param = cmd_format("--defaults-file={} {}{} \
                        --backup \
                        --parallel={} {}\
                        --host={} --user={} \
                        --password'{}' --port={} \
                        --target-dir={}", my_cnf_path, database_cmd, database_name, channel_number, lock_ddl_cmd,
                                          self._mysql_ip, self._mysql_user, passwd_str, self._mysql_port, copy_path)
            ret, output = self.exec_xtrabackup_cmd(backup_cmd_param, tool_name, self._mysql_pwd)
            self._backup_time = int(time.time())
        finally:
            clear(passwd_str)
        if not ret:
            log.error(f"Exec backup cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret = self.parse_backup_lock_detail(output)
        if not ret:
            log.error("Get backup lock time failed.")
            return False
        log.info(f"Exec backup cmd success. output: pid:{self._p_id} jobId:{self._job_id}")
        parper_cmd_param = cmd_format("--prepare {} --parallel={} --target-dir={}",
                                      app_log_only, channel_number, copy_path)
        ret, output = self.exec_xtrabackup_cmd(parper_cmd_param, tool_name)
        if not ret:
            log.error(f"Exec prepare cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Exec prepare cmd success. output pid:{self._p_id} jobId:{self._job_id}")
        if not database_cmd:
            return True
        # 数据库备份 需要导出表结构
        ret = self.backup_table_struct(database_name, copy_path)
        if not ret:
            return False
        return True

    @exter_attack
    def exec_sub_job(self):
        """
        执行备份任务
        :return 
        """
        log.info(f"Fullbackup begin. pid:{self._p_id} jobId:{self._job_id}")
        ret, copy_path = self.set_backup_all_param()
        if not ret:
            log.error(f"Exec set_backup_all_param failed. pid:{self._p_id} jobId:{self._job_id}")
            return False

        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1,
                                 MySQLProgressFileType.COMMON)
        progress_thread = Thread(target=self.report_backup_progress_thread)
        progress_thread.daemon = True
        progress_thread.start()
        ret = self.exec_backup(copy_path)
        self._report_progress_thread_start = False
        progress_thread.join()
        if not ret:
            return False
        # 副本信息记录cache仓
        ret = self.write_copy_info(self._backup_time)
        if not ret:
            log.error(f"Write_copy_info_failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret, size = scan_dir_size(self._job_id, copy_path)
        if ret and size != 0:
            self._data_size = size
        log.info(f"Success to get full copy size {self._data_size}.")
        log.info(f"Full backup success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def exec_backup(self, copy_path):
        ret = self.prepare_sub_job(copy_path)
        if ret:
            log.info(f"Exec prepare_sub_job success.No need to re-backup. pid:{self._p_id} jobId:{self._job_id}")
            return True

        ret, my_cnf_path = self.find_mycnf_path(self.my_cnf_path)
        if not ret:
            log.error(f"Find my.cnf failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Find my.cnf success. pid:{self._p_id} jobId:{self._job_id}")
        ret = self.flush_log()
        if not ret:
            return False
        # 安全校验
        if not check_repo_path(copy_path):
            log.error("Check full copy_path invalid.")
            return False
        ret = self.call_xtrabackup(my_cnf_path, copy_path)
        if not ret:
            return False
        self.backup_my_cnf_files(my_cnf_path)
        ret, tmp_time = self.get_backup_log_time(copy_path)
        self.backup_connect_param()
        if ret and tmp_time != 0:
            self._backup_time = int(tmp_time)
        return True

    def backup_my_cnf_files(self, my_cnf_path):
        origin_backup_cnf = os.path.join(self._data_path, 'backup.cnf')
        log.info(f"meta_path_my_cnf:{origin_backup_cnf}")
        exec_cp_cmd(my_cnf_path, origin_backup_cnf, is_check_white_list=False)
