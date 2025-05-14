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
from unittest.mock import Mock, patch
import json
from enum import Enum
from sqlalchemy import and_
from unittest import mock
from tests.test_cases import common_mocker # noqa
from tests.test_cases.common.events import mock_producer  # noqa
from tests.test_cases.common.mock_settings import fake_settings  # noqa
from tests.test_cases.tools import timezone
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
from app.common.enums.resource_enum import ResourceTypeEnum, ResourceSubTypeEnum
from app.resource.models.resource_models import ResourceTable
from app.resource.service.host import host_service

class ClusterNodeTable:
    def __init__(self):
        self.host_uuid = "1"
        self.cluster_info = '{"ClusterIP": "127.0.0.1"}'


class EnvironmentTable:
    host_uuid = "1"
    link_status = "1"
    sub_type = 'host'

class ResourceTable:
    host_uuid = "1"
    link_status = "1"
    sub_type = 'host'

class DatabaseTable:
    def __init__(self):
        self.inst_name = 'oltp'
        self.name = 'oracle'
        self.valid = False
        self.uuid = '123123'
        self.verify_status = True
        self.time_zone = "zone"

class ResExtendInfoTable:
    def __init__(self):
        self.uuid = '12315'
        self.resource_id = '456789'
        self.key = 'src_deduption'
        self.value = True

class TimeZone:
    def __init__(self):
        self.time_zone = "zone"


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
    "user_id": "123",
    "authorized_user": "ABC",
    'uuid': '123',
    'name': 'host',
    'endpoint': '0.0.0.0',
    "port": "8888",
}

CLUSTER = '{"ClusterIP": "0.0.0.0", "ClusterName": "rac-cluster", "ClusterType": 1,' \
          ' "Nodes": [{"NodeIP": "0.0.0.1", "NodeName": "rac01"}, ' \
          '{"NodeIP": "0.0.0.2", "NodeName": "rac02"},' \
          ' {"NodeIP": "0.0.0.3", "NodeName": "rac03"}]}'


class PARAMS(str, Enum):
    sub_type = 'host'


ENVIRONMENTTABLE_NONE = None


class HostServerTest(unittest.TestCase):
    def setUp(self):
        super(HostServerTest, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        self.data = DatabaseTable()
        self.host_service = host_service
        self.TimeZone = TimeZone

    @patch("app.base.db_base.database.session")
    def test_query_environment(self, database_mock):
        res = self.host_service.query_environment(ENV)
        self.assertEqual(res, [])

    @patch("app.base.db_base.database.session")
    def test_get_environment(self, database_mock):
        res = self.host_service.get_environment(self.data.uuid)
        self.assertIsNotNone(res)

    @patch("app.base.db_base.database.session")
    def test_get_environment_when_none(self, database_mock):
        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.uuid == self.data.uuid) \
            .first.return_value = None
        res = self.host_service.get_environment(ENV)
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_get_resource_when_none(self, database_mock):
        database_mock().__enter__().query(self.host_service.ResourceTable) \
            .filter(self.host_service.ResourceTable.uuid == self.data.uuid) \
            .first.return_value = None
        res = self.host_service.get_resource(ENV)
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_get_environment_by_ip_port(self, database_mock):
        res = self.host_service.get_environment_by_ip_port(ENV.get("uuid"), ENV.get("port"))
        self.assertIsNotNone(res)

    @patch("app.base.db_base.database.session")
    def test_get_environment_by_ip_port_when_none(self, database_mock):
        database_mock().__enter__().query(self.host_service.EnvironmentTable).first.return_value = None
        res = self.host_service.get_environment_by_ip_port("", "")
        self.assertIsNone(res)

    def test_automatic_authorization_by_agent_userid(self):
        user_info = {
            "sysAdmin": False,
            "authorized_user": "123",
            "user_id": '312',
            "userName": "123",
        }
        self.host_service.get_user_info_by_user_id = Mock(return_value=user_info)
        env = self.host_service.automatic_authorization_by_agent_userid(ENV)
        self.assertEqual(env.get("user_id"), ENV.get("user_id"))
        user_info['sysAdmin'] = True
        env = self.host_service.automatic_authorization_by_agent_userid(ENV)
        self.assertIsNone(env.get("user_id"))
        self.host_service.get_user_info_by_user_id = Mock(return_value={})
        env = self.host_service.automatic_authorization_by_agent_userid(ENV)
        self.assertIsNone(env.get("user_id"))

    @patch("app.base.db_base.database.session")
    def test_update_application(self, database_mock):
        cluster_info = {
            "name": "123",
            "uuid": "123",
            "applications": {
                "timezone": 'zone',
                "application": {
                    "oracleHome": "oracle",
                    "version": "18.0.0"
                }
            }
        }
        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.uuid == "123") \
            .all.return_value = [ClusterNodeTable()]
        id = self.host_service.update_application(ENV.get("uuid"), self.data, cluster_info=cluster_info)
        self.assertIsNone(id)

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_update_application_when_exist_application(self, database_mock):
        cluster_info = {
            "name": "123",
            "uuid": "123",
            "applications": {
                "timezone": 'zone',
                "application": {
                    "oracleHome": "oracle",
                    "version": "18.0.0"
                }
            }
        }
        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.uuid == "123") \
            .one_or_none.return_value = self.TimeZone

        filters = {ResourceTable.type == ResourceTypeEnum.Application.value,
                   ResourceTable.sub_type == ResourceSubTypeEnum.OracleApp.value,
                   ResourceTable.root_uuid == "123"}
        database_mock().__enter__().query(self.host_service.ResourceTable) \
            .filter(*filters)\
            .one_or_none.return_value = None
        res = self.host_service.update_application(ENV.get("uuid"), self.data, cluster_info=cluster_info)
        self.assertEqual(res, None)


    @patch("app.base.db_base.database.session")
    def test_update_cluster_offline_input_none(self, database_mock):
        database_mock().__enter__().query(self.host_service.ClusterNodeTable) \
            .filter(self.host_service.ClusterNodeTable.cluster_info == CLUSTER) \
            .all.return_value = [ClusterNodeTable()]

        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.uuid == ClusterNodeTable().host_uuid) \
            .one_or_none.return_value = ENVIRONMENTTABLE_NONE

        ret = host_service.update_cluster_offline(CLUSTER)
        self.assertEqual(ret, None)

    @patch("app.base.db_base.database.session")
    def test_update_cluster_offline_input_on_line(self, database_mock):
        database_mock().__enter__().query(self.host_service.ClusterNodeTable) \
            .filter(self.host_service.ClusterNodeTable.cluster_info == CLUSTER) \
            .all.return_value = [ClusterNodeTable()]

        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.uuid == ClusterNodeTable().host_uuid) \
            .one_or_none.return_value = EnvironmentTable()

        ret = host_service.update_cluster_offline(CLUSTER)
        self.assertEqual(ret, None)

    def test_create_new_cluster_node_table(self):
        result = {
            'host_uuid': '123',
            'cluster_name': 'cluster_name',
            'env_type': 'host',
            'type': 'cluster',
            'cluster_info': 'cluster_info_json_str'
        }
        res = self.host_service.create_new_cluster_node_table(ENV['uuid'], 'cluster_name', PARAMS, 'cluster',
                                                              'cluster_info_json_str')
        self.assertEqual(res, result)

    @patch("app.base.db_base.database.session")
    def test_update_database_none(self, database_mock):
        self.host_service.comment_event_message = Mock()
        asm_info = 'asm'
        database_mock().__enter__().query(self.host_service.DatabaseTable) \
            .filter(self.host_service.DatabaseTable.root_uuid == ENV['uuid']) \
            .all.return_value = [DatabaseTable()]
        AGENT_DATA = {}
        res = self.host_service.update_database(ENV, PARAMS, AGENT_DATA, asm_info)
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_update_database(self, database_mock):
        asm_info = 'asm'
        database_mock().__enter__().query(self.host_service.DatabaseTable) \
            .filter(self.host_service.DatabaseTable.root_uuid == ENV['uuid']) \
            .all.return_value = [DatabaseTable()]
        self.host_service.comment_event_message = Mock()
        self.host_service.choose_available_databases_instance = Mock(return_value=AGENT_DATA)
        self.host_service.create_dme_database = Mock()
        self.host_service.create_host_auth = Mock(return_value='asm')
        res = self.host_service.update_database(ENV, PARAMS, {'oracle': AGENT_DATA}, asm_info)
        self.assertIsNone(res)

    @patch("app.base.db_base.database.session")
    def test_check_exited_cluster_when_has_exited_cluster(self, database_mock):
        node_info = {'NodeIP': '0.0.0.3', 'NodeName': 'rac03'}

        database_mock().__enter__().query(self.host_service.ClusterNodeTable) \
            .filter(self.host_service.ClusterNodeTable.cluster_info.like(f"%{json.dumps(node_info)}%")) \
            .first.return_value = None

        database_mock().__enter__().query(self.host_service.EnvironmentTable) \
            .filter(self.host_service.EnvironmentTable.children_uuids.any('123')) \
            .one_or_none.return_value = None
        ret = self.host_service.check_exited_cluster(CLUSTER)
        self.assertIsNone(ret)

    @patch("app.base.db_base.database.session")
    def update_database_when_agent_is_none(self, database_mock):
        database_mock().__enter__().query(self.host_service.DatabaseTable).all().return_value = None
        ret = self.host_service.update_database_when_agent_is_none(ENV)
        self.assertIsNone(ret)
        pass

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_valid_get_relative_extend_info(self, database_mock):
        database_mock().__enter__().query(self.host_service.ResExtendInfoTable) \
            .filter(and_(self.host_service.ResExtendInfoTable.key == "src_deduption",
                         self.host_service.ResExtendInfoTable.resource_id == "456789"))\
            .first.return_value = ResExtendInfoTable()
        ret = self.host_service.get_relative_extend_info("456789")
        self.assertTrue(ret.get("value"))

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_invalid_get_relative_extend_info(self, database_mock):
        database_mock().__enter__().query(self.host_service.ResExtendInfoTable) \
            .filter(and_(self.host_service.ResExtendInfoTable.key == "src_deduption",
                         self.host_service.ResExtendInfoTable.resource_id == "456789")) \
            .first.return_value = None
        ret = self.host_service.get_relative_extend_info("456789")
        self.assertDictEqual(ret, {})

    """
    用例场景：检验资源规格是否超过规格
    检查点: 通过校验
    """
    @patch("app.base.db_base.database.session")
    def test_pass_check_resource_exceeds_limit(self, database_mock):
        resource_obj = dict(
            uuid="123456"
        )
        database_mock().__enter__().query().filter().first.return_value = resource_obj
        database_mock().__enter__().query().count = 1
        ret = self.host_service.check_resource_exceeds_limit("123456")
        self.assertIsNone(ret)

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_error_check_resource_exceeds_limit(self, database_mock):
        """
        用例场景：检验资源规格是否超过规格
        检查点: 未通过校验
        """
        database_mock().__enter__().query().filter().first.return_value = None
        database_mock().__enter__().query().count.return_value = 20002
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException):
            self.host_service.check_resource_exceeds_limit("123456")

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_check_and_limit_resources(self, database_mock):
        """
       用例场景：检验添加某数量后资源规格是否超过规格
       检查点: 未通过校验，抛出异常
       """
        database_mock().__enter__().query().filter().first.return_value = None
        database_mock().__enter__().query().count.return_value = 19999
        from app.common.exception.unified_exception import EmeiStorBizException
        resources = self.host_service.check_and_limit_resources(['res_id1','res_id2'])
        self.assertEqual(1, len(resources))

if __name__ == '__main__':
    unittest.main(verbosity=2)
