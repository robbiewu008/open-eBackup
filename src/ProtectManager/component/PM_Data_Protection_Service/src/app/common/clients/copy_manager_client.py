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
from app.common.clients.client_util import (
    SystemBaseHttpsClient, parse_response_data
)
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOGGER = logger.get_logger(__name__)


class CopyManagerClient(object):

    @staticmethod
    def query_associated_copies(copy_id):
        """
        查询关联的副本

        Args:
            copy_id: 副本ID

        Returns: List<str> 关联的副本ID

        """
        url = f'/v1/internal/copies/associated?copy_id={copy_id}'
        LOGGER.info(f'invoke api to query associated copies, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="invoke sla api failed or timeout")
        return parse_response_data(response.data)
