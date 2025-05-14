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
import os
from http import HTTPStatus

import urllib3

from app.common.clients.client_util import parse_response_data, InternalSslContext
from app.common.concurrency import async_route
from app.common.constants.constant import ServiceConstants
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger

agent_penetrate_router = async_route()
log = get_logger(__name__)


# 从osa中获取源ip
def get_resource_ip(destination_ip: str, destination_port: int):
    """
    :param params: str(destination_ip) 目的地IP
    :param params: int(destination_port) 目的地端口
    :return str() 源ip
    """
    osa_host = os.environ.get('POD_IP')
    url = f"/v1/internal/deviceManager/rest/logical_ip"
    body = {
        "task_type": "backup",
        "destination_ip": destination_ip,
        "port": str(destination_port)
    }
    http = urllib3.HTTPSConnectionPool(
        osa_host,
        port=int(ServiceConstants.OSA_PORT),
        ssl_context=InternalSslContext().ssl_context,
        assert_hostname=False
    )
    response = http.request(method='Get', url=url, body=json.dumps(body))
    if response.status == HTTPStatus.OK and parse_response_data(response.data).get('error', {}).get('code', 1) == 0:
        log.info(f"get resource ip:{parse_response_data(response.data).get('logical_ip', '')} success,"
                 f" destination_ip is: {destination_ip}")
        return parse_response_data(response.data).get('logical_ip', '')
    else:
        log.error(
            f'invoke api to get resource ip failed!, '
            f'response.status: {response.status}, request url: {url}, destination_ip is: {destination_ip}')
        raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                   message="get resource ip failed.")
