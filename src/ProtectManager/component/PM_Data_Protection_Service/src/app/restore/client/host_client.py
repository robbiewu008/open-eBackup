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
from app.common.clients.client_util import ProtectionServiceHttpsClient, parse_response_data

LOGGER = logger.get_logger(__name__)


class HostClient(object):

    @staticmethod
    def query_host_info(host_id):
        query_info_res = None
        url = f'/v1/resource/host/{host_id}'
        LOGGER.info(f'invoke api to create jon, request url:{url}')
        response = ProtectionServiceHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data)
        else:
            LOGGER.info(f'Failed to query resource info, resource id is host_id:{host_id}')
        return query_info_res
