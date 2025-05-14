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
import unittest
import requests

from unittest import mock
from unittest.mock import patch, Mock
from urllib3 import HTTPResponse
from app.common.security.kmc_util import Kmc
from tests.test_cases import common_mocker # noqa
from app.common.clients.client_util import SystemBaseHttpsClient
from tests.test_cases.tools import http, env
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()
from app.common.enums.schedule_enum import ScheduleTypes

from requests import Session

from app.backup.client.scheduler_client import SchedulerClient


class ProtectedObjMock:
    def __init__(self):
        self.resource_id = "resource_id"
        self.sla_id = "sla_id123"


@patch.object(requests, "post", Mock(status_code=1))
@patch.object(requests, "delete", Mock(status_code=1))
@patch.object(Session, "send", Mock(return_value=None))
class TestSchedulerClient(unittest.TestCase):
    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_create_interval_schedule(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=500)
        schedule = {"interval":18, "interval_unit":"m", "start_time":"20:12:10"}
        policy = {"schedule":schedule, "type":"123"}
        schedule_type = "schedule_type"
        protected_obj = ProtectedObjMock()
        chain_id = "chain_id"
        create_interval_schedule = SchedulerClient.create_interval_schedule(policy, schedule_type, protected_obj, chain_id)
        self.assertIsNone(create_interval_schedule)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_create_delay_schedule(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=500)
        schedule_params = {}
        topic = "topic"
        start_date = "2021, 01"
        schedule_req = {
            "schedule_type": ScheduleTypes.delayed.value,
            "action": topic,
            "params": json.dumps(schedule_params),
            'start_date': start_date
        }
        create_delay_schedule = SchedulerClient.create_delay_schedule(schedule_req)
        self.assertIsNone(create_delay_schedule)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_create_immediate_schedule(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=500)
        schedule = {"interval":18, "interval_unit":"m", "start_time":"20:12:10"}
        policy = {"schedule":schedule, "type":"123"}
        resource_id = "resource_id"
        sla_id = "sla_id123"
        chain_id = "chain_id123"
        create_immediate_schedule = SchedulerClient.create_immediate_schedule(policy, resource_id, sla_id, chain_id)
        self.assertIsNone(create_immediate_schedule)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_batch_delete_schedules(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=500)
        schedule_id_list = ["schedule_id"]
        batch_delete_schedules = SchedulerClient.batch_delete_schedules(schedule_id_list)
        self.assertFalse(batch_delete_schedules)


if __name__ == '__main__':
    unittest.main(verbosity=2)
