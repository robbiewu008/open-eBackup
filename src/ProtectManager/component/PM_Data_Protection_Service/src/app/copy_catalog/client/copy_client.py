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
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data, ProtectionServiceHttpsClient
from app.common.enums.sla_enum import RetentionTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.schemas import CopyRetentionPolicySchema
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_timestamp

LOGGER = logger.get_logger(__name__)


def query_copy_by_scn(database_id, scn, database_type):
    url = f'/v1/eb/internal/copies/scn'
    target_req = {
        "scn": scn,
        "databaseId": database_id,
        "apptype": database_type,
    }
    LOGGER.info(f'invoke api to query copies by scn, request url is {url}')
    response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
    if response.status == HTTPStatus.OK:
        timestamp_list = parse_response_data(response.data)
        LOGGER.info(f'query copies by scn, timestamp_list is {timestamp_list}')
        copies = query_copy_by_timestamp(timestamp_list, database_id)
        return copies
    else:
        LOGGER.error(f'Failed to query copies by scn, scn is {scn}')
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke query copies api by scn failed or timeout")


def create_import_resource(resource_name: str):
    url = f'/v1/internal/resources/action/import'
    data = {"name": resource_name}
    response = ProtectionServiceHttpsClient().request("POST", url, body=json.dumps(data))
    if response.status == HTTPStatus.OK:
        LOGGER.info("create import copy resource success")
        response_json = parse_response_data(response.data)
        return response_json.get('uuid')
    else:
        LOGGER.error(f"failed to create import copy resource {response.status}, {response.data}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke create import copy resource failed or timeout")


def create_copy_worm_task(copy_id, retention_policy: CopyRetentionPolicySchema):
    url = f'/v1/internal/anti-ransomware/action/create-copy-worm-task'
    expired_time_unit = None
    if retention_policy.duration_unit:
        expired_time_unit = retention_policy.duration_unit.value
    data = {
            "copy_id": copy_id,
            "expired_time": retention_policy.retention_duration,
            "expired_time_unit": expired_time_unit,
            "retention_type": retention_policy.retention_type.value
    }
    response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(data))
    if response.status == HTTPStatus.OK:
        LOGGER.info(f"create copy worm task success, copy_id:{copy_id}")
        response_json = parse_response_data(response.data)
        return response_json.get('uuid')
    else:
        LOGGER.error(f"failed to create copy worm task {response.status}, {response.data}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke create copy worm task failed or timeout")
