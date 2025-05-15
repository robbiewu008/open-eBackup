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
# import sys
# import unittest
# from unittest import mock
# from unittest.mock import Mock
#
# from tests.test_cases.tools import functiontools, timezone
# from fastapi import HTTPException
#
#
# from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema
#
# sys.modules['app.common.events.producer'] = mock.Mock()
# sys.modules['app.common.events.topics'] = mock.Mock()
# sys.modules['app.common.logger'] = mock.Mock()
# mock.patch("pydantic.validator", functiontools.mock_decorator).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", timezone.dmc.query_time_zone).start()
#
#
# PARAMS = {
#     'uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1',
#     'name': 'host-8-40-97-167',
#     'type': 'Host',
#     'user_name': '',
#     'password': '',
#     'endpoint': '1.1.1.1',
#     'sub_type': 'DWSBackupAgent',
#     'port': '59525',
#     'rescan_interval_in_sec': 180,
#     'verify_cert': None,
#     'user_id': None,
#     'extend_context': {'host': {'uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'name': 'host-8-40-97-167',
#                                 'type': 'Host', 'sub_type': 'DWSBackupAgent', 'endpoint': '8.40.97.167',
#                                 'root_uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'port': '59525',
#                                 'link_status': '0', 'os_type': 'linux', 'os_name': 'linux',
#                                 'path': '8.40.97.167', 'user_id': '88a94c476f12a21e016f12a246e50009'}},
# }
#
# UPDATE_PARAMS = {
#     'uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1',
#     'name': 'host-8-40-97-167',
#     'user_name': '',
#     'password': '',
#     'endpoint': '1.1.1.1',
#     'port': '59525',
#     'verify_cert': None,
#     'extend_context': {'host': {'uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'name': 'host-8-40-97-167',
#                                 'type': 'Host', 'sub_type': 'DWSBackupAgent', 'endpoint': '8.40.97.167',
#                                 'root_uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'port': '59525',
#                                 'link_status': '0', 'os_type': 'linux', 'os_name': 'linux',
#                                 'path': '8.40.97.167', 'user_id': '88a94c476f12a21e016f12a246e50009'}},
# }
#
# ENV = {'uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'name': 'host-8-40-97-167',
#                                 'type': 'Host', 'sub_type': 'DWSBackupAgent', 'endpoint': '8.40.97.167',
#                                 'root_uuid': '6f4dc27b-51b7-4cf8-8cf7-4a6099d559e1', 'port': '59525',
#                                 'link_status': '0', 'os_type': 'linux', 'os_name': 'linux',
#                                 'path': '8.40.97.167', 'user_id': '88a94c476f12a21e016f12a246e50009'}
#
#
# class DWSBackupAgentDiscoveryPluginTest(unittest.TestCase):
#     def setUp(self):
#         super(DWSBackupAgentDiscoveryPluginTest, self).setUp()
#         sys.modules['app.common.database'] = Mock()
#         sys.modules['app.common.config'] = Mock()
#         from app.resource.discovery.plugins import dws_discovery_plugin
#         from app.resource.rpc import hw_agent_rpc as agent_rpc
#         self.plugin_service = dws_discovery_plugin
#         self.plugin = dws_discovery_plugin.DWSBackupAgentDiscoveryPlugin()
#         self.agent_rpc = agent_rpc
#         self.param = ScanEnvSchema(**PARAMS)
#         self.UPDATE_PARAMS = UpdateEnvSchema(**UPDATE_PARAMS)
#
#     def test_do_scan_env_when_add_host(self):
#         self.plugin.do_fetch_resource = mock.Mock()
#         res = self.plugin.do_scan_env(self.param, True)
#         self.assertEqual(res, ENV)
#
#     def test_do_scan_env_when_refresh_host(self):
#         self.plugin_service.upsert_environment = mock.Mock()
#         self.plugin.do_fetch_resource = mock.Mock()
#         res = self.plugin.do_scan_env(self.param, False)
#         self.assertEqual(res, ENV)
#
#     def test_do_fetch_resource_when_success(self):
#         self.plugin_service.update_host_online = mock.Mock()
#         self.plugin.host_online_clear_alarm = mock.Mock()
#         self.agent_rpc.query_dws_host_agent = mock.Mock()
#         res = self.plugin.do_fetch_resource(self.param)
#         self.assertIsNone(res)
#
#     @mock.patch('app.resource.client.system_base.get_user_info_by_user_id', mock.Mock(return_value=False))
#     @mock.patch("app.resource.discovery.plugins.dws_discovery_plugin.automatic_authorization_by_agent_userid",
#                 mock.Mock(return_value=ENV))
#     @mock.patch("app.base.db_base.database.session")
#     def test_do_modify_env(self, mock_session):
#         res = self.plugin.do_modify_env(self.UPDATE_PARAMS)
#         self.assertEqual(res, self.UPDATE_PARAMS)
#
#     def test_do_fetch_resource_when_failed(self):
#         self.plugin_service.update_databases_hosts_all_offline = mock.Mock()
#         self.plugin.host_online_clear_alarm = mock.Mock()
#         self.plugin.host_offline_send_alarm = mock.Mock()
#         self.agent_rpc.query_dws_host_agent = mock.Mock(return_value=HTTPException)
#         res = self.plugin.do_fetch_resource(self.param)
#         self.assertIsNone(res)
#
#
# if __name__ == '__main__':
#     unittest.main(verbosity=2)
