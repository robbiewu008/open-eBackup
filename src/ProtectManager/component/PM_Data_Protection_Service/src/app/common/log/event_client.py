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
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, is_response_status_ok
from app.common.log.event_schemas import SendEventReq

log = logger.get_logger(__name__)


class EventClient(object):
    @staticmethod
    def send_event(operation_req: SendEventReq):
        response = SystemBaseHttpsClient().request(
            "PUT", f"/v1/internal/alarms/log", body=operation_req.json())
        if not is_response_status_ok(response):
            log.error(f"invoke api to send alarm failed, message is {response.data}")
        else:
            log.info(f"invoke api to send alarm success, alarm id is {response.data}")

    @staticmethod
    def send_running_event(req: SendEventReq):
        """发送运行事件"""
        response = SystemBaseHttpsClient().request(
            "POST", f"/v1/internal/event",
            body=json.dumps(req.dict(by_alias=True)))
        if not is_response_status_ok(response):
            log.error(f"invoke api to send alarm failed, message is {response.data}")
        else:
            log.info(f"invoke api to send alarm success, alarm id is {response.data}")
            return decode_response_data(response.data)
