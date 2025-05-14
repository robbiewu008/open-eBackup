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
import uuid
from unittest import mock
from unittest.mock import MagicMock, Mock, patch

from tests.test_cases import common_mocker # noqa
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum, LinkStatusEnum
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.resource.schemas.env_schemas import ScanEnvSchema
from app.resource.schemas.host_models import HostCreateInfo
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()

class GaussDBCluster:
    uuid = "d405da14-f860-4334-adcf-1586b7859b83"
    children_uuids = ['eb03fbc94a7d4d89235d5513e60d9caa', 'ad87e2c952525638e75afd03b71d6d7d']
    is_cluster = True
    link_status = "0"


AGENT_DATA = {
    'instName': 'oltp',
    'dbName': 'oracle',
    'authType': 1,
    'dbUUID': '123',
    'state': 1,
    'isAsmInst': 1,
    'version': '18.0.0.0',
    'dbRole': 1,
    'name': ''
}
ENV = {
    'uuid': '123',
    'name': 'host',
    'endpoint': '10.10.10.10'
}
env_param = {
    "host_id": '123',
    'name': 'host',
    'proxy_type': 'Database',
    'user_name': "",
    'password': "",
    'endpoint': '10.10.10.10',
    'port': 10001,
    'extend_context': {'host': 'env',
                       'extend_info': {
                           'install_path': '/opt'
                       }
                       },
    'rescan_interval_in_sec': 180
}
HOST = {
    'uuid': 'host_id_01',
    'endpoint': '1.1.1.1',
    'type': ResourceTypeEnum.Host.value,
    'sub_type': ResourceSubTypeEnum.DBBackupAgent.value,
    'link_status': LinkStatusEnum.Online.value,
    'is_cluster': True,
    'children_uuids': 'host_id_01'
}


class TestHostResService(unittest.TestCase):
    def setUp(self) -> None:
        sys.modules['app.protection.object.schemas.protected_object'] = Mock()
        sys.modules['app.resource_lock.kafka.rollback_utils'] = Mock()
        sys.modules['app.resource_lock.service.lock_service'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        sys.modules['app.kafka'] = Mock()
        _mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        from tests.test_cases.common.mock_settings import fake_settings
        self._mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        self._mock_common_db_init.start()
        from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock
        self._mock_kafka_producer = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
        self._mock_kafka_producer.start()
        from app.resource.service.host import host_res_service
        self.host_res_service = host_res_service

    def tearDown(self) -> None:
        self._mock_common_db_init.stop()
        self._mock_kafka_producer.stop()

    def test_should_raise_ex_if_env_is_cluster_and_is_protected_when_delete_host_or_cluster(self):
        from app.resource.models.resource_models import EnvironmentTable
        from app.common.exception.unified_exception import EmeiStorBizException
        mock_session_ins = MagicMock()
        mock_session_ins.query().filter().one.return_value = EnvironmentTable(
            uuid="10001", children_uuids=["10002", "10003"], is_cluster=True)
        mock_session_ins.query().filter().all.return_value = [EnvironmentTable(uuid="10002", is_cluster=False),
                                                              EnvironmentTable(uuid="10003", is_cluster=False)]
        self.host_res_service.check_resource_has_sla = Mock(side_effect=EmeiStorBizException(
            ResourceErrorCodes.DELETE_RESOURCE_HAS_SLD, "fake_sla"))
        # 集群下主机和资源被保护抛出异常
        with self.assertRaises(EmeiStorBizException):
            self.host_res_service.delete_host_or_cluster(mock_session_ins, "10001")

    def test_should_return_none_if_env_is_host_when_delete_host_or_cluster(self):
        from app.resource.models.resource_models import EnvironmentTable
        from app.resource.models.resource_models import ResourceTable
        mock_session_ins = MagicMock()
        mock_session_ins.query().filter().one.return_value = EnvironmentTable(uuid="10004", is_cluster=False)
        self.host_res_service.delete_schedule_task = Mock
        mock_session_ins.query().filter().one_or_none.return_value = None
        mock_session_ins.query().filter().all.return_value = [
            ResourceTable(uuid="10005", type=ResourceTypeEnum.Database.value,
                          sub_type=ResourceSubTypeEnum.Oracle.value)]
        self.host_res_service.comment_event_message = Mock
        self.host_res_service.produce = Mock
        result = self.host_res_service.delete_host_or_cluster(mock_session_ins, "10004")
        self.assertIsNone(result)

    @mock.patch("app.resource.service.host.host_res_service.DiscoveryManager.__init__", Mock(return_value=None))
    def test_should_return_dict_if_same_ip_when_add_host(self):
        self.host_res_service.get_environment_type_by_proxy_type = Mock(return_value='DBBackupAgent')
        self.host_res_service.host_to_env_param = Mock(return_value=ScanEnvSchema(**env_param))
        self.host_res_service.get_host_by_id = Mock(return_value=None)
        self.host_res_service.delete_extend_info = Mock
        from app.resource.models.resource_models import EnvironmentTable
        ip = "10.10.10.10"
        env_uuid = str(uuid.uuid4())
        env = EnvironmentTable(
            uuid=env_uuid,
            endpoint=ip,
            type=ResourceTypeEnum.Host.value,
            sub_type=ResourceSubTypeEnum.UBackupAgent.value,
            link_status=LinkStatusEnum.Online.value
        )
        self.host_res_service.get_host_by_ip = Mock(return_value=env)
        host = HostCreateInfo(
            name='localhost',
            ip=ip,
            host_id=env_uuid,
            proxy_type=0,
            link_status='1',
            register_type='1'
        )
        ret = self.host_res_service.add_host(host)
        self.assertIsInstance(ret, dict)
        self.assertEqual(ret.get("host_id"), env_uuid)

    @patch("app.base.db_base.database.session")
    def test_should_return_env_when_get_host_by_ip(self, database_mock):
        ip = '10.10.10.10'
        from app.resource.models.resource_models import EnvironmentTable
        env = EnvironmentTable(
            uuid=str(uuid.uuid4()),
            endpoint=ip,
            type=ResourceTypeEnum.Host.value,
            sub_type=ResourceSubTypeEnum.UBackupAgent.value,
            link_status=LinkStatusEnum.Online.value
        )
        database_mock().__enter__().query(self.host_res_service.EnvironmentTable) \
            .filter(self.host_res_service.EnvironmentTable.endpoint == ip) \
            .first.return_value = env
        ret = self.host_res_service.get_host_by_ip(ip)
        self.assertIsInstance(ret, EnvironmentTable)

    @patch("app.base.db_base.database.session")
    def test_manual_modify_host_name_success(self, database_mock):
        host_id = "89cd0ca63769368f3e94dd1abd3c76ce"
        self.host_res_service.get_host_by_id = Mock(return_value={
            'sub_type': ResourceSubTypeEnum.ABBackupClient.value,
        })
        database_mock.__enter__().query(self.host_res_service.ResourceTable).filter(
            self.host_res_service.ResourceTable.root_uuid == host_id,
            self.host_res_service.ResourceTable.sub_type == ResourceSubTypeEnum.Fileset.value
        ).update.return_value = None
        database_mock.__enter__().merge.return_value = None
        self.host_res_service.manual_modify_host_name(host_id, "new_name")
        self.assertIsNotNone(host_id)

    @patch("app.base.db_base.database.session")
    def test_compare_agent_success(self, database_mock):
        """
        *用例场景：测试比较版本是否可升级成功
        *前置条件：环境版本号参数有效
        *检查点: 返回结果非空，无异常信息
        """
        database_mock.__enter__().update.return_value = None
        database_mock.__enter__().merge.return_value = None
        env = {
            "agent_version": "1.1.RC2.014",
            "version": "1.1.RC2.014"
        }
        new_agent_version = {
            "releaseVersion": "1.1.RC2.015",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.2.1RC2.014",
            "version": "1.2.1RC2.014"
        }
        new_agent_version = {
            "releaseVersion": "1.2.1",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.2.1RC1.014",
            "version": "1.2.1RC2.014"
        }
        new_agent_version = {
            "releaseVersion": "1.2.1",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.2.1.014",
            "version": "1.2.1.014"
        }
        new_agent_version = {
            "releaseVersion": "1.2.1RC2.014",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertFalse(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.2.1.014",
            "version": "1.2.1.014"
        }
        new_agent_version = {
            "releaseVersion": "1.3.RC1.014",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.2.1",
            "version": "1.2.1"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertFalse(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.RC1",
            "version": "1.3.RC1"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "016"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertFalse(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.0.018",
            "version": "1.3.0.018"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC1.001",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.0.SPC1.001",
            "version": "1.3.0.SPC1.001"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC1.002",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.0.SPC1.001",
            "version": "1.3.0.SPC1.001"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC2.002",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.0.SPC1.001",
            "version": "1.3.0.SPC1.001"
        }
        new_agent_version = {
            "releaseVersion": "1.5.0.018",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertIsNotNone(self.host_res_service.compare_agent(env, new_agent_version))
        env = {
            "agent_version": "1.3.0.SPC2.001",
            "version": "1.3.0.SPC2.001"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC1.001",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertFalse(self.host_res_service.compare_agent(env, new_agent_version))

        # 1.2.1.SPC升级
        env = {
            "agent_version": "1.2.1.SPC2.010",
            "version": "1.2.1.SPC2.001"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC1.010",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))

        # 1.2.1 升级到1.3.0的SPC版本
        env = {
            "agent_version": "1.2.1.010",
            "version": "1.2.1.010"
        }
        new_agent_version = {
            "releaseVersion": "1.3.0.SPC1.010",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))

        # 1.6.RC1.SPC2.156 升级到1.6.RC1.SPC2.157的SPC版本
        env = {
            "agent_version": "1.6.RC1.SPC2.156",
            "version": "1.6.RC1.SPC2.156"
        }
        new_agent_version = {
            "releaseVersion": "1.6.RC1.SPC2.157",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))

        # 1.6.RC1.SPC3.156 升级到1.6.RC1.SPC3.157的SPC版本
        env = {
            "agent_version": "1.6.RC1.SPC3.156",
            "version": "1.6.RC1.SPC3.156"
        }
        new_agent_version = {
            "releaseVersion": "1.6.RC1.SPC3.157",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))

        # 1.6.RC1.SPC2.156 升级到1.6.RC1.SPC3.57的SPC版本
        env = {
            "agent_version": "1.6.RC1.SPC2.156",
            "version": "1.6.RC1.SPC2.156"
        }
        new_agent_version = {
            "releaseVersion": "1.6.RC1.SPC3.57",
            "upgradeVersions": [{"minBVesion": "011", "maxBVesion": "999"}],
            "agentId": "89cd0ca63769368f3e94dd1abd3c76ce",
        }
        self.assertTrue(self.host_res_service.compare_agent(env, new_agent_version))

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_asm_auth_success(self, database_mock):
        """
        *用例场景：测试修改ASM授权成功
        *前置条件：主机存在，授权参数有效
        *检查点: 无异常抛出
        """
        from app.resource.rpc import hw_agent_rpc
        from app.common.rpc import system_base_rpc
        from app.resource.schemas.host_models import AsmAuthRequest
        from app.resource.models.resource_models import EnvironmentTable

        host_id = str(uuid.uuid4())
        auth_req = AsmAuthRequest(
            auth_type=0,
            asm_insts=[],
            username='uname_01',
            password='pass_01',
        )
        host = EnvironmentTable(
            uuid=host_id,
            endpoint='1.1.1.1',
            type=ResourceTypeEnum.Host.value,
            sub_type=ResourceSubTypeEnum.DBBackupAgent.value,
            link_status=LinkStatusEnum.Online.value,
            is_cluster=True,
            children_uuids=['123', '123']
        )

        class Result:
            ip = ''
            uuid = ''
            asm_instances = []
            username = ''
            password = ''
            verify_status = ''
            auth_type = ''
            asm = ''

        class QueryMock:
            def __init__(self, sub_type):
                self.type = sub_type
                pass

            def filter(self):
                return QueryMock('host')

            def one(self):
                return host

            def all(self):
                return [Result()]

        database_mock().__enter__().query().filter.return_value = QueryMock('host')
        database_mock.__enter__().merge.return_value = None
        hw_agent_rpc.query_asm_instance = MagicMock(return_value=[{'instName': 'instName-1'}])
        hw_agent_rpc.testconnection = MagicMock(return_value=None)
        system_base_rpc.encrypt = MagicMock(return_value='passwd')
        self.host_res_service.asm_auth(host_id, auth_req)
        auth_req.auth_type = 1
        self.host_res_service.asm_auth(host_id, auth_req)
        self.assertIsNotNone(host_id)

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_get_db_asm_auth_success(self, database_mock):
        """
        *用例场景：测试根据主机ID查询主机ASM授权信息成功
        *前置条件：ASM授权信息非空
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.models.database_models import AsmInfoTable
        host_id = 'host_id_01'
        database_mock.query(AsmInfoTable) \
            .filter(AsmInfoTable.root_uuid == host_id) \
            .one_or_none.return_value = AsmInfoTable(asm_instances='in', auth_type=1, username='user01',
                                                     password='pass01')
        asm = self.host_res_service.get_db_asm_auth(host_id, database_mock)
        self.assertIsNotNone(asm)

    @patch("app.base.db_base.database.session")
    def test_refresh_cluster_host_info_if_host_is_cluster_success(self, database_mock):
        """
        *用例场景：测试更新主机信息成功
        *前置条件：主机是集群且参数正确
        *检查点: 返回结果非空，无异常信息
        """
        from app.protection.object.models.projected_object import ProtectedObject
        from app.resource.models.resource_models import EnvironmentTable
        from app.resource.discovery.res_discovery_plugin import DiscoveryManager
        import asyncio
        host_id = 'host_id_01'
        project_obj = ProtectedObject(
            uuid='obj_uuid'
        )
        host = EnvironmentTable(**HOST)

        class Filter:
            def all(self):
                return [project_obj]

            def delete(self):
                pass

            def one(self):
                return host

        database_mock.query().filter.return_value = Filter()
        DiscoveryManager.refresh_env = MagicMock(return_value=None)
        #asyncio.run = MagicMock(return_value=None)
        self.host_res_service.refresh_cluster_host_info(host_id, database_mock, 'db_name', False, True)
        res = self.host_res_service.refresh_cluster_host_info(host_id, database_mock, None, False, True)
        self.assertIsNotNone(res)

    @patch("app.base.db_base.database.session")
    def test_refresh_cluster_host_info_if_host_is_not_cluster_success(self, database_mock):
        """
        *用例场景：测试更新非集群主机信息成功
        *前置条件：主机不是集群且参数正确
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.discovery.res_discovery_plugin import DiscoveryManager
        from app.protection.object.models.projected_object import ProtectedObject
        from app.resource.models.resource_models import EnvironmentTable

        class Filter:
            def all(self):
                return [project_obj]

            def delete(self):
                pass

            def one(self):
                return host

        host_id = 'host_id_01'
        project_obj = ProtectedObject(
            uuid='obj_uuid'
        )
        host = EnvironmentTable(**HOST)
        host.is_cluster = False
        database_mock.query().filter.return_value = Filter()
        database_mock.__enter__().merge.return_value = None
        with patch.object(DiscoveryManager, "__init__", lambda arg1, arg2: None):
            self.host_res_service.refresh_cluster_host_info(host_id, database_mock, 'db_name', False, True)
            res = self.host_res_service.refresh_cluster_host_info(host_id, database_mock, None, True, True)
            self.assertIsNotNone(res)

    @mock.patch("app.resource.service.host.host_res_service.DiscoveryManager.__init__", Mock(return_value=None))
    def test_should_return_dict_if_when_add_host_old_host_is_UBackup(self):
        from app.resource.discovery.res_discovery_plugin import DiscoveryManager
        self.host_res_service.get_environment_type_by_proxy_type = Mock(return_value='DBBackupAgent')
        env = {
            "host_id": '123',
            'name': 'host',
            'proxy_type': 'Database',
            'user_name': "",
            'password': "",
            'endpoint': '10.10.10.10',
            'port': 10001,
            'extend_context': {'host': {},
                       'extend_info': {
                           'install_path': '/opt'
                       }},
            'rescan_interval_in_sec': 180
        }
        self.host_res_service.host_to_env_param = Mock(return_value=ScanEnvSchema(**env))
        self.host_res_service.delete_extend_info = Mock
        ip = "10.10.10.10"
        env_uuid = str(uuid.uuid4())
        old_host = {
            'host_id': env_uuid,
            'name': '208d062b-02d4-4225-9048-3be19ee40340',
            'endpoint': ip,
            'port': '5985',
            'sub_type': 'UBackupAgent',
            'link_status': '0'
        }
        self.host_res_service.get_host_by_id = Mock(return_value=old_host)
        host = HostCreateInfo(
            name='localhost',
            ip=ip,
            host_id=env_uuid,
            proxy_type=0,
            link_status='1',
            register_type='1'
        )
        DiscoveryManager.modify_env = MagicMock(return_value=None)
        with mock.patch(
                "app.common.events.producer.produce") \
                as _mock_produce:
            self.host_res_service.producer = _mock_produce
            self.host_res_service.add_host(host)
            _mock_produce.produce.assert_called_once()
            host = HostCreateInfo(
                name='localhost',
                ip=ip,
                host_id=env_uuid,
                proxy_type=0,
                link_status='0'
            )
            DiscoveryManager.modify_env = MagicMock(return_value=None)
            self.host_res_service.add_host(host)
            self.assertTrue(True)

if __name__ == '__main__':
    unittest.main(verbosity=2)
