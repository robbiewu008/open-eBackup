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
from http import HTTPStatus

from app.common.clients.client_util import parse_response_data, ProtectionServiceHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common import logger
from app.common.exception.unified_exception import EmeiStorBizException

LOGGER = logger.get_logger(__name__)


def query_copy_by_resource_id(resource_id, generated_by: str = None):
    url = f'/v1/internal/copies/resource'
    LOGGER.info(f'invoke api to query copies resource, request url is {url}')
    params = {
        "resource_id": resource_id,
        "generated_by": generated_by
    }
    response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
    if response.status != HTTPStatus.OK:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke copies resource api failed or timeout")
    return parse_response_data(response.data)


def query_copy_by_root_uuid(root_uuid, generated_by, resource_sub_type):
    url = f'/v1/internal/copies/resource-properties'
    LOGGER.info(f'invoke api to query copies resource, request url is {url}')
    params = {
        "root_uuid": root_uuid,
        "generated_by": generated_by,
        "resource_sub_type": resource_sub_type
    }
    response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
    if response.status != HTTPStatus.OK:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke copies resource properties api failed or timeout")
    return parse_response_data(response.data)
