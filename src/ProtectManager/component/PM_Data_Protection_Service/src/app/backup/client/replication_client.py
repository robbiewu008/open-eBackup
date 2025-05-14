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
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class ReplicationClient(object):

    @staticmethod
    @exter_attack
    def query_external_system() -> any:
        url = f'/v1/internal/clusters/target-details'
        LOGGER.info(f'invoke api to query external system, request url:{url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke external system api failed or timeout")
        return parse_response_data(response.data)
