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
from typing import List
from unittest import mock
from unittest.mock import Mock, MagicMock

from pydantic import BaseModel

from app.common.deploy_type import DeployType
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.common.events import mock_producer  # noqa
from tests.test_cases.common.mock_settings import fake_settings  # noqa
from tests.test_cases.tools import functiontools, timezone

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.protection.object.common.db_config'] = Mock()
DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
from app.common.exception.unified_exception import EmeiStorBizException
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.protection.object.schemas.extends.params.fileset_protection_ext_param import FilesetProtectionExtParam
from app.protection.object.service.projected_object_service import check_action_and_small_file
from sqlalchemy.orm import Session
from app.protection.object.db import crud_projected_object
from app.protection.object.models.projected_object import ProtectedObject


class BaseExtParams(BaseModel):
    ext_parameters = {}


class ExtParameters(BaseExtParam):
    @staticmethod
    def support_values() -> List[object]:
        pass


class ProtectedObjectUpdate:
    def __init__(self):
        self.resource_id = "d8893969-a01e-4b8c-be8d-f09a56d74e33",
        self.sla_id = "34926913-b6a6-4c3b-812e-4fba893cbe5e",
        self.ext_parameters = BaseExtParams()


@mock.patch("sqlalchemy.orm.session.Session", MagicMock)
class TestProtectionService(unittest.TestCase):
    def setUp(self):
        super(TestProtectionService, self).setUp()
        self.crud_projected_object = crud_projected_object
        self.protected_object = ProtectedObject

    @mock.patch("sqlalchemy.orm.Query.count")
    def test_should_return_zero_if_page_query_when_data_is_null(self, mock_query_count):
        from app.protection.object.schemas.protected_object import ProtectedObjectQueryRequest

        mock_query_count.return_value = 0
        page_req = ProtectedObjectQueryRequest(sla_id='00001',
                                               name='',
                                               sla_compliance=[],
                                               sub_type=[], path='',
                                               page_no=0, page_size=10)
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        page_query_data = crud_protected_object.page_query(Session(), page_req)
        self.assertIsInstance(page_query_data, dict)
        self.assertEqual(page_query_data.get("total"), 0)

    @mock.patch("sqlalchemy.orm.Query.all")
    @mock.patch("sqlalchemy.orm.Query.count")
    def test_page_query_success(self, mock_query_count, mock_query_all):
        from app.protection.object.schemas.protected_object import ProtectedObjectQueryRequest
        mock_query_count.return_value = 2
        mock_query_all.return_value = []
        page_req1 = ProtectedObjectQueryRequest(sla_id='8556bb41-abe6-4821-870d-a0252f304dfcmock',
                                                name='test001',
                                                sla_compliance=[],
                                                sub_type=[],
                                                path='',
                                                page_no=0,
                                                page_size=10)
        crud_protected_object1 = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        page_query_data1 = crud_protected_object1.page_query(Session(), page_req1)
        self.assertIsInstance(page_query_data1, dict)
        self.assertEqual(page_query_data1.get("total"), 2)

        from app.common.enums.resource_enum import ResourceSubTypeEnum
        mock_query_count.return_value = 0
        mock_query_all.return_value = []
        data = {
            "total": 1,
            "pages": 1,
            "page_size": 20,
            "page_no": 0,
            "items": []
        }
        page_req2 = ProtectedObjectQueryRequest(sla_id='8556bb41-abe6-4821-870d-a0252f304dfcmock',
                                                name='omm186',
                                                sla_compliance=[],
                                                sub_type=[ResourceSubTypeEnum.ABBackupClient],
                                                path="8.40.99.186",
                                                page_no=0,
                                                page_size=20)
        crud_protected_object2 = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        page_query_data2 = crud_protected_object2.page_query(Session(), page_req2)
        self.assertNotEqual(page_query_data2, data)

    @mock.patch("sqlalchemy.orm.Query.all")
    def test_count_protection_object_success(self, mock_query_all):
        sla_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        domain_id = "9cf2779d-0ba1-461c-ab30-a34575bb17dfmock"
        mock_query_all.return_value = 0
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        mock_obj_list = crud_protected_object.query_by_sla_id(db=Session(),
                                                              sla_id=sla_id,
                                                              domain_id=domain_id)
        self.assertEqual(mock_obj_list, 0)

    @mock.patch("sqlalchemy.orm.Query.first")
    def test_query_protection_object_success(self, mock_query_first):
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        mock_query_first.return_value = 0
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        mock_obj_list = crud_protected_object.query_one_by_resource_id(db=Session(),
                                                                       resource_id=resource_id)
        self.assertEqual(mock_obj_list, 0)

    @mock.patch("sqlalchemy.orm.Query.first")
    def test_query_protection_time_success(self, mock_query_first):
        from app.resource.service.common.protect_obj_service import get_protect_obj
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        mock_query_first.return_value = None
        protect_obj = get_protect_obj(db=Session(),
                                      resource_id=resource_id)
        self.assertEqual(protect_obj, None)

    @mock.patch("sqlalchemy.orm.Query.update")
    @mock.patch("sqlalchemy.orm.Query.first")
    def test_update_compliance_success(self, mock_query_update, mock_query_first):
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        mock_query_update.return_value = None
        mock_query_first.return_value = None
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        update_conditions = {}
        protect_obj = crud_protected_object.update_by_params(db=Session(),
                                                             resource_id=resource_id,
                                                             update_conditions=update_conditions)
        self.assertEqual(protect_obj, None)

    @mock.patch("app.resource.service.common.protect_obj_service.get_protect_obj")
    @mock.patch("sqlalchemy.orm.Query.update")
    def test_synchronize_time_no_object_success(self, mock_get_protect_obj, mock_query_update):
        from app.resource.service.common.protect_obj_service import sync_time
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        protect_obj = sync_time(db=Session(),
                                resource_id=resource_id)
        self.assertEqual(protect_obj, None)

    @mock.patch("sqlalchemy.orm.Query.first")
    def test_create_projected_object_success(self, mock_query_first):
        from app.protection.object.models.projected_object import ProtectedTask
        from app.protection.object.service import projected_object_service
        pro_obj_id = "d8893969-a01e-4b8c-be8d-f09a56d74e33"
        resource = {"uuid": "d8893969-a01e-4b8c-be8d-f09a56d74e33"}
        sla = {"name": "Gold", "type": "1", "application": "Fileset", "policy_list": "policy_list"}
        mock_query_first.return_value = None
        create_req = ProtectedObjectUpdate()
        resource_id = pro_obj_id
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        current_protected_obj = crud_protected_object.query_one_by_resource_id(db=Session(), resource_id=resource_id)
        projected_object_service.build_protection_task = Mock(return_value=ProtectedTask(uuid="123",
                                                                                         policy_id="123",
                                                                                         protected_object_id="123",
                                                                                         schedule_id="123"))
        projected_object_service.filter_sla_policy = Mock(return_value=False)
        project_object = projected_object_service.build_protection_object(resource, sla, create_req.ext_parameters)
        self.assertEqual(pro_obj_id, project_object.uuid)
        self.assertEqual(current_protected_obj, None)

    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject."
                "query_one_by_resource_id")
    @mock.patch("sqlalchemy.orm.Query.first")
    def test_query_obj_success(self, mock_query_first, mock_query_one_by_resource_id):
        db = Session()
        mock_query_first.return_value = None
        mock_query_one_by_resource_id.return_value = None
        resource_id = "8546bb41-abe6-4821-870d-a0252f04df"
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        protect_obj = crud_protected_object.query_one_by_resource_id(db=db,
                                                                     resource_id=resource_id)
        self.assertIsNone(protect_obj)

    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject.query_by_resource_ids")
    @mock.patch("sqlalchemy.orm.Query.update")
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    def test_count_by_sla_id(self, mock_query_one_by_resource_id, mock_query_update, mock_query_sla):
        from app.protection.object.models.projected_object import ProtectedObject
        db = Session()
        resource_id = "8546bb41-abe6-4821-870d-a0252f04df"
        mock_query_one_by_resource_id.return_value = [{}, {}]
        mock_query_update.return_value = []
        sla = {"uuid": "8556bb41-abe6-4821-870d-a0252f304dfcc", "application": "Fileset"}
        mock_query_sla.return_value = sla
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(ProtectedObject)
        crud_protected_object.query_one_by_resource_id = Mock(return_value=[{}, {}])
        obj_list = crud_protected_object.query_one_by_resource_id(db=db,
                                                                  resource_id=resource_id)
        result = len(obj_list)
        self.assertEqual(result, 2)

    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject.query_obj_by_sla_id")
    @mock.patch("sqlalchemy.orm.Query.update")
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    def test_count_replica_by_sla_id(self, mock_query_obj_by_sla_id, mock_query_update, mock_query_sla):
        from app.protection.object.models.projected_object import ProtectedObject
        database = Session()
        sla_id = "8546bb41-abe6-4821-870d-a0252f04df"
        mock_query_obj_by_sla_id.return_value = [{}, {}]
        mock_query_update.return_value = []
        sla = {"uuid": "8546bb41-abe6-4821-870d-a0252f04df", "application": "Replica"}
        mock_query_sla.return_value = sla
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(ProtectedObject)
        obj_list = crud_protected_object.query_obj_by_sla_id(db=database, sla_id=sla_id)
        result = len(obj_list)
        self.assertEqual(result, 2)

    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject.query_by_resource_ids")
    @mock.patch("sqlalchemy.orm.Query.update")
    def test_batch_activate(self, mock_query_one_by_resource_ids, mock_query_update):
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.protection.object.models.projected_object import ProtectedObject
        resource_ids = ["8546bb41-abe6-4821-870d-a0252f04df",
                        "8546bb41-abe6-4821-870d-a0252f04df"]
        session = Session()
        protected_object = ProtectedObject(name="test_1234",
                                           resource_id="8546bb41-abe6-4821-870d-a0252f04df",
                                           sub_type="VMBackupAgent")
        mock_query_update.return_value = [protected_object]
        batch_activate = BatchProtectionService.batch_activate(session=session,
                                                               resource_ids=resource_ids)
        self.assertIsNone(batch_activate)

    def test_create_plugin_raises_exception(self):
        from app.common.enums.resource_enum import ResourceSubTypeEnum
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.protection.object.service.protection_plugin_factory import ProtectionPluginFactory
        plugin_factory = ProtectionPluginFactory()
        self.assertRaises(EmeiStorBizException, plugin_factory.create_plugin,
                          ResourceSubTypeEnum.VMware)

    @mock.patch("app.protection.object.service.protection_plugin_factory."
                "ProtectionPluginFactory._list_plugins")
    def test__list_plugins(self, mock_list_plugins):
        from app.protection.object.service.protection_plugin_factory import ProtectionPluginFactory
        mock_list_plugins.return_value = [
            'app.protection.object.service.plugins.hyper_v_protection_plugin',
            'app.protection.object.service.plugins.vmware_protection_plugin',
            'app.protection.object.service.plugins.kubernetes_protection_plugin']
        plugin_factory = ProtectionPluginFactory()
        plugins = plugin_factory._list_plugins()
        self.assertIsNotNone(plugins)

    @mock.patch("app.protection.object.service.protection_plugin_factory.ProtectionPluginFactory._list_plugins")
    def test_get_plugin_module_suceess(self, mock_list_plugins):
        from app.protection.object.service.protection_plugin_factory import ProtectionPluginFactory
        _factory = None
        mock_list_plugins.return_value = [
            'app.protection.object.service.plugins.hyper_v_protection_plugin',
            'app.protection.object.service.plugins.vmware_protection_plugin',
            'app.protection.object.service.plugins.kubernetes_protection_plugin']
        result = ProtectionPluginFactory.get_plugin_module("test_res_sub_type")
        self.assertIsNone(result)

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.batch_delete_schedules")
    def test_common_resource_protection_modify(self, mock_batch_del_sche):
        from app.protection.object.models.projected_object import ProtectedTask
        from app.protection.object.service import projected_object_service
        projected_object = self.protected_object(uuid="d8893969-a01e-4b8c-be8d-f09a56d74e66",
                                                 resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33",
                                                 type="Fileset",
                                                 chain_id="8546bb41-abe6-4821-870d-a0252f0555",
                                                 sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                                 ext_parameters=BaseExtParams(),
                                                 task_list=[ProtectedTask(uuid="123",
                                                                          policy_id="123",
                                                                          protected_object_id="123",
                                                                          schedule_id="123")])
        sla = {"uuid": "8556bb41-abe6-4821-870d-a0252f304dfcc", "name": "Gold", "type": "1", "application": "Fileset",
               "policy_list": "policy_list"}
        mock_batch_del_sche.return_value = False
        projected_object_service.build_protection_task = Mock(return_value=ProtectedTask(uuid="123",
                                                                                         policy_id="123",
                                                                                         protected_object_id="123",
                                                                                         schedule_id="123"))
        projected_object.sla_id = sla.get("uuid")
        projected_object.sla_name = sla.get("name")
        self.assertEqual(projected_object.uuid, "d8893969-a01e-4b8c-be8d-f09a56d74e66")

    def test_build_schedule_params(self):
        """
            构造schedule 调度执行之后消息体数据(预留归档、复制扩展)
            :param topic: 执行调度发送的topic
            :param resource_id: 资源id
            :param sla_id: sla的id
            :param chain_id: 当前调度的链id
            :param policy: 当前调度对应的策略
            :param execute_type: 执行类型
            """
        topic1 = "backup"
        topic2 = "replication"
        topic3 = "ScanVmUnderComputeResource"
        topic4 = "test_exception_topic"
        resource_id = "a28f4c46da3511eba016286ed488d337"
        sla_id = "9aaa1f77-f27a-4068-a153-8c29322d5e6b"
        chain_id = "af2bd0d8-72f3-42c5-bbd7-f802d4ea4cf0"
        policy = {"ext_parameters": {"auto_retry": True, "auto_retry_times": 10,
                                     "auto_retry_wait_minutes": 20}}
        execute_type = "test_type"
        from app.protection.object.service.projected_object_service import build_schedule_params
        result1 = build_schedule_params(topic1, resource_id, sla_id, chain_id, policy, execute_type)
        backup_params1 = {
            "resource_id": resource_id,
            "sla_id": sla_id,
            "chain_id": chain_id,
            "policy": policy,
            "execute_type": execute_type,
            "auto_retry": policy.get("ext_parameters").get("auto_retry"),
            "auto_retry_times": 10,
            "auto_retry_wait_minutes": 20
        }

        self.assertEqual(result1, backup_params1)

        backup_params2 = {
            "resource_id": resource_id,
            "sla_id": sla_id,
            "chain_id": chain_id,
            "policy": policy,
            "execute_type": execute_type,
        }
        result2 = build_schedule_params(topic2, resource_id, sla_id, chain_id, policy, execute_type)
        self.assertEqual(result2, backup_params2)

        backup_params3 = {
            "resource_id": resource_id
        }
        result3 = build_schedule_params(topic3, resource_id, sla_id, chain_id, policy, execute_type)
        self.assertEqual(result3, backup_params3)

        from app.common.exception.unified_exception import EmeiStorBizException
        self.assertRaises(EmeiStorBizException, build_schedule_params, topic4,
                          resource_id, sla_id, chain_id, policy, execute_type)

    def test_construct_schedule_start_time_success(self):
        from app.protection.object.service.projected_object_service import \
            _construct_schedule_start_time
        schedule = {"trigger": 1,
                    "start_time": "2021-7-18T00:00:00",
                    "window_start": "02:00:00"}
        result = _construct_schedule_start_time(schedule)
        self.assertEqual(result["start_time"], schedule.get("start_time", ""))
        schedule = {"trigger": 4,
                    "trigger_action": "year",
                    "days_of_year": "2021-7-18",
                    "window_start": "02:00:00",
                    "start_time": "2021-07-18T02:00:00"}
        result = _construct_schedule_start_time(schedule)
        self.assertEqual(result["start_time"], "2021-07-18T02:00:00")

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new")
    @mock.patch("uuid.uuid4")
    def test_build_protection_task_not_success(self, mock_create_interval_schedule_new, mock_uuid4):
        """
        测试构造保护对象任务函数
        :param obj_id: 保护对象id
        :param policy: 对应sla中的策略
        :param topic: 保护任务对应的调度任务的topic
        :param schedule_params: 调度执行的参数
        """
        from app.protection.object.service.projected_object_service import build_protection_task

        obj_id = "d72c8dcb-104f-49ac-aae9-06dca85bdd3f"
        policy = {"schedule": {"interval": "interval",
                               "interval_unit": "10",
                               "start_time": "2021.07.05"
                               },
                  "uuid": "966b5cad-ac48-4774-aff1-3411c75a11b5"}
        topic = ""
        schedule_params = {}
        mock_create_interval_schedule_new.return_value = "966b5cad-ac48-4774-aff1-3411c75a11b5"
        mock_uuid4.return_value = "15b1d5de-7e8e-4386-b0a5-ebc3eaa5ebf7"
        result = build_protection_task(obj_id, policy, topic, schedule_params)

        from app.protection.object.models.projected_object import ProtectedTask

        task = ProtectedTask(uuid="966b5cad-ac48-4774-aff1-3411c75a11b5",
                             policy_id=policy.get("uuid"),
                             protected_object_id=obj_id,
                             schedule_id="966b5cad-ac48-4774-aff1-3411c75a11b5")
        self.assertNotEqual(result.__dict__, task.__dict__)

    def test_generate_schedule_topic_success(self):
        from app.protection.object.service.projected_object_service import generate_schedule_topic

        topic1 = ""
        topic2 = "backup"
        policy_type = "archiving"
        result1 = generate_schedule_topic(topic1, policy_type)

        data = "schedule." + policy_type
        self.assertEqual(result1, data)

        result2 = generate_schedule_topic(topic2, policy_type)
        self.assertEqual(result2, topic2)

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule")
    def test_create_schedule_and_build_task_success(self, mock_create_interval_schedule_new):
        """
        测试创建调度任务并且构造保护对象任务
        """
        mock_create_interval_schedule_new.return_value = "966b5cad-ac48-4774-aff1-3411c75a11b5"

        from app.protection.object.service.projected_object_service import create_schedule_and_build_task
        from app.protection.object.models.projected_object import ProtectedObject

        protected_obj = ProtectedObject(uuid="d72c8dcb-104f-49ac-aae9-06dca85bdd3f",
                                        chain_id="966b5cad-ac48-4774-aff1-3411c75a11b5")
        policy = {"schedule": {"interval": "interval",
                               "interval_unit": "10",
                               "start_time": "2021.07.05"
                               },
                  "uuid": "966b5cad-ac48-4774-aff1-3411c75a11b5"}

        result = create_schedule_and_build_task(policy, protected_obj)

        self.assertIsNotNone(result)

    # def test_check_os_type_script_exception(self):
    #     """
    #     检查脚本的类型
    #     :return:
    #     """
    #     from app.protection.object.service.projected_object_service import check_os_type_script
    #
    #     os_type1 = "windows"
    #     os_type2 = "linux"
    #     value1 = "test.ba"
    #     value2 = "test.shh"
    #     value3 = "test.sh"
    #     resource_sub_type = "Oracle"
    #     from app.common.exception.unified_exception import EmeiStorBizException
    #
    #     self.assertRaises(EmeiStorBizException, check_os_type_script,
    #                       os_type1, value1, resource_sub_type)
    #     self.assertRaises(EmeiStorBizException, check_os_type_script,
    #                       os_type2, value2, resource_sub_type)
    #     self.assertRaises(EmeiStorBizException, check_os_type_script,
    #                       os_type2, value3, resource_sub_type)

    def test_check_protected_obj_script(self):
        # VMWare先不校验 脚本

        from app.common.exception.unified_exception import EmeiStorBizException
        from app.protection.object.schemas.protected_object import ModifyProtectionSubmitReq
        from app.protection.object.service.projected_object_service import check_protected_obj_script
        resource_obj1 = {"environment_os_type": "linux",
                         "sub_type": "TsetOther"}
        resource_obj2 = {"environment_os_type": "linux",
                         "sub_type": "Oracle"}

        ext_parameters1 = Mock()
        ext_parameters1.dict.return_value = {"sla_id": "d72c8dcb-104f-49ac-aae9-06dca85bdd3f",
                                             "resource_id": "966b5cad-ac48-4774-aff1-3411c75a11b5",
                                             "ext_parameters": {},
                                             "pre_script": b'1' * 8193}

        result = check_protected_obj_script(resource_obj1, ext_parameters1)
        self.assertIsNone(result)

        self.assertRaises(EmeiStorBizException, check_protected_obj_script,
                          resource_obj2, ext_parameters1)

    def test_check_is_container_resource(self):
        """
         校验是否是容器类资源
         :param sub_type: 资源子类型
         :return:
         """

        from app.protection.object.service.projected_object_service import check_is_container_resource
        sub_type = "Oracle"
        result_false: bool = check_is_container_resource(sub_type)
        self.assertFalse(result_false)

        sub_type1 = "vim.ClusterComputeResource"
        result_true: bool = check_is_container_resource(sub_type1)
        self.assertTrue(result_true)

    # def test_protect_pre_check(self):
    #     from app.protection.object.service.projected_object_service import protect_pre_check
    #     from app.common.exception.unified_exception import EmeiStorBizException
    #     """
    #     根据资源类型进行保护前置校验
    #     :param resource_obj:  资源对象
    #     :return:
    #     """
    #     sla_obj = {"application": "Oracle"}
    #     resource_obj = {"sub_type": "Common"}
    #
    #     self.assertRaises(EmeiStorBizException, protect_pre_check,
    #                       sla_obj, resource_obj)
    #
    #     sla_obj = {"application": "VMWare"}
    #     resource_obj = {"sub_type": "vim.ClusterComputeResource"}
    #
    #     self.assertRaises(EmeiStorBizException, protect_pre_check,
    #                       sla_obj, resource_obj)

    def test_select_manual_backup_policies(self):
        """
            选择可以执行的手动备份的SLA策略: 选择类型为备份，action非日志的策略
        """
        policy = {"type": "backup", "action": "log"}
        policy1 = {"type": "backup", "action": "full"}
        from app.protection.object.service.projected_object_service import select_manual_backup_policies
        result = select_manual_backup_policies(policy1)
        self.assertTrue(result)

    @mock.patch("app.protection.object.service.projected_object_service.load_resource")
    def test_get_endpoint_by_resource_ids_success(self, mock_load_resource):
        """
        :param BatchProtectionSubmitReq: 批量保护参数
        :return:
        记录操作日志时获取资源名称和path地址

        """
        from app.protection.object.schemas.protected_object import BatchProtectionSubmitReq
        from app.protection.object.schemas.extends.params.vmware_ext_param import ProtectResource
        from app.protection.object.service.projected_object_service import get_endpoint_by_resource_ids

        batch_create_req1 = None

        mock_load_resource.return_value = [{"name": "test1",
                                            "path": "8.40.99.123"},
                                           {"name": "test2",
                                            "path": "8.40.99.456"}]
        batch_create_req2 = BatchProtectionSubmitReq(
            sla_id="d72c8dcb-104f-49ac-aae9-06dca85bdd3f",
            resources=[
                ProtectResource(resource_id="e844796a-46e9-4bfe-8f3d-c8e5972a617b"),
                ProtectResource(resource_id="dfb923b6-1bde-41ad-a544-33a0da3838b4")],
            ext_parameters={}
        )
        params1 = {"user_info":
                       {"userName": "sysadmin",
                        "password": "hxxxxxx@1234"},
                   "batch_create_req": batch_create_req1}

        params2 = {"user_info":
                       {"userName": "sysadmin",
                        "password": "hxxxxxx@1234"},
                   "batch_create_req": batch_create_req2}

        res1 = get_endpoint_by_resource_ids(params1)

        data = "--, --"
        self.assertEqual(res1, data)

        log_data = "test1:8.40.99.123, test2:8.40.99.456"
        res2 = get_endpoint_by_resource_ids(params2)
        self.assertEqual(res2, log_data)

        mock_load_resource.return_value = [{"name": "test1",
                                            "test_path": "8.40.99.123"},
                                           {"name": "test2",
                                            "test_path": "8.40.99.123"}]

        log_data2 = "test1:--, test2:--"
        res2 = get_endpoint_by_resource_ids(params2)
        self.assertEqual(res2, log_data2)

    def test_construct_schedule_start_time_not_modify_input_object(self):
        """
        验证场景：构造schedule的开始时间
        前置条件：无
        验证点：函数不修改入参对象
        """
        from app.protection.object.service.projected_object_service import _construct_schedule_start_time

        schedule = {
            "days_of_week": ["mon"],
            "trigger": 4,
            "trigger_action": "week"
        }
        constructed_schedule = _construct_schedule_start_time(schedule=schedule)
        self.assertEqual(['mon'], schedule.get("days_of_week"))
        self.assertEqual("2", constructed_schedule.get("days_of_week"))

    def test_check_action_and_small_file_correct(self):
        # check_action_and_small_file方法正常传参
        policy = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65",
            name="full",
            action="full"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        extend_parameters = FilesetProtectionExtParam(
            cross_file_system=True,
            consistent_backup=True,
            backup_nfs=True,
            channels=2,
            sparse_file_detection=True,
            backup_continue_with_files_backup_failed=True,
            small_file_aggregation=True,
            aggregation_file_size=4096,
            aggregation_file_max_size=1024,
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh")
        check_action_and_small_file(sla_obj.get("policy_list"), extend_parameters)
        self.assertTrue(True, "无异常抛出")

    def test_check_action_and_small_file(self):
        # 当同时选择永久增量备份和小文件聚合，报错
        policy = dict(
            uuid="9b17382f-7164-4f5b-8d77-2910a0be348c",
            name="permanent_increment",
            action="permanent_increment"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        extend_parameters = FilesetProtectionExtParam(
            cross_file_system=True,
            consistent_backup=True,
            backup_nfs=True,
            channels=2,
            sparse_file_detection=True,
            backup_continue_with_files_backup_failed=True,
            small_file_aggregation=True,
            aggregation_file_size=4096,
            aggregation_file_max_size=1024,
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh")
        self.assertRaises(EmeiStorBizException, check_action_and_small_file,
                          sla_obj.get("policy_list"), extend_parameters)

    @mock.patch("sqlalchemy.orm.Query.all")
    def test_query_projected_object_by_user_id_success(self, mock_query_all):
        domain_id = "9cf2779d-0ba1-461c-ab30-a34575bb17dfmock"
        mock_query_all.return_value = []
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        mock_obj_list = crud_protected_object.query_projected_object_by_user_id(db=Session(),
                                                                                domain_id=domain_id)
        self.assertEqual(mock_obj_list, [])


if __name__ == '__main__':
    unittest.main(verbosity=2)
