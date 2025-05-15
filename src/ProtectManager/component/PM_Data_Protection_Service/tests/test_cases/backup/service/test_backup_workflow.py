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
import json
import time
import unittest
import uuid
import sys
from unittest import mock
from unittest.mock import patch, Mock

from urllib3 import HTTPResponse


from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobStatus
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context  # noqa
from app.backup.client.archive_client import ArchiveClient

from app.common.clients.client_util import SystemBaseHttpsClient, InfrastructureHttpsClient

_mock_validator_manager_init = mock.patch("app.backup.common.validators.validator_manager.ValidatorManager.__init__",
                                          mock.Mock(return_value=None))
_mock_internal_client = mock.patch("app.common.clients.client_util.InternalHttpsClient.__init__",
                                   mock.Mock(return_value=None))
_mock_validator_manager_init.start()
from app.backup.client.job_client import JobClient
from app.backup.client.protection_client import ProtectionClient
from app.backup.client.resource_client import ResourceClient
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.log.event_client import EventClient
from app.common.clients.anti_ransomware_client import AntiRansomwareClient
from app.backup.client.scheduler_client import SchedulerClient

class RequestMock:
    def __init__(self):
        self.request_id = "785f6333-c826-4515-bee4-0f5ac0243852"
        self.user_id = "1a881ee6-69f8-4615-9b7c-da599402619d"


schedule = {"window_end": "21:12:10", "window_start": "21:12:10", "interval": 2,
            "interval_unit": "d"}


class BackupTest(unittest.TestCase):
    @classmethod
    def tearDownClass(cls) -> None:
        _mock_validator_manager_init.stop()

    def setUp(self) -> None:
        sys.modules['app.protection.object.db'] = mock.Mock()
        from app.backup.service import backup_workflow
        self.backup_workflow = backup_workflow

    @patch("app.backup.service.backup_workflow.lock_resource_list", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.modify_job_lock_id", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.get_next_time", Mock(return_value="2021-10-01 00:00:00"))
    @patch("sqlalchemy.orm.session.sessionmaker.__init__", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.set", mock.Mock(return_value=None))
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @patch("app.common.clients.job_center_client.query_is_job_present", mock.Mock(return_value=True))
    @patch.object(ResourceClient, "sync_protection_time", Mock(return_value=None))
    @patch.object(ProtectionClient, "query_sla")
    @patch.object(ResourceClient, "query_protected_object", Mock(return_value={"earliest_time": "2021-10-01 00:00:00"}))
    @patch.object(ResourceClient, "query_resource", Mock(return_value={"name": "fake_resource"}))
    @patch("app.backup.service.backup_workflow.update_backup_policy_if_possible", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.toolkit.modify_task_log", Mock())
    def test_backup_context_initialize(self, _mock_query_sla):
        policy = {"action": "full", "schedule": schedule}
        params = {
            "policy": policy,
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "execute_type": "w",
            "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "uuid": uuid.uuid1(),
            "auto_retry": "True",
            "auto_retry_times": "8",
            "auto_retry_wait_minutes": 100,
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "job_type": "BACKUP",
            "copy_name": "fake_copy"
        }
        _mock_query_sla.return_value = "1a881ee6-69f8-4615-9b7c-da599402619d"
        initialize_backup = self.backup_workflow.backup_context_initialize(RequestMock(), **params)
        self.assertIsNone(initialize_backup)

    @patch("app.backup.service.backup_workflow.lock_resource_list", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.modify_job_lock_id", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.get_next_time", Mock(return_value="2021-10-01 00:00:00"))
    @patch("app.backup.service.backup_workflow.query_copy_count_by_resource_id", Mock(return_value=1))
    @patch("app.backup.service.backup_workflow.get_license_check_result", Mock(return_value=True))
    @patch("app.backup.service.backup_workflow.record_job_step")
    @patch("sqlalchemy.orm.session.sessionmaker.__init__", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.set", mock.Mock(return_value=None))
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @patch("app.common.clients.job_center_client.query_is_job_present", mock.Mock(return_value=True))
    @patch.object(ResourceClient, "sync_protection_time", Mock(return_value=None))
    @patch.object(ProtectionClient, "query_sla")
    @patch.object(ResourceClient, "query_protected_object",Mock(return_value={ "earliest_time": "2021-10-01 00:00:00",
                                                                               "consistent_status": "inconsistent"}))
    @patch.object(ResourceClient, "query_resource", Mock(return_value={"name": "fake_resource"}))
    @patch("app.backup.service.backup_workflow.producer.produce", Mock(return_value=None))
    def test_backup_inc_to_full_success_when_protect_object_is_in_consistence(self,
                                                                              _mock_record_job_step,
                                                                              _mock_query_sla):
        """
        用例名称：备份过程中，如果任务类型为增量备份且保护对象不完整，则增量转为全量
        前置条件：任务类型为增量备份且保护对象不完整(inconsistent)
        check点：备份成功，增量转为全量
        """
        policy = {"action": "permanent_increment", "schedule": schedule}
        protected_obj = {"consistent_status": "inconsistent"}
        sla = {"policy_list": []}
        params = {
            "policy": policy,
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "execute_type": "w",
            "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "uuid": uuid.uuid1(),
            "auto_retry": "True",
            "auto_retry_times": "8",
            "auto_retry_wait_minutes": 100,
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "job_type": "BACKUP",
            "copy_name": "fake_copy",
            "status": "success"
        }

        def side_effect(*args, **kwargs):
            if args[0] == 'policy':
                return policy
            elif args[0] == 'protected_object':
                if len(args) > 1:
                    return protected_obj
                else:
                    return json.dumps(protected_obj)
            elif args[0] == 'sla':
                return sla

        _mock_query_sla.return_value = "1a881ee6-69f8-4615-9b7c-da599402619d"
        with patch('app.backup.service.backup_workflow.Context') as mock_backup_context:
            instance = mock_backup_context.return_value
            instance.exist.return_value = ["123", "456"]
            instance.get.side_effect = side_effect
            initialize_backup = self.backup_workflow.resource_locked(RequestMock(), **params)
            self.assertIsNone(initialize_backup)
            self.assertEqual("permanent_increment", policy.get("action"))

    @patch("app.backup.service.backup_workflow.producer.produce", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.update_backup_policy_if_possible", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.get_license_check_result", Mock(return_value=True))
    @patch("app.common.event_messages.Eam.eam.BackupRequest.__init__", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.create_backup_timeout_task", Mock(return_value=None))
    @patch("app.backup.service.backup_service.backup_execute_check")
    @patch("app.common.license.validate_license_by_resource_uuid")
    @patch("app.backup.service.backup_workflow.notify_backup_fail", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.log_backup_job_lock_failed", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.set", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.backup.redis.context.Context.exist")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @patch("app.common.clients.job_center_client.query_is_job_present", mock.Mock(return_value=True))
    def test_resource_locked(self, _mock_ctx_exist, _mock_ctx_get, _mock_valid_lic,
                             _mock_bak_exec_chk):
        fake_resource_id = "1a881ee6-69f8-4615-9b7c-da599402619d"
        fake_ctx_map = {
            "resource_id": fake_resource_id,
            "protected_object": ""
        }
        _mock_ctx_exist.return_value = fake_ctx_map
        fake_ctx_map2 = {
            "resource_id": fake_resource_id,
            "protected_object": "{\"resource_id\": \"1a881ee6-69f8-4615-9b7c-da599402619d\"}",
            "policy": {"action": ""},
            "execute_type": "",
            "sla": {},
            "chain_id": ""
        }
        _mock_ctx_get.side_effect = lambda x, t="": fake_ctx_map2.get(x)
        error_desc = "1a881ee6-69f8-4615-9b7c-da599402619d"
        status = "false"
        resource_locked = self.backup_workflow.resource_locked(RequestMock(), error_desc=error_desc, status=status)
        self.assertIsNone(resource_locked)

        # status为“success”
        # copy_query_service.check_max_backup_copies_number结果为True
        status = "success"
        resource_locked = self.backup_workflow.resource_locked(RequestMock(), error_desc=error_desc, status=status)
        self.assertIsNone(resource_locked)

        # copy_query_service.check_max_backup_copies_number结果为True
        # license.validate_license_by_resource_uuid结果为False
        _mock_valid_lic.return_value = False
        ret = self.backup_workflow.resource_locked(RequestMock(), error_desc=error_desc, status=status)
        self.assertIsNone(ret)

        # license.validate_license_by_resource_uuid结果为True
        # backup_service.backup_execute_check结果为False
        _mock_valid_lic.return_value = True
        _mock_bak_exec_chk.return_value = False
        ret = self.backup_workflow.resource_locked(RequestMock(), error_desc=error_desc, status=status)
        self.assertIsNone(ret)

        # backup_service.backup_execute_check结果为True
        _mock_bak_exec_chk.return_value = True
        ret = self.backup_workflow.resource_locked(RequestMock(), error_desc=error_desc, status=status)
        self.assertIsNone(ret)

    @patch("app.backup.redis.context.Context.delete_all", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.backup_retry", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.backup_complete_notify", Mock(return_value=None))
    @patch("app.backup.service.backup_service.update_compliance", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.unlock_resource", Mock(return_value=None))
    @patch.object(JobClient, "update_job", Mock(return_value=None))
    @patch.object(AntiRansomwareClient, "query_policy_by_resource_id", Mock(return_value={
        "schedule": {"setWorm": True}
    }))
    @patch("app.backup.service.backup_workflow.alarm_after_failure", Mock(return_value=None))
    @patch("app.backup.service.backup_workflow.query_copy_by_id")
    @patch("app.backup.service.backup_workflow.backup_service.projected_object.update_by_params")
    @patch("app.backup.redis.context.Context.exist")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.common.clients.job_center_client.query_is_job_present", mock.Mock(return_value=True))
    def test_backup_done(self, _mock_ctx_get, _mock_ctx_exist, _mock_update, _mock_copy):
        """
        测试备份完成逻辑处理, 如果上下文已经删除，说明已经处理过，无需重复处理

        期望：
        1.如果上下文不存在，说明是重复消息，已经清理过上下文，不必再次进行备份完成逻辑处理
        2.如果上下文存在，需要进行备份逻辑处理
        :return:
        """
        policy = {"action": "full", "schedule": schedule}
        advanced_map_str = "{\"proxy_id\": null, \"host\": null, \"name\": null, \"port\": null, \"pre_script\": null, \"post_script\": null, \"all_disk\": true, \"disk_info\": [\"6000c29b-abae-2a0d-2a4e-3327350523cb\", \"6000c293-2799-3255-e555-2ba52131dab4\", \"6000c29d-3265-ecb7-0102-fcfea1a6ba62\"]}";
        fake_ctx_map = {
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "resource": {"root_uuid": "1a881ee6-69f8-4615-9b7c-da599402619d"},
            "resource_name": "",
            "policy": policy,
            "time_window_start": f"{time.time()}",
            "time_window_end": f"{time.time()}",
            "ext_parameters": advanced_map_str
        }
        obj = Mock()
        obj.properties = "{\"format\": 1}"
        _mock_copy.return_value = obj
        _mock_ctx_exist.return_value = fake_ctx_map
        _mock_ctx_get.return_value = policy

        request = RequestMock()
        copy_ids = "785f6333-c826-4515-bee4-0f5ac0243852"
        job_id = "1a881ee6-69f8-4615-9b7c-da599402619d"
        status = 1 # 成功
        backup_done = self.backup_workflow.backup_done(request, copy_ids, job_id, status=status)
        self.assertIsNone(backup_done)
        _mock_update.assert_called()
        _mock_update.reset_mock()

        status = 0 # 失败
        backup_done = self.backup_workflow.backup_done(request, copy_ids, job_id, status=status)
        self.assertIsNone(backup_done)
        self.assertTrue(_mock_update.not_called)

    @patch("app.backup.service.backup_workflow.send_verify_copy")
    @patch("app.backup.service.backup_workflow.query_copy_by_id")
    @patch.object(ResourceClient, "query_protected_object", Mock(return_value={}))
    def test_notify_verify_copy_success(self, mock_query_copy, mock_send_verify_copy):
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy_info = CopyTable(uuid='21d07c5d-f647-4d39-9d8d-08c7c86678f4',
                  generated_by='Backup',
                  backup_type="full",
                  properties="{\"format\": 0,\"verifyStatus\": 0}",
                  resource_type='VM',
                  resource_sub_type='vim.VirtualMachine',
                  resource_location='8.40.97.171', resource_status='EXIST')
        mock_query_copy.return_value = copy_info
        mock_send_verify_copy.return_value = None
        copy_ids = ["785f6333-c826-4515-bee4-0f5ac0243852"]
        user_id = "1a881ee6-69f8-4615-9b7c-da599402619d"
        res_id = "1a881ee6-69f8-4615-9b7c-da5994026191"
        policy = {'name': 'no_j','ext_parameters': {'copy_verify': True}}
        self.backup_workflow.notify_verify_copy(copy_ids, user_id, policy, res_id)
        self.assertEqual(mock_send_verify_copy.call_count, 1)

    @patch("app.backup.service.backup_workflow.send_verify_copy")
    @patch("app.backup.service.backup_workflow.query_copy_by_id")
    @patch.object(ResourceClient, "query_protected_object", Mock(return_value={}))
    def test_notify_verify_copy_fail(self, mock_query_copy, mock_send_verify_copy):
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy_info = CopyTable(uuid='21d07c5d-f647-4d39-9d8d-08c7c86678f4',
                  generated_by='Backup',
                  backup_type="full",
                  properties="{\"format\": 0,\"verifyStatus\": 0}",
                  resource_type='VM',
                  resource_sub_type='vim.VirtualMachine',
                  resource_location='8.40.97.171', resource_status='EXIST')
        mock_query_copy.return_value = copy_info
        copy_ids = ["785f6333-c826-4515-bee4-0f5ac0243852"]
        user_id = "1a881ee6-69f8-4615-9b7c-da599402619d"
        res_id = "1a881ee6-69f8-4615-9b7c-da5994026191"
        policy = {'name': 'no_j','ext_parameters': {'copy_verify': False}}
        self.backup_workflow.notify_verify_copy(copy_ids, user_id, policy, res_id)
        self.assertEqual(0, mock_send_verify_copy.call_count)

    @patch.object(ResourceClient, "update_protected_object_compliance", Mock())
    @patch.object(EventClient, "send_running_event", Mock())
    @patch.object(ResourceClient, "query_resource", Mock(return_value={"user_id": "88a94c476f12a21e016f12a246e50010"}))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.backup.redis.context.Context.exist")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @patch("app.common.clients.job_center_client.query_is_job_present", mock.Mock(return_value=True))
    def test_backup_timeout_check(self, _mock_ctx_exist, _mock_ctx_get):
        """
        用例场景：备份超出时间窗
        前置条件：安全一体机/非安全一体机
        检点点：功能正常，已调用发送告警接口
        :return:
        """
        request = RequestMock()
        params = {
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
        }
        fake_ctx_map = {
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "resource": {"root_uuid": "1a881ee6-69f8-4615-9b7c-da599402619d"},
            "resource_name": "",
            "time_window_start": f"{time.time()}",
            "time_window_end": f"{time.time()}",
            "policy": {"alarm_over_time_window": False}
        }
        _mock_ctx_exist.return_value = fake_ctx_map
        _mock_ctx_get.side_effect = lambda x, t="": fake_ctx_map.get(x)
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        backup_timeout_check = self.backup_workflow.backup_timeout_check(request, **params)
        self.assertIsNone(backup_timeout_check)
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        backup_timeout_check = self.backup_workflow.backup_timeout_check(request, **params)
        self.assertIsNone(backup_timeout_check)
        self.assertEqual(EventClient.send_running_event.call_count, 2)

    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=HTTPResponse(status=200, body=b'root_uuid')))
    @patch.object(ResourceClient, "query_protected_object", Mock(return_value={"earliest_time": "2021-10-01 00:00:00"}))
    @patch.object(ResourceClient, "query_resource", Mock(return_value={"name": "fake_resource"}))
    @patch.object(JobClient, "create_job", Mock(return_value="123"))
    @patch("app.backup.service.backup_workflow.get_job_payload_queue_scope", Mock(return_value={}))
    @patch("app.backup.service.backup_workflow.backup_service.backup_pre_check", Mock(return_value=True))
    @patch("app.resource.service.common.domain_resource_object_service.get_domain_id_list", Mock(return_value=['test']))
    @patch("app.backup.client.protection_client.ProtectionClient.query_sla", Mock(return_value={}))
    @patch("app.backup.service.backup_workflow._get_sla_id_from_policy", Mock(return_value='test'))
    def test_backup_start(self):
        """
        测试备份任务检查及启动

        期望：
        1.备份任务检查完成，启动
        :return: None
        """
        policy = {"action": "full", "schedule": schedule, "ext_parameters": {"source_deduplication": False}}
        params = {
            "policy": policy,
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "execute_type": "w",
            "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "uuid": uuid.uuid1(),
            "auto_retry": "True",
            "auto_retry_times": "8",
            "auto_retry_wait_minutes": 100,
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d"
        }
        from app.common.deploy_type import DeployType
        DeployType.is_hyper_detect_deploy_type = Mock(return_value=None)
        request = RequestMock()
        backup_start = self.backup_workflow.backup_start(request, **params)
        self.assertEquals(backup_start, "123")

    @mock.patch.object(SystemBaseHttpsClient, "request", Mock(return_value=HTTPResponse(status=200, body=b'root_uuid')))
    @mock.patch.object(InfrastructureHttpsClient, "request", Mock(return_value=HTTPResponse(status=200)))
    @patch.object(ResourceClient, "query_protected_object",
                  Mock(return_value={"status": 1, "earliest_time": "2021-10-01 00:00:00"}))
    @patch.object(ResourceClient, "query_resource", Mock(return_value={"name": "fake_resource"}))
    @patch.object(ArchiveClient, "query_storage_info", Mock(return_value={}))
    @patch("app.backup.service.backup_workflow.get_job_payload_queue_scope", Mock(return_value={}))
    @patch("app.backup.service.backup_service.backup_pre_check", Mock(return_value=True))
    @patch.object(JobClient, "create_job")
    @patch("app.resource.service.common.domain_resource_object_service.get_domain_id_list", Mock(return_value=['test']))
    @patch("app.backup.client.protection_client.ProtectionClient.query_sla", Mock(return_value={}))
    @patch("app.backup.service.backup_workflow._get_sla_id_from_policy", Mock(return_value='test'))
    def test_backup_start_success_when_backup_storage_exits(self, _mock_create):
        """
        当存在备份存储时，测试备份任务检查及启动

        期望：
        1.备份任务检查完成，启动
        :return: None
        """
        job_id = str(uuid.uuid4())
        # _mock_create_backup_job.return_value = job_id
        policy = {"action": "full", "schedule": schedule, "ext_parameters": {"storage_id": uuid.uuid4()}}
        params = {
            "policy": policy,
            "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "execute_type": "w",
            "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "uuid": uuid.uuid1(),
            "auto_retry": "True",
            "auto_retry_times": "8",
            "auto_retry_wait_minutes": 100,
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d"
        }
        request = RequestMock()
        _mock_create.return_value = job_id
        ret = self.backup_workflow.backup_start(request, **params)
        self.assertEqual(ret, job_id)

    @patch("app.backup.service.backup_workflow.query_copy_count_by_resource_id", Mock(return_value=1))
    def test_should_skip_update_backup_policy_success_given_log_backup_and_Oracle_type(self):
        """
        用例名称：备份过程中，如果为Oracle日志备份，则日志备份不转全量
        前置条件：前一次全量备份失败，执行Oracle日志备份
        check点：备份成功，日志备份不转全量
        """
        request_id = str(uuid.uuid4())
        policies = [{"action": "log", "schedule": schedule}]
        protected_object = {
            "sub_type": "Oracle",
        }
        result = self.backup_workflow.update_backup_policy_if_possible(request_id, policies, protected_object, {})
        self.assertEqual("", result)
        self.assertEqual(policies[0].get("action"), "log")

    @patch("app.backup.service.backup_workflow.query_copy_count_by_resource_id", Mock(return_value=1))
    @patch("app.backup.service.backup_workflow.toolkit.modify_task_log", Mock())
    @patch.object(ResourceClient, "query_next_backup_type_and_cause",
                  Mock(return_value={"next_backup_type": "full"}))
    def test_should_skip_update_backup_policy_success_when_is_consistency(self):
        """
        用例名称：备份过程中，如果为Oracle日志备份，则日志备份不转全量
        前置条件：前一次全量备份失败，执行Oracle日志备份
        check点：备份成功，日志备份不转全量
        """
        request_id = str(uuid.uuid4())
        policies = [{"action": "permanent_increment", "schedule": schedule}]
        protected_object = {
            "sub_type": "FileSet",
            "consistent_status": "123"
        }
        result = self.backup_workflow.update_backup_policy_if_possible(request_id, policies, protected_object,
                                                                       {"policy_list": []})
        self.assertEqual("by_next_backup", result)

    def test_get_next_backup_ext_parameter_None(self):
        """
        用例场景：根据资源ID查询下次备份的扩展参数
        前置条件：资源存在
        检查点：资源参数为空的时候，去保护对象中取
        """
        protected_obj = {"resource_id": "ed6191a9-7d4b-53e6-82ea-d305647c0ad",
                         "ext_parameters": {"next_backup_type": "sda"}}
        ResourceClient.query_next_backup_type_and_cause = Mock(return_value=None)
        result = self.backup_workflow.get_next_backup_ext_parameter(protected_obj)
        self.assertEqual(ResourceClient.query_next_backup_type_and_cause.call_count, 1)
        ext_parameters = {"next_backup_type": "sda"}
        self.assertEqual(ext_parameters, result)

    def test_get_next_backup_ext_parameter_not_None(self):
        """
        用例场景：根据资源ID查询下次备份的扩展参数
        前置条件：资源存在
        检查点：资源参数不为空的时候，直接返回资源参数
        """
        protected_obj = {"resource_id": "ed6191a9-7d4b-53e6-82ea-d305647c0ad"}
        ext_parameter = {"next_backup_type": "sda"}
        ResourceClient.query_next_backup_type_and_cause = Mock(return_value=ext_parameter)
        result = self.backup_workflow.get_next_backup_ext_parameter(protected_obj)
        self.assertEqual(ResourceClient.query_next_backup_type_and_cause.call_count, 1)
        self.assertEqual(ext_parameter, result)

    @patch("app.backup.service.backup_workflow.check_need_retry", Mock(return_value=True))
    @patch.object(SchedulerClient, "create_delay_schedule", Mock())
    def test_backup_retry(self):
        """
        用例场景：备份失败重试
        前置条件：备份失败
        检查点：
        """
        get_list = {
            'resource_id': '123',
            'sla_id': '123',
            'chain_id': '123',
            'policy': {},
            'execute_type': "backup",
            'retry_times': "20",
            'wait_minutes': "20",
        }

        def side_effect(*args, **kwargs):
            return get_list.get(args[0])
        request_id = "123456789"
        context = Mock()
        context.get.side_effect = side_effect
        result = self.backup_workflow.backup_retry(request_id, context, JobStatus.FAIL)
        self.assertIsNone(result)


if __name__ == '__main__':
    unittest.main(verbosity=2)
