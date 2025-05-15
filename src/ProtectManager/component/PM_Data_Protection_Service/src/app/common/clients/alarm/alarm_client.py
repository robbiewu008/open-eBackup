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

from app.common import logger
from app.common.clients.alarm.alarm_schemas import ClearAlarmReq, SendAlarmReq
from app.common.clients.client_util import (
    SystemBaseHttpsClient, decode_response_data, is_response_status_ok, parse_response_data
)

LOGGER = logger.get_logger(__name__)


class AlarmClient(object):
    @staticmethod
    def send_alarm(req: SendAlarmReq):
        url = f'/v1/internal/alarms'
        req_data = req.dict(by_alias=True)
        LOGGER.info(f'invoke api to send alarm, request url is {url}, request is {req_data}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(req_data))
        if not is_response_status_ok(response):
            LOGGER.error(f"invoke api to send alarm failed, message is {parse_response_data(response.data)}")
        else:
            LOGGER.info(f"invoke api to send alarm success, alarm id is {response.data}")
            return decode_response_data(response.data)

    @staticmethod
    def clear_alarm(req: ClearAlarmReq):
        url = f'/v1/internal/alarms/action/clear'
        req_data = req.dict(by_alias=True)
        LOGGER.info(f'invoke api to clear alarm, request url is {url}, request is {req_data}')
        response = SystemBaseHttpsClient().request("PUT", url, body=json.dumps(req_data))
        if not is_response_status_ok(response):
            LOGGER.error(f"invoke api to clear alarm failed, message is {parse_response_data(response.data)}")
        else:
            LOGGER.info(f"invoke api to send clear success, alarm id is {response.data}")

    @staticmethod
    def clear_entity_alarm(req: SendAlarmReq):
        url = f'/v1/internal/alarms/action/clear/entity'
        req_data = req.dict(by_alias=True)
        LOGGER.info(f'invoke api to clear entity alarm, request url is {url}')
        response = SystemBaseHttpsClient().request("PUT", url, body=json.dumps(req_data))
        if not is_response_status_ok(response):
            LOGGER.error(f"invoke api to clear alarm failed, message is {parse_response_data(response.data)}")
        else:
            LOGGER.info(f"invoke api to send clear success, alarm id is {response.data}")
