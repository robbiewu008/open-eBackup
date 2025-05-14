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
import json
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, MagicMock

from dateutil import parser

from app.common.deploy_type import DeployType
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()
from app.copy_catalog.util import copy_util
from app.common.enums.copy_enum import GenerationType
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum, RetentionTypeEnum, WormValidityTypeEnum, RetentionTimeUnit
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyAntiRansomwareTable
from app.copy_catalog.service import copy_expire_service
from app.copy_catalog.service.curd import copy_delete_service
from app.copy_catalog.service import copy_delete_workflow
from app.copy_catalog.service.copy_expire_service import copy_expire_thread

copy_obj = CopyTable(uuid=str(uuid.uuid4()),
                     backup_type=BackupTypeEnum.cumulative_increment.value,
                     expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                     display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                     properties=json.dumps({"backup_type": "cumulative_increment"}),
                     generated_by=GenerationType.BY_BACKUP.value,
                     resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                     status=CopyStatus.NORMAL)

get_deleting_copy_result = MagicMock()
get_deleting_copy_result.expiration_time = parser.parse("9090-09-09 09:09:09")
get_deleting_copy_result_2 = MagicMock()
get_deleting_copy_result_2.expiration_time = parser.parse("2020-02-02 02:02:02")
get_deleting_copy_result_2.user_id = str(uuid.uuid4())
associated_copy = MagicMock()
associated_copy.expiration_time = parser.parse("9090-09-09 09:09:09")
associated_copy_2 = MagicMock()
associated_copy_2.expiration_time = parser.parse("2020-02-02 02:02:02")
associated_copies = [associated_copy, associated_copy_2]
associated_copies_2 = [associated_copy_2, associated_copy_2]
cache_recent_expiring_copy = MagicMock()
cache_recent_expiring_copy.timestamp = parser.parse("2020-02-02 02:02:02")
get_cache_recent_expiring_copies =[cache_recent_expiring_copy, cache_recent_expiring_copy]

class CopyExpireServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        self.copy_expire_service = copy_expire_service
        self.copy_service = copy_delete_service
        self.copy_delete_workflow = copy_delete_workflow
        self.copy_util = copy_util

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy", Mock(return_value=copy_obj))
    @mock.patch("sqlalchemy.orm.query.Query.first", Mock(return_value=copy_obj))
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none", Mock(return_value=None))
    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware",
                Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_expire_service.is_generate_type_io_detect", Mock(return_value=False))
    def test_delete_cached_expired_copy_failed_when_copy_exist_clone_file_system(self):
        """
        用例场景：副本过期失败，副本存在克隆文件系统
        前置条件：副本存在克隆文件系统
        检查点：不会下发副本过期任务，调用abort取消副本过期
        :return:
        """
        copy_delete_workflow.check_copy_can_be_deleted = MagicMock(
            return_value=(1, 'can not be deleted,mission aborted!'))
        with mock.patch.object(copy_delete_workflow, "abort") as mock_abort:
            mock_abort.return_value = None
            self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
            self.assertEqual(copy_delete_workflow.abort.call_count, 1)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy", Mock(return_value=copy_obj))
    @mock.patch("sqlalchemy.orm.query.Query.first", Mock(return_value=copy_obj))
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none", Mock(return_value=None))
    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware",
                Mock(return_value=False))
    @mock.patch("app.copy_catalog.service.copy_expire_service.is_generate_type_io_detect", Mock(return_value=True))
    def test_delete_cached_expired_copy_failed_when_copy_exist_clone_file_system(self):
        """
        用例场景：副本过期失败，副本生成方式为IO_DETECT
        前置条件：副本生成方式为IO_DETECT
        检查点：不会生成副本过期任务
        :return:
        """
        copy_delete_workflow.check_copy_can_be_deleted = MagicMock(
            return_value=(1, 'can not be deleted,mission aborted!'))
        with mock.patch.object(copy_delete_workflow, "abort") as mock_abort:
            mock_abort.return_value = None
            self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
            self.assertEqual(copy_delete_workflow.abort.call_count, 0)

    def test_should_raise_EmeiStorBizException_when_reduce_worm_copy_retention_time(self):
        """
        用例场景：减小防篡改副本保留时间，则会抛出异常
        前置条件：存在防篡改副本
        检查点：不能减小防篡改副本的保留时间
        :return:
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         display_timestamp=123456,
                         status="Normal", location="Local", generated_by="Backup", generated_time=123456, worm_status=3,
                         gn=1, retention_type=2, retention_duration=2, duration_unit='d')
        retention_info = ('d', 234567, 3, True, WormValidityTypeEnum.copy_retention_time_consistent)
        from app.copy_catalog.service.copy_expire_service import check_worm_copy_retention
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        raw_retention_policy = CopyRetentionPolicySchema(**{"retention_type": RetentionTypeEnum.temporary,
                                                            "retention_duration": 5,
                                                            "duration_unit": 'd'})
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=False)
        with self.assertRaises(EmeiStorBizException) as ex:
            check_worm_copy_retention(raw_retention_policy, copy, retention_info)
        self.assertEqual(ex.exception.error_code, CopyErrorCode.MODIFY_WORM_COPY_RETENTION_FAIL['errorCode'])

    def test_should_raise_EmeiStorBizException_when_modify_setting_copy(self):
        """
        用例场景：修改worm状态为setting的防篡改副本的保留时间，则会抛出异常
        前置条件：存在防篡改副本
        检查点：不能修改worm状态为setting的防篡改副本的保留时间
        :return:
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         display_timestamp=123456,
                         status="Normal", location="Local", generated_by="Backup", generated_time=123456, worm_status=2,
                         gn=1, retention_type=2, retention_duration=2, duration_unit='d')
        retention_info = ('d', 234567, 1, True)
        from app.copy_catalog.service.copy_expire_service import check_worm_copy_retention
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        raw_retention_policy = CopyRetentionPolicySchema()

        with self.assertRaises(EmeiStorBizException) as ex:
            check_worm_copy_retention(raw_retention_policy, copy, retention_info)
        self.assertEqual(ex.exception.error_code,
                         CopyErrorCode.CAN_NOT_MODIFY_COPY_RETENTION.get("errorCode"))

    @mock.patch("threading.Thread.start", Mock(return_value=True))
    @mock.patch("threading.Thread.is_alive")
    def test_restart_copy_expire_thread_when_copy_expire_thread_is_not_alive(self, mock_is_alive):
        """
        用例场景：副本过期线程挂掉
        前置条件：副本过期线程is_alive()方法返回false
        检查点：重新起一个副本过期线程
        """
        mock_is_alive.return_value = False
        copy_expire_thread.start_copy_expire_thread()
        with mock.patch.object(copy_expire_service.copy_expire_thread, "start_copy_expire_thread") as _:
            copy_expire_thread.check_copy_expire_thread()
        mock_is_alive.return_value = True
        self.assertTrue(copy_expire_service.copy_expire_thread.copy_expire_thread.is_alive())

    @mock.patch("app.common.database.Database.session")
    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware", Mock(return_value=False))
    @mock.patch("sqlalchemy.orm.query.Query.update", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.update_copy_cache", MagicMock)
    def test_update_inner_direction_copy_retention_success(self, mock_session):
        """
        用例场景：修改目录格式副本过期时间成功
        前置条件：存在目录个事副本
        检查点：校验通过,未抛出异常
        :return:
        """

        copy = CopyTable(uuid=str(uuid.uuid4()),
                         backup_type=BackupTypeEnum.cumulative_increment.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime.now(), gn=1,
                         properties=json.dumps({"backup_type": "cumulative_increment", "format": 1}),
                         generated_by=GenerationType.BY_REPLICATED.value,
                         resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                         retention_duration=3,
                         retention_type=2,
                         duration_unit=RetentionTimeUnit.weeks,
                         worm_retention_duration=1,
                         worm_duration_unit=RetentionTimeUnit.weeks,
                         worm_status=1,
                         status=CopyStatus.NORMAL
                         )

        copy_anti_ransomware = CopyAntiRansomwareTable(copy_id=str(uuid.uuid4()),
                                                       generate_type="COPY_DETECT")
        mock_session().__enter__().query().filter().one_or_none.side_effect = [copy, copy_anti_ransomware]
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        copy_retention_policy = CopyRetentionPolicySchema(**{
                                        "resource_id": str(uuid.uuid4()),
                                        "retention_type": RetentionTypeEnum.temporary,
                                        "retention_duration": 1,
                                        "duration_unit":"w"
                                         })
        self.copy_expire_service.update_copy_retention(copy.uuid, copy_retention_policy)
        self.assertTrue(True)

    def test_should_raise_EmeiStorBizException_when_copy_retention_type_is_permanent(self):
        """
        用例场景：副本保留时间是永久保留，则会抛出异常
        前置条件：存在防篡改副本
        return:抛出EmeiStorBizException异常
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         status="Normal", location="Local", generated_by="Backup", worm_status=3,
                         gn=1, retention_type=1, retention_duration=2, duration_unit='d')
        retention_info = ('d', 234567, 1, True, WormValidityTypeEnum.copy_retention_time_consistent)
        from app.copy_catalog.service.copy_expire_service import check_worm_copy_retention
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        raw_retention_policy = CopyRetentionPolicySchema()
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=True)
        with self.assertRaises(EmeiStorBizException) as ex:
            check_worm_copy_retention(raw_retention_policy, copy, retention_info)
        self.assertEqual(ex.exception.error_code, CopyErrorCode.MODIFY_SECURITY_SNAP_RETENTION_FAIL['errorCode'])

    def test_check_worm_copy_retention_when_worm_status_is_UNSET_should_return_minus_one(self):
        """
        用例场景：当worm状态为设置时，应返回FOREVER(-1)
        前置条件：存在防篡改副本
        return:-1
        """
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        raw_retention_policy = CopyRetentionPolicySchema()
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         status="Normal", location="Local", generated_by="Backup", worm_status=1,
                         gn=1, retention_type=1, retention_duration=2, duration_unit='d')
        retention_info = ('d', 234567, 1, True)
        from app.copy_catalog.service.copy_expire_service import check_worm_copy_retention
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=True)
        result = check_worm_copy_retention(raw_retention_policy, copy, retention_info)
        self.assertEqual(-1, result)

    @mock.patch("app.copy_catalog.service.copy_expire_service.check_expire_copy_reach_to_delete_time",
                Mock(return_value=False))
    def test_delete_expire_copy_when_not_reach_to_delete_time(self):
        """
        用例场景：删除副本时检查副本过期是否到删除时间，没到直接返回
        前置条件：存在副本
        return:直接返回
        """
        from app.copy_catalog.service.copy_expire_service import delete_expire_copy
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         status="Normal", location="Local", generated_by="Backup", worm_status=1,
                         gn=1, retention_type=1, retention_duration=2, duration_unit='d')
        from app.copy_catalog.service.copy_delete_workflow import CopyDeleteParam
        copy_delete_param = CopyDeleteParam(user_id="user_id", strict=True)
        delete_expire_copy(copy, copy_delete_param)

    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_job_list", Mock(return_value=None))
    def test_delete_expire_copy_when_(self):
        """
        用例场景：删除副本时未感染副本只有一个，并且是当前副本
        前置条件：存在副本
        return:直接返回
        """
        from app.copy_catalog.service.copy_expire_service import delete_expire_copy
        copy = CopyTable(uuid=str(uuid.uuid4()), chain_id=str(uuid.uuid4()), timestamp="123456",
                         status="DeleteFailed", location="Local", generated_by="Backup", worm_status=1,
                         gn=1, retention_type=1, retention_duration=2, duration_unit='d')
        from app.copy_catalog.service.copy_delete_workflow import CopyDeleteParam
        copy_delete_param = CopyDeleteParam(user_id="user_id", strict=True)
        delete_expire_copy(copy, copy_delete_param)

    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware", Mock(return_value=False))
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_update_copy_retention_when_copy_status_is_deleting(self, mock_one_or_none):
        """
        用例场景：修改副本保留时间，副本状态是正在删除中
        前置条件：存在副本
        return:抛出异常
        """
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         backup_type=BackupTypeEnum.cumulative_increment.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                         properties=json.dumps({"backup_type": "cumulative_increment", "format": 1}),
                         generated_by=GenerationType.BY_REPLICATED.value,
                         resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                         status=CopyStatus.DELETING)
        mock_one_or_none.return_value = copy
        from app.copy_catalog.service.copy_expire_service import update_copy_retention
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=False)
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        copy_retention_policy = CopyRetentionPolicySchema()
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        with self.assertRaises(EmeiStorBizException) as ex:
            update_copy_retention(copy.uuid, copy_retention_policy)
        self.assertEqual(ex.exception.error_code, CopyErrorCode.ALREADY_IN_DELETING.get("code"))

    @mock.patch("app.copy_catalog.service.copy_expire_service.cache_recent_expiring_copies", Mock(return_value=False))
    def test_handle_copy_check(self):
        """
        用例场景：copy check
        前置条件：存在副本
        return:运行不报错
        """
        from app.copy_catalog.service.copy_expire_service import handle_copy_check
        result = handle_copy_check("")
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy", Mock(return_value=None))
    def test_delete_cached_expired_copy_when_get_deleting_copy_is_None(self):
        """
        用例场景：副本过期失败，删除缓存的副本
        前置条件：未查询到副本
        return:直接返回
        """
        result = self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy",
                Mock(return_value=get_deleting_copy_result))
    def test_delete_cached_expired_copy_when_deleting_copy_is_not_expire(self):
        """
        用例场景：副本过期失败，删除缓存的副本
        前置条件：副本未到过期时间
        return:直接返回
        """
        result = self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy",
                Mock(return_value=get_deleting_copy_result_2))
    @mock.patch("app.copy_catalog.service.copy_expire_service.resource_has_copy_link", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.associated_deletion_copies", Mock(return_value=None))
    def test_delete_cached_expired_copy_when_get_associated_deletion_copies_is_None(self):
        """
        用例场景：副本过期失败，删除缓存的副本
        前置条件：关联的删除副本为空
        return:直接返回
        """
        result = self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy",
                Mock(return_value=get_deleting_copy_result_2))
    @mock.patch("app.copy_catalog.service.copy_expire_service.resource_has_copy_link", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.associated_deletion_copies",
                Mock(return_value=associated_copies))
    def test_delete_cached_expired_copy_when_copy_is_not_expire(self):
        """
        用例场景：副本过期失败，删除缓存的副本
        前置条件：关联副本存在大于当前时间
        return:直接返回
        """
        result = self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.get_deleting_copy",
                Mock(return_value=get_deleting_copy_result_2))
    @mock.patch("app.copy_catalog.service.copy_expire_service.resource_has_copy_link", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.associated_deletion_copies",
                Mock(return_value=associated_copies_2))
    @mock.patch("app.copy_catalog.service.copy_expire_service.delete_cached_copy", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.delete_expire_copy", Mock(return_value=True))
    def test_delete_cached_expired_copy_should_delete_copies(self):
        """
        用例场景：副本过期失败，删除缓存的副本
        前置条件：关联副本存在小于当前时间
        return:删除副本
        """
        result = self.copy_expire_service.delete_cached_expired_copy(str(uuid.uuid4()))
        self.assertIsNone(result)

    @mock.patch("app.common.redis.redis_lock.setnx", Mock(return_value=True))
    @mock.patch("app.common.redis.redis_lock.release", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.zrem", Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.copy_expire_service.query_recent_expiring_copies",
                Mock(return_value=get_cache_recent_expiring_copies))
    @mock.patch("app.copy_catalog.service.copy_expire_service.cache_recent_expiring_copy", Mock(return_value=True))
    def test_cache_recent_expiring_copies(self):
        """
        用例场景：缓存最近过期的副本
        前置条件：无
        return:运行不报错
        """
        result = self.copy_expire_service.cache_recent_expiring_copies(str(uuid.uuid4()))
        self.assertIsNone(result)

    def test_check_worm_retention_with_copy_retention_should_success(self):
        '''
        用例场景：设置worm过期时间小于副本过期时间
        前置条件：无
        return:运行不报错
        '''
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         backup_type=BackupTypeEnum.cumulative_increment.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                         properties=json.dumps({"backup_type": "cumulative_increment", "format": 1}),
                         generated_by=GenerationType.BY_REPLICATED.value,
                         resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                         retention_duration=3,
                         duration_unit="w",
                         status=CopyStatus.DELETING)
        from app.copy_catalog.schemas import CopyRetentionPolicySchema
        worm_retention_policy = CopyRetentionPolicySchema(**{"retention_type":2, "retention_duration":2,
                                "duration_unit":"w"})
        result = self.copy_expire_service.check_worm_retention_with_copy_retention(copy, worm_retention_policy)
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session")
    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware", Mock(return_value=False))
    @mock.patch("app.copy_catalog.client.copy_client.create_copy_worm_task")
    def test_update_worm_copy_setting_success_when_is_not_worm_copy(self, mock_detection_task, mock_session):
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         backup_type=BackupTypeEnum.cumulative_increment.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime.now(), gn=1,
                         properties=json.dumps({"backup_type": "cumulative_increment", "format": 1}),
                         generated_by=GenerationType.BY_REPLICATED.value,
                         resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                         retention_duration=3,
                         duration_unit=RetentionTimeUnit.weeks,
                         worm_status=1,
                         retention_type=2,
                         status=CopyStatus.DELETING)
        mock_session().__enter__().query().filter().one_or_none.return_value = copy
        mock_detection_task.return_value = None
        from app.copy_catalog.schemas import CopyWormRetentionSettingSchema
        worm_setting = CopyWormRetentionSettingSchema(**{"convert_worm_switch": True,
                                                         "worm_validity_type": WormValidityTypeEnum.copy_retention_time_consistent,
                                                         "retention_duration": 1,
                                                         "duration_unit": RetentionTimeUnit.weeks
                                                         })
        self.copy_expire_service.update_worm_copy_setting(copy.uuid, worm_setting)
        mock_detection_task.assert_called_once()

    @mock.patch("app.common.database.Database.session")
    @mock.patch("app.copy_catalog.service.copy_expire_service.check_copy_anti_ransomware", Mock(return_value=False))
    @mock.patch("app.copy_catalog.client.dme_client.update_dm_copy_retention")
    def test_update_worm_copy_setting_success_when_is_worm_copy(self, mock_dm_copy_retention, mock_session):
        """
        用例场景：当设置worm设置开启时，才做修改worm设置
        前置条件：无
        return:运行不报错
        """
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         backup_type=BackupTypeEnum.cumulative_increment.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime.now(), gn=1,
                         properties=json.dumps({"backup_type": "cumulative_increment", "format": 1}),
                         generated_by=GenerationType.BY_REPLICATED.value,
                         resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                         retention_duration=3,
                         duration_unit=RetentionTimeUnit.weeks,
                         worm_retention_duration=1,
                         worm_duration_unit=RetentionTimeUnit.weeks,
                         worm_status=3,
                         retention_type=2,
                         status=CopyStatus.DELETING)
        copy_anti_ransomware = CopyAntiRansomwareTable(copy_id="test")
        mock_session().__enter__().query().filter().one_or_none.side_effect = [copy, copy_anti_ransomware]
        mock_dm_copy_retention.return_value = None
        from app.copy_catalog.schemas import CopyWormRetentionSettingSchema
        worm_setting = CopyWormRetentionSettingSchema(**{"convert_worm_switch": True,
                                                         "worm_validity_type": WormValidityTypeEnum.custom_retention_time,
                                                         "retention_duration": 2,
                                                         "duration_unit": RetentionTimeUnit.weeks
                                                         })
        self.copy_expire_service.update_worm_copy_setting(copy.uuid, worm_setting)
        mock_dm_copy_retention.assert_called_once()
