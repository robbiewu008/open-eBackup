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

from common.common_models import SubJobDetails
from common.const import SubJobStatusEnum


class BaseService:
    def __init__(self, job_id, sub_job_id, pid):
        self.job_id = job_id
        self.sub_job_id = sub_job_id
        self.pid = pid

    def get_log_comm(self):
        return f"pid:{self.pid} jobId:{self.job_id} sub_job_id:{self.sub_job_id}"

    def build_running_details(self):
        process = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=50,
                                taskStatus=SubJobStatusEnum.RUNNING)
        return process
