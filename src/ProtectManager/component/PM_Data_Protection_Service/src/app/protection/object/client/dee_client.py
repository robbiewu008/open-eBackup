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

from app.common.clients.client_util import DataEnableEngineHttpsClient, is_response_status_ok, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger

log = get_logger(__name__)


def update_self_learning_config(resource_id, device_id, is_open=False, cycle_type=0, duration=15):
    params = {
        "resourceId": resource_id,
        "deviceId": device_id,
        "isOpen": is_open,
        "type": cycle_type,
        "duration": duration
    }
    url = '/v1/internal/anti/ransomware/self-learning-config'
    log.info(f"update self learning config params: {params} handler start, url: {url}")
    response = DataEnableEngineHttpsClient().request("POST", url, body=json.dumps(params))
    response_data = parse_response_data(response.data)
    if not is_response_status_ok(response):
        log.error(f"update self learning config {resource_id} handler error, response: {response_data}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, error_message="Dee api error")
    return response_data


def get_replication_pair_by_id(device_id: str, vstore_id: str, filesystem_id: str):
    url = f'/v1/internal/anti/ransomware/replication-pair'
    log.debug(f'invoke api to query replication pair, '
              f'device_id:{device_id}, vstore_id:{vstore_id}, filesystem_id:{filesystem_id}')

    response = DataEnableEngineHttpsClient().request("GET", url, fields={"device_id": device_id,
                                                                         "vstore_id": vstore_id,
                                                                         "filesystem_id": filesystem_id})
    if not is_response_status_ok(response):
        log.error(f"invoke api to query replication pair. Status: {response.status}, Data: {response.data}")
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, message="invoke dee api failed or timeout")
    return parse_response_data(response.data)

