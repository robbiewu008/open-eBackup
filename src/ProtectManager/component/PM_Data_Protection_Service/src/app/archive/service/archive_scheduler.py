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
import uuid
from http import HTTPStatus

from app.archive.client.archive_client import ArchiveClient
from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, parse_response_data
from app.common.enums.schedule_enum import ScheduleTypes
from app.common.event_messages.event import EventBase


LOGGER = logger.get_logger(__name__)


class ArchiveMessage(EventBase):
    def __init__(self, topic, params: dict, request_id=None, response_topic=''):
        if params:
            for k, v in params.items():
                setattr(self, k, v)
        request_id = request_id if request_id else str(uuid.uuid4())
        EventBase.__init__(self, request_id, topic, response_topic)


class ArchiveScheduler(object):

    @staticmethod
    def create_immediate_schedule(schedule_param):
        create_res = None
        task = ArchiveClient.create_task(schedule_param["copy_id"], json.loads(schedule_param["policy"]),
                                         schedule_param["resource_sub_type"], schedule_param["resource_type"],
                                         sla_name=schedule_param.get("sla_name"))
        url = f'/v1/schedules'
        schedule_req = {
            "schedule_type": ScheduleTypes.immediate.value,
            "action": "archive",
            "context": True,
            "params": json.dumps(schedule_param),
            "task": json.dumps(task)
        }

        LOGGER.info(f'[ARCHIVE_TASK]:invoke api to create immediate schedule, request url: {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.error(f'[ARCHIVE_TASK]:create immediate schedule failed')
        return create_res

    @staticmethod
    def create_delay_schedule(schedule_param):
        task = ArchiveClient.create_task(schedule_param["copy_id"], json.loads(schedule_param["policy"]),
                                         schedule_param["resource_sub_type"], schedule_param["resource_type"],
                                         sla_name=schedule_param.get("sla_name"))
        url = f'/v1/schedules'

        schedule_req = {
            "schedule_type": ScheduleTypes.delayed.value,
            "action": "archive",
            "context": True,
            "params": json.dumps(schedule_param),
            "task": json.dumps(task),
            "start_date": schedule_param["start_date"]
        }

        LOGGER.info(f'[ARCHIVE_TASK]:invoke api to create delay schedule, request url: {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            LOGGER.info(f'[ARCHIVE_TASK]:schedule_id: {parse_response_data(response.data).get("schedule_id")}')
        else:
            LOGGER.error(f'[ARCHIVE_TASK]:create delay schedule failed')
