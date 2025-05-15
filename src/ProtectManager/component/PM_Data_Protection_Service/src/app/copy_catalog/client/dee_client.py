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

from fastapi import HTTPException

from app.common.clients.client_util import DataEnableEngineHttpsClient, DataEnableEngineParserHttpsClient, \
    is_response_status_ok, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.copy_catalog.schemas import ModifyCopyAntiRansomwareStatusBody

log = get_logger(__name__)


def alarm_handler(copy_id_list, generate_type, ext_parameters=None):
    params = {
        "copyIds": copy_id_list,
    }
    if isinstance(ext_parameters, ModifyCopyAntiRansomwareStatusBody):
        params.update(ext_parameters.dict())
    if generate_type == "IO_DETECT":
        url = '/v1/internal/anti/ransomware/io-detect/copy/false/alarm'
    else:
        url = '/v1/internal/anti/ransomware/detect/copy/false/alarm'
    log.info(f"alarm params: {params} handler start, url: {url}")
    response = DataEnableEngineHttpsClient().request("PUT", url, body=json.dumps(params))
    if not is_response_status_ok(response):
        log.error(f"alarm copies {copy_id_list} handler error, response: {response.data}")
        if str(parse_response_data(response.data)['errorCode']) == \
                CommonErrorCodes.FALSE_ALARM_NOT_INIT_WORM.get("code"):
            raise EmeiStorBizException(CommonErrorCodes.FALSE_ALARM_NOT_INIT_WORM)
        raise HTTPException(response.status)
    return parse_response_data(response.data)


def delete_report(copy_ids):
    params = {
        "copyIdList": copy_ids
    }
    url = '/v1/internal/anti/ransomware/detect/copy/feature'
    log.info(f"delete copies {copy_ids} report start, url: {url}")
    response = DataEnableEngineHttpsClient().request("DELETE", url, body=json.dumps(params))
    if not is_response_status_ok(response):
        log.error(f"delete copy anti-ransomware report error, response: {response.data}")
        raise HTTPException(response.status)
    log.info(f"delete copy anti-ransomware report success, response: {response.data}")
    return parse_response_data(response.data)


def check_copy_browse_status(copy_id):
    url = f'/v1/internal/browse/query-used/{copy_id}'
    log.info(f"check copy {copy_id} is being used, url: {url}")
    response = DataEnableEngineParserHttpsClient().request("GET", url)
    if not is_response_status_ok(response):
        log.error(f"check copy is being used failed, response: {response.data}")
        raise HTTPException(response.status)
    log.info(f"check copy is being used success, response: {response.data}")
    return parse_response_data(response.data)
