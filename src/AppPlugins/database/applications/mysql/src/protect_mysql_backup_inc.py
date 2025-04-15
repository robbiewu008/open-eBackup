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

from common.cleaner import clear
from common.common import exter_attack
from common.const import SubJobStatusEnum
from common.util.check_utils import check_repo_path
from common.util.cmd_utils import cmd_format
from common.util.scanner_utils import scan_dir_size
from mysql import log
from mysql.src.common.constant import MySQLProgressFileType, MysqlBackupToolName
from mysql.src.common.execute_cmd import safe_get_environ, check_path_in_white_list
from mysql.src.common.model.backup_cmd_param import BackupCmdParamModel
from mysql.src.protect_mysql_backup_full import MysqlBackupFull
from mysql.src.protect_mysql_base import MysqlBase


class MysqlBackupInc(MysqlBase):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._report_progress_thread_start = True
        self._backup_time = 0
        self._data_size = 0
        self._average_speed = 0
        self._full_backup = MysqlBackupFull(p_id, job_id, sub_job_id, json_param)

    @staticmethod
    def calc_progress():
        return True, "5"

    def call_xtrabackup(self, my_cnf_path, copy_path, copy_full_path):
        """
        执行备份命令
        :return 
        """
        passwd_str = ""
        database_cmd = ""
        app_log_only = "--apply-log-only"
        try:
            ret, database_name = self.get_backup_database()
            if ret:
                database_cmd = "--databases="
                if self._full_backup.check_database_dir(database_name):
                    return False
            tool_name = self.get_backup_tool_name()
            if tool_name == MysqlBackupToolName.MARIADBBACKUP:
                # mariaDB 不能命令码中拼接密码，会有ps明文密码安全问题
                passwd_str = f"{safe_get_environ(self._mysql_pwd)}"
                # mariaDB 没有app_log_only 参数
                app_log_only = ""
                os.environ["MYSQL_PWD"] = passwd_str
            backup_param_model = BackupCmdParamModel(copy_full_path, copy_path, database_cmd, database_name,
                                                     my_cnf_path, passwd_str, tool_name)
            backup_cmd_param = self.build_backup_cmd_param(backup_param_model)

            begin_time = time.time()
            ret, output = self.exec_xtrabackup_cmd(backup_cmd_param, tool_name, self._mysql_pwd)
            self._backup_time = int(time.time())
        finally:
            clear(passwd_str)
            self.clear_password_environ_for_maria()
        if not ret:
            log.error(f"Exec backup cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret = self.parse_backup_lock_detail(output)
        if not ret:
            log.error("Get backup lock time failed.")
            return False
        self.set_average_speed(begin_time, self._backup_time, copy_path)
        log.info(f"Exec backup cmd success. output: pid:{self._p_id} jobId:{self._job_id}")
        parper_cmd_param = cmd_format("--prepare {} --parallel={} \
            --target-dir={} --incremental-dir={}", app_log_only, self.get_channel_number(), copy_full_path, copy_path)
        ret, output = self.exec_xtrabackup_cmd(parper_cmd_param, tool_name)
        if not ret:
            log.error(f"Exec prepare cmd failed. output:{output} pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Exec prepare cmd success. output: pid:{self._p_id} jobId:{self._job_id}")
        if not database_cmd:
            return True
        # 数据库备份 需要导出表结构
        ret = self.backup_table_struct(database_name, copy_full_path)
        if not ret:
            return False
        return True

    def build_backup_cmd_param(self, backup_param_model):
        if backup_param_model.tool_name == MysqlBackupToolName.MARIADBBACKUP:
            backup_cmd_param = cmd_format("--defaults-file={} {}{} \
                                                                --backup  --parallel={} {}\
                                                                --host={} --user={} --port={} \
                                                                --target-dir={} \
                                                                --incremental-basedir={}",
                                          backup_param_model.my_cnf_path,
                                          backup_param_model.database_cmd,
                                          backup_param_model.database_name,
                                          self.get_channel_number(),
                                          self.get_lock_ddl_cmd(),
                                          self._mysql_ip,
                                          self._mysql_user,
                                          self._mysql_port,
                                          backup_param_model.copy_path,
                                          backup_param_model.copy_full_path)
        else:
            backup_cmd_param = cmd_format("--defaults-file={} {}{} \
                                                --backup  --parallel={} {}\
                                                --host={} --user={}  --password'{}' --port={} \
                                                --target-dir={} \
                                                --incremental-basedir={}",
                                          backup_param_model.my_cnf_path,
                                          backup_param_model.database_cmd,
                                          backup_param_model.database_name,
                                          self.get_channel_number(),
                                          self.get_lock_ddl_cmd(),
                                          self._mysql_ip,
                                          self._mysql_user,
                                          backup_param_model.passwd_str,
                                          self._mysql_port,
                                          backup_param_model.copy_path,
                                          backup_param_model.copy_full_path)
        return backup_cmd_param


    @exter_attack
    def exec_sub_job(self):
        """
        执行备份任务
        :return 
        """
        log.info(f"Incbackup begin. pid:{self._p_id} jobId:{self._job_id}")
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
        # 计算备份出的增量副本大小
        ret, size = scan_dir_size(self._job_id, copy_path)
        if ret and size != 0:
            self._data_size = size
        log.info(f"Success to get inc copy size: {self._data_size} KB.")
        # 需要删除备份出来的增量副本
        if not check_path_in_white_list(copy_path):
            log.error(f"Invalid copy_path.")
            return False
        self.check_and_del_target_dir(copy_path)
        # 副本信息记录cache仓
        ret = self.write_copy_info(self._backup_time, self.get_full_copy_path())
        if not ret:
            log.error(f"Write_copy_info_failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Incbackup success. pid:{self._p_id} jobId:{self._job_id}")
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

        copy_full_path = self.get_full_copy_path()
        if not copy_full_path:
            log.error(f"get full copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret = self.flush_log()
        if not ret:
            return False
            # 安全校验
        if not check_repo_path(copy_path) or not check_repo_path(copy_full_path):
            log.error("Check inc copy_path invalid.")
            return False
        ret = self.call_xtrabackup(my_cnf_path, copy_path, copy_full_path)
        if not ret:
            return False
        self.backup_connect_param()
        ret, tmp_time = self.get_backup_log_time(copy_path)
        if ret and tmp_time != 0:
            self._backup_time = int(tmp_time)
        return True

    def set_average_speed(self, begin_time, end_time, copy_path):
        time_interval = int(end_time - begin_time)
        if time_interval < 1:
            time_interval = 1
        data_size = self.get_dir_size(copy_path)
        try:
            self._average_speed = int((data_size / 1024) / time_interval)
        except Exception as exception_str:
            log.error(f"Get backup average speed failed. error:{exception_str}")
        log.info(f"Get backup average speed:{self._average_speed}. pid:{self._p_id} jobId:{self._job_id}")
