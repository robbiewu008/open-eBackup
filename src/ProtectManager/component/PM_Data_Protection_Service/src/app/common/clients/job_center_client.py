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
import json
from http import HTTPStatus

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

log = logger.get_logger(__name__)


@exter_attack
def get_job_queue_scope(sub_type: str, job_type: str):
    url = f'/v1/internal/jobs/queue-scope'
    target_req = {
        "sub_type": sub_type,
        "job_type": job_type
    }
    log.info(f'invoke api to query job queue scope, request url is {url}')
    response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
    if response.status == HTTPStatus.OK:
        return bytes.decode(response.data)
    else:
        log.error(f'Failed to query job queue scope.')
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])


@exter_attack
def batch_query_job_list(conditions: dict = None):
    results = []
    page_no = 0
    page_size = 200
    url = f'/v1/internal/jobs'
    conditions_not_empty = conditions if conditions is not None else {}
    log.info(f'invoke api to query jobs, request url is {url}')
    while page_no >= 0:
        conditions_not_empty.update({"page_no": page_no, "page_size": page_size})
        response = SystemBaseHttpsClient().request("GET", url, fields=conditions_not_empty)
        if not is_response_status_ok(response):
            raise EmeiStorBizException(
                CommonErrorCodes.SYSTEM_ERROR,
                message=f"Failed to get request , url is {url}")
        data = parse_response_data(response.data)
        items: list = data.get("records", [])
        results.extend(items)
        if len(items) < page_size:
            page_no = -1
        else:
            page_no += 1
    return results


def query_is_job_present(job_id: str):
    if not job_id:
        return False
    target_req = {
        "jobId": job_id
    }
    response = SystemBaseHttpsClient().request("GET", f"/v1/internal/jobs/present", fields=target_req)
    if response.status == HTTPStatus.OK:
        result = json.loads(response.data.decode('utf-8'))
        log.debug(f'job {job_id} present result:{result}')
        return result
    else:
        log.error(f'Failed to query job:{job_id}.')
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[])
