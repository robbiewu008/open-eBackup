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

from app.resource.models.resource_models import EnvironmentTable, ResExtendInfoTable
from tests.test_cases.resource.client.mock_const import RequestReturnValue
from tests.test_cases.resource.client.mock_const import Environment
from tests.test_cases.backup.common.context import mock_context  # noqa


class AgentClientTest(unittest.TestCase):
    @mock.patch('app.resource.client.agent_client.SystemBaseHttpsClient')
    def test_query_can_update_agent_versions(self, mock_system_base_https_client):
        from app.resource.client import agent_client
        mock_system_base_https_client_instance = mock_system_base_https_client.return_value
        mock_system_base_https_client_instance.request.return_value = RequestReturnValue(200,
                                                                                         '{"data": 123456}'.encode())
        update_res = agent_client.query_can_update_agent_versions()
        self.assertIsNotNone(update_res)

    @mock.patch('app.resource.client.agent_client.url_request')
    def test_query_agent_version_info(self, mock_url_request):
        from app.resource.client import agent_client
        mock_url_request.return_value = RequestReturnValue(200, '{"data": 123456}'.encode())
        response = agent_client.query_agent_version_info('192.168.111.164', 8090, '123')
        self.assertIsNotNone(response)

    @mock.patch('app.resource.client.agent_client.database.session')
    def test_get_can_update_agent_versions(self, mock_session):
        from app.resource.client import agent_client
        ev = EnvironmentTable()
        ev.uuid = 'abed'
        re = ResExtendInfoTable()
        re.value = "domain1"
        mock_session().__enter__().query().filter().first.return_value = [ev, re]
        proxy = agent_client.query_proxy('127.0.0.1')
        self.assertEqual(proxy, "https://domain1:8090")

    @mock.patch('app.resource.client.agent_client.database.session')
    @mock.patch('sqlalchemy.orm.aliased', mock.Mock(return_value=Environment(1)))
    def test_get_can_update_agent_versions(self, mock_session):
        from app.resource.client import agent_client
        mock_session().__enter__().query().all.return_value = "ver"
        enum_name = agent_client.get_can_update_agent_versions()
        self.assertEqual(enum_name.r.name, "r")


if __name__ == '__main__':
    unittest.main()
