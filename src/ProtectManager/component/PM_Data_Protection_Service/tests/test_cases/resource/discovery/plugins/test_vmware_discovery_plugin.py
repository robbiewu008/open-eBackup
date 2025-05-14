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
import json
import unittest
from unittest import mock
from unittest.mock import MagicMock, patch
from tests.test_cases import common_mocker  # noqa

from app.common.enums.resource_enum import ProtectionStatusEnum

class VMWareDiscoveryPluginTest(unittest.TestCase):
    def setUp(self) -> None:
        self._mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        self._mock_common_db_init.start()
        from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock
        self._mock_kafka_producer = mock.patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
        self._mock_kafka_producer.start()
        sys.modules['app.resource.service.vmware.service_instance_manager'] = MagicMock()

    @patch("app.base.db_base.database.session")
    @patch("app.common.clients.system_base_client.SystemBaseClient.encrypt",
           MagicMock(return_value={'ciphertext': 'ciphertext1'}))
    def test_save_cert_infos_success(self, database_mock):
        """
        *用例场景：测试保存证书信息成功
        *前置条件：环境信息参数正确
        *检查点: 无异常信息返回
        """
        from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
        from app.resource.models.resource_models import ResExtendInfoTable
        from app.resource.schemas.env_schemas import ScanEnvSchema
        params = ScanEnvSchema(
            extend_context={
                'cert_name': 'cert_name1',
                'crl_name': 'crl_name1',
                'certification': 'certification1',
                'revocation_list': 'revocation_list1',
            }
        )
        env_id = 'env_id'
        key = 'cert_name'
        existed_cert = ResExtendInfoTable(
            uuid='111',
            resource_id=env_id,
            key=key,
            value='val'
        )
        database_mock().__enter__().query(ResExtendInfoTable) \
            .filter(ResExtendInfoTable.resource_id == env_id,
                    ResExtendInfoTable.key == key) \
            .first.return_value = existed_cert
        VMwareDiscoveryPlugin.save_cert_infos(env_id, params)
        self.assertIsNotNone(VMwareDiscoveryPlugin)

    @patch("app.base.db_base.database.session")
    def test_do_modify_env_success(self, database_mock):
        """
        *用例场景：测试修改注册vsphere成功
        *前置条件：环境信息参数正确
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
        from app.resource.models.resource_models import ResExtendInfoTable
        from app.common.clients.system_base_client import SystemBaseClient
        from app.resource.schemas.env_schemas import UpdateEnvSchema
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        from app.common.enums.resource_enum import ResourceSubTypeEnum
        from app.resource.models.resource_models import EnvironmentTable
        from app.common.clients.alarm.alarm_client import AlarmClient

        params = UpdateEnvSchema(
            verify_cert=1,
            sub_type=ResourceSubTypeEnum.vCenter.value,
            uuid="12345",
            endpoint="1.1.1.1",
            port=443,
            extend_context={
                'cert_name': 'cert_name1',
                'crl_name': 'crl_name1',
                'certification': 'certification1',
                'revocation_list': 'revocation_list1',
            }
        )
        existed_cert = ResExtendInfoTable(
            uuid='111',
            resource_id='env_id',
            key='cert_name',
            value='val'
        )
        env = EnvironmentTable(
            sub_type=ResourceSubTypeEnum.vCenter.value,
            type="vsphere"
        )

        class Instance:
            def __init__(self):
                self.about = self
                self.instanceUuid = '12345'

            def RetrieveContent(self):
                return self

        service_instance = Instance()
        database_mock().__enter__().query().filter_by().first.return_value = env
        database_mock().__enter__().query() \
            .filter() \
            .first.return_value = existed_cert
        database_mock().__enter__().query() \
            .filter() \
            .all.return_value = [existed_cert]
        with mock.patch.object(VMwareDiscoveryService, "login",
                               MagicMock(return_value=(service_instance, "cert_name"))), \
                mock.patch.object(AlarmClient, "clear_entity_alarm", MagicMock()), \
                mock.patch.object(SystemBaseClient, "encrypt", MagicMock(return_value={'ciphertext': 'ciphertext'})), \
                mock.patch.object(SystemBaseClient, "decrypt", MagicMock(return_value={'plaintext': 'plaintext'})), \
                mock.patch.object(VMwareDiscoveryPlugin, "clear_cert_alarm", MagicMock(return_value=None)), \
                mock.patch.object(VMwareDiscoveryService, "disconnect", MagicMock()):
            modify_env = VMwareDiscoveryPlugin.do_modify_env(params)
            self.assertIsNotNone(modify_env)

    @patch("app.base.db_base.database.session")
    @patch("app.resource.service.common.resource_service.query_resource_by_id", MagicMock(return_value={}))
    @patch("app.resource.service.common.resource_service.query_environment", MagicMock(return_value={}))
    @patch("app.resource.service.common.resource_service.query_resource", MagicMock(return_value={}))
    def test_pre_check_success(self, database_mock):
        """
        *用例场景：测试手动扫描环境成功
        *前置条件：环境信息参数正确
        *检查点: 无异常信息返回
        """
        from app.common.enums.resource_enum import ResourceSubTypeEnum
        from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
        from app.resource.schemas.env_schemas import ScanEnvSchema
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService

        params = ScanEnvSchema(
            name="test_name",
            verify_cert=1,
            sub_type=ResourceSubTypeEnum.vCenter.value,
            uuid="12345",
            endpoint="1.1.1.1",
            port=443,
            extend_context={
                'cert_name': 'cert_name1',
                'crl_name': 'crl_name1',
                'certification': 'certification1',
                'revocation_list': 'revocation_list1',
            }
        )
        class Instance:
            def __init__(self):
                self.about = self
                self.instanceUuid = '12345'

            def RetrieveContent(self):
                return self

        service_instance = Instance()
        database_mock().__enter__().query() \
            .filter() \
            .all.return_value = []
        from app.common.clients.system_base_client import SystemBaseClient
        with mock.patch.object(VMwareDiscoveryService, "login",
                               MagicMock(return_value=(service_instance, "cert_name"))), \
                mock.patch.object(SystemBaseClient, "encrypt", MagicMock(return_value={'ciphertext': 'ciphertext'})), \
                mock.patch.object(VMwareDiscoveryService, "get_host_info", MagicMock(return_value=[{'uuid': 'id1'}])), \
                mock.patch.object(VMwareDiscoveryService, "get_vsphere_info", MagicMock(
                    return_value={'uuid': 'id1', 'sub_type': ResourceSubTypeEnum.vCenter.value})):
            VMwareDiscoveryPlugin.pre_check(params)
            self.assertIsNotNone(VMwareDiscoveryPlugin)




if __name__ == '__main__':
    unittest.main(verbosity=2)
