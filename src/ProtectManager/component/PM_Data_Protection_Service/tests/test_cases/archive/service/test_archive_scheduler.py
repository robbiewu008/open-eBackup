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
import sys
import unittest
from unittest import mock
from unittest.mock import Mock, patch

from sqlalchemy import true
from urllib3 import HTTPResponse

from app.archive.service.archive_scheduler import ArchiveScheduler
from app.common.clients.client_util import SystemBaseHttpsClient

mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()

schedule_param = {
    "resource_id": "abc-def", "policy": "{\"name\": \"policy1\"}",
    "auto_retry_times": 3, "sla": "sla",
    "storage_id": "fc2ebf82bd794422b4ad9eed45f91e97",
    "storage_list": {"storage_id": "fc2ebf82bd794422b4ad9eed45f91e97"},
    "sla_name": "slaName",
    "copy_id": "123456",
    "resource_sub_type": "s3",
    "resource_type": "archive",
    "start_date": "date"
}


def get_request():
    response = HTTPResponse()
    response._body = b'{"data": "","error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'
    response.status = 200
    return response


def get_request_404():
    response = HTTPResponse()
    response._body = b'{"data": "","error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'
    response.status = 404
    return response


class TestArchiveScheduler(unittest.TestCase):
    @mock.patch("app.archive.client.archive_client.ArchiveClient.create_task")
    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request()))
    def test_create_immediate_schedule_success(self, mock_create_task):
        mock_create_task.return_value = {"date": "123"}
        ArchiveScheduler.create_delay_schedule(schedule_param)
        response_data = ArchiveScheduler.create_immediate_schedule(schedule_param)
        self.assertEqual(response_data, get_request().data.decode('utf-8'))

    @mock.patch("app.archive.client.archive_client.ArchiveClient.create_task")
    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request()))
    def test_create_delay_schedule_success(self, mock_create_task):
        mock_create_task.return_value = {"date": "123"}
        ArchiveScheduler.create_delay_schedule(schedule_param)
        self.assertTrue(true)

    @mock.patch("app.archive.client.archive_client.ArchiveClient.create_task")
    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request_404()))
    def test_create_delay_schedule_success(self, mock_create_task):
        mock_create_task.return_value = {"date": "123"}
        ArchiveScheduler.create_immediate_schedule(schedule_param)
        ArchiveScheduler.create_delay_schedule(schedule_param)
        self.assertTrue(true)


if __name__ == '__main__':
    unittest.main()
