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
from unittest.mock import Mock, MagicMock, patch

from app.backup.client.job_client import JobClient
from app.common.deploy_type import DeployType
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.tools import timezone, functiontools

sys.modules['app.common.events.producer'] = mock.Mock()
sys.modules['app.common.events.topics'] = mock.Mock()
DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

mock.patch("app.common.database.Database.initialize", mock.Mock).start()

from sqlalchemy.orm import Session
from pydantic import BaseModel

from app.protection.object.schemas.protected_copy_object import ProtectedCopyObjectUpdate, ManualReplicationReq, \
    ProtectedCopyBaseExtParam
from app.protection.object.service.projected_copy_object_service import filter_sla_policy, build_copy_protection_task, \
    build_copy_protection_object, ProtectedCopyObjectService
from app.common.clients.protection_client import ProtectionClient
from app.common.clients.resource_client import ResourceClient
from app.common.clients.scheduler_client import SchedulerClient
from app.copy_catalog.models.tables_and_sessions import CopyProtectionTable, CopyProtectedTask
from app.replication.client.replication_client import ReplicationClient


class BaseExtParam(BaseModel):
    ext_parameters = {}


class ResourceObject(BaseModel):
    uuid = "d8893969-a01e-4b8c-be8d-f09a56d74e33"
    application = "Replica"
    task_list = [CopyProtectedTask(
        uuid="34926913-b6a6-4c3b-812e-4fba893cb777",
        protected_resource_id="34926913-b6a6-4c3b-812e-4fba893cbe55",
        policy_id="34926913-b6a6-4c3b-812e-4fba893cbe88",
        schedule_id="34926913-b6a6-4c3b-812e-4fba893cbe99")]
    sla_id = "34926913-b6a6-4c3b-812e-4fba893cbe88"


copy_protected_object = CopyProtectionTable(protected_resource_id="34926913-b6a6-4c3b-812e-4fba893cbe55",
                                            protected_object_uuid="34926913-b6a6-4c3b-812e-4fba893cbe55",
                                            protected_type="Fileset",
                                            protected_chain_id="34926913-b6a6-4c3b-812e-4fba893cbe66")
resource_object = {'uuid': 'd8893969-a01e-4b8c-be8d-f09a56d74e33', 'application': 'Replica',
                   'resource_sub_type': 'Fileset', 'task_list': [CopyProtectedTask],
                   'resource_id': '34926913-b6a6-4c3b-812e-4fba893cbe55'}
sla_object = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
              'application': 'Replica', 'name': 'Golden', 'policy_list': []}


class TestProtectedCopyObjectService(unittest.TestCase):
    def setUp(self):
        super(TestProtectedCopyObjectService, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.protection.object.db import crud_projected_object
        from app.protection.object.service import projected_copy_object_service
        from app.protection.object.models.projected_object import ProtectedObject
        self.crud_projected_object = crud_projected_object
        self.projected_copy_object_service = projected_copy_object_service
        self.protected_object = ProtectedObject

    @mock.patch.object(ResourceClient, "query_resource", Mock(return_value=None))
    @mock.patch.object(SchedulerClient, "batch_delete_schedules", Mock(return_value=True))
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value=sla_object))
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id")
    def test_create_protected_object_success(self, mock_query_resource):
        mock_query_resource.return_value = copy_protected_object
        create_req = ProtectedCopyObjectUpdate(sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                               resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33")
        create_req.ext_parameters = ProtectedCopyBaseExtParam()
        self.projected_copy_object_service.query_replicated_copy_by_resource_id = Mock(return_value=resource_object)
        from app.base.db_base import database
        with database.session() as session:
            result = ProtectedCopyObjectService().create_protected_object(session, create_req)
            self.assertEqual(result, copy_protected_object.protected_resource_id)

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value=sla_object))
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id")
    def test_check_copy_protected_when_copy_is_none(self, mock_query_resource):
        """
        用例场景：创建复制副本保护
        前置条件：复制副本为空
        检查点：创建失败
        """
        mock_query_resource.return_value = copy_protected_object
        req = ProtectedCopyObjectUpdate(sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                               resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33")
        sla = ProtectionClient.query_sla(str(req.sla_id))
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            self.projected_copy_object_service.check_copy_protect(None, sla, copy_protected_object, req, True)
        message = f"Replication copies cannot be replication or archived in master"
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id")
    def test_check_copy_protected_when_sla_is_none(self, mock_query_resource):
        """
        用例场景：创建复制副本保护
        前置条件：sla为空
        检查点：创建失败
        """
        mock_query_resource.return_value = copy_protected_object
        req = ProtectedCopyObjectUpdate(sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                               resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33")
        self.projected_copy_object_service.query_replicated_copy_by_resource_id = Mock(return_value=resource_object)
        copy = {"resource_sub_type", "Fileset"}
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            self.projected_copy_object_service.check_copy_protect(copy, None, copy_protected_object, req, True)
        message = f"sla not exist"
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value=sla_object))
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id")
    def test_check_copy_protected_when_resource_sub_type_not_in_copy_archive_types(self, mock_query_resource):
        """
        用例场景：创建复制副本保护
        前置条件：sla包含归档，但却不在归档特性内
        检查点：创建失败
        """
        mock_query_resource.return_value = copy_protected_object
        req = ProtectedCopyObjectUpdate(sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                               resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33")
        copy = {"resource_sub_type": "Fileset1"}
        sla = {"policy_list": [{"type": "archiving"}],"application": "Replica"}
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            self.projected_copy_object_service.check_copy_protect(copy, sla, copy_protected_object, req, True)
        message = "rep copy can not archive."
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch.object(ResourceClient, "query_resource", Mock(return_value=None))
    @mock.patch.object(SchedulerClient, "batch_delete_schedules", Mock(return_value=True))
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value=sla_object))
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id")
    def test_modify_protection(self, mock_query_resource):
        mock_query_resource.return_value = copy_protected_object
        resource = ResourceObject()
        update_req = ProtectedCopyObjectUpdate(sla_id="34926913-b6a6-4c3b-812e-4fba893cbe5e",
                                               resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33")
        update_req.ext_parameters = ProtectedCopyBaseExtParam()
        self.projected_copy_object_service.query_replicated_copy_by_resource_id = Mock(return_value=resource_object)
        current_protected_obj = resource
        sla = ProtectionClient.query_sla(str(update_req.sla_id))
        is_sla_changed = current_protected_obj.sla_id != sla.get("uuid")

        if is_sla_changed:
            result = ProtectedCopyObjectService().modify_protection(Session(), update_req)
            self.assertEqual(result, copy_protected_object.protected_resource_id)

    def test_filter_sla_policy(self):
        sla = {"type": "archiving"}
        policy = {"application": "Replica",
                  "schedule": {
                      "trigger": 1
                  }}
        result = filter_sla_policy(sla, policy)
        self.assertFalse(result)

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new")
    def test_build_protection_task(self, mock_create_interval_schedule_new):
        obj_id = "34926913-b6a6-4c3b-812e-4fba893cbe5e"
        policy = {"application": "Replica",
                  "schedule": {
                      "trigger": 1,
                      "interval": "1d",
                      "interval_unit": "2",
                      "start_time": "2020-02-17 21:30:00"
                  }}
        topic = "mock_test"
        schedule_params = {}
        mock_create_interval_schedule_new.return_value = "72136654-eaad-4a2f-a24a-9c44ffaa105c"
        result = build_copy_protection_task(obj_id, policy, topic, schedule_params)
        self.assertIsNotNone(result)

    @mock.patch("app.protection.object.service.projected_copy_object_service.build_copy_protection_task")
    def test_build_protection_object(self, mock_build_protection_task):
        copy = {
            "resource_id": "f9e3b2a2-1627-423d-b432-1fc837a5b7e6",
            "resource_name": "mock_test",
            "resource_location": "/home/lxx",
            "resource_type": "Oracle",
        }
        sla = {
            "uuid": "2202665a-ac1e-45a0-b1ef-da446465dd01",
            "policy_list": [],
            "name": "mock_sla"
        }
        copy_protected_task = CopyProtectedTask()
        mock_build_protection_task.return_value = CopyProtectedTask()
        ext_parameters = ProtectedCopyBaseExtParam()
        result = build_copy_protection_object(copy, sla, ext_parameters)
        self.assertNotEqual(result, copy_protected_task)

    @mock.patch("app.protection.object.db.copy_projected_object.query_by_resource_ids")
    @mock.patch.object(SchedulerClient, "batch_delete_schedules", Mock(return_value=True))
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.delete_by_condition",
                Mock(return_value=True))
    def test_remove_replication_copy_protection_success(self, mock_query_by_resource_ids):
        """
        用例场景：移除复制副本保护
        前置条件：复制副本已绑定sla，资源对应的sla被移除
        检查点：移除副本保护成功
        """
        resource_ids = ['501feb28-8e2d-3623-e7b4-f78d94f8d5bf']
        mock_query_by_resource_ids.return_value = [copy_protected_object]
        self.assertEqual(ProtectedCopyObjectService().batch_remove_protection(Session(), resource_ids), resource_ids)

    @mock.patch("sqlalchemy.orm.Query.first", Mock(return_value=None))
    @mock.patch(
        "app.protection.object.service.projected_copy_object_service.db.copy_projected_object.query_one_by_resource_id")
    def test_should_raise_EmeiStorBizException_when_manual_replicate_if_protected_object_is_None(self,
                                                                                                 mock_query_first):
        """
        用例场景：对复制副本资源进行手动复制
        前置条件：无
        检查点：如果保护对象不存在，则抛出异常
        """
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        obj = Mock()

        def as_dict():
            return None
        obj.as_dict = as_dict
        mock_query_first.return_value = obj
        mock_protected_obj = crud_protected_object.query_one_by_resource_id(db=Session(), resource_id=resource_id)
        self.projected_copy_object_service.get_resource_obj_from_copy = Mock(return_value={})
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            ProtectedCopyObjectService.manual_replicate("user_id",
                                                        ManualReplicationReq(resource_id=resource_id, sla_id="sla_id",
                                                                             policy_id="policy_id"), Session())
        message = "protected obj of 8556bb41-abe6-4821-870d-a0252f304dfcmock is null"
        self.assertTrue(ex.exception._parameters.__contains__(message))

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value=None))
    def test_should_raise_EmeiStorBizException_when_build_manual_replicate_payload_if_sla_is_None(self):
        """
        用例场景：对复制副本资源进行手动复制，如果查询到的sla为空，则抛出异常
        前置条件：保护对象存在
        检查点：如果sla不存在，则不能进行复制
        """
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            self.projected_copy_object_service.build_manual_replicate_payload("123", ManualReplicationReq(
                resource_id="resource_id", sla_id="123",
                policy_id="policy_id"))
        message = "sla of 123 is null."
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"policy_list": [{"uuid": "123456"}]}))
    def test_should_raise_EmeiStorBizException_when_build_manual_replicate_payload_if_sla_is_None(self):
        """
        用例场景：对复制副本资源进行手动复制，如果选择的policy_id不为sla中的policy，则抛出异常
        前置条件：保护对象存在
        检查点：如果选择的policy_id不在保护的sla当中，则不能进行复制
        """
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            self.projected_copy_object_service.build_manual_replicate_payload("123", ManualReplicationReq(
                resource_id="resource_id", sla_id="123",
                policy_id="12345"))
        message = "selected policy: 12345 is null."
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.count_obj_by_sla_id",
                mock.Mock(return_value=0))
    def test_count_by_sla_id(self):
        from app.base.db_base import database
        with database.session() as session:
            count_one = ProtectedCopyObjectService().count_by_sla_id(session, '34926913-b6a6-4c3b-812e-4fba893cbe5e');
            self.assertEqual(0, count_one)


    @mock.patch("app.copy_catalog.db.copies_db.count_by_user_id_and_resource_ids",
                mock.Mock(return_value=1))
    def test_should_raise_EmeiStorBizException_verify_protect_copy_objects_ownership_when_user_is_none(self):
        """
        用例场景：校验复制副本用户权限
        前置条件：复制副本存在
        检查点：user_id为空时抛出异常
        """
        from app.common.exception.unified_exception import EmeiStorBizException
        resource_uuid_list = ["a55c3800-9e25-439d-b70a-5fa996440fc8"]
        with self.assertRaises(EmeiStorBizException) as ex:
            ProtectedCopyObjectService().verify_protect_copy_objects_ownership(None, resource_uuid_list)
        message = "Access denied."
        self.assertEqual(message, ex.exception._error_message)

    @mock.patch("app.protection.object.service.projected_copy_object_service.count_by_user_id_and_resource_ids",
                mock.Mock(return_value=0))
    @mock.patch("app.protection.object.common.db_config.get_session", MagicMock)
    def test_should_raise_EmeiStorBizException_verify_protect_copy_objects_ownership_when_count_zero(self):
        """
        用例场景：校验复制副本用户权限
        前置条件：复制副本存在
        检查点：count为0
        """
        from app.common.exception.unified_exception import EmeiStorBizException
        resource_uuid_list = ["a55c3800-9e25-439d-b70a-5fa996440fc8"]
        with self.assertRaises(EmeiStorBizException) as ex:
            ProtectedCopyObjectService().verify_protect_copy_objects_ownership("a55c3800-9e25-439d-b70a-5fa996440fc8", resource_uuid_list)
        message = "Access denied."
        self.assertEqual(message, ex.exception._error_message)


    @mock.patch("app.protection.object.common.db_config.get_session", MagicMock)
    @mock.patch("app.protection.object.service.projected_copy_object_service.count_by_user_id_and_resource_ids")
    def test_verify_protect_copy_objects_ownership_success(self, mock_count):
        """
        用例场景：校验复制副本用户成功
        前置条件：复制副本存在
        检查点：校验成功
        """
        mock_count.return_value = 1
        resource_uuid_list = ["a55c3800-9e25-439d-b70a-5fa996440fc8"]
        ProtectedCopyObjectService().verify_protect_copy_objects_ownership("a55c3800-9e25-439d-b70a-5fa996440fc8", resource_uuid_list)
        self.assertEqual(mock_count.call_count, 1)

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.crud_copy_projected_object.CRUDCopyProtectedObject.query_one_by_resource_id",
                Mock(return_value=0))
    @patch.object(ProtectionClient, "query_sla", Mock(return_value={"policy_list": [
        {"uuid": "policy_id", "type": "replication",
         "schedule": {"trigger": 2, "interval": 0, "interval_unit": "m", "start_time": "2021-03-10T01:26:23"},
         "ext_parameters": {"start_replicate_time": "2023-01-28 00:16:02"}}]}))
    @patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd"},
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":0,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd"}]))
    def test_query_copy_projected_object_success(self):
        from app.base.db_base import database
        with database.session() as session:
            count_one = ProtectedCopyObjectService().query_copy_projected_object(session)
            self.assertEqual(0, len(count_one))

    @unittest.skip("skipping this test")
    @mock.patch("sqlalchemy.orm.Query.first", Mock(return_value=None))
    @mock.patch(
        "app.protection.object.service.projected_copy_object_service.db.copy_projected_object.query_one_by_resource_id")
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"policy_list": [{"uuid": "123456", "ext_parameters": {"qos_id":"","external_system_id":"2","link_deduplication":True,"link_compression":True,"alarm_after_failure":True,"start_replicate_time":"2023-03-09 00:27:00"}}]}))
    @mock.patch.object(ReplicationClient, "query_copy_statistic", Mock(return_value=[[], []]))
    def test_should_raise_Exception_when_manual_replicate_if_copies_is_None(self, mock_query_first):
        """
        用例场景：对复制副本资源进行手动复制
        前置条件：无
        检查点：如果没有满足复制规则的副本，则提示无副本
        """
        resource_id = "8556bb41-abe6-4821-870d-a0252f304dfcmock"
        crud_protected_object = self.crud_projected_object.CRUDProtectedObject(self.protected_object)
        obj = Mock()

        def as_dict():
            return {"resource_id": "d30d9c24e36842a88ed33e3529108261"}
        obj.as_dict = as_dict
        mock_query_first.return_value = obj
        mock_protected_obj = crud_protected_object.query_one_by_resource_id(db=Session(), resource_id=resource_id)
        self.projected_copy_object_service.get_resource_obj_from_copy = Mock(return_value={})
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException) as ex:
            ProtectedCopyObjectService.manual_replicate("user_id",
                                                        ManualReplicationReq(resource_id=resource_id, sla_id="sla_id",
                                                                             policy_id="123456"), Session())
        self.assertTrue(ex.exception.error_code == '1677749504')


if __name__ == '__main__':
    unittest.main(verbosity=2)