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
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


@exter_attack
def get_user_info_by_user_id(user_id: str):
    url = f'/v1/internal/users/{user_id}'
    LOGGER.info(f'query user information url: {url}')
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status != HTTPStatus.OK:
        LOGGER.info(f'failed to query the user: {user_id}')
        return {}
    return parse_response_data(response.data)


@exter_attack
def text_decrypt(text: str):
    url = f'/v1/kms/decrypt'
    LOGGER.info(f'use kmc to decrypt')
    params = {"ciphertext": text}
    response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(params))
    if response.status != HTTPStatus.OK:
        LOGGER.info('failed to use kmc to decrypt.')
        return False
    results = parse_response_data(response.data).get("plaintext")
    if not results:
        LOGGER.info('kmc decrypt failed, it is none.')
        return False
    return results


@exter_attack
def get_snmp_trap_config():
    url = f"/v1/internal/snmp/config"
    LOGGER.info(f'get snmp config by url: {url}')
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status != HTTPStatus.OK:
        LOGGER.error('failed to get snmp config')
        raise EmeiStorBizException(CommonErrorCodes.OPERATION_FAILED,
                                   message=f"Get snmp config failed")
    return parse_response_data(response.data)


@exter_attack
def get_snmp_trap_addresses():
    url = f"/v1/internal/snmp/trap/addresses"
    LOGGER.info(f'get snmp trap addresses by url: {url}')
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status != HTTPStatus.OK:
        LOGGER.error('failed to get snmp trap addresses')
        raise EmeiStorBizException(CommonErrorCodes.OPERATION_FAILED,
                                   message=f"Get snmp config failed")
    return parse_response_data(response.data)
