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
from unittest import TestCase
from unittest import mock
from unittest.mock import Mock

from tests.test_cases import common_mocker  # noqa
from tests.test_cases.tools import functiontools, timezone

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.protection.object.common.db_config'] = Mock()


class AgentPenetrateTest(TestCase):

    def setUp(self):
        sys.modules['app.common.events.producer'] = Mock()
        sys.modules['app.copy_catalog.service.copy_delete_workflow'] = Mock()
        sys.modules['app.archive.service.service'] = Mock()

    def tearDown(self) -> None:
        del sys.modules['app.common.events.producer']
        del sys.modules['app.copy_catalog.service.copy_delete_workflow']
        del sys.modules['app.archive.service.service']

    class MockResponse:
        def __init__(self, json_data, status):
            self.data = json_data
            self.status = status

        def data(self):
            return self.data

    # @mock.patch("app.resource.client.agent_client.url_request")
    # def test_query_agent_api_success(self, mock_url_request):
    #     from app.resource.service.common.penetrate_agent_service import query_agent_api
    #     res = '{"revStatus": 0}'.encode()
    #     method = "POST"
    #     ip = "8.40.97.168"
    #     port = "59538"
    #     suffix = "/agent/host/action/agent/upgrade"
    #     res1 = {"revStatus": 0}
    #     mock_url_request.return_value = self.MockResponse(status=200, json_data=res)
    #     data = query_agent_api(method, ip, port, suffix, headers=None, body=None)
    #     self.assertEqual(res1, data)

    @mock.patch("app.resource.service.common.penetrate_agent_service.query_agent_api")
    @mock.patch("app.resource.client.agent_client.url_request")
    def test_action_agent_upgrade_success(self, mock_query_agent_api, mock_url_request):
        from app.resource.service.common.penetrate_agent_service import action_agent_upgrade
        from app.resource.schemas.agent_penetrate_schema import AgentPenetrateUpgradeRequestSchema
        query_upgrade_req = AgentPenetrateUpgradeRequestSchema(
            ip="8.40.97.168",
            port="59538",
            downloadLink="https://[2016:8:40:96:c11::105,8.40.102.106,2016:8:40:96:c11::106,8.40.102.105]:25081/"
                         "v1/host-agent/download?uuid=crmdownloadlink9b11830894014d42900fd250cd7511cf",
            agentId="25cfd7e2-884b-49c4-986b-ae3430114b90",
            agentName="Agent02",
            jobId="12313231",
            connect_need_proxy=True,
            cert_secret_key="xxx",
            new_package_size=1024,
            packageType="tar"
        )
        res = '{"revStatus": 0}'.encode()
        mock_query_agent_api.return_value = self.MockResponse(status=200, json_data=res)
        data = action_agent_upgrade(query_upgrade_req)
        self.assertIsNotNone(data)

    @mock.patch("app.resource.service.common.penetrate_agent_service.query_agent_api")
    @mock.patch("app.resource.client.agent_client.url_request")
    def test_check_agent_upgrade_success(self, mock_query_agent_api, mock_url_request):
        from app.resource.schemas.agent_penetrate_schema import CheckAgentUpgradeStatusRequestSchema
        check_agent_upgrade_status_req = CheckAgentUpgradeStatusRequestSchema(
            ip="8.40.97.168_test",
            port="59538",
            connect_need_proxy=True)
        res = '{"revStatus": 0}'.encode()
        mock_query_agent_api.return_value = self.MockResponse(status=500, json_data=res)
        from app.resource.service.common.penetrate_agent_service import check_agent_upgrade_status
        data = check_agent_upgrade_status(check_agent_upgrade_status_req)
        self.assertIsNotNone(data)


if __name__ == '__main__':
    unittest.main(verbosity=2)
