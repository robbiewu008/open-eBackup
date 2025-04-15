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

from common.common import get_host_sn
from common.const import ReportDBLabel, BackupTypeEnum
from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd
from mysql import log
from mysql.src.common.constant import SubJobName
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import mysql_backup_files
from mysql.src.service.backup.backup_func import get_backup_tool_name
from mysql.src.service.backup.backup_service import BackupService, BackupDecorator
from mysql.src.utils.common_func import get_binlog_filenames, find_log_bin_path_dir
from mysql.src.utils.mysql_utils import MysqlUtils


class EAPPBackupService(BackupService):

    def backup_task_sub_job_dict(self):
        sub_job_dict = {
            SubJobName.FLUSH_LOG.value: self.flush_log,
            SubJobName.BACKUP.value: self.exec_data_backup,
            SubJobName.REPORT_COPY: self.report_copy,
        }
        return sub_job_dict

    @BackupDecorator.report_progress(start_label=ReportDBLabel.BACKUP_SUB_START_COPY,
                                     end_label=ReportDBLabel.SUB_JOB_SUCCESS,
                                     error_label=ReportDBLabel.BACKUP_SUB_FAILED)
    def exec_backup(self):
        sub_job_dict = self.backup_task_sub_job_dict()
        sub_job = self.param.get_sub_job_name()
        log.info(f"execute sub job:{sub_job}")
        sub_job_dict.get(sub_job)()

    def exec_data_backup(self):
        backup_tool = get_backup_tool_name(self.param.get_version())
        my_cnf_path = self.find_valid_my_cnf()
        extra_param = self.extra_xtrabackup_param(backup_tool)
        copy_path = os.path.join(self.param.get_copy_path(self.job_id), get_host_sn())
        log.info(f"eapp mysql backup copy_path:{copy_path},{self.get_log_comm()}")
        if self.param.backup_type == BackupTypeEnum.FULL_BACKUP.value:
            self.exec_full_backup(backup_tool, my_cnf_path, extra_param, copy_path)
        else:
            self.exec_incr_backup(backup_tool, my_cnf_path, extra_param, copy_path)
        self.backup_log(my_cnf_path, copy_path)

    def backup_log(self, my_cnf_path, copy_path):
        with open(self.param.get_last_bin_log_file(), mode="r") as f:
            last_bin_log = f.read()
        binlog_filenames = get_binlog_filenames(self.param.sql_param)
        log_bin_dir = find_log_bin_path_dir(self.param.sql_param, my_cnf_path)
        backup_log_files = []
        for bin_log in binlog_filenames:
            if bin_log >= last_bin_log:
                bin_log_path = os.path.join(log_bin_dir, bin_log)
                backup_log_files.append(bin_log_path)
        mysql_backup_files(self.job_id, backup_log_files, copy_path)
        log.info(f"Backup log file success,{self.get_log_comm()},backup_log_files:{backup_log_files}")

    def flush_log(self):
        super().flush_log()
        binlog_filenames = get_binlog_filenames(self.param.sql_param)
        if len(binlog_filenames) > 3:
            last_bin_log = binlog_filenames[-3]
        else:
            last_bin_log = binlog_filenames[-2]
        MysqlUtils.save_last_bin_log(self.param.get_last_bin_log_file(), last_bin_log)
        log.info(f"Save last bin log success,{self.get_log_comm()},last_bin_log:{last_bin_log}")

    def exec_full_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        copy_path = os.path.join(self.param.get_copy_path(self.job_id), get_host_sn())
        if os.path.exists(copy_path) and not su_exec_rm_cmd(copy_path):
            log.error(f"remove file[{copy_path}] failed.")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR, message=f"remove file[{copy_path}] failed.")
        if not exec_mkdir_cmd(copy_path):
            log.error(f"create file[{copy_path}] failed.")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR, message=f"create file[{copy_path}] failed.")
        ret = self.prepare_backup(backup_tool, copy_path)
        if ret:
            log.info(f"Exec prepare_sub_job success.No need to re-backup. {self.get_log_comm()}")
            return
        super().exec_full_backup(backup_tool, my_cnf_path, extra_param, copy_path)

    def exec_incr_backup(self, backup_tool, my_cnf_path, extra_param, copy_path):
        copy_path = os.path.join(self.param.get_copy_path(self.job_id), get_host_sn())
        ret = self.prepare_backup(backup_tool, copy_path)
        if ret:
            log.info(f"Exec prepare_sub_job success.No need to re-backup. {self.get_log_comm()}")
            return
        super().exec_incr_backup(backup_tool, my_cnf_path, extra_param, copy_path)

    def report_copy(self):
        copy_path = os.path.join(self.param.get_copy_path(self.job_id), get_host_sn())
        self.write_copy_info(copy_path)
