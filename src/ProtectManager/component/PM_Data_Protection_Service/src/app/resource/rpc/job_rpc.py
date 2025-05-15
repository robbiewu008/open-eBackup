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
from app.common import toolkit, logger
from app.common.enums.job_enum import JobStatus

log = logger.get_logger(__name__)


class JobClient(object):
    @staticmethod
    def create_job(
            request_id: str,
            user_id: str,
            resource_obj,
            job_type,
    ):
        return toolkit.create_job_center_task(request_id, {
            'requestId': request_id,
            'sourceId': resource_obj.uuid,
            'sourceLocation': resource_obj.path,
            'sourceName': resource_obj.name,
            'sourceType': resource_obj.type,
            'sourceSubType': resource_obj.sub_type,
            "targetLocation": "Local",
            "targetName": "Local",
            "type": job_type,
            'userId': user_id,
            'enableStop': True
        })

    @staticmethod
    def update_job(
            request_id: str,
            task_id: str,
            status: JobStatus
    ):
        req = dict(status=status.value, progress=100)
        toolkit.complete_job_center_task(request_id, task_id, req)
