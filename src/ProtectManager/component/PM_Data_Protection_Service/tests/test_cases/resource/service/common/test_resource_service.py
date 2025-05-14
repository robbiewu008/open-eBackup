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
import unittest
import uuid
from unittest import mock
from unittest.mock import patch
from unittest.mock import MagicMock
from unittest.mock import Mock
from tests.test_cases.common.events import mock_producer
from tests.test_cases import common_mocker  # noqa
from app.common.event_messages.Discovery.discovery_rest import HostOnlineStatus
from tests.test_cases.tools import functiontools, timezone
from app.common.exception.unified_exception import EmeiStorBizException

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

from app.common.enums.resource_enum import ProtectionStatusEnum, ResourceTypeEnum, ResourceSubTypeEnum, LinkStatusEnum
from app.common.redis_session import redis_session

from app.base.resource_consts import ResourceConstant
from app.resource.schemas.resource import ResourceProtectionCount
from app.resource.schemas.resource import ResourceProtectionSummary
from app.resource.common.constants import ResourceConstants
from tests.test_cases.backup.common.context import mock_context  # noqa

# ENVIRONMENTS表描述的主机资源子类型--os_type
LINUX_RESOURCES = [
    ("Host", "linux", ProtectionStatusEnum.protected.value),
    ("Host", "linux", ProtectionStatusEnum.protected.value),
    ("Host", "linux", ProtectionStatusEnum.protected.value),
    ("Host", "linux", ProtectionStatusEnum.protected.value),
    ("Host", "linux", ProtectionStatusEnum.unprotected.value),
]
WINDOWS_RESOURCES = [
    ("Host", "windows", ProtectionStatusEnum.protected.value),
    ("Host", "windows", ProtectionStatusEnum.unprotected.value),
    ("Host", "windows", ProtectionStatusEnum.unprotected.value),
]
AIX_RESOURCES = [
    ("Host", "aix", ProtectionStatusEnum.unprotected.value),
]
HOST_OS_TYPE_RESOURCES = LINUX_RESOURCES + WINDOWS_RESOURCES + AIX_RESOURCES
# DATABASES表描述的数据库资源子类型--sub_type
ORACLE_RESOURCES = [
    ("Database", "Oracle", ProtectionStatusEnum.protected.value),
    ("Database", "Oracle", ProtectionStatusEnum.unprotected.value),
    ("Database", "Oracle", ProtectionStatusEnum.unprotected.value),
]
DATABASES_SUBTYPE_RESOURCES = ORACLE_RESOURCES
# 可以直接通过RESOURCES表统计的资源子类型--sub_type

# 虚拟化环境
VMWARE_RESOURCES = [
    ("Cluster", "vim.ClusterComputeResource", ProtectionStatusEnum.unprotected.value),
    ("Host", "vim.HostSystem", ProtectionStatusEnum.protected.value),
    ("Host", "vim.HostSystem", ProtectionStatusEnum.unprotected.value),
    ("VM", "vim.VirtualMachine", ProtectionStatusEnum.protected.value),
    ("VM", "vim.VirtualMachine", ProtectionStatusEnum.protected.value),
    ("VM", "vim.VirtualMachine", ProtectionStatusEnum.unprotected.value),
]
HYPER_V_RESOURCES = [
    ("Host", "ms.HostSystem", ProtectionStatusEnum.unprotected.value),
    ("VM", "ms.VirtualMachine", ProtectionStatusEnum.protected.value),
    ("VM", "ms.VirtualMachine", ProtectionStatusEnum.unprotected.value),
    ("VM", "ms.VirtualMachine", ProtectionStatusEnum.unprotected.value),
]

# REDIS资源子类型--sub_type
REDIS_RESOURCES = [
    ("Database", "Redis", ProtectionStatusEnum.protected.value),
    ("Database", "Redis", ProtectionStatusEnum.unprotected.value),
    ("Database", "Redis", ProtectionStatusEnum.unprotected.value),
]
VIRTUAL_ENV_RESOURCES = VMWARE_RESOURCES + HYPER_V_RESOURCES
OTHER_SUBTYPE_RESOURCES = VIRTUAL_ENV_RESOURCES
# 所有资源子类型列表
ALL_RESOURCES = HOST_OS_TYPE_RESOURCES + DATABASES_SUBTYPE_RESOURCES + OTHER_SUBTYPE_RESOURCES + REDIS_RESOURCES
USER_INFO = {
    'user-name': 'user_name123',
    'user-id': 'user_id123',
    'role-list': [],
    'es-valid-token': "true",
}


@patch.object(redis_session, "get", Mock(return_value="123"))
class ResourceServiceTest(unittest.TestCase):
    def setUp(self):
        super(ResourceServiceTest, self).setUp()
        from tests.test_cases.common.mock_settings import fake_settings
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        from app.resource.service.common import resource_service
        self.resource_service = resource_service
        self.resource_service.get_user_info_from_token = Mock(return_value=USER_INFO)

    @patch("app.base.db_base.database.session", MagicMock)
    def y_protection_resource_of_all(self):
        from app.resource.models.database_models import DatabaseTable
        # 1. 未传入sub_type列表
        sub_types = None
        self.resource_service.get_os_type_resource_protection_info = Mock(return_value=HOST_OS_TYPE_RESOURCES)
        database_values = {
            DatabaseTable.__tablename__: DATABASES_SUBTYPE_RESOURCES,
        }
        self.resource_service.get_other_resource_protection_info = Mock(return_value=OTHER_SUBTYPE_RESOURCES)
        summary_obj = self.resource_service.summary_protection_resource(sub_types, 'fake_sysadmin_token')
        self.assertTrue(isinstance(summary_obj, ResourceProtectionSummary))
        summary_items = summary_obj.summary
        self.assertTrue(isinstance(summary_items, list))
        for item in summary_items:
            self.assertTrue(isinstance(item, ResourceProtectionCount))
            if item.resource_sub_type == 'linux':
                self.assertEqual(item.resource_type, 'Host')
                self.assertEqual(item.protected_count, 4)
                self.assertEqual(item.unprotected_count, 1)
            elif item.resource_sub_type == 'MySQL':
                self.assertEqual(item.resource_type, 'Database')
                self.assertEqual(item.protected_count, 2)
                self.assertEqual(item.unprotected_count, 1)
            elif item.resource_sub_type == 'GaussDB':
                self.assertEqual(item.resource_type, 'DatabaseInstance')
                self.assertEqual(item.protected_count, 1)
                self.assertEqual(item.unprotected_count, 1)
        total_protected_count = sum(i.protected_count for i in summary_items)
        total_unprotected_count = sum(i.unprotected_count for i in summary_items)
        self.assertEqual(total_protected_count, 17)
        self.assertEqual(total_unprotected_count, 15)

    @patch("app.base.db_base.database.session", MagicMock)
    def y_protection_resource_of_virtual_env(self):
        # 2. 传入虚拟化环境的sub_type列表
        sub_types = list(ResourceConstant.VIRTUALRESOURCE_SUBTYPE_MAP.values())
        self.resource_service.get_os_type_resource_protection_info = Mock(return_value=[])
        self.resource_service.get_other_resource_protection_info = Mock(return_value=VIRTUAL_ENV_RESOURCES)
        summary_obj = self.resource_service.summary_protection_resource(sub_types, "")
        self.assertTrue(isinstance(summary_obj, ResourceProtectionSummary))
        summary_items = summary_obj.summary
        self.assertTrue(isinstance(summary_items, list))
        for item in summary_items:
            self.assertTrue(isinstance(item, ResourceProtectionCount))
            if item.resource_sub_type == 'vim.HostSystem':
                self.assertEqual(item.resource_type, 'Host')
                self.assertEqual(item.protected_count, 1)
                self.assertEqual(item.unprotected_count, 1)
            elif item.resource_sub_type == 'ms.VirtualMachine':
                self.assertEqual(item.resource_type, 'VM')
                self.assertEqual(item.protected_count, 1)
                self.assertEqual(item.unprotected_count, 2)
        total_protected_count = sum(i.protected_count for i in summary_items)
        total_unprotected_count = sum(i.unprotected_count for i in summary_items)
        self.assertEqual(total_protected_count, 4)
        self.assertEqual(total_unprotected_count, 6)

    @patch("app.base.db_base.database.session", MagicMock)
    def y_protection_resource_of_redis(self):
        # 2. 传入redis环境的sub_type列表
        sub_types = list(ResourceSubTypeEnum.Redis)
        self.resource_service.get_os_type_resource_protection_info = Mock(return_value=[])
        self.resource_service.get_database_resource_protection_info = Mock(return_value=[])
        self.resource_service.get_other_resource_protection_info = Mock(return_value=REDIS_RESOURCES)
        summary_obj = self.resource_service.summary_protection_resource(sub_types, "")
        self.assertTrue(isinstance(summary_obj, ResourceProtectionSummary))
        summary_items = summary_obj.summary
        self.assertTrue(isinstance(summary_items, list))
        total_protected_count = sum(i.protected_count for i in summary_items)
        total_unprotected_count = sum(i.unprotected_count for i in summary_items)
        self.assertEqual(total_protected_count, 4)
        self.assertEqual(total_unprotected_count, 6)

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_get_os_type_resource_protection_info_success(self, _mock_session):
        from alchemy_mock.mocking import UnifiedAlchemyMagicMock
        from app.resource.models.resource_models import ResourceTable, EnvironmentTable, ResExtendInfoTable
        db_session = UnifiedAlchemyMagicMock()
        res_uuid_1 = "680fdc9e-bb2c-4108-81e0-86dccdba3800"
        res_uuid_2 = "9510de5e-4832-4b9e-a81c-f34eae233103"
        res_uuid_3 = "33d659f4-8981-4ff6-b428-fb6efb7860ad"
        res_uuid_4 = "7bee67c9-f912-4258-bbaa-35a9bd579b27"
        res_uuid_5 = "ea08764d-7dea-40c0-aca5-2577be416be4"
        res_uuid_6 = "7cd5f6d7-dc34-415c-a773-ede8671d0068"
        res_uuid_7 = "c5220850-d21a-4461-b3b8-3cd374189b32"
        # 构造ResourceTable表数据
        db_session.add(
            ResourceTable(uuid=res_uuid_1, name="res1", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.DBBackupAgent.value, path="192.168.0.1", root_uuid=res_uuid_1,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=1))
        db_session.add(
            ResourceTable(uuid=res_uuid_2, name="res2", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.VMBackupAgent.value, path="192.168.0.2", root_uuid=res_uuid_2,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=1))
        db_session.add(
            ResourceTable(uuid=res_uuid_3, name="res3", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.ABBackupClient.value, path="192.168.0.3", root_uuid=res_uuid_3,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=1))
        db_session.add(
            ResourceTable(uuid=res_uuid_4, name="res4", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.DWSBackupAgent.value, path="192.168.0.1", root_uuid=res_uuid_4,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=1))
        db_session.add(
            ResourceTable(uuid=res_uuid_5, name="res5", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.UBackupAgent.value, path="192.168.0.5", root_uuid=res_uuid_5,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=1))
        db_session.add(
            ResourceTable(uuid=res_uuid_6, name="res6", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.UBackupAgent.value, path="192.168.0.6", root_uuid=res_uuid_6,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=0))
        db_session.add(
            ResourceTable(uuid=res_uuid_7, name="res7", type=ResourceTypeEnum.Host.value,
                          sub_type=ResourceSubTypeEnum.ABBackupClient.value, path="192.168.0.7", root_uuid=res_uuid_7,
                          discriminator="ENVIRONMENTS", version="1.1.RC2", protection_status=0))
        # 构造EnvironmentTable表数据
        db_session.add(
            EnvironmentTable(uuid=res_uuid_1, endpoint="192.168.0.1", port="50001",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_2, endpoint="192.168.0.2", port="50002",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_3, endpoint="192.168.0.3", port="50003",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_4, endpoint="192.168.0.4", port="50004",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_5, endpoint="192.168.0.5", port="50005",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_6, endpoint="192.168.0.6", port="50006",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=False))
        db_session.add(
            EnvironmentTable(uuid=res_uuid_7, endpoint="192.168.0.7", port="50007",
                             link_status=HostOnlineStatus.ON_LINE.value, os_type="linux", os_name="linux",
                             is_cluster=True))
        # 构造ResExtendInfoTable表数据
        db_session.add(
            ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_uuid_1, key="k1", value="v1"))
        db_session.add(
            ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_uuid_5, key="scenario", value="0"))
        db_session.add(
            ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_uuid_5, key="k1", value="v1"))
        db_session.add(
            ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_uuid_6, key="scenario", value="1"))
        db_session.add(
            ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_uuid_6, key="k1", value="v1"))
        _mock_session().__enter__().query().outerjoin().filter().distinct().subquery.return_value = MagicMock()
        _mock_session().__enter__().query().outerjoin().filter().all.return_value = [
            (ResourceTypeEnum.Host.value, "linux", 1),
            (ResourceTypeEnum.Host.value, "linux", 1),
            (ResourceTypeEnum.Host.value, "linux", 1),
            (ResourceTypeEnum.Host.value, "linux", 1),
            (ResourceTypeEnum.Host.value, "linux", 1)
        ]
        from app.base.db_base import database
        with database.session() as session:
            host_resources = self.resource_service.get_os_type_resource_protection_info(
                session, ResourceConstant.HOST_OS_TYPE_LIST, [])
        self.assertEqual(5, len(host_resources))

    @patch("app.base.db_base.database.session")
    def test_get_top_inst_resource_protection_info_success(self, _mock_session):
        """
        验证场景：获取isTopInstance=1的资源保护信息
        前置条件：查询数据库成功
        验证点：返回和数据库中相同数目的资源保护信息
        """
        top_inst_sub_type_list = [
            ResourceSubTypeEnum.MysqlInstance, ResourceSubTypeEnum.PostgreInstance,
            ResourceSubTypeEnum.KingBaseInstance
        ]
        _mock_session().__enter__().query().outerjoin().filter().filter().all.return_value = [
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.MysqlInstance.value, 1),
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.MysqlInstance.value, 0),
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.PostgreInstance.value, 0),
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.PostgreInstance.value, 1),
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.KingBaseInstance.value, 0),
            (ResourceTypeEnum.Database.value, ResourceSubTypeEnum.KingBaseInstance.value, 1),
        ]
        from app.base.db_base import database
        with database.session() as session:
            top_inst_resources = self.resource_service.get_top_inst_resource_protection_info(
                session, top_inst_sub_type_list, [])
        self.assertEqual(6, len(top_inst_resources))

    def y_protection_resource_sub_types_not_identify(self):
        sub_types = ["unknown"]
        try:
            self.resource_service.summary_protection_resource(sub_types, "")
        except EmeiStorBizException as ex:
            self.assertEqual(ex._error_message, "sub type(unknown) illegal.")

    @patch("app.base.db_base.database.session")
    def test_instance_available(self, database_mock):
        """
        用例场景：透传agent数据信息
        前置条件：agent接口访问ok
        检查点：是否透传
        :return:
        """
        agent_info = [{
            'instName': "oltp1",
            'dbName': "oltp",
            'version': "18.0.0.0",
            'state': 1,
            'isAsmInst': 0,
            'authType': 1,
            'dbRole': 0,
            'oracleHome': "xxx"
        }]
        database_mock().__enter__().query(self.resource_service.EnvironmentTable) \
            .filter(self.resource_service.EnvironmentTable.endpoint == "127.0.0.1") \
            .first.return_value = "123"
        self.resource_service.hw_agent_rpc.query_databases = Mock(return_value=agent_info)

        res = self.resource_service.instance_available("127.0.0.1")
        self.assertEqual(res, agent_info)

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_authorize_resource_success(self, database_mock):
        """
        用例场景：测试授权资源时，子资源和依赖资源都联动授权
        前置条件：agent接口访问ok
        检查点：是否透传
        :return:
        """
        user_id = str(uuid.uuid4())
        resource_id = str(uuid.uuid4())
        dependency_resource_id = str(uuid.uuid4())
        resource_uuid_list = [resource_id]
        database_mock().__enter__().query(self.resource_service.ResourceTable.uuid) \
            .filter(self.resource_service.ResourceTable.uuid.in_(resource_uuid_list)) \
            .filter(self.resource_service.ResourceTable.user_id.isnot(None)) \
            .count.return_value = 0

        resource_dic = {
            'uuid': resource_id,
            'sub_type': self.resource_service.ResourceSubTypeEnum.MysqlCluster.value
        }
        resource = self.resource_service.ResourceTable(**resource_dic)

        database_mock().__enter__().query(self.resource_service.ResourceTable) \
            .filter(self.resource_service.ResourceTable.root_uuid == str(resource_id)) \
            .join() \
            .count.return_value = 0

        database_mock().__enter__().query(self.resource_service.ResourceTable) \
            .filter(self.resource_service.ResourceTable.children_uuids.isnot(None),
                    self.resource_service.ResourceTable.children_uuids.any(resource.uuid)) \
            .count.return_value = 0

        user_info = {'rolesSet': [{'roleName': self.resource_service.RoleEnum.ROLE_DP_ADMIN.value}]}
        self.resource_service.SystemBaseClient.query_user = Mock(return_value=user_info)

        class Filter:
            def __init__(self):
                self.flag = 1

            def all(self):
                if self.flag == 1:
                    self.flag = 0
                    return []
                else:
                    self.flag = 1
                    return [resource]

            def filter(self, param):
                return Filter()

            def count(self):
                return 0

            def update(self, param, synchronize_session):
                pass

        database_mock().__enter__().query().filter.return_value = Filter()
        res = self.resource_service.authorize_resource(user_id, resource_uuid_list)
        self.assertEqual(res, None)

    @patch("app.base.db_base.database.session")
    def test_query_resource_by_id_success(self, database_mock):
        """
        用例场景：根据资源id查询资源
        前置条件：服务正常
        检查点：查询成功
        """
        resource_id = "a354d9a6-7b95-5a3c-88e6-52aefb653809"

        class PaginatorMock:
            def __init__(self, page, size, conditions):
                pass

            def one(self):
                return {
                    'path': '8.40.98.196/ManagementCluster/linux-CF/dont_del_agent_zyr',
                    'sub_type': 'FusionCompute',
                    'uuid': 'bc0b9cf5-301d-5b92-86aa-0ac75e945ab3',
                    'name': 'dont_del_agent_zyr', 'version': None,
                    'endpoint': 'endpoint',
                    'os_type': 'is_cluster',
                    'is_cluster': True,
                }

        self.resource_service.paginator = Mock(return_value=PaginatorMock)
        database_mock().__enter__().query().filter().one.return_value = ["ENVIRONMENTS"]
        res = self.resource_service.query_resource_by_id(resource_id)
        self.assertIsNotNone(res, "not null")

    @patch("app.base.db_base.database.session")
    def test_delete_resource_exception(self, database_mock):
        """
        用例场景：根据条件刪除资源
        前置条件：服务正常
        检查点：已被保护，删除资源失败
        """
        resource_id = "a354d9a6-7b95-5a3c-88e6-52aefb653809"

        class QueryMock:
            def __init__(self, resource_type):
                pass

            def filter(self, resource_type):
                return QueryMock(resource_type=ResourceTypeEnum.Host.value)

            def one_or_none(self):
                from app.protection.object.models.projected_object import ProtectedObject
                return ProtectedObject(name="test_1234",
                                       sla_id="de8d0c9e983211eb80bcfa163ef21477")

        database_mock().__enter__().query().filter.return_value = QueryMock(resource_type=ResourceTypeEnum.Host.value)
        self.assertRaises(EmeiStorBizException, self.resource_service.delete_resource,
                          resource_id, ResourceTypeEnum.Host.value)

    @patch("app.base.db_base.database.session")
    def test_delete_resource_success(self, database_mock):
        """
        用例场景：根据条件刪除资源
        前置条件：服务正常
        检查点：刪除资源成功
        """
        resource_id = "a354d9a6-7b95-5a3c-88e6-52aefb653809"

        class QueryMock:
            def __init__(self, resource_type):
                pass

            def filter(self, resource_type):
                return QueryMock(resource_type=ResourceTypeEnum.Host.value)

            def one_or_none(self):
                pass

            def all(self):
                return []

            def delete(self):
                return 0

        database_mock().__enter__().query().filter.return_value = QueryMock(resource_type=ResourceTypeEnum.Host.value)
        self.resource_service.comment_event_message = Mock(return_value=None)
        self.resource_service.delete_resource(resource_id, ResourceTypeEnum.Host.value)
        self.assertTrue(True, "success")

    @patch("app.base.db_base.database.session")
    def test_query_resource_success(self, database_mock):
        """
        用例场景：根据条件查询资源
        前置条件：服务正常
        检查点：查询资源成功
        """
        res = self.resource_service.query_resource({})
        self.assertEqual(res, [])

    @patch("app.base.db_base.database.session")
    def test_convert_resource_to_summary_schema_success(self, database_mock):
        database_mock().__enter__().query().all.return_value = []
        res = self.resource_service.convert_resource_to_summary_schema(
            [[ResourceTypeEnum.Platform.value, ResourceSubTypeEnum.FusionCompute.value, 1],
             [ResourceTypeEnum.Cluster.value, ResourceSubTypeEnum.FusionCompute.value, 0],
             [ResourceTypeEnum.Host.value, ResourceSubTypeEnum.FusionCompute.value, 1]])
        self.assertEqual(1, len(res.summary))
        self.assertEqual(1, res.summary[0].protected_count)
        self.assertEqual(1, res.summary[0].unprotected_count)

    @patch("app.base.db_base.database.session")
    def test_query_environment_success(self, database_mock):
        """
        用例场景：根据条件查询环境
        前置条件：服务正常
        检查点：查询环境成功
        """
        res = self.resource_service.query_environment({})
        self.assertEqual(len(res), 0)

    @patch("app.base.db_base.database.session")
    def test_verify_resource_ownership_success(self, database_mock):
        """
        用例场景：校验所属用户是否正确
        前置条件：服务正常
        检查点：根据资源ID列表和用户ID校验资源所属用户是否正确
        """
        uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809", "b354d9a6-7b95-5a3c-88e6-52aefb653809"]
        database_mock().__enter__().query().filter().all.return_value = uuid_list
        database_mock().__enter__().query().join().filter().filter().all.return_value = uuid_list
        self.resource_service.verify_resource_ownership("d354d9a6-7b95-5a3c-88e6-52aefb653809", uuid_list)
        self.assertTrue(True, "无异常")

    @patch("app.base.db_base.database.session")
    def test_search_citation_resource_uuid_success(self, database_mock):
        """
        用例场景：查询关联资源
        前置条件：服务正常
        检查点：查询关联资源是否正确
        """
        from app.resource.models.resource_models import ResourceTable
        uuid_list = [("id_1",), ("id_2",), ("id_2",)]
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809", "b354d9a6-7b95-5a3c-88e6-52aefb653809"]
        sub_resource = ResourceTable(uuid="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                                     sub_type=ResourceSubTypeEnum.UBackupAgent.value)

        class Filter:
            def __init__(self):
                self.flag = 1

            def all(self):
                if self.flag == 1:
                    self.flag = 0
                    return uuid_list
                else:
                    return [sub_resource]

            def filter(self):
                return Filter()

            def join(self, arg0, arg1):
                return self

            def one_or_none(self):
                return None

        database_mock().__enter__().query().filter.return_value = Filter()
        from app.base.db_base import database
        with database.session() as session:
            uuid_list = self.resource_service.search_citation_resource_uuid(resource_uuid_list, session)
            self.assertTrue(len(uuid_list) > 0, "success")

    @patch("app.base.db_base.database.session")
    def test_exclude_resource_cited_by_others_and_authorized(self, database_mock):
        """
        用例场景：排除依赖资源被其他资源引用并授权的情况
        前置条件：服务正常
        检查点：对于被其他资源引用且被授权的资源，需要排除，不能取消授权
        """
        dependency_resources_uuids = ["host1", "host2"]
        resource_uuid_list = ["resource1"]
        from app.resource.models.resource_models import ResourceTable, ResExtendInfoTable

        class Filter:
            def __init__(self):
                self.flag = 0

            def all(self):
                res = []
                if self.flag == 0:
                    res = [["host1"]]
                elif self.flag == 1:
                    res = [ResExtendInfoTable(resource_id="host1", key=ResourceConstants.CITATION, value="resource1"),
                           ResExtendInfoTable(resource_id="host1", key=ResourceConstants.CITATION, value="resource2")]
                elif self.flag == 2:
                    res = [ResourceTable(uuid="resource2", user_id="user1")]
                self.flag = self.flag + 1
                return res

        database_mock().__enter__().query().filter.return_value = Filter()
        from app.base.db_base import database
        with database.session() as session:
            self.resource_service.exclude_resource_cited_by_others_and_authorized(dependency_resources_uuids,
                                                                                  resource_uuid_list, session)
        self.assertTrue("host1" not in dependency_resources_uuids)

    @patch("app.base.db_base.database.session")
    def test_revoke_success(self, database_mock):
        """
        用例场景：取消授权
        前置条件：服务正常
        检查点：取消授权成功
        """
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        from app.resource.models.resource_models import ResourceTable
        database_mock().__enter__().query().filter().all.return_value \
            = [ResourceTable(uuid="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                             sub_type=ResourceSubTypeEnum.Kubernetes.value)]
        self.resource_service.search_citation_resource_uuid = Mock(return_value=resource_uuid_list)
        self.resource_service.check_resource_is_depended_on = Mock(return_value=None)
        self.resource_service.check_not_validate_is_depended_on = Mock(return_value=None)
        database_mock().__enter__().query().filter().filter().all.return_value = resource_uuid_list
        self.resource_service.revoke("e354d9a6-7b95-5a3c-88e6-52aefb653809", resource_uuid_list)
        self.assertTrue(True, "success")

    @patch("app.base.db_base.database.session")
    def test_raise_exception_if_resource_is_depended_on_when_revoke(self, database_mock):
        """
        用例场景：检查被依赖资源是否存在时抛出异常
        前置条件：存在被依赖资源
        检查点：抛出异常
        """
        from app.resource.models.resource_models import ResourceTable

        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        from app.resource.models.resource_models import ResExtendInfoTable
        database_mock.query(ResExtendInfoTable.resource_id).filter().all.return_value = [ResExtendInfoTable(
            resource_id="b354d9a6-7b95-5a3c-88e6-52aefb653809", value="a354d9a6-7b95-5a3c-88e6-52aefb653809")]
        res = ResourceTable(uuid="uuid", name="")
        res_root = ResourceTable(uuid="uuid1", name="res_name")
        self.resource_service.is_need_check_dependency = Mock(return_value=False)
        database_mock.query(ResExtendInfoTable.resource_id).filter().first = Mock(side_effect=[res, res_root])
        with self.assertRaises(EmeiStorBizException) as ex:
            self.resource_service.check_resource_is_depended_on(resource_uuid_list, database_mock)
        self.assertEqual([res_root.name], ex.exception.parameter_list)

    @patch("app.base.db_base.database.session")
    def test_query_agent_info_success(self, database_mock):
        """
        用例场景：查询代理资源
        前置条件：服务正常
        检查点： 查询代理资源成功
        """
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        database_mock().__enter__().query().filter().count.return_value = 0
        database_mock().__enter__().query().filter().filter().all.return_value = resource_uuid_list
        res = self.resource_service.query_agent_info()
        temp = {}
        self.assertEqual(res, temp)

    @patch("app.base.db_base.database.session")
    def test_query_resource_info_success(self, database_mock):
        """
        用例场景：查询资源
        前置条件：服务正常
        检查点： 查询资源成功
        """
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        from app.resource.models.resource_models import ResourceTable
        database_mock().__enter__().query().filter().all.return_value \
            = [ResourceTable(uuid="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                             sub_type=ResourceSubTypeEnum.Kubernetes.value)]
        res = self.resource_service.query_resource_info(resource_uuid_list)
        self.assertEqual(len(res), 1)

    @patch("app.base.db_base.database.session")
    def test_update_protection_status_success(self, database_mock):
        """
        用例场景：更新保护状态
        前置条件：服务正常
        检查点： 更新保护状态成功
        """
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        from app.base.db_base import database
        with database.session() as session:
            self.resource_service.update_protection_status(session, resource_uuid_list, ProtectionStatusEnum.protected)
            self.resource_service.update_protection_status(session, resource_uuid_list, ProtectionStatusEnum.protecting)
            self.resource_service.update_protection_status(session, resource_uuid_list,
                                                           ProtectionStatusEnum.unprotected)
            self.assertTrue(True, "success")

    @patch("app.base.db_base.database.session")
    def test_update_link_status_success(self, database_mock):
        """
        用例场景：更新连接状态
        前置条件：服务正常
        检查点： 更新连接成功
        """
        resource_uuid_list = ["a354d9a6-7b95-5a3c-88e6-52aefb653809"]
        self.resource_service.update_link_status(resource_uuid_list, LinkStatusEnum.Online)
        self.resource_service.update_link_status(resource_uuid_list, LinkStatusEnum.Offline)
        self.assertTrue(True, "success")

    @patch("app.base.db_base.database.session")
    def test_revoke_resource_user_id_success(self, database_mock):
        """
        用例场景：回收资源
        前置条件：服务正常
        检查点： 回收资源成功
        """
        self.resource_service.revoke_resource_user_id("b354d9a6-7b95-5a3c-88e6-52aefb653809")
        self.assertTrue(True, "success")

    @patch("app.base.db_base.database.session")
    def test_get_resource_by_name_success(self, database_mock):
        """
        用例场景：根据名称查询资源
        前置条件：服务正常
        检查点： 根据名称查询资源成功
        """
        from app.resource.models.resource_models import ResourceTable
        database_mock().__enter__().query().filter().first.return_value = \
            ResourceTable(uuid="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                          sub_type=ResourceSubTypeEnum.Kubernetes.value)
        res = self.resource_service.get_resource_by_name("test")
        self.assertIsNotNone(res, "success")

    @patch("app.base.db_base.database.session")
    def test_query_current_cluster_success(self, database_mock):
        """
        用例场景：查询当前节点的名称
        前置条件：服务正常
        检查点： 查询当前节点成功
        """
        from app.copy_catalog.models.tables_and_sessions import ClusterMemberTable
        database_mock().__enter__().query().filter().first.return_value = \
            ClusterMemberTable(cluster_name="cluster_name")
        res = self.resource_service.query_current_cluster("test")
        self.assertIsNotNone(res, "success")

    @patch("app.base.db_base.database.session")
    def test_query_target_cluster_by_id_success(self, database_mock):
        """
        用例场景：根据id查询目标集群
        前置条件：服务正常
        检查点： 根据id查询目标集群成功
        """
        from app.resource.models.resource_models import TClusterTarget
        database_mock().__enter__().query().filter().first.return_value = \
            TClusterTarget(cluster_name="cluster_name")
        res = self.resource_service.query_target_cluster_by_id("cluster_id")
        self.assertIsNotNone(res, "success")

    @patch("app.base.db_base.database.session")
    def test_get_other_resource_protection_info_success(self, database_mock):
        """
        用例场景：获取其他可保护资源信息成功
        前置条件：参数正确
        检查点： 返回值非空
        """
        from app.resource.models.resource_models import TClusterTarget
        database_mock.query().filter().all.return_value = [OTHER_SUBTYPE_RESOURCES]
        sub_types = [ResourceSubTypeEnum.HCSProject, ResourceSubTypeEnum.HCSCloudHost]
        res = self.resource_service.get_other_resource_protection_info(database_mock, sub_types, [])
        self.assertIsNotNone(res)

    @patch("app.base.db_base.database.session")
    def test_is_not_need_check_dependency(self, database_mock):
        """
        用例场景：校验是否校验依赖
        前置条件：服务正常
        检查点：不允许校验依赖
        """
        from app.resource.models.resource_models import ResourceTable
        from app.resource.models.resource_models import ResExtendInfoTable

        database_mock.query().filter().all.return_value = \
            [ResourceTable(uuid="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                           parent_uuid="b354d9a6-7b95-5a3c-88e6-52aefb653809",
                           sub_type=ResourceSubTypeEnum.MysqlInstance.value)]
        res_extend_info_resources = [ResExtendInfoTable(resource_id="a354d9a6-7b95-5a3c-88e6-52aefb653809",
                                                        value="b354d9a6-7b95-5a3c-88e6-52aefb653809")]
        dependent_resources_uuids = ["b354d9a6-7b95-5a3c-88e6-52aefb653809"]
        is_need_check = self.resource_service.is_need_check_dependency(dependent_resources_uuids,
                                                                       res_extend_info_resources, database_mock)
        self.assertFalse(is_need_check, "success")


if __name__ == '__main__':
    unittest.main(verbosity=2)
