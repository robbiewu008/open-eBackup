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
from typing import List

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, parse_response_data
from app.common.enums.schedule_enum import ScheduleTypes

LOGGER = logger.get_logger(__name__)


class SchedulerClient(object):
    @staticmethod
    def create_interval_schedule(
            policy,
            schedule_type,
            protected_obj,
            chain_id
    ) -> str:
        url = f'/v1/schedules'
        policy_schedule = policy["schedule"]
        schedule_req = {
            "interval": f'{policy_schedule["interval"]}{policy_schedule["interval_unit"]}',
            "schedule_type": schedule_type,
            "action": "schedule." + policy["type"],
            "params": json.dumps({
                "resource_id": protected_obj.resource_id,
                "sla_id": str(protected_obj.sla_id),
                "chain_id": chain_id,
                "policy": policy
            }),
            'start_date': policy_schedule["start_time"]
        }
        LOGGER.info(f'invoke api to create schedule, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data).get("schedule_id")
        else:
            LOGGER.info(f'invoke api to create schedule, url:{url}')

    @staticmethod
    def create_delay_schedule(schedule_req) -> str:
        url = f'/v1/schedules'
        LOGGER.info(f'invoke api to create schedule, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data).get("schedule_id")
        else:
            LOGGER.info(f'invoke api to create schedule, url:{url}')

    @staticmethod
    def create_immediate_schedule(
            policy,
            resource_id,
            sla_id,
            chain_id
    ) -> str:
        url = f'/v1/schedules'
        schedule_req = {
            "schedule_type": ScheduleTypes.immediate.value,
            "action": "schedule." + policy["type"],
            "params": json.dumps({
                "resource_id": resource_id,
                "sla_id": sla_id,
                "chain_id": chain_id,
                "policy": policy
            })
        }
        LOGGER.info(f'invoke api to create schedule, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.info(f'invoke api to create schedule, url:{url}')

    @staticmethod
    def batch_delete_schedules(
            schedule_id_list: List[str]
    ) -> bool:
        for schedule_id in schedule_id_list:
            url = f'/v1/schedules/{schedule_id}'
            response = SystemBaseHttpsClient().request("DELETE", url)
            if response.status != HTTPStatus.OK:
                return False
        return True
