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

from common.logger import Logger
from common.parse_parafile import ParamFileUtil
from dameng.restore.dm_restore_task import DMRestoreTask
from dameng.commons.const import ProgressType, RetoreCmd

log = Logger().get_logger("dameng.log")

CMD_ARRAY = [
    RetoreCmd.RESTORE_ALLOW,
    RetoreCmd.RESTORE_PRE,
    RetoreCmd.RESTORE_GEN_SUB,
    RetoreCmd.RESTORE_EXEC_SUB,
    RetoreCmd.RESTORE_POST,
    f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.PRE}",
    f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.RESTORE_SUB}",
    f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.POST}"
]


class DMRestore:
    def __init__(self):
        self._task_obj = None

    @staticmethod
    def check_cmd(cmd):
        if cmd not in CMD_ARRAY:
            log.error("Cmd not found.")
            return False
        return True

    def init_task(self, p_id, job_id, sub_job_id, cmd):
        json_param = ParamFileUtil.parse_param_file(p_id)
        self._task_obj = DMRestoreTask(p_id, job_id, sub_job_id, json_param)
        if not self._task_obj:
            log.error(f"Init DMRestoreTask failed. pid:{p_id} jobId:{job_id}.")
            return False
        ret = self._task_obj.init_fun(cmd)
        if not ret:
            log.error(f"Init param failed. pid:{p_id} jobId:{job_id}.")
            return False
        return True

    def dispatch_task(self, cmd):
        return self._task_obj.dispatch_task(cmd)

    def exec_cmd_when_execpt(self, cmd):
        return self._task_obj.exec_cmd_when_execpt(cmd)

    def exec_cmd_when_success(self, cmd):
        return self._task_obj.exec_cmd_when_success(cmd)

    def exec_cmd_when_failed(self, cmd):
        return self._task_obj.exec_cmd_when_failed(cmd)
