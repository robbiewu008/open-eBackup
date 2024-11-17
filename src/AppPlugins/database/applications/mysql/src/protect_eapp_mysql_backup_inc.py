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
from mysql.src.common.constant import MySQLJsonConstant, SubJobName
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import check_path_in_white_list
from mysql.src.protect_mysql_backup_inc import MysqlBackupInc
from mysql.src.utils.mysql_utils import MysqlUtils


class EAppMysqlBackupInc(MysqlBackupInc):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._report_progress_thread_start = True
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
        log.info(f"Incbackup begin. pid:{self._p_id} jobId:{self._job_id}, subJob:{sub_job_name}")
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
        elif sub_job_name == SubJobName.BACKUP:
            ret = self.exec_backup(copy_path)
        elif sub_job_name == SubJobName.REPORT_COPY:
            ret = self.report_copy(copy_path)
        else:
            ret = False
        self._report_progress_thread_start = False
        progress_thread.join()
        if not ret:
            return False
        log.info(f"Incbackup success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def exec_backup(self, copy_path):
        copy_path = os.path.join(copy_path, self.get_host_sn())
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

        copy_full_path = self.get_full_copy_path()
        if not copy_full_path:
            log.error(f"get full copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        copy_full_path = os.path.join(copy_full_path, self.get_host_sn())
        ret = self.call_xtrabackup(my_cnf_path, copy_path, copy_full_path)
        if not ret:
            log.error("Failed to call xtrabackup cmd")
            return False
        return self.backup_log(copy_full_path)

    def report_copy(self, copy_path):
        ret, tmp_time = self.get_backup_log_time(os.path.join(copy_path, self.get_host_sn()))
        if not ret or tmp_time == 0:
            log.error("Failed to get copy time")
            return False
        backup_time = int(tmp_time)
        log.info(f"Success to get backup time:{backup_time}")
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
        ret = self.write_copy_info(backup_time, self.get_full_copy_path())
        if not ret:
            log.error(f"Write_copy_info_failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Incbackup success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def set_average_speed(self, begin_time, end_time, copy_path):
        # 获取父级目录
        copy_path = copy_path.rstrip("/")
        copy_path = copy_path[:copy_path.rindex("/")]
        super().set_average_speed(begin_time, end_time, copy_path)
