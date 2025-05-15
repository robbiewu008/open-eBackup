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
from unittest import TestCase, mock
from unittest.mock import Mock, patch, MagicMock

from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases import common_mocker # noqa
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.resource.common.common_enum import HostMigrationAgentType, HostMigrationIpType, ProxyHostTypeEnum
from app.resource.schemas.host_models import MigrationReq, HostMigrationReq


class TClusterTargetMock:
    def __init__(self):
        self.cluster_id = 1
        self.cluster_ip = "127.1.1.1,127.0.0.0"
        self.username = "123"
        self.password = "sdf"


MigrationResponseMock = {"targetClusterId": 1,
                         "hostMigrateReq": [{"clusterId": "1",
                                             "hostId": "b12f4665-8600-4e12-9da3-5c2b130729b5",
                                             "hostUserName": "root",
                                             "hostPassword": "123",
                                             "sshMacs": "safe"
                                             }]
                         }

Mock_1 = {"host_migrate_req": MigrationReq(**{"targetClusterId": 1,
                                              "hostMigrateReq": [HostMigrationReq(**{
                                                  "hostId": "b12f4665-8600-4e12-9da3-5c2b130729b5",
                                                  "hostUserName": "root",
                                                  "hostPassword": "123",
                                                  "sshMacs": "safe"
                                              }),
                                                                 HostMigrationReq(**{
                                                                     "hostId": "b12f4665-8600-4e12-9da"
                                                                               "3-5c2b130729b5",
                                                                     "hostUserName": "root",
                                                                     "hostPassword": "123",
                                                                     "sshMacs": "safe"
                                                                 })]
                                              })
          }


class HostMigrationResponseMock:
    def __init__(self):
        self.host_id = "1"
        self.proxy_host_type = "DBBackupAgent"
        self.os_type = "Linux"
        self.ip_type = ""
        self.ip_address = "127.0.0.1"
        self.host_username = "123"
        self.host_password = "123"


class HostInfoMock:
    def __init__(self):
        self.uuid = 'b12f4665-8600-4e12-9da3-5c2b130729b5'
        self.endpoint = "127.0.0.1"
        self.link_status = "1"
        self.name = 'abc'
        self.os_type = 'linux'


class TargetClusterMock:
    def __init__(self):
        self.status = "127.0.0.1"
        self.role = 1


class ProtectionObjectMock:
    def __init__(self):
        self.ext_parameters = "{'proxy_id': '123'}"
        self.name = "123"


class ResExtendInfoMock:
    def __init__(self):
        self.resource_id = "123"


class HostMigrationScheduleMock:
    def __init__(self):
        # 后端获取
        self.host_id: str = "b12f4665-8600-4e12-9da3-5c2b130729b5"
        self.host_user_name: str = "name"
        self.host_password: str = "123"
        self.proxy_host_type: str = "DBBackupAgent"
        self.os_type: str = "Linux"
        self.ip_type: str = "IPV4"
        self.ip_address: str = "127.0.0.1"
        self.target_cluster_ip: str = "127.0.0.0"
        self.target_cluster_port: int = 99
        self.job_id: str = "1234"
        self.target_cluster_job_id: str = "1234"

    def dict(self):
        return self.__dict__


class TestHostMigrateObjectService(TestCase):

    def setUp(self) -> None:
        from tests.test_cases.common.mock_settings import fake_settings
        self._mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        self._mock_common_db_init.start()
        sys.modules['app.protection.object.db.crud_projected_object'] = Mock()
        sys.modules['app.resource.client.agent_client'] = Mock()
        from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock
        self._mock_kafka_producer = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
        self._mock_kafka_producer.start()
        from app.resource.service.host import host_migrate_service
        self.host_migrate_service = host_migrate_service

    def tearDown(self) -> None:
        self._mock_common_db_init.stop()
        del sys.modules['app.protection.object.db.crud_projected_object']
        del sys.modules['app.resource.client.agent_client']
        self._mock_kafka_producer.stop()

    @patch("app.base.db_base.database.session")
    def test_update_host_status_migrate(self, database_mock):
        """
        校验更新为迁移中是否存在问题
        """
        database_mock().__enter__().query(self.host_migrate_service.EnvironmentTable) \
            .filter(self.host_migrate_service.EnvironmentTable.uuid == "123") \
            .first.return_value = None
        res = self.host_migrate_service.update_host_status_migrate("123")
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_query_target_cluster_by_id(self, database_mock):
        """
        校验查询
        """
        from app.resource.models.resource_models import TClusterTarget
        database_mock().__enter__().query(TClusterTarget) \
            .filter(TClusterTarget.cluster_id == "123") \
            .one_or_none.return_value = 123
        res = self.host_migrate_service.query_target_cluster_by_id("123")
        self.assertNotEqual(res, "123")

    def test_get_proxy_host_type(self):
        """
        校验是否能获取到正确的返回值
        """
        res_db = ProxyHostTypeEnum.get_type_by_proxy("DBBackupAgent")
        res_vm = ProxyHostTypeEnum.get_type_by_proxy("VMBackupAgent")
        res_dws = ProxyHostTypeEnum.get_type_by_proxy("DWSBackupAgent")
        res_ub = ProxyHostTypeEnum.get_type_by_proxy("UBackupAgent")
        self.assertEqual(res_db, HostMigrationAgentType.HOST_AGENT_ORACLE.value)
        self.assertEqual(res_vm, HostMigrationAgentType.REMOTE_AGENT_VMWARE.value)
        self.assertEqual(res_dws, HostMigrationAgentType.REMOTE_AGENT.value)
        self.assertEqual(res_ub, HostMigrationAgentType.REMOTE_AGENT.value)

    def test_get_ip_type(self):
        """
        校验是否能获取到正确的返回值
        """
        ipv4 = "127.0.0.0"
        ipv6 = "f000::5000:000a:00a:00a"
        res_ipv4 = self.host_migrate_service.get_ip_type(ipv4)
        res_ipv6 = self.host_migrate_service.get_ip_type(ipv6)
        self.assertEqual(res_ipv4, HostMigrationIpType.IPV4.value)
        self.assertEqual(res_ipv6, HostMigrationIpType.IPV6.value)

    def test_create_migrate_object_when_fail(self):
        self.host_migrate_service.query_target_cluster_by_id = Mock(return_value=TargetClusterMock())
        self.host_migrate_service.HostMigrateObjectService.check_host_migrate = Mock(return_value="localhost")
        self.host_migrate_service.check_has_associated_protected_objects = Mock()
        self.host_migrate_service.set_target_cluster_token = Mock()
        host_mock = HostInfoMock()
        host_mock.os_type = "windows"
        self.host_migrate_service.get_environment = Mock(return_value=host_mock)
        self.host_migrate_service.HostMigrateObjectService.create_host_migrate_job = Mock(return_value="1234")
        self.host_migrate_service.HostMigrateObjectService.create_host_migrate_immediate_schedule = Mock()
        self.host_migrate_service.SchedulerClient.submit_interval_job = Mock()
        migrate_mock = MigrationReq(**MigrationResponseMock)
        try:
            res = self.host_migrate_service.HostMigrateObjectService.create_migrate_object("abc", migrate_mock)
        except EmeiStorBizException as ex:
            self.assertIsNotNone(ex)

    def test_create_migrate_object_when_success(self):
        self.host_migrate_service.query_target_cluster_by_id = Mock(return_value=TargetClusterMock())
        self.host_migrate_service.HostMigrateObjectService.check_host_migrate = Mock(return_value="localhost")
        self.host_migrate_service.check_has_associated_protected_objects = Mock()
        self.host_migrate_service.set_target_cluster_token = Mock()
        self.host_migrate_service.get_environment = Mock(return_value=HostInfoMock())
        self.host_migrate_service.HostMigrateObjectService.create_host_migrate_job = Mock(return_value="123")
        self.host_migrate_service.HostMigrateObjectService.create_host_migrate_immediate_schedule = Mock()
        self.host_migrate_service.SchedulerClient.submit_interval_job = Mock()
        migrate_mock = MigrationReq(**MigrationResponseMock)
        res = self.host_migrate_service.HostMigrateObjectService.create_migrate_object("abc", migrate_mock)
        self.assertEqual(res, ['123'])

    def test_get_migrate_host_ip_by_params(self):
        self.host_migrate_service.query_host_info_by_uuid = Mock(return_value=HostInfoMock())
        res = self.host_migrate_service.get_migrate_host_ip_by_params(Mock_1)
        self.assertEqual("127.0.0.1,127.0.0.1", res)

    def test_host_migrate_interval_schedule_task(self):
        Mock_2 = {
            "totalCount": 1,
            "records": [{"status": "PENDING"}]
        }
        self.host_migrate_service.TargetClusterRpc.query_job_task_detail_rpc = Mock(return_value=Mock_2)
        self.host_migrate_service.HostMigrateObjectService.host_migrate_schedule_task_pending = Mock(return_value=True)
        res = self.host_migrate_service.HostMigrateObjectService.host_migrate_interval_schedule_task(Mock_1)
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_check_has_associated_protected_objects(self, database_mock):
        from app.protection.object.models.projected_object import ProtectedObject
        from app.resource.models.resource_models import ResExtendInfoTable, ResourceTable
        database_mock().__enter__().query(ProtectedObject) \
            .filter(ProtectedObject.sub_type == ResourceSubTypeEnum.VirtualMachine.value) \
            .all.return_value = []

        database_mock().__enter__().query(ResExtendInfoTable) \
            .filter(ResExtendInfoTable.key == "agents", ResExtendInfoTable.value.like(f'%123%')) \
            .all.return_value = [ResExtendInfoMock]

        database_mock().__enter__().query(ResourceTable.name) \
            .filter(ResourceTable.uuid.in_(["123"])) \
            .all.return_value = []

        res = self.host_migrate_service.check_has_associated_protected_objects({"123": "123"})
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_delete_host_choose(self, database_mock):
        host_info = {
            "sub_type": ResourceSubTypeEnum.UBackupAgent.value,
            "host_id": "123"
        }
        self.host_migrate_service.delete_host_ubackup_agent = Mock()
        res = self.host_migrate_service.delete_host_choose(host_info, database_mock)
        self.assertIsNone(res)


if __name__ == '__main__':
    unittest.main(verbosity=2)
