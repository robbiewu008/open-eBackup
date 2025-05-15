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
from app.common.clients.client_util import ProtectionServiceHttpsClient, parse_response_data, SystemBaseHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class ProtectionClient(object):

    @staticmethod
    @exter_attack
    def query_sla(sla_id: str) -> any:
        url = f'/v1/internal/slas/{sla_id}'
        LOGGER.info(f'invoke api to query sla, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api to query sla failed. Status: {response.status}, Data: {response.data}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="invoke sla api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def query_policy(policy_id: str) -> any:
        url = f'/v1/internal/slas/policies/{policy_id}/detail'
        LOGGER.info(f'invoke api to query policy, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(f"invoke api to query policy failed. Status: {response.status}, Data: {response.data}")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="invoke policy api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def check_exist_copies_location_before_protect(sla_id: str, resource_id: str) -> any:
        url = f'/v1/internal/protected-objects/check-before-protect'
        target_req = {
            "slaId": sla_id,
            "resourceId": resource_id
        }
        LOGGER.info(f'invoke api to check before protect, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
        if response.status != HTTPStatus.OK:
            response_data = json.loads(response.data.decode('utf-8'))
            error_code = response_data.get("errorCode")
            error_message = response_data.get("errorMessage")
            error_parameters = response_data.get("parameters")
            error_dict = {
                "code": error_code,
                "message": error_message
            }
            LOGGER.error(f"invoke api to check before protect. Status: {response.status}, Data: {response.data}")
            if error_parameters is None:
                raise EmeiStorBizException(error_dict, message="invoke policy api to check before protect failed")
            raise EmeiStorBizException(error_dict,
                                       *error_parameters,
                                       message="invoke policy api to check before protect failed")
