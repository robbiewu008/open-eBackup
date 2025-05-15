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
# import unittest
# from unittest import mock
# from tests.test_cases.tools import functiontools, timezone
# import sys
# sys.modules['app.common.events.producer'] = mock.Mock()
# sys.modules['app.common.events.topics'] = mock.Mock()
# mock.patch("pydantic.validator", functiontools.mock_decorator).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
#            timezone.dmc.query_time_zone).start()
#
# from app.common.enums.resource_enum import ResourceSubTypeEnum
# from app.common.exception.unified_exception import EmeiStorBizException
#
# from app.resource.discovery.plugin_factory import plugin_factory
#
#
# @mock.patch("threading.Timer.start", mock.Mock())
# class PluginFactoryTest(unittest.TestCase):
#     def setUp(self):
#         super(PluginFactoryTest, self).setUp()
#         self.plugin_factory = plugin_factory
#
#     def test_create_vmware_plugin(self):
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.VMware)
#         from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
#         self.assertTrue(True, isinstance(plugin, VMwareDiscoveryPlugin))
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.vCenter)
#         self.assertTrue(True, isinstance(plugin, VMwareDiscoveryPlugin))
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.ESX)
#         self.assertTrue(True, isinstance(plugin, VMwareDiscoveryPlugin))
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.ESXi)
#         self.assertTrue(True, isinstance(plugin, VMwareDiscoveryPlugin))
#
#     def test_create_hyper_v_plugin(self):
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.HyperV)
#         from app.resource.discovery.plugins.hyper_v_discovery_plugin import HypervDiscoveryPlugin
#         self.assertTrue(True, isinstance(plugin, HypervDiscoveryPlugin))
#
#     def test_create_vm_backup_angent_plugin(self):
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.VMBackupAgent)
#         from app.resource.discovery.plugins.ebackup_vmware_host_discovery_plugin import VMwareBackupAgentDiscoveryPlugin
#         self.assertTrue(True, isinstance(plugin, VMwareBackupAgentDiscoveryPlugin))
#
#     def test_create_db_backup_angent_plugin(self):
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.DBBackupAgent)
#         from app.resource.discovery.plugins.ebackup_database_host_discovery_plugin import \
#             DbNativeBackupAgentDiscoveryPlugin
#         self.assertTrue(True, isinstance(plugin, DbNativeBackupAgentDiscoveryPlugin))
#
#     def test_create_ab_backup_client_plugin(self):
#         plugin = self.plugin_factory.create_plugin(ResourceSubTypeEnum.ABBackupClient)
#         from app.resource.discovery.plugins.anybackup_host_discovery_plugin import AnyBackupHostDiscoveryPlugin
#         self.assertTrue(True, isinstance(plugin, AnyBackupHostDiscoveryPlugin))
#
#     def test_should_raise_exception_when_plugin_is_not_exists(self):
#         self.assertRaises(EmeiStorBizException, self.plugin_factory.create_plugin, ResourceSubTypeEnum.DB2)
