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
from unittest.mock import MagicMock, patch

import app

mock.patch("app.common.config.Settings.get_db_password", mock.Mock(return_value="1234")).start()
mock.patch("app.common.config.Settings.get_kafka_password", mock.Mock(return_value="1234")).start()
mock.patch("app.common.config.Settings.get_redis_password", mock.Mock(return_value="1234")).start()

from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

RES = {
    'name': 'test_res',
    'uuid': '1d0ecd9a-2d74-45b3-911a-0a7efa246df4',
    'root_uuid': '1d0ecd9a-2d74-45b3-911a-0a7efa246df5'
}
ROOT_RES = [{
    'uuid': '1d0ecd9a-2d74-45b3-911a-0a7efa246df5',
    'endpoint': '8.40.98.222',
    'port': '443',
    'user_name': 'administrator@vsphere.local',
    'link_status': '1',
}]

DISK_INFO = [{
    'type': 'VirtualDisk',
    'sub_type': 'vim.vm.device.VirtualDisk',
    'uuid': '6000C290-dfc0-ad4b-63f0-281a3730ec9e',
    'slot': 'SCSI(0:1)',
    'name': 'Hard disk 2',
    'path': '[135store2] 68f83815-6d7e-4cdd-a5f9-7d2e165/20210720T183852.vmdk',
    'parent_name': 'zyx_test_vmware_3',
    'parent_uuid': '501f99d5-be0a-acde-b5f1-fcf2dab60c85',
    'root_uuid': '1d0ecd9a-2d74-45b3-911a-0a7efa246df5',
    'capacity': 62914560,
    'datastore': {
        'mo_id': 'datastore-11037',
        'uuid': '5e50c384-6a74c29a-2ece-749d8f8889b0',
        'url': 'ds:///vmfs/volumes/5e50c384-6a74c29a-2ece-749d8f8889b0/',
        'name': '135store2',
        'type': 'VMFS',
        'partitions': ['naa.5000cca25dc3125f']
    }}]

ENV = {
    'endpoint': '1.1.1.1',
    'port': '443',
    'name': 'vm1',
    'user_name': 'zs',
    'password': 'pass',
    'verify_cert': '0',
    'extend_context': {
        'certification': '-----BEGIN CERTIFICATE-----\n'
                         'MIIEHzCCAwegAwIBAgIJAORHVcsWbA7xMA0GCSqGSIb3DQEBCwUAMIGaMQswCQYD\n'
                         'VQQDDAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTATBgoJkiaJk/IsZAEZ\n'
                         'FgVsb2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExHDAaBgNV\n'
                         'BAoME3ZjMTQ3LTIwLnZtd2FyZS5jb20xGzAZBgNVBAsMElZNd2FyZSBFbmdpbmVl\n'
                         'cmluZzAeFw0yMjAyMDUxMDA2MTVaFw0zMjAyMDMxMDA2MTVaMIGaMQswCQYDVQQD\n'
                         'DAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTATBgoJkiaJk/IsZAEZFgVs\n'
                         'b2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExHDAaBgNVBAoM\n'
                         'E3ZjMTQ3LTIwLnZtd2FyZS5jb20xGzAZBgNVBAsMElZNd2FyZSBFbmdpbmVlcmlu\n'
                         'ZzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANPUh+5tPoj70CF8ufJD\n'
                         'k4q7NGpDKsJAW6uzHTxMMSI+hlTWqDkGXE5n/AiLyQnWk/8QrVWPVX1M3nZjhenQ\n'
                         'GsV6iugQuqXRfGza2rCY1Qm31SRG7q2ghPxcOCs+MGlvKKpmLIwhxlUv7j95Ktna\n'
                         'qDnzxSt5xaKZE5IxX9ZfSPDMGiNe1grnULVYAZuShlm58cjTWstK9n4nNUboaULa\n'
                         'Goovbre5P+Pw1qoTB0DuOM0Jnxj9NGdlVLuAW2XeQCw8LwbOEl+wAtd6wzrkGtKF\n'
                         'kbL7zdp8GoJ8Gxlet1xb+raFvoBCbN0IXyQZuep9GSmO9xfg8KwGKUdw/x52/S9j\n'
                         'o90CAwEAAaNmMGQwHQYDVR0OBBYEFBVeNSKZdMPfnPXnbLAwDAN4AI4RMB8GA1Ud\n'
                         'EQQYMBaBDmVtYWlsQGFjbWUuY29thwR/AAABMA4GA1UdDwEB/wQEAwIBBjASBgNV\n'
                         'HRMBAf8ECDAGAQH/AgEAMA0GCSqGSIb3DQEBCwUAA4IBAQCce+gD+k+9v8n5NeZ4\n'
                         'ejywcgesvKhgBzJKye8WLcmS6Us+Dezf1dABQnv7PGU46y48brHm3Kz3S7BQuVOb\n'
                         'afyN8MUYDz4wwCGvauhQAbXNH4Qe1ovjGm/ExN4d62tlzIVhe5YtXLp48zrAhQZ9\n'
                         'l98PPbCffcOkFpIxJFjmvFoLzoiao0NPtU+wqdoClRLgyE9wXwB67gdWjEPRjooV\n'
                         '8BOBcWD46pdnQK4jBSUdNAHvOewa850JqjSPvJtwTsGEPOdAZV91qB/jG3eA7+Uu\n'
                         'bYpDReBYrvmIup8Y8BuTFrWRca+VW1bjM8UFlZqo7FAGo2DG5cgzkjxf/AeoT7t6\n'
                         'wwpI\n'
                         '-----END CERTIFICATE-----\n',
        'revocation_list': '',
        'tls_compatible': 'False',
        'cert_name': 'cert_name',
        'cert_size': 0,
        'crl_size': 0
    }
}


class HostObj:
    def mock_name(self, name):
        self.__class__.__name__ = name


HOST_OBJ = HostObj()


class VMWareDiscoveryServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        from tests.test_cases.common.mock_settings import fake_settings
        self._mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        self._mock_common_db_init.start()
        from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock
        self._mock_kafka_producer = mock.patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
        self._mock_socket_connect = mock.patch("socket.socket.connect", MagicMock())
        self._mock_kafka_producer.start()
        self._mock_socket_connect.start()
        sys.modules['app.resource.service.vmware.service_instance_manager'] = MagicMock()

    def tearDown(self) -> None:
        self._mock_common_db_init.stop()
        self._mock_socket_connect.stop()
        self._mock_kafka_producer.stop()

    @patch("app.resource.service.common.resource_service.query_resource_by_id")
    @patch("app.resource.service.common.resource_service.query_environment")
    def test_get_vm_disks_success(self, _mock_query_by_id, _mock_query_env):
        from app.resource.schemas.virtual_resource_schemas import VirtualResourceSchema
        from app.resource.service.vmware.service_instance_manager import ServiceInstanceManager
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService

        res = VirtualResourceSchema(**RES)
        _mock_query_by_id = MagicMock(return_value=res)
        _mock_query_env = MagicMock(return_value=ROOT_RES)
        with mock.patch.object(VMwareDiscoveryService, "get_vm_disk", MagicMock(return_value=DISK_INFO)), \
             mock.patch.object(ServiceInstanceManager, "get_service_instance", MagicMock(return_value=None)):
            disks = VMwareDiscoveryService.get_vm_disks('53a12846-09b9-4be9-a713-c0e4019c292e')
            self.assertEqual(len(disks), 1)
            self.assertEqual(disks[0].get("uuid"), DISK_INFO[0].get("uuid"))

    def test_get_host_success(self):
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        from app.resource.schemas.env_schemas import ScanEnvSchema
        from app.resource.service.vmware.vmware_discovery_service import ScanNode

        self._init_host_obj()
        node = ScanNode(HOST_OBJ)

        host = VMwareDiscoveryService(None, env=ScanEnvSchema(**{'uuid': '11'}))._get_host(node=node, is_cluster=False)
        self.assertIsNotNone(host)
        self.assertEqual(host.children, 1)

    @patch("pyVim.connect.SmartConnect", MagicMock(
        side_effect=EmeiStorBizException(ResourceErrorCodes.NETWORK_CONNECTION_TIMEDOUT)))
    def test_throw_Exception_if_env_can_not_connect_when_login(self):
        """
        *用例场景：测试证书校验关闭时登陆vsphere正常
        *前置条件：环境信息参数正确
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        from app.resource.schemas.env_schemas import ScanEnvSchema
        class Instance:
            def __init__(self):
                self.about = self
                self.instanceUuid = '12345'

            def RetrieveContent(self):
                return self

        service_instance = Instance()
        with mock.patch.object(app.resource.service.vmware.vmware_discovery_service.VMwareDiscoveryService,
                               "get_service_instance_with_proxy",
                               MagicMock(return_value=service_instance)):
            with mock.patch.object(app.resource.service.vmware.vmware_discovery_service.VMwareDiscoveryService,
                                   "clear_alarm_if_env_exist", MagicMock()):
                ENV['verify_cert'] = '0'
                service_instance, cert_name = VMwareDiscoveryService.login(ScanEnvSchema(**ENV))
                self.assertIsNotNone(service_instance)

    @patch("ssl.create_default_context", MagicMock())
    @patch("socket.socket.connect_ex", MagicMock(return_value=1))
    def test_login_with_ssl_success(self):
        """
        *用例场景：测试ssl登陆vsphere成功
        *前置条件：环境信息参数正确
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        from app.resource.schemas.env_schemas import ScanEnvSchema

        class Instance:
            def __init__(self):
                self.about = self
                self.instanceUuid = '12345'

            def RetrieveContent(self):
                return self

        service_instance = Instance()
        ENV['verify_cert'] = '1'
        with mock.patch.object(app.resource.service.vmware.vmware_discovery_service.VMwareDiscoveryService,
                               "get_service_instance_with_proxy",
                               MagicMock(return_value=service_instance)):
            service_instance, cert_name = VMwareDiscoveryService.login(ScanEnvSchema(**ENV))
            self.assertIsNotNone(service_instance)
            self.assertIsNotNone(cert_name)

    @patch("app.base.db_base.database.session")
    @patch("app.resource.service.common.resource_service.comment_event_message", MagicMock)
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_delete_vm_by_name(self, database_mock):
        """
        *用例场景：测试登陆通过名称删除原虚拟机
        *前置条件：任务id非空
        *检查点: 无异常信息
        """
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        from app.resource.schemas.virtual_resource_schemas import VirtualResourceSchema
        res = VirtualResourceSchema(**RES)
        database_mock().__enter__().query().filter().delete.return_value = []
        with mock.patch.object(app.resource.service.vmware.vmware_discovery_service.VMwareDiscoveryService,
                               "get_vm_by_id",
                               MagicMock(return_value=None)):
            VMwareDiscoveryService(None, None).delete_vm(res)
            self.assertIsNotNone(res)

    @staticmethod
    def _init_host_obj():
        HOST_OBJ.summary = HostObj
        HOST_OBJ.summary.config = HostObj
        HOST_OBJ.summary.config.product = HostObj
        HOST_OBJ.summary.config.name = 'name'
        HOST_OBJ.summary.config.product.version = 'version'
        HOST_OBJ.summary.hardware = HostObj
        HOST_OBJ.summary.hardware.uuid = 'hardware_id'

        HOST_OBJ.parent = HostObj()
        HOST_OBJ.parent.resourcePool = HostObj
        HOST_OBJ.parent.resourcePool._moId = 'moId'
        HOST_OBJ.parent.resourcePool.vm = ['vm1']
        HOST_OBJ.parent.resourcePool.resourcePool = []
        HOST_OBJ.parent.mock_name(name='vim.ComputeResource')

        HOST_OBJ.runtime = HostObj
        HOST_OBJ.runtime.powerState = ''
        HOST_OBJ.runtime.connectionState = ''


if __name__ == '__main__':
    unittest.main(verbosity=2)
