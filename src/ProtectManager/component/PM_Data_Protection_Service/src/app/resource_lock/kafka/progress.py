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
import time

from app.resource_lock.common import consts
from app.common import toolkit, logger
from app.common.enums import job_enum

msg_status = {
    consts.PROGRESS_SUCCEEDED: job_enum.JobLogLevel.INFO.value,
    consts.PROGRESS_FAILED: job_enum.JobLogLevel.ERROR.value,
    consts.PROGRESS_START: job_enum.JobLogLevel.INFO.value
}

log = logger.get_logger(__name__)


def notify_progress(request_id, status, operation, lock_id, resource_id=None):
    """
    通知任务管理进度

    :param request_id: 请求ID
    :param status: 操作结果（"running": 请求中, "succeeded":成功，"failed":失败）
    :param operation: 操作
    :param lock_id: 分布式锁ID
    :param resource_id: 资源id，加锁失败的时候值不为空，其余情况值为空
    :return:
    """

    now_time = time.time()
    log_info = operation + "_" + status + "_label"
    job_id = lock_id[lock_id.find('@') + 1:]
    job_log = {
        "jobId": job_id,
        "startTime": int(now_time * 1000),
        "logInfo": log_info,
        "level": msg_status.get(status),
        "unique": True
    }
    if consts.PROGRESS_FAILED == status:
        job_log['logDetail'] = "1677931286"
        job_log['logDetailParam'] = []
    if resource_id:
        job_log['logDetailParam'] = [resource_id]
    req = {
        "jobLogs": [job_log]
    }
    if job_id != 'ignore':
        toolkit.modify_task_log(request_id, job_id, req)
