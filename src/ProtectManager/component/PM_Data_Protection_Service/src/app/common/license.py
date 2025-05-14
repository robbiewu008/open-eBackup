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
from datetime import datetime
from typing import Union

from app.common.clients.client_util import (
    ProtectionServiceHttpsClient, SystemBaseHttpsClient, is_response_status_ok, parse_response_data
)
from app.common.enums.job_enum import JobLogLevel
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.toolkit import modify_task_log
from app.common import logger

log = logger.get_logger(__name__)


def validate_license_by_resource_uuid(
        function,
        resource_uuid,
        job_id,
        request_id,
        strict=True,
):
    """
    validate license by resource uuid

    :param function: function enum
    :param resource_uuid: resource uuid
    :param job_id: job id
    :param request_id: request id
    :param strict: strict mode
    :return: check result
    """
    url = f'/v1/internal/resource/{resource_uuid}'
    response = ProtectionServiceHttpsClient().request("GET", url, build_exception=True)
    data = parse_response_data(response.data)
    return validate_license_by_resource_type(function, data['sub_type'], job_id, request_id, strict)


def validate_license_by_resource_type(
        function: FunctionEnum,
        resource: Union[ResourceSubTypeEnum, str],
        job_id: str = None,
        request_id: str = None,
        strict: bool = True,
):
    """
    validate license by resource type

    :param function: function enum
    :param resource: resource sub type
    :param job_id: job id
    :param request_id: request id
    :param strict: strict mode
    :return: check result
    """
    params = {
        "function": function.name,
        "resourceType": resource.value if isinstance(resource, ResourceSubTypeEnum) else resource,
        "jobId" : job_id
    }
    log.info(f'request license with params:{params}')
    response = SystemBaseHttpsClient().request(
        "GET", f"/v1/internal/license/function", fields=params)
    log.info(f'get response from license check, response:{response}')
    job_log = {}
    if not is_response_status_ok(response):
        error = parse_response_data(response.data)
        job_log['logInfoParam'] = ["job_status_fail_label"]
        job_log['level'] = JobLogLevel.ERROR.value
        job_log['logDetail'] = error.get('errorCode')
        job_log['logDetailParam'] = error.get('parameters') or []
    else:
        error = None
        job_log['logInfoParam'] = ["job_status_success_label"]
        job_log['level'] = JobLogLevel.INFO.value
    if job_id is not None:
        job_log.update({
            "jobId": job_id,
            "startTime": int(datetime.now().timestamp() * 1000),
            "logInfo": "job_log_license_check_label",
        })
        modify_task_log(request_id, job_id, {
            "jobLogs": [job_log]
        })
    if error:
        if strict:
            raise EmeiStorBizException.build_from_error(error)
        result = False
    else:
        result = True
    return result
