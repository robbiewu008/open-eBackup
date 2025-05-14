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

from app.common import logger
from app.common.clients.client_util import parse_response_data, ProtectionServiceHttpsClient, SystemBaseHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class ProtectionClient(object):

    @staticmethod
    @exter_attack
    def query_sla(sla_id: str) -> any:
        url = f'/v1/internal/slas/{sla_id}'
        LOGGER.info(f'invoke api to query sla, request url:{url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data)

    @staticmethod
    @exter_attack
    def query_sla_by_ext_parameters(key: str, value: str, is_all_sla: bool):
        url = f'/v1/internal/slas/policies/ext-parameters'
        target_req = {
            "key": key,
            "value": value,
            "is_all_sla": is_all_sla
        }
        LOGGER.info(f'invoke api to query sla by ext parameters, request url:{url}')
        response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data)
