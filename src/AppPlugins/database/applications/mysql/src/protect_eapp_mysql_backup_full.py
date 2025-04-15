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

import os.path
from threading import Thread

from common.common import exter_attack
from common.const import SubJobStatusEnum
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd
from common.util.scanner_utils import scan_dir_size
from mysql import log
from mysql.src.common.constant import SubJobName, MySQLJsonConstant
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.protect_mysql_backup_full import MysqlBackupFull
from mysql.src.utils.mysql_utils import MysqlUtils


class EAppMysqlBackupFull(MysqlBackupFull):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._report_progress_thread_start = True
        self._backup_time = 0
        self._data_size = 0
        self._error_code = 0

    @exter_attack
    def exec_sub_job(self):
        """
        执行备份任务
        :return
        """
        if not MysqlUtils.eapp_is_running():
            self._error_code = MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING
            return False
        sub_job_name = self.get_json_param().get(MySQLJsonConstant.SUBJOB, {}).get(MySQLJsonConstant.JOBNAME, "")
        log.info(f"Fullbackup begin. pid:{self._p_id} jobId:{self._job_id}, subJobName:{sub_job_name}")
        ret, copy_path = self.set_backup_all_param()
        if not ret:
            log.error(f"Exec set_backup_all_param failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, self._sub_job_id)
        progress_thread = Thread(target=self.report_backup_progress_thread)
        progress_thread.daemon = True
        progress_thread.start()
        if sub_job_name == SubJobName.FLUSH_LOG.value:
            ret = self.flush_log_for_eapp()
        elif sub_job_name == SubJobName.BACKUP.value:
            ret = self.exec_backup(copy_path)
        elif sub_job_name == SubJobName.REPORT_COPY.value:
            ret = self.report_copy(copy_path)
        else:
            log.error(f"Invalid sub job, name:{sub_job_name}")
            ret = False
        self._report_progress_thread_start = False
        progress_thread.join()
        if not ret:
            return False
        log.info(f"Fullbackup success. pid:{self._p_id} jobId:{self._job_id}, subJobName:{sub_job_name}")
        return True

    def exec_backup(self, copy_path):
        host_id = self.get_host_sn()
        if not host_id:
            log.error("Failed to get host id")
            return False
        copy_path = os.path.join(copy_path, host_id)
        if os.path.exists(copy_path) and not su_exec_rm_cmd(copy_path):
            log.error(f"remove file[{copy_path}] failed.")
            return False
        if not exec_mkdir_cmd(copy_path):
            log.error(f"create file[{copy_path}] failed.")
            return False
        ret = self.prepare_sub_job(copy_path)
        if ret:
            log.info(f"Exec prepare_sub_job success.No need to re-backup. pid:{self._p_id} jobId:{self._job_id}")
            return True

        ret, my_cnf_path = self.find_mycnf_path(self.my_cnf_path)
        if not ret:
            log.error(f"Find my.cnf failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Find my.cnf success. pid:{self._p_id} jobId:{self._job_id}")
        ret = self.call_xtrabackup(my_cnf_path, copy_path)
        if not ret:
            return False
        return self.backup_log(copy_path)

    def report_copy(self, copy_path):
        self._backup_time = self.get_backup_log_time(os.path.join(copy_path, self.get_host_sn()))
        ret = self.write_copy_info(self._backup_time, copy_path)
        if not ret:
            log.error(f"Write_copy_info_failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        ret, size = scan_dir_size(self._job_id, copy_path)
        if ret and size != 0:
            self._data_size = size
        return True
