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

from common.common import exter_attack
from common.const import SubJobStatusEnum
from common.exception.common_exception import ErrCodeException
from mysql import log
from mysql.src.protect_mysql_restore_base import MysqlRestoreBase
from mysql.src.service.restore.db_restore import DBRestore
from mysql.src.service.restore.restore_param import RestoreParam


class MysqlDatabaseRestore(MysqlRestoreBase):
    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)

    @exter_attack
    def exec_sub_job(self, restore_type, log_restore):
        log.info(f"DB restore begin. pid:{self._p_id} jobId:{self._job_id}")
        progress_type = self._sub_job_id if self._sub_job_id else self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, progress_type)
        try:
            with RestoreParam(self._p_id, self._json_param) as restore_param:
                db_restore = DBRestore(self._job_id, self._sub_job_id, restore_param)
                db_restore.exec_restore()
        except Exception as error:
            if isinstance(error, ErrCodeException):
                self._error_code = error.error_code
            log.info(f"DB restore failed. pid:{self._p_id} jobId:{self._job_id}, error:{error}")
            return False
        log.info(f"DB restore success. pid:{self._p_id} jobId:{self._job_id}")
        return True
