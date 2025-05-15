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
import datetime
import sys
import time
import unittest
import uuid
from collections import namedtuple
from http import HTTPStatus
from unittest import mock
from unittest.mock import Mock, patch

from urllib3 import HTTPResponse

from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.enums.copy_enum import GenerationType
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.clients import client_util
from app.common.security import kmc_util
from app.common.security.kmc_util import Kmc
from app.restore.client.copy_client import CopyClient
from app.common.clients.protection_client import ProtectionClient
from tests.test_cases import common_mocker  # noqa
from app.backup.client.resource_client import ResourceClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.enums.job_enum import JobStatus
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.schedule_enum import ExecuteType
from app.common.enums.sla_enum import BackupTypeEnum
from app.protection.object.client import dee_client
from tests.test_cases.backup.common.context import mock_context  # noqa

sys.modules['app.common.logger'] = mock.Mock()


class BackupServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        sys.modules['app.protection.object.db'] = mock.Mock()
        from app.backup.service import backup_service
        self.backup_service = backup_service

    def tearDown(self) -> None:
        super(BackupServiceTest, self).tearDown()
        del sys.modules["app.protection.object.db"]

    def test_get_end_of_time_window(self):
        schedule = {"window_start": "20:12:10", "window_end": "21:12:10"}
        policy = {"schedule": schedule}
        get_end_of_time_window = self.backup_service.get_end_of_time_window(policy=policy)
        self.assertEqual(len(get_end_of_time_window), 2)
        self.assertTrue(get_end_of_time_window[1] > get_end_of_time_window[0])

    def test_should_return_false_if_action_is_log_when_check_need_delay_schedule(self):
        policy = {"action": "log"}
        ret = self.backup_service.check_need_delay_schedule(policy)
        self.assertFalse(ret)

    @mock.patch("app.backup.service.backup_service.get_end_of_time_window")
    def test_should_return_true_if_time_window_not_equal_one_day_when_check_need_delay_schedule(
            self, _mock_get_end_of_time_window):
        start_time = datetime.datetime(2022, 1, 1, 0, 0, 0)
        end_time = datetime.datetime(2022, 1, 1, 6, 0, 0)
        _mock_get_end_of_time_window.return_value = (start_time, end_time)
        policy = {"action": "full"}
        ret = self.backup_service.check_need_delay_schedule(policy)
        self.assertTrue(ret)

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    def test_backup_pre_check(self):
        request_id = None
        execute_type = "MANUAL"
        schedule = {"window_end": "21:12:10", "window_start": "21:12:10"}
        policy = {"action": "full", "schedule": schedule}
        protected_obj = {"status": 1}
        resource_obj = {"sub_type": 1}
        is_first_backup = {"sub_type": 1}
        pre_backup_check = self.backup_service.backup_pre_check(request_id, execute_type, policy, protected_obj,
                                                                resource_obj, is_first_backup)
        self.assertTrue(pre_backup_check)

    def test_get_next_execute_time(self):
        schedule = {"interval": 2, "interval_unit": "d", "trigger": 1}
        policy = {"schedule": schedule}
        get_next_execute_time = self.backup_service.get_next_execute_time(policy=policy)
        self.assertIsNotNone(get_next_execute_time)

    def test_should_return_true_when_check_nfs(self):
        ret = self.backup_service.check_nfs({})
        self.assertTrue(ret)

    def test_should_return_true_when_check_vmware(self):
        ret = self.backup_service.check_vmware({})
        self.assertTrue(ret)

    def test_should_return_true_if_linke_status_is_0_when_check_oracle(self):
        resource_obj = {"link_status": "0"}
        ret = self.backup_service.check_nfs(resource_obj)
        self.assertTrue(ret)

    @mock.patch("app.backup.service.backup_service.modify_task_log", mock.Mock)
    @mock.patch.object(ResourceClient, "query_resource")
    @mock.patch("app.backup.service.backup_service.time.sleep", mock.Mock)
    def test_should_return_false_if_vm_and_vcenter_offline_when_backup_execute_check(self, _mock_query_res):
        uuid_1 = "539c8372-31c0-49b8-9f9b-9f51c770b6c1"
        uuid_2 = "6b728b23-e4cb-43d3-aaef-06ae5e83c608"
        fake_res_infos = {
            uuid_1: {"uuid": uuid_1, "sub_type": ResourceSubTypeEnum.VirtualMachine.value, "root_uuid": uuid_2},
            uuid_2: {"uuid": uuid_2, "sub_type": ResourceSubTypeEnum.Datacenter.value, "root_uuid": uuid_2,
                     "link_status": 0}
        }
        _mock_query_res.side_effect = lambda x: fake_res_infos.get(x)
        ret = self.backup_service.backup_execute_check(str(uuid.uuid4()), str(uuid.uuid4()), uuid_1)
        self.assertFalse(ret)

    @mock.patch.object(ResourceClient, "query_resource")
    def test_should_return_true_if_res_sub_type_not_vm_when_backup_execute_check(self, _mock_query_res):
        _mock_query_res.return_value = {"sub_type": ResourceSubTypeEnum.HostSystem.value}
        ret = self.backup_service.backup_execute_check(str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()))
        self.assertTrue(ret)

    @mock.patch("app.backup.redis.context.Context.exist", mock.Mock(return_value={}))
    @mock.patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    def test_should_return_none_if_ctx_not_exist_when_update_compliance(self):
        ret = self.backup_service.update_compliance(str(uuid.uuid4()), JobStatus.terminated)
        self.assertIsNone(ret)

    @mock.patch.object(ResourceClient, "update_protected_object_compliance", mock.Mock)
    @mock.patch("app.backup.redis.context.Context.get")
    @mock.patch("app.backup.redis.context.Context.exist")
    @mock.patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    def test_should_return_none_if_ctx_exist_when_update_compliance(self, _mock_ctx_exist, _mock_ctx_get):
        fake_ctx_map = {
            "resource_id": str(uuid.uuid4()),
            "execute_type": ExecuteType.MANUAL.value,
            "timeout_time": f"{time.time()}",
            "first_backup": "True",
        }
        _mock_ctx_exist.return_value = fake_ctx_map
        _mock_ctx_get.side_effect = lambda x: fake_ctx_map.get(x)
        # execute_type为”MANUAL“
        ret = self.backup_service.update_compliance(str(uuid.uuid4()), JobStatus.terminated)
        self.assertIsNone(ret)

        # (1)execute_type不为”MANUAL“
        # (2)first_backup为“True”且compliance为False
        fake_ctx_map["execute_type"] = ExecuteType.AUTOMATIC.value
        ret = self.backup_service.update_compliance(str(uuid.uuid4()), JobStatus.terminated)
        self.assertIsNone(ret)

        # (1)execute_type不为”MANUAL“
        # (2)first_backup不为“True”
        fake_ctx_map["first_backup"] = 'False'
        ret = self.backup_service.update_compliance(str(uuid.uuid4()), JobStatus.SUCCESS)
        self.assertIsNone(ret)

    @mock.patch.object(ResourceClient, "is_support_data_and_log_parallel_backup", Mock(return_value=False))
    @mock.patch.object(ResourceClient, "query_custom_resource", Mock(return_value=[]))
    def test_get_backup_lock_resource_ids_should_not_return_copy_id_when_backup_is_permanent_increment(self):
        """
        验证场景：验证备份类型为永久增量时，获取的资源锁只包含资源id
        前置条件：无
        验证点：1.锁定资源列表元素为1  2.锁定资源id与给定资源id一致
        """
        # Given
        resource_id = str(uuid.uuid4())
        policy = {
            "action": BackupTypeEnum.permanent_increment.name
        }
        # When
        lock_resource_ids = self.backup_service.get_backup_lock_resource_ids(resource_id, policy,
                                                                             ResourceSubTypeEnum.HBaseBackupSet.value)
        # Then
        self.assertIs(len(lock_resource_ids), 1)
        self.assertIn(resource_id, [lock_resource.get("id") for lock_resource in lock_resource_ids])

    @mock.patch.object(ResourceClient, "is_support_data_and_log_parallel_backup", Mock(return_value=True))
    @mock.patch.object(ResourceClient, "query_custom_resource", Mock(return_value=[]))
    def test_get_backup_lock_resource_ids_should_not_return_copy_id_when_backup_is_full(self):
        """
        验证场景：验证备份类型为全量时，获取的资源锁只包含资源id
        前置条件：无
        验证点：1.锁定资源列表元素为1  2.锁定资源id与给定资源id一致
        """
        # Given
        resource_id = str(uuid.uuid4())
        policy = {
            "action": BackupTypeEnum.log.name
        }
        # When
        lock_resource_ids = self.backup_service.get_backup_lock_resource_ids(resource_id, policy,
                                                                             ResourceSubTypeEnum.Oracle.value)
        # Then
        self.assertIs(len(lock_resource_ids), 1)
        self.assertIn(resource_id + "@log", [lock_resource.get("id") for lock_resource in lock_resource_ids])

    @mock.patch.object(SystemBaseClient, "query_filesystem")
    @mock.patch.object(SystemBaseClient, "query_hyper_metro_pair", Mock(return_value=None))
    def test_has_primary_hyper_metro_pair_filesystem_name_hyper_detect(self, _mock_query_fs):
        """
        验证场景：防勒索部分部署形态下获取双活域从端名称
        前置条件：无
        验证点：1.验证名称
        """
        from app.backup.client.resource_client import ResourceClient
        protected_obj = {"ext_parameters": {"file_system_ids": [256]}}
        ResourceClient.query_protected_object = Mock(return_value=protected_obj)
        filesystem_info = {"hyperMetroPairIds": ["123"]}
        _mock_query_fs.return_value = filesystem_info
        from app.common.deploy_type import DeployType
        DeployType.is_hyper_detect_deploy_type = Mock(return_value=True)
        self.assertEqual(self.backup_service.check_hyper_metro_pair("222"), "")

    @mock.patch.object(SystemBaseClient, "query_filesystem")
    def test_has_smart_mobility_hyper_detect(self, _mock_query_fs):
        """
        验证场景：防勒索部分部署形态下获取smart mobility
        前置条件：无
        验证点：1.验证名称
        """
        from app.backup.client.resource_client import ResourceClient
        protected_obj = {"ext_parameters": {"file_system_ids": [256]}}
        ResourceClient.query_protected_object = Mock(return_value=protected_obj)
        filesystem_info = {"hasSmartMobility": "1"}
        _mock_query_fs.return_value = filesystem_info
        from app.common.deploy_type import DeployType
        DeployType.is_hyper_detect_deploy_type = Mock(return_value=True)
        self.assertEqual(self.backup_service.check_smart_mobility("222"), "")

    @mock.patch.object(SystemBaseClient, "query_remote_storage_filesystem")
    @mock.patch.object(SystemBaseClient, "query_remote_storage_hyper_metro_pair", Mock(return_value=None))
    def test_has_primary_hyper_metro_pair_filesystem_name_cyber_engine(self, _mock_query_fs):
        """
        验证场景：安全一体机部署形态下获取双活域从端名称
        前置条件：无
        验证点：1.验证名称
        """
        from app.backup.client.resource_client import ResourceClient
        resource = {"ext_parameters": {"file_system_ids": [257]}, "root_uuid": "device1"}
        ResourceClient.query_resource = Mock(return_value=resource)
        filesystem_info = {"hyperMetroPairIds": ["124"]}
        _mock_query_fs.return_value = filesystem_info
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        self.assertEqual(self.backup_service.check_cyber_engine_metro_pair("222"), "")

    @mock.patch.object(dee_client, "get_replication_pair_by_id")
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    @mock.patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_check_cyber_engine_replication_pair_success(self, _mock_query_replication):
        """
        验证场景：安全一体机部署形态下校验基于远程租户pair的远程复制从端名称
        前置条件：无
        验证点：1.验证名称
        """
        from app.backup.client.resource_client import ResourceClient
        resource = {"ext_parameters": {"file_system_ids": [0]}, "root_uuid": "device1", "parent_uuid": "v1"}
        ResourceClient.query_resource = Mock(return_value=resource)
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        dee_client.DataEnableEngineHttpsClient.request = Mock(return_value=HTTPResponse(status=200))
        replication_pair_info = {"ISPRIMARY": True, "VSTOREPAIRID": "----"}
        _mock_query_replication.return_value = replication_pair_info

        self.assertEqual(self.backup_service.check_cyber_engine_replication_pair("0"), "")

    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    def test_backup_complete_protect_object_success(self):
        context = {"policy": {}, "resource_id": "123", "cause_of_change": "by_next_backup",
                   "protected_object": {"resource_id": "123"}}
        ResourceClient.clean_next_backup = Mock(return_value=None)
        self.backup_service.backup_complete_protect_object(context, JobStatus.SUCCESS)
        self.assertEqual(ResourceClient.clean_next_backup.call_count, 1)

    def test_clean_next_backup_info_success(self):
        protect_obj = {"resource_id": "123"}
        ResourceClient.clean_next_backup = Mock(return_value=None)
        self.backup_service.clean_next_backup_info(protect_obj)
        self.assertEqual(ResourceClient.clean_next_backup.call_count, 1)

    def test_clean_next_backup_info_in_protect_obj_success(self):
        protect_obj = {"resource_id": "123"}
        self.backup_service.clean_next_backup_info_in_protect_obj(protect_obj)
        self.assertIsNotNone(protect_obj)

    @patch.object(CopyClient, "query_copy_info", autospec=True)
    def test_get_difference_increment_copy_ids_success(self, _mock_query):
        last_copy = {"generated_by": GenerationType.BY_BACKUP.value, "uuid": "123456", "prev_copy_id": "123456"}
        _mock_query.return_value = {"backup_type": BackupTypeEnum.full.value, "uuid": "123456"}
        id_list = self.backup_service.get_difference_increment_copy_ids(last_copy)
        self.assertEqual(2, len(id_list))

    @patch.object(CopyClient, "query_copies", autospec=True)
    def test_query_latest_copy_by_resource_id_fail(self, _mock_query):
        _mock_query.return_value = None
        with self.assertRaises(EmeiStorBizException):
            self.backup_service.query_latest_copy_by_resource_id("123")

    @patch.object(CopyClient, "query_copies", autospec=True)
    def test_query_latest_copy_by_resource_id_success(self, _mock_query):
        _mock_query.return_value = {"items": []}
        res = self.backup_service.query_latest_copy_by_resource_id("123")
        self.assertIsNone(res)


if __name__ == '__main__':
    unittest.main(verbosity=2)
