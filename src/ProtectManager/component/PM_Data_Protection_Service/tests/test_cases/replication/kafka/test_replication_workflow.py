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
from datetime import datetime, timedelta
from unittest import mock

from app.common.enums.resource_enum import DeployTypeEnum
from tests.test_cases import common_mocker  # noqa

_mock_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_db_init.start()
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.tools import http, env

mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("os.getenv", env.get_env).start()
# mock为了导入Schedule
_mock_validator_manager_init = mock.patch("app.backup.common.validators.validator_manager.ValidatorManager.__init__",
                                          mock.Mock(return_value=None))
_mock_validator_manager_init.start()
from app.backup.schemas.policy import Schedule
# mock处理“from app.resource.service.common import resource_service”中“from app.common.events import producer”
from app.backup.client.job_client import JobClient
from app.backup.client.resource_client import ResourceClient
from app.replication.client.replication_client import ReplicationClient
from app.common.events.consumer import EsEvent
import json
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, patch
from urllib3 import HTTPResponse
from tests.test_cases.common.events import mock_producer  # noqa
from tests.test_cases.common import mock_kafka_client  # noqa
from app.common.enums.protected_object_enum import Status
from tests.test_cases.common.mock_settings import fake_timezone_data
from app.common.clients.client_util import ProtectEngineEDmaHttpsClient
from app.common.clients.protection_client import ProtectionClient

mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()
_mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_common_db_init.start()

req_dma_patcher = patch.object(
    ProtectEngineEDmaHttpsClient, "request",
    Mock(return_value=HTTPResponse(status=200, body=bytes(json.dumps(fake_timezone_data), encoding="utf-8"))))
req_dma_patcher.start()


class SubmitRestoreJobTest(unittest.TestCase):
    def setUp(self) -> None:
        mock.patch("app.common.deploy_type.DeployType.get_deploy_type", Mock(return_value=DeployTypeEnum.X8000)).start()
        from app.replication.kafka import replication_workflow

        self.replication_workflow = replication_workflow

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    @patch("app.replication.kafka.replication_workflow.is_start_time_effect", Mock(return_value=True))
    @patch.object(ResourceClient, "query_protected_object", mock.Mock(return_value={"status": 1}))
    @patch("app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService."
           "query_copy_protected_dict", )
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[{"uuid":"a55c3800-9e25-439d-b70a-5fa996440fc8","timestamp":"1678434059000000","display_timestamp":"2023-03-10T15:40:59", "generated_time":"2023-03-10T15:40:59", "deletable":True,"status":"Normal","retention_type":2,"retention_duration":30,"duration_unit":"d","expiration_time":"2023-04-09T15:40:59","properties":"{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}","resource_id":"868a5c250bc6499cac77604be56998bd"}]))
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    @unittest.skip
    def test_schedule_replication_success(self, _mock_query_copy_protected_dict, _mock_query_res):
        """
        测试场景：消费调度复制任务的消息
        前提条件: （1）任务是激活的；（2）消费消息成功
        检查点: 返回None
        """
        _mock_query_res.return_value = {
            "status": 1, "sla_id": "c584d517-f2ef-4512-8fdc-7fdf6b5d0a1a", "sub_type": "fileset", "user_id": "admin"}
        _mock_query_copy_protected_dict.return_value = {}
        request = EsEvent("request_id")
        result = self.replication_workflow.schedule_replication(
            request=request,
            resource_id="resource_id",
            status=Status.Active.value,
            policy={"schedule": {
                "trigger": 1,
                "interval": 1,
                "interval_unit": "w",
                "start_time": datetime.today() + timedelta(hours=1),
                "window_duration": 24,
                "duration_unit": "h"
            },
            "ext_parameters" :{"start_replicate_time":"2023-03-22 17:32:16"}})
        self.assertIsNone(result)

    @mock.patch("app.replication.kafka.replication_workflow.is_start_time_effect")
    def test_is_start_time_effect(self, mock_is_start_time_effect):
        """
        测试场景：检查任务调度开始时间是否有效
        前提条件: 任务调度开始时间早于当前时间
        检查点: 返回True
        """
        time = "2020-03-04 11:04:16.820982"
        start_time = Schedule(trigger=1, start_time=time)
        result = self.replication_workflow.is_start_time_effect(schedule=start_time)
        self.assertTrue(result)

    def test_start_log(self):
        """
        测试场景：记录复制开始日志
        前提条件: 记录日志成功
        检查点: 返回None
        """
        res = self.replication_workflow.start_log()
        self.assertIsNone(res)

    @patch("app.common.events.producer.produce", Mock(return_value=None))
    def test_protection_backup_complete_success(self):
        """
        测试场景：消费备份完成的消息
        前提条件: （1）任务不是Oracle日志备份；（2）消费消息成功
        检查点: 返回None
        """
        request = EsEvent(request_id="8546bb41-abe6-4821-870d-a0252f04mock")
        payload = {
            "uuid": "'c584d517-f2ef-4512-8fdc-7fdf6b5d0a9a'",
            "application": "Fileset",
            "is_global": False,
            "sla": {
                "policy_list": [{"uuid": "0817badf-60b8-4a01-819c-3915d01b5261",
                                 "type": "replication",
                                 "schedule": {"trigger": 2,
                                              "interval": 0,
                                              "interval_unit": "m",
                                              "start_time": "2021-03-10T01:26:23"
                                              }}]
            },
            "policy": {"action": "full", "type": "replication"},
            "resource": {"sub_type": "Fileset"}
        }
        ret = self.replication_workflow.protection_backup_complete(request, **payload)
        self.assertIsNone(ret)

    @patch("app.common.context.context.Context.get")
    @patch("app.common.context.context.Context.__init__", Mock(return_value=None))
    def test_return_none_if_copy_id_not_exists_when_replication_complete(self, _mock_ctx_get):
        """
        测试场景：消费调度复制任务的消息
        前提条件: redis中存在copy_id
        检查点: 返回None
        """
        request = EsEvent(request_id="8546bb41-abe6-4821-870d-a1252f04mock")
        _mock_ctx_get.return_value = {"copy_id": str(uuid.uuid4())}
        ret = self.replication_workflow.replication_complete(request)
        self.assertIsNone(ret)

    @patch("app.common.context.context.Context.get")
    @patch("app.common.context.context.Context.__init__", Mock(return_value=None))
    def test_send_alarm_when_reverse_replication_fail(self, _mock_ctx_get):
        """
        测试场景：消费调度复制任务的消息
        前提条件: redis中存在copy_id
        检查点: 返回None
        """
        request = EsEvent(request_id="8546bb41-abe6-4821-870d-a1252f04mock")
        _mock_ctx_get.return_value = {"copy_id": str(uuid.uuid4())}
        _mock_ctx_get.return_value = {"job_status": "0"}
        ret = self.replication_workflow.replication_complete(request)
        self.replication_workflow.alarm_after_failure = Mock()
        self.replication_workflow.alarm_after_failure.called
        self.assertIsNone(ret)

    @patch("app.common.events.producer.produce", Mock(return_value=None))
    @patch("app.replication.kafka.replication_workflow.alarm_after_failure", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    @patch("app.common.context.context.Context.get")
    @patch("app.common.context.context.Context.__init__", mock.Mock(return_value=None))
    def test_replication_complete_success(self, _mock_ctx_get, _mock_query_res):
        """
        测试场景：消费调度复制任务的消息
        前提条件: redis中不存在copy_id
        检查点: 返回None
        """
        fake_resource_id = str(uuid.uuid4())
        ctx_map = {
            "job_status": "running",
            "sla": {"uuid": "c584d517-f2ef-4512-8fdc-7fdf6b5d0a1a"},
            "resource": {"uuid": fake_resource_id}
        }
        _mock_ctx_get.side_effect = lambda x, t=None: ctx_map.get(x)
        request = EsEvent(request_id="8546bb41-abe6-4821-870d-a1252f04mock")
        # current_resource_obj存在
        _mock_query_res.return_value = {"sla_id": "62b83d28-436e-4bdd-aa53-d3661a8e690b"}
        ret = self.replication_workflow.replication_complete(request)
        self.assertIsNone(ret)

        # current_resource_obj不存在
        _mock_query_res.return_value = None
        ret = self.replication_workflow.replication_complete(request)
        self.assertIsNone(ret)

    @patch("app.common.events.producer.produce", Mock(return_value=None))
    @patch("app.common.context.context.Context.set", Mock(return_value=None))
    @patch("app.common.context.context.Context.__init__", Mock(return_value=None))
    @patch("app.replication.kafka.replication_workflow.validate_license_by_resource_type", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    def test_initialize_replication_success(self, _mock_query_res):
        """
        测试场景：消费初始化复制任务的消息
        前提条件: 消费消息成功
        检查点: 返回字典，且topic值为“protection.replication”
        """
        request = EsEvent(request_id="8546bb41-abe6-4821-870d-a0252f04mock")
        schedule = {"trigger": {
            "trigger": "1",
            "interval": "1",
            "interval_unit": "w",
            "start_time": "datetime.today() + timedelta(hours=1)",
            "window_duration": 24,
            "duration_unit": "h",
        }}

        payload = {
            "request_id": "8546bb41-abe6-4821-870d-a0252f04mock",
            "task_id": "8546bb41-abe6-4821-870d-a0252f04df",
            "resource_id": "8546bb41-abe6-4821-870d-a0252f04df",
            "protected_obj": {"protected_obj": "value"},
            "chain_id": "6bb41-abe6-4821-870d-a0252f04df",
            "backup_type": "backup",
            "config": {},
            "sla": {"policy_list":
                        [{"type": 1, "schedule": schedule}],
                    },
            "policy": "policy"
        }
        _mock_query_res.return_value = {"sub_type": ""}
        result = self.replication_workflow.initialize_replication(request, **payload)
        self.assertIsInstance(result, dict)
        self.assertEqual(result.get("topic"), "protection.replication")

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    @patch("app.replication.kafka.replication_workflow.is_start_time_effect", Mock(return_value=True))
    @patch.object(ResourceClient, "query_protected_object", mock.Mock(return_value={"status": 1}))
    @patch("app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService."
           "query_copy_protected_dict", )
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-23 17:32:16", "deletable": True,
         "status": "Normal", "retention_type": 2, "retention_duration": 30, "duration_unit": "d",
         "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd", "device_esn": "123"}]))
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    @unittest.skip
    def test_schedule_replication_with_out_resource(self, _mock_query_copy_protected_dict, _mock_query_res):
        """
        测试场景：消费调度复制任务的消息,payload中沒有resource信息
        前提条件: payload中沒有resource信息
        检查点: 设置resource值
        """
        _mock_query_res.return_value = {
            "status": 1, "sla_id": "c584d517-f2ef-4512-8fdc-7fdf6b5d0a1a", "sub_type": "fileset", "user_id": "admin"}
        _mock_query_copy_protected_dict.return_value = {}
        request = EsEvent("request_id")
        result = self.replication_workflow.schedule_replication(
            request=request,
            resource_id="resource_id",
            status=Status.Active.value,
            policy={"schedule": {
                "trigger": 1,
                "interval": 1,
                "interval_unit": "w",
                "start_time": datetime.today() + timedelta(hours=1),
                "window_duration": 24,
                "duration_unit": "h"
            },
            "ext_parameters" :{"start_replicate_time":"2023-03-22 17:32:16"}})
        
        self.assertIsNone(result)


    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    @patch("app.replication.kafka.replication_workflow.is_start_time_effect", Mock(return_value=True))
    @patch.object(ResourceClient, "query_protected_object", mock.Mock(return_value={"status": 1}))
    @patch("app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService."
           "query_copy_protected_dict", )
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd",
         "device_esn": "123"},
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":0,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd",
         "device_esn": "123"}]))
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    @unittest.skip
    def test_schedule_replication_success_when_copy_format_different(self, _mock_query_copy_protected_dict, _mock_query_res):
        """
		测试场景：消费调度复制任务的消息,资源下有快照和目录格式的副本
		前提条件: （1）任务是激活的；（2）消费消息成功
		检查点: 创建两个任务
		"""
        _mock_query_res.return_value = {
            "status": 1, "sla_id": "c584d517-f2ef-4512-8fdc-7fdf6b5d0a1a", "sub_type": "fileset", "user_id": "admin"}
        _mock_query_copy_protected_dict.return_value = {}
        request = EsEvent("request_id")
        result = self.replication_workflow.schedule_replication(
            request=request,
            resource_id="resource_id",
            status=Status.Active.value,
            policy={"schedule": {
                "trigger": 1,
                "interval": 1,
                "interval_unit": "w",
                "start_time": datetime.today() + timedelta(hours=1),
                "window_duration": 24,
                "duration_unit": "h"
            },
                "ext_parameters": {"start_replicate_time": "2023-03-01 17:32:16"}})
        self.assertIsNone(result)
        assert JobClient.create_job.call_count == 2

    @patch("app.replication.kafka.replication_workflow.is_start_time_effect", Mock(return_value=True))
    @patch.object(ResourceClient, "query_protected_object", mock.Mock(return_value={}))
    @patch("app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService."
           "query_copy_protected_dict", )
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    def test_schedule_replication_protect_obj_is_null(self, _mock_query_copy_protected_dict):
        """
        测试场景：消费调度复制任务的消息
        前提条件: 保护对象不存在
        检查点: 返回None
        """
        _mock_query_copy_protected_dict.return_value = {}
        request = EsEvent("request_id")
        result = self.replication_workflow.schedule_replication(
            request=request,
            resource_id="resource_id",
            status=Status.Active.value,
            policy={"schedule": {
                "trigger": 1,
                "interval": 1,
                "interval_unit": "w",
                "start_time": datetime.today() + timedelta(hours=1),
                "window_duration": 24,
                "duration_unit": "h"
            },
            "ext_parameters" :{"start_replicate_time":"2023-03-22 17:32:16"}})
        self.assertIsNone(result)

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(ResourceClient, "query_resource")
    @patch("app.replication.kafka.replication_workflow.is_start_time_effect", Mock(return_value=True))
    @patch.object(ResourceClient, "query_protected_object", mock.Mock(return_value={"status": 1}))
    @patch("app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService."
           "query_copy_protected_dict", )
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd",
         "device_esn": "2102355MGF485762AH87"},
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-03-10T15:40:59", "generated_time": "2023-03-10T15:40:59", "retention_type": 2,
         "retention_duration": 30, "duration_unit": "d", "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":0,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd",
         "device_esn": "2102355MGF485762AH87"}]))
    @unittest.skip
    def test_schedule_replication_success_when_copy_in_replica_backup_unit(self, _mock_query_copy_protected_dict,
                                                                     _mock_query_res):
        """
		测试场景：消费调度复制任务的消息,复制策略指定备份存储单元
		前提条件: （1）任务是激活的；（2）副本在复制策略指定的备份存储单元上
		检查点: 不创建任务
		"""
        _mock_query_res.return_value = {
            "status": 1, "sla_id": "c584d517-f2ef-4512-8fdc-7fdf6b5d0a1a", "sub_type": "fileset", "user_id": "admin"}
        _mock_query_copy_protected_dict.return_value = {}
        request = EsEvent("request_id")
        result = self.replication_workflow.schedule_replication(
            request=request,
            resource_id="resource_id",
            status=Status.Active.value,
            policy={"schedule": {
                "trigger": 1,
                "interval": 1,
                "interval_unit": "w",
                "start_time": datetime.today() + timedelta(hours=1),
                "window_duration": 24,
                "duration_unit": "h"
            },
                "ext_parameters": {"start_replicate_time": "2023-03-01 17:32:16", 
                                   "replication_target_type": 1,
                                   "replication_storage_type": 2,
                                   "replication_storage_id": "2102355MGF485762AH87", 
                                   "replication_target_mode": 2
                                   }})
        self.assertIsNone(result)
        assert JobClient.create_job.call_count == 0
if __name__ == '__main__':
    unittest.main(verbosity=2)
