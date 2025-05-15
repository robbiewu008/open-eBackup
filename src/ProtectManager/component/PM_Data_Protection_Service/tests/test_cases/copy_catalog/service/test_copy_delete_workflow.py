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
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, MagicMock

from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobStatus
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.common.exception.common_error_codes import CommonErrorCodes
from tests.test_cases import common_mocker # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()

from app.copy_catalog.service.copy_delete_workflow import request_delete_copy, CopyDeleteParam, \
    check_exist_copy_delete_job, handle_protection_removed, is_task_finished, handle_copy_delete_locked, \
    process_copy_delete_context_init
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.models.tables_and_sessions import CopyAntiRansomwareTable, CopyTable

from app.copy_catalog.service import copy_delete_workflow


copy_anti_ransomware_table = CopyAntiRansomwareTable(
    copy_id=str(uuid.uuid4()),
    status=AntiRansomwareEnum.UNDETECTED
)

copy_delete_param = CopyDeleteParam(user_id="user_id", strict=True)

delete_copy = CopyTable(uuid=str(uuid.uuid4()), properties=json.dumps({"isMemberDeleted": "true"}), gn=1,
                        resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))

class CopyDeleteWorkFlow(unittest.TestCase):
    def setUp(self) -> None:
        self.copy_delete_workflow = copy_delete_workflow
        self.request_delete_copy = request_delete_copy
        self.check_exist_copy_delete_job = check_exist_copy_delete_job

    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("app.copy_catalog.util.copy_util.check_associated_copies_can_be_deleted")
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none", Mock(return_value=copy_anti_ransomware_table))
    @mock.patch("app.copy_catalog.util.copy_util.check_copy_browse_status", Mock(return_value=False))
    def test_copy_exist_clone_file_system(self, can_be_deleted, mock_query_first):
        """
                删除存在克隆文件系统的副本
                期望：
                删除失败
                :return:
        """
        copy = construct_copy_by_generated(ResourceSubTypeEnum.VirtualMachine.value)
        copy.properties = '{"isMemberDeleted":"False"}'
        mock_query_first.return_value = copy
        DeployType.is_ocean_protect_type = MagicMock(return_value=False)
        DeployType.is_cloud_backup_type = MagicMock(return_value=False)
        DeployType.is_not_support_dee_restful_deploy_type = MagicMock(return_value=False)
        self.assertRaises(EmeiStorBizException, self.request_delete_copy, copy, copy_delete_param)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none", Mock(return_value=copy_anti_ransomware_table))
    def test_import_copy_exist_clone_file_system(self, mock_query_first):
        import_copy = construct_copy_by_generated(ResourceSubTypeEnum.ImportCopy.value)
        mock_query_first.return_value = import_copy
        self.assertRaises(Exception, self.request_delete_copy, import_copy, "user_id", strict=True)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    @mock.patch("app.copy_catalog.util.copy_util.check_associated_copies_can_be_deleted")
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted",
                Mock(return_value=(None, None)))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_delete_job", Mock(return_value=None))
    def test_copy_not_exist_clone_file_system(self, mock_query_first, mock_one_or_none, can_be_deleted):
        """
                删除不存在克隆文件系统的副本
                期望：
                删除继续执行
                :return:
        """
        copy = construct_copy_by_generated(ResourceSubTypeEnum.ImportCopy.value)
        copy.resource_sub_type = ResourceSubTypeEnum.Fileset
        copy.generated_by = GenerationType.BY_CLOUD_ARCHIVE.value
        copy.properties = '{"isMemberDeleted":"False"}'
        DeployType.is_ocean_protect_type = MagicMock(return_value=False)
        mock_query_first.return_value = None
        mock_copy_delete_workflow = MagicMock()
        copy_delete_workflow.create_delete_copy_job = mock_copy_delete_workflow
        copy_delete_workflow.request_delete_copy(copy, copy_delete_param)
        mock_copy_delete_workflow.assert_called_once()

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted", Mock(return_value=["", ""]))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_exist_clone_file_system",
                Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_job", Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.create_delete_copy_job")
    def test_delete_copy_generated_by_tape_archive(self, mock_delete_copy_job):
        """
                删除VMware虚拟机的归档副本
                期望：最终下发副本过期job，无异常抛出
                :return:
        """
        copy = construct_copy_by_generated(ResourceSubTypeEnum.VirtualMachine)
        copy.status = CopyStatus.NORMAL
        copy.generated_by = GenerationType.BY_TAPE_ARCHIVE.value
        copy.properties = '{"isMemberDeleted":"False"}'
        self.request_delete_copy(copy, copy_delete_param)
        assert mock_delete_copy_job.call_args[0][0]['jobType'] == 'COPY_EXPIRE'

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted", Mock(return_value=["", ""]))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_exist_clone_file_system",
                Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_job", Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.create_delete_copy_job")
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy",
                Mock(return_value="last_copy_uuid"))
    def test_delete_copy_generated_by_backup(self, mock_delete_copy_job):
        """
                删除HDFS文件集的备份副本
                期望：最终下发副本删除job，无异常抛出
                :return:
        """
        copy = construct_copy_by_generated(ResourceSubTypeEnum.HDFSFileset)
        copy.status = CopyStatus.NORMAL
        copy.generated_by = GenerationType.BY_BACKUP.value
        copy.properties = '{"isMemberDeleted":"False"}'
        self.request_delete_copy(copy, copy_delete_param)
        assert mock_delete_copy_job.call_args[0][0]['jobType'] == 'COPY_DELETE'

    @mock.patch("app.copy_catalog.util.copy_util.query_job_list")
    def test_should_raise_EmeiStorBizException_when_exist_copy_expire_job(self, mock_query_job_list):
        """
        用例场景：删除副本
        前置条件：已有副本删除任务
        检查点：已有删除任务，不重复下发删除任务
        """
        copy_id = str(uuid.uuid4())
        expire_job_response = {
            "totalCount": 1,
        }
        mock_query_job_list.return_value = json.dumps(expire_job_response)
        running_job_status = [JobStatus.READY.value, JobStatus.PENDING.value, JobStatus.RUNNING.value,
                              JobStatus.ABORTING.value]
        with self.assertRaises(EmeiStorBizException) as ex:
            self.check_exist_copy_delete_job(copy_id, running_job_status)
        self.assertEqual(ex.exception._error_code, CopyErrorCode.EXIST_COPY_DELETE_JOB['code'])


    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy",
                Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_copy_by_id", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_leftover_copy", Mock(return_value=mock.Mock()))
    def test_handle_protection_removed_when_latest_copy_is_not_None(self):
        """
        用例场景：删除副本
        前置条件：存在副本
        返回：运行不报错
        """
        result = handle_protection_removed("123", "123")
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy", Mock(return_value=None))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_copy_by_id", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_leftover_copy", Mock(return_value=mock.Mock()))
    def test_handle_protection_removed_when_latest_copy_id_is_None(self):
        """
        用例场景：删除副本
        前置条件：last copy的id为None
        返回：直接返回，不报错
        """
        result = handle_protection_removed("123", "123")
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy",
                Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_copy_by_id", Mock(return_value=None))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_leftover_copy", Mock(return_value=mock.Mock()))
    def test_handle_protection_removed_when_latest_copy_is_None(self):
        """
        用例场景：删除副本
        前置条件：last copy不存在
        返回：直接返回，不报错
        """
        result = handle_protection_removed("123", "123")
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_job_list", Mock(return_value=None))
    def test_is_task_finished_when_query_job_list_result_is_None(self):
        """
        查询任务是否已结束
        期望：未能根据job id查到job list，返回false
        return:False
        """
        result = is_task_finished("123")
        self.assertFalse(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_job_list", Mock(return_value="{\"totalCount\":1}"))
    def test_is_task_finished_when_job_count_is_1(self):
        """
        查询任务是否已结束
        期望：job list中job数量大于0，返回true
        return:True
        """
        result = is_task_finished("123")
        self.assertTrue(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.query_job_list", Mock(return_value="{\"totalCount\":0}"))
    def test_is_task_finished_when_job_count_is_0(self):
        """
        查询任务是否已结束
        期望：job list中job数量等于0，返回false
        return:False
        """
        result = is_task_finished("123")
        self.assertFalse(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.Context",
                Mock(return_value={"job_id":"123", "copy_id":"456"}))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.update_copy_status", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.complete_job_center_task",
                Mock(return_value=mock.Mock()))
    def test_handle_copy_delete_locked_when_job_status_is_failed(self):
        """
        测试删除资源锁
        期望：任务状态不为success，删除资源锁失败，直接返回
        return:直接返回
        """
        result = handle_copy_delete_locked("1234", "111", "failed")
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.Context",
                Mock(return_value={"job_id":"123", "copy_id":"456"}))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.update_copy_status", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.complete_job_center_task",
                Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.is_task_finished", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.lock_service.unlock", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.restore_copy_status", Mock(return_value=mock.Mock()))
    def test_handle_copy_delete_locked_when_job_status_is_success_and_job_is_finished(self):
        """
        测试删除资源锁
        期望：任务状态为success，但是任务已结束，解锁资源并返回
        return:运行不报错
        """
        result = handle_copy_delete_locked("1234", "111", "success")
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.Context",
                Mock(return_value={"job_id":"123", "copy_id":"456","job_type":"Backup"}))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.update_copy_status", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.complete_job_center_task",
                Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.is_task_finished", Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.DeleteCopyRequest", Mock(return_value=mock.Mock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.producer.produce", Mock(return_value=mock.Mock()))
    def test_handle_copy_delete_locked_when_job_status_is_success_and_job_is_not_finished(self):
        """
        测试删除资源锁
        期望：任务状态为success，任务未结束，删除资源锁，删除副本
        return:运行不报错
        """
        result = handle_copy_delete_locked("1234", "111", "success")
        self.assertEqual(None, result)


    @mock.patch("app.copy_catalog.service.copy_delete_workflow.job_center_client.query_is_job_present",
                Mock(return_value=False))
    def test_process_copy_delete_context_init_1(self):
        """
        测试删除初始化上下文
        期望：查询任务出错，直接返回
        return:直接返回
        """
        result = process_copy_delete_context_init("111","222",mock.Mock(),mock.Mock(),mock.Mock())
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.job_center_client.query_is_job_present",
                Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_should_be_stop_by_job_status",
                Mock(return_value=True))
    def test_process_copy_delete_context_init_2(self):
        """
        测试删除初始化上下文
        期望：根据任务状态应停止任务，直接返回
        return:直接返回
        """
        result = process_copy_delete_context_init("111","222",mock.Mock(),mock.Mock(),mock.Mock())
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_deleting_copy", MagicMock(return_value=delete_copy))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.request_delete_copy", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.resource_has_copy_link", MagicMock(return_value=False))
    def test_request_delete_copy_by_id(self):
        """
        测试根据副本ID删除副本
        期望：资源对应副本存在链路关系，删除这些副本
        return:运行不报错，删除副本
        """
        copy_id = str(uuid.uuid4())
        from app.copy_catalog.service.copy_delete_workflow import request_delete_copy_by_id
        result = request_delete_copy_by_id(copy_id, copy_delete_param)
        self.assertTrue(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_deleting_copy", MagicMock(return_value=delete_copy))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.request_delete_copy", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.resource_has_copy_link", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.associated_deletion_copies", MagicMock(return_value=[delete_copy, delete_copy]))
    def test_request_delete_copy_by_id_2(self):
        """
        测试根据副本ID删除副本
        期望：资源对应副本不存在链路关系，删除副本
        return:None
        """
        copy_id = str(uuid.uuid4())
        from app.copy_catalog.service.copy_delete_workflow import request_delete_copy_by_id
        result = request_delete_copy_by_id(copy_id, copy_delete_param)
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.query.Query.update")
    def test_restore_copy_status(self, _mock_update):
        """
        测试恢复删除副本状态
        期望：更新副本状态从DELETING到NORMAL
        return:运行不报错
        """
        _mock_update.return_value = None
        from app.copy_catalog.service.copy_delete_workflow import restore_copy_status
        result = restore_copy_status(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_single_copy", MagicMock(return_value=True))
    def test_request_delete_copy_when_create_job_is_False(self):
        """
        测试删除备份副本
        期望：不下发删除任务，只用删除数据库记录
        return:None
        """
        copy = construct_copy_by_generated(ResourceSubTypeEnum.VirtualMachine)
        copy.properties = '{"isMemberDeleted":"False"}'
        copy_delete_param_mock = CopyDeleteParam(user_id="user_id", strict=True, create_job=False)
        result = request_delete_copy(copy, copy_delete_param_mock)
        self.assertEqual(None, result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_single_copy", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_exist_clone_file_system", MagicMock(return_value=MagicMock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted", MagicMock(return_value=(False, False)))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy", MagicMock(return_value=None))
    def test_request_delete_copy_when_latest_copy_is_None(self):
        """
        测试删除备份副本
        期望：副本已被删除，抛出异常
        return:抛出OBJ_NOT_EXIST异常
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), parent_copy_uuid=str(uuid.uuid4()), status='Normal',
                         resource_sub_type=ResourceSubTypeEnum.VirtualMachine, generated_by='Backup')
        copy.properties = '{"isMemberDeleted":"False"}'
        copy_delete_param_mock = CopyDeleteParam(user_id="user_id", strict=True, create_job=True)
        with self.assertRaises(EmeiStorBizException) as ex:
            request_delete_copy(copy, copy_delete_param_mock)
        self.assertEqual(ex.exception.error_code, CommonErrorCodes.OBJ_NOT_EXIST['code'])

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_single_copy", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_exist_clone_file_system", MagicMock(return_value=MagicMock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted", MagicMock(return_value=(False, False)))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy", MagicMock(return_value="123"))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_delete_copy_job_type", MagicMock(return_value="COPY_DELETE"))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_delete_job", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.build_delete_copy_job_param", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.create_delete_copy_job", MagicMock(return_value=True))
    def test_request_delete_copy_when_job_type_is_COPY_DELETE(self):
        """
        测试删除备份副本
        前置条件：副本job_type为COPY_DELETE
        return:True
        """
        copy = CopyTable(uuid="123", parent_copy_uuid=str(uuid.uuid4()), status='Normal',
                         resource_sub_type=ResourceSubTypeEnum.VirtualMachine, generated_by='Backup')
        copy_delete_param_mock = CopyDeleteParam(user_id="user_id", strict=True, create_job=True)
        copy.properties = '{"isMemberDeleted":"False"}'
        result = request_delete_copy(copy, copy_delete_param_mock)
        self.assertTrue(result)

    @mock.patch("app.copy_catalog.service.copy_delete_workflow.delete_single_copy", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_exist_clone_file_system", MagicMock(return_value=MagicMock()))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_copy_can_be_deleted", MagicMock(return_value=(False, False)))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_resource_latest_copy", MagicMock(return_value="123"))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.get_delete_copy_job_type", MagicMock(return_value="BACKUP"))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_delete_job", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_exist_copy_job", MagicMock(return_value=True))
    def test_request_delete_copy_when_job_type_is_BACKUP(self):
        """
        测试删除备份副本
        前置条件：副本job_type为BACKUP，存在副本删除任务，取消创建副本删除任务
        return:None
        """
        copy = CopyTable(uuid="123", parent_copy_uuid=str(uuid.uuid4()), status='Normal',
                         resource_sub_type=ResourceSubTypeEnum.VirtualMachine, generated_by='Backup')
        copy_delete_param_mock = CopyDeleteParam(user_id="user_id", strict=True, create_job=True)
        copy.properties = '{"isMemberDeleted":"False"}'
        result = request_delete_copy(copy, copy_delete_param_mock)
        self.assertEqual(None, result)

    @mock.patch("sqlalchemy.orm.query.Query.all", MagicMock(return_value=[delete_copy]))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.request_delete_copy", MagicMock(return_value=True))
    def test_delete_leftover_copy(self):
        """
        测试删除剩余副本
        前置条件：存在剩余副本
        return:运行不报错
        """
        from app.copy_catalog.service.copy_delete_workflow import delete_leftover_copy
        result = delete_leftover_copy("123", delete_copy, "BACKUP")
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.hset", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.job_center_client.query_is_job_present", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.check_should_be_stop_by_job_status", MagicMock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.update_copy_status", MagicMock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.LockService.copy_delete_lock_resources", MagicMock(return_value=True))
    def test_process_copy_delete_context_init_3(self):
        """
        测试删除初始化上下文
        期望：根据任务状态初始化上下文
        return:运行不报错
        """
        process_copy_delete_context_init("111", "222", mock.Mock(), mock.Mock(), mock.Mock())

def construct_copy_by_generated(resource_sub_type):
    copy_parent_id = str(uuid.uuid4())
    copy_id = str(uuid.uuid4())
    copy = CopyTable(uuid=copy_id, parent_copy_uuid=copy_parent_id, status='Normal',
                     resource_sub_type=resource_sub_type)
    return copy
