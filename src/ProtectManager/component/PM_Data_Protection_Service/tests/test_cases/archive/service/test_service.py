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
import sys
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, patch
from urllib3 import HTTPResponse

from app.archive.schemas.archive_request import ArchiveMsg, ArchiveRequest, ArchiveStorageInfo
from app.common.enums.copy_enum import GenerationType
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.common.copy_status import CopyWormStatus
from app.common.clients.protection_client import ProtectionClient
from app.copy_catalog.schemas import CopyInfoWithArchiveFieldsSchema
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.common.events import mock_producer  # noqa
from tests.test_cases.common import mock_kafka_client  # noqa
from app.common.enums.protected_object_enum import Status
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import ArchiveTypeEnum, CopyTypeEnum, TimeRangeYearEnum, RetentionTimeUnit, \
    TimeRangeMonthEnum, TimeRangeWeekEnum, TriggerEnum
from tests.test_cases.archive.service.compose_patch import POLICY, COPY_INFO, NOT_FULL_OR_NATIVE_COPY_INFO, \
    INVALID_COPY_INFO, HAS_ARCHIVE_TO_STORAGE_COPY_INFO, DELETING_COPY_INFO, DELETEFAILED_COPY_INFO, \
    CUMULATIVE_INCREMENT_COPY_INFO, DIFF_INCREMENT_COPY_INFO
from tests.test_cases.common.mock_settings import fake_timezone_data
from app.common.clients.client_util import ProtectionServiceHttpsClient, ProtectEngineEDmaHttpsClient, \
    SystemBaseHttpsClient
from app.common.redis_session import redis_session

mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()
sys.modules['app.resource_lock.kafka.rollback_utils'] = Mock()
sys.modules['app.resource_lock.service.lock_service'] = Mock()
sys.modules['app.backup.common.config.db_config'] = Mock()
_mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_common_db_init.start()

req_dma_patcher = patch.object(
    ProtectEngineEDmaHttpsClient, "request",
    Mock(return_value=HTTPResponse(status=200, body=bytes(json.dumps(fake_timezone_data), encoding="utf-8"))))
req_dma_patcher.start()


class RequestMock:
    def __init__(self):
        self.request_id = "785f6333-c826-4515-bee4-0f5ac0243852"


schedule = {"window_end": "21:12:10", "window_start": "21:12:10", "interval": 2,
            "interval_unit": "d"}


class Request:
    request_id = "abc"


def get_request():
    response = HTTPResponse()
    response._body = b'{"data": "","error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'
    response.status = 200
    return response


def get_request_404():
    response = HTTPResponse()
    response._body = b'{"data": "","error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'
    response.status = 404
    return response


def copy_item_mock():
    response = HTTPResponse()
    response._body = b'{"data": {"items": [""]},"error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'

    response.status = 200
    return response


def copy_item_mock_has_value():
    response = HTTPResponse()
    response._body = b'{"items": [{"name":"123", "resource_id":"123", "resource_sub_type":"Common"}],' \
                     b'"error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'

    response.status = 200
    return response


class mock_one_or_none:
    @staticmethod
    def one_or_none():
        return []


class ProcessArchiveByPolicy(unittest.TestCase):
    def setUp(self):
        sys.modules['app.protection.object.db'] = Mock()
        super(ProcessArchiveByPolicy, self).setUp()
        from app.archive.service import service
        self.service = service
        self.kwargs = {
            "copy_id": "123",
            "user_id": "123",
            "policy": json.dumps(POLICY),
            "sla": json.dumps("sla"),
            "sla_id": "123",
            "execute_type": "AUTOMATIC",
            "2925c6d5-e313-4591-8940-5bebbbd72087": "2925c6d5-e313-4591-8940-5bebbbd72087",
            "resource_id": "2925c6d5-e313-4591-8940-5bebbbd72087",
            "AUTO_RETRY_FLAG2925c6d5-e313-4591-8940-5bebbbd72087":
                "2925c6d5-e313-4591-8940-5bebbbd72087",
            "d8a9a370-2591-41c5-be08-6e8eca64c89b": "d8a9a370-2591-41c5-be08-6e8eca64c89b"
        }

    @mock.patch("app.common.redis_session.redis_session.delete", mock.Mock())
    @mock.patch("app.common.toolkit.complete_job_center_task", mock.Mock())
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_archive_error_callback(self):
        """
        验证场景：归档回滚是否正常
        前置条件：无
        验证点：归档回滚是否正常
        """
        res = self.service.archive_error_callback(Request())
        self.assertIsNone(res)

    @mock.patch("app.archive.service.service.process_archive", Mock(return_value=None))
    @mock.patch("app.archive.service.service.anti_ransomware_check", Mock(return_value=True))
    def test_back_success_handler(self):
        """
        验证场景：备份成功后处理
        前置条件：备份成功
        验证点：备份成功后处理
        """
        res = self.service.back_success_handler(RequestMock(), **self.kwargs)
        self.assertIsNone(res)

    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request()))
    @mock.patch("app.archive.service.service.process_archive", Mock(return_value=None))
    @mock.patch("app.archive.service.service.anti_ransomware_check", Mock(return_value=True))
    def test_replica_success_handler(self):
        """
        验证场景：复制成功后处理
        前置条件：复制成功
        验证点：复制成功后处理
        """
        res = self.service.replica_success_handler(RequestMock(), **self.kwargs)
        self.assertIsNone(res)

    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request()))
    @mock.patch("app.archive.service.service.get_next_archive_copy")
    @patch.object(ProtectionServiceHttpsClient, "request")
    @patch("app.backup.redis.context.Context.set", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @mock.patch("app.archive.service.service.process_archive", Mock(return_value=None))
    @mock.patch("app.common.redis_session.redis_session.delete", mock.Mock())
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    @patch("app.backup.redis.context.Context.delete_all", Mock(return_value=None))
    def test_archive_done_handler(self, _mock_get_context, _mock_copy_info, _mock_get_next_archive_copy):
        """
        验证场景：归档后置处理逻辑是否正常
        前置条件：复制成功
        验证点：归档后置处理逻辑正常
        """
        # 模拟备份副本归档
        copy_info1 = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b6850",
                      "generated_by": "Backup",
                      "generated_time": "2021-08-12T19:27:20",
                      "properties": "{\"format\":1 } ",
                      "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
                      "resource_name": "test1",
                      "resource_type": "VM",
                      "resource_sub_type": "vim.VirtualMachine",
                      "gn": 2,
                      "name": "copy2"
                      }
        copy_list = [copy_info1]
        mock.patch("app.restore.client.copy_client.CopyClient.query_copies",
                   mock.Mock(return_value={"total": 0, "items": copy_list})).start()
        mock.patch("app.backup.client.job_client.JobClient.update_job").start()
        _mock_get_next_archive_copy.return_value = None
        fake_ctx_map = {
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "object_type": "",
            "resource_id": "",
            "ext_parameters": {"archiving_scope": "all_no_archiving"}
        }
        _mock_get_context.return_value = json.dumps(fake_ctx_map)
        self.service.alarm_after_failure = Mock()
        _mock_copy_info.return_value = copy_item_mock()
        res = self.service.archive_done_handler(RequestMock(), "123", 1, "123", 5)
        self.assertIsNone(res)

        _mock_copy_info.return_value = copy_item_mock_has_value()
        res = self.service.archive_done_handler(RequestMock(), "123", 1, "123", 5)
        self.assertIsNone(res)

        res = self.service.archive_done_handler(RequestMock(), "123", 1, "123", 0)
        self.assertIsNone(res)

        # 失败流程
        res = self.service.archive_done_handler(RequestMock(), "123", 0, "123", 5)
        self.assertIsNone(res)

        # 中止流程
        res = self.service.archive_done_handler(RequestMock(), "123", 3, "123", 5)
        self.assertIsNone(res)

    @mock.patch("app.common.redis_session.redis_session.hdel", mock.Mock())
    def test_process_archive_aborted(self):
        """
        验证场景：归档停止处理逻辑是否正常
        前置条件：复制成功
        验证点：归档停止处理逻辑正常
        """
        POLICY['ext_parameters'] = {"archiving_scope": "all_no_archiving", "storage_id": '123'}
        res = self.service.process_archive_aborted(POLICY, copy_info=COPY_INFO)
        self.assertIsNone(res)

    @mock.patch("app.common.redis_session.redis_session.get")
    @mock.patch("app.archive.client.archive_client.ArchiveClient.update_copy_status")
    @mock.patch("app.archive.service.service.get_next_archive_copy")
    @mock.patch("app.common.redis_session.redis_session.hexists")
    @patch.object(ProtectionServiceHttpsClient, "request", Mock(return_value=HTTPResponse(status=200)))
    def test_process_archive_success(self, _mock_hexists, _mock_get_next_archive_copy, _mock_update_copy_status,
                                     _mock_get):
        """
        验证场景：归档处理逻辑是否正常
        前置条件：复制成功
        验证点：归档处理逻辑正常
        """
        _mock_get_next_archive_copy.return_value = None
        _mock_update_copy_status.return_value = None
        _mock_hexists.return_value = False
        _mock_get.return_value = None

        copy_info = {"resource_id": "abc-def", "status": "success", "uuid": "1234"}
        policy = {"ext_parameters": {"storage_id": str(uuid.UUID)}}
        res = self.service.process_archive_success(copy_info, policy)
        self.assertIsNone(res)

    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_process_archive_by_policy(self):
        """
        验证场景：归档处理逻辑是否正常
        前置条件：复制成功
        验证点：归档处理逻辑正常
        """
        sla_info = {"policy_list": ""}
        res = self.service.process_archive(sla_info, "abc", "1", None, "AUTOMATIC")
        self.assertIsNone(res)

        POLICY['schedule'] = {"trigger": 1}
        sla_info = {"policy_list": [POLICY]}
        res = self.service.process_archive(sla_info, "abc", None, None, "AUTOMATIC")

        POLICY['schedule'] = {"trigger": 2}
        sla_info = {"policy_list": [POLICY],
                    "uuid": "abc-dec"}
        res = self.service.process_archive(sla_info, "abc", None, None, "AUTOMATIC")
        self.assertIsNone(res)

    @patch.object(SystemBaseHttpsClient, "request", Mock(return_value=get_request()))
    @mock.patch("app.common.redis_session.redis_session.delete", mock.Mock())
    @mock.patch("app.common.redis_session.redis_session.set", mock.Mock())
    @mock.patch("app.archive.service.archive_scheduler.ArchiveScheduler.create_delay_schedule")
    def test_process_archive_failed(self, mock_create_delay_schedule):
        """
        验证场景：归档失败处理逻辑是否正常
        前置条件：NA
        验证点：归档失败处理逻辑正常
        """
        mock_create_delay_schedule.return_value = "schedule_id_1"
        policy = {"ext_parameters": {}, "archiving_scope": ""}
        sla = None
        auto_retry_times = 0
        res = self.service.process_archive_failed(auto_retry_times, policy, COPY_INFO, sla)
        self.assertIsNone(res)
        COPY_INFO["param"] = "123"
        COPY_INFO["copy"] = {}
        self.service.get_next_archive_copy = Mock(return_value=COPY_INFO)
        self.service.submit_archive_schedule = Mock()
        policy = {"ext_parameters": {"archiving_scope": "all_no_archiving", "storage_id": "123456"},
                  "archiving_scope": ""}
        res = self.service.process_archive_failed(auto_retry_times, policy, COPY_INFO, sla)
        self.assertIsNone(res)

        auto_retry_times = 1
        self.service.get_next_archive_copy = Mock(return_value=COPY_INFO)
        self.service.submit_archive_schedule = Mock()
        policy = {"ext_parameters":
                      {"archiving_scope": "all_no_archiving",
                       "auto_retry_wait_minutes": 5,
                       "resource_sub_type": ResourceSubTypeEnum.Common.value,
                       "storage_id": "123456"},
                  "archiving_scope": ""}
        sla = json.loads('{"name": "common"}')
        res = self.service.process_archive_failed(auto_retry_times, policy, COPY_INFO, sla)
        self.assertIsNone(res)

    @mock.patch("app.common.redis_session.redis_session.hexists")
    @mock.patch("app.common.redis_session.redis_session.hget")
    @mock.patch("app.common.redis_session.redis_session.hset")
    @mock.patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list",
                Mock(return_value=[{"copy_id": "123"}]))
    @mock.patch("ast.literal_eval")
    def test_get_next_archive_copy(self, mock_ast_literal_eval, _mock_hset, _mock_hget, _mock_hexists):
        """
        验证场景：获取下一个归档副本
        前置条件：NA
        验证点：获取下一个归档副本
        """
        _mock_hexists.return_value = False
        resource_id = "abc"
        storage_id = str(uuid.UUID)
        res = self.service.get_next_archive_copy(resource_id, storage_id, False)
        self.assertIsNone(res)

        context = [{'copy': 'test_copy_1', 'param': 'test_param_1'}, {'copy': 'test_copy_2', 'param': 'test_param_2'}]
        mock_ast_literal_eval.return_value = context
        _mock_hexists.return_value = True
        resource_id = "abc"
        storage_id = str(uuid.UUID)
        _mock_hget.return_value = True
        _mock_hset.return_value = True
        res = self.service.get_next_archive_copy(resource_id, storage_id, False)
        self.assertIsNotNone(res)

    @patch("app.backup.redis.context.Context.set", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_add_resource_lock_success(self, mock_context_get_id):
        mock_context_get_id.return_value = "id11111"
        request = Mock()
        request.request_id = "test_request_id"

        # 设置mock_context的返回值和属性
        self.service.add_resource_lock(request, key1="value1", key2="value2")

    @patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list")
    @patch("app.common.redis_session.redis_session.exists")
    @patch("app.archive.client.archive_client.ArchiveClient.create_copy_archive_map", Mock(return_value=None))
    def test_prepare_archive_success(self, mock_redis_exists, mock_no_archive_copy_list):
        pararm = {"resource_id": "res-123", "storage_id": "storage-456,", "auto_retry_flag": 1, "copy_id": "wuyanzu"}
        self.service.prepare_archive(pararm)

        pararm = {"resource_id": "res-123", "storage_id": "storage-456,", "copy_id": "123"}
        self.service.prepare_archive(pararm)

        # 模拟备份副本归档
        copy_info1 = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b6850",
                      "generated_by": "Backup",
                      "generated_time": "2021-08-12T19:27:20",
                      "properties": "{\"format\":1 } ",
                      "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
                      "resource_name": "test1",
                      "resource_type": "VM",
                      "resource_sub_type": "vim.VirtualMachine",
                      "gn": 2,
                      "name": "copy2",
                      "worm_status": CopyWormStatus.SETTING.value
                      }
        copy_list = [copy_info1]
        mock_no_archive_copy_list.return_value = copy_list
        mock_redis_exists.return_value = 1
        self.service.prepare_archive(pararm)

        mock_redis_exists.return_value = 2
        self.service.prepare_archive(pararm)

    @mock.patch("app.archive.client.archive_client.ArchiveClient.query_resource")
    @mock.patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list",
                Mock(return_value=[COPY_INFO]))
    @mock.patch("app.common.config.Settings.get_key_from_config_map")
    @patch(
        "app.protection.object.service.projected_copy_object_service.ProtectedCopyObjectService.query_copy_protected_dict")
    @mock.patch("app.backup.client.resource_client.ResourceClient.query_protected_object")
    @patch("app.backup.client.protection_client.ProtectionClient.query_sla")
    @mock.patch("app.archive.service.check_policy.CheckPolicy.filter_copies")
    @unittest.skip
    def test_handle_schedule_archiving_success(self, mock_filter_copies, mock_sla, mock_protected_obj,
                                               mock_query_copy_projected, mock_local_esn, mock_resource):
        self.mock_query_root_resource(mock_resource)
        self.mock_query_no_archive_copy(mock_local_esn)
        self.mock_specified_copy(mock_filter_copies)

        self.mock_is_protected_obj_and_active(mock_protected_obj, mock_query_copy_projected)

        archive_msg = ArchiveMsg(
            request_id="1234567890abcdef",
            params={"resource_id": "res1", "sla_id": "sla_id_1", "policy": POLICY, "user_id": "user1"}
        )
        # sla为none
        mock_sla.return_value = None
        self.service.handle_schedule_archiving(archive_msg)

        # 资源未保护
        mock_sla.return_value = {"sla_id": "1"}
        self.service.is_protected_obj_and_active = Mock(return_value=False)
        self.service.handle_schedule_archiving(archive_msg)

        sla = {"application": ResourceSubTypeEnum.NasShare.value}
        mock_sla = sla
        # 资源已保护 副本类型 Backup 查询归档副本为空
        self.service.is_protected_obj_and_active = Mock(return_value=True)
        self.service.query_no_archive_copy = Mock(return_value=[])
        self.service.handle_schedule_archiving(archive_msg)

        # 资源已保护 副本类型 Backup 用户id为空 查询归档副本为空
        archive_msg = ArchiveMsg(
            request_id="1234567890abcdef",
            params={"resource_id": "res1", "sla_id": "sla_id_1", "policy": POLICY}
        )
        self.service.query_root_resource = Mock(return_value=None)
        self.service.handle_schedule_archiving(archive_msg)

        self.service.query_root_resource = Mock(return_value={"user_id": "userid1"})
        self.service.handle_schedule_archiving(archive_msg)

        # 归档副本存在 备份完立即归档
        copy_list = [COPY_INFO]
        self.service.query_no_archive_copy = Mock(return_value=copy_list)
        self.service.all_copy = Mock(return_value=None)
        self.service.handle_schedule_archiving(archive_msg)

        # 归档副本存在 备份指定时间归档
        archive_msg = ArchiveMsg(
            request_id="1234567890abcdef",
            params={"resource_id": "res1", "sla_id": "sla_id_1", "policy": POLICY, "user_id": "user1",
                    "schedule_type": TriggerEnum.after_backup_complete}
        )
        self.service.handle_schedule_archiving(archive_msg)

        POLICY["ext_parameters"]["archive_target_type"] = ArchiveTypeEnum.specified_copy
        self.service.specified_copy = Mock(return_value=None)
        self.service.handle_schedule_archiving(archive_msg)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(INVALID_COPY_INFO)))
    def test_manual_archive_invalid_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=2)],
            retention_duration=1,
            duration_unit="d",
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.STATUS_ERROR)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(DELETING_COPY_INFO)))
    def test_manual_archive_deleting_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=2)],
            retention_duration=1,
            duration_unit="d",
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.STATUS_ERROR)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(DELETEFAILED_COPY_INFO)))
    def test_manual_archive_deletefailed_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=2)],
            retention_duration=1,
            duration_unit="d",
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.STATUS_ERROR)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(NOT_FULL_OR_NATIVE_COPY_INFO)))
    def test_manual_archive_not_full_or_native_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=2)],
            retention_duration=1,
            duration_unit="d",
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.NOT_A_FULL_OR_NATIVE_COPY)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(HAS_ARCHIVE_TO_STORAGE_COPY_INFO)))
    @mock.patch("app.backup.client.archive_client.ArchiveClient.query_storage_info",
                Mock(return_value={"storageName": "test"}))
    @mock.patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list",
                Mock(return_value=[]))
    def test_manual_archive_copy_has_archive_to_storage_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=2)],
            retention_duration=1,
            duration_unit="d",
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.COPY_HAS_ARCHIVED_TO_STORAGE)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(HAS_ARCHIVE_TO_STORAGE_COPY_INFO)))
    @mock.patch("app.backup.client.archive_client.ArchiveClient.query_storage_info",
                Mock(return_value={"storageName": "test"}))
    def test_manual_archive_tape_index_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=7)],
            retention_duration=1,
            duration_unit="d",
            auto_index=True
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.APPLICATION_NOT_SUPPORT_TAPE_ARCHIVE_AUTO_INDEX)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(CUMULATIVE_INCREMENT_COPY_INFO)))
    def test_manual_archive_cumulative_incre_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=7)],
            retention_duration=1,
            duration_unit="d",
            auto_index=True
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.ERR_PARAM)

    @mock.patch("app.archive.service.service.query_copy_info_by_copy_id",
                Mock(return_value=CopyInfoWithArchiveFieldsSchema.parse_obj(DIFF_INCREMENT_COPY_INFO)))
    def test_manual_archive_difference_incre_copy_failed(self):
        archive_request = ArchiveRequest(
            copy_id="f8bd7544-c6ae-43af-9755-bb33dbb6ec9b",
            retention_type=2,
            network_access=True,
            storage_list=[ArchiveStorageInfo(storage_id="a312b1e4d3824d239149104d925193a0", protocol=7)],
            retention_duration=1,
            duration_unit="d",
            auto_index=True
        )
        with self.assertRaises(EmeiStorBizException) as ex:
            self.service.manual_archive(archive_request)
            self.assertEquals(ex.exception.error_code, CommonErrorCodes.ERR_PARAM)

    def mock_query_root_resource(self, mock_resource):
        mock_resource.return_value = None
        result = self.service.query_root_resource("abc")
        mock_resource.return_value = {"uuid": "123", "root_uuid": "123"}
        result = self.service.query_root_resource("abc")
        mock_resource.return_value = {"uuid": "123", "root_uuid": "456"}
        result = self.service.query_root_resource("abc")

    def mock_query_no_archive_copy(self, mock_local_esn):
        sla = {}
        # ext为空
        copy_list = self.service.query_no_archive_copy("resid", GenerationType.BY_REPLICATED.value, None, "userId", sla)
        # ext不为空
        mock_local_esn.return_value = "abcde"
        copy_list = self.service.query_no_archive_copy("resid", GenerationType.BY_REPLICATED.value,
                                                       {"storage_id": "123"}, "userId", sla)
        mock_local_esn.return_value = "gggg"
        copy_list = self.service.query_no_archive_copy("resid", GenerationType.BY_REPLICATED.value,
                                                       {"storage_id": "123"}, "userId", sla)

    def mock_is_protected_obj_and_active(self, mock_protected_obj, mock_query_copy_projected):
        sla = {"application": ResourceSubTypeEnum.Replica}
        mock_query_copy_projected.return_value = None
        result = self.service.is_protected_obj_and_active(sla, "res1")
        mock_query_copy_projected.return_value = {"id": "1"}
        result = self.service.is_protected_obj_and_active(sla, "res1")
        sla = {"application": ResourceSubTypeEnum.Common}
        mock_query_copy_projected.return_value = {"protected_status": False}
        mock_protected_obj.return_value = None
        result = self.service.is_protected_obj_and_active(sla, "res1")
        mock_protected_obj.return_value = {"status": Status.Inactive.value}
        result = self.service.is_protected_obj_and_active(sla, "res1")
        mock_protected_obj.return_value = {"status": Status.Active.value}
        result = self.service.is_protected_obj_and_active(sla, "res1")

    def mock_specified_copy(self, mock_filter_copies):
        params = {"resource_id": "abc", "sla_id": "aa", "policy": {
            "ext_parameters":
                {"storage_id": str(uuid.UUID), "auto_retry_times": 1, "auto_retry": 2,
                 "archive_target_type": ArchiveTypeEnum.specified_copy,
                 "specified_scope": [{"copy_type": CopyTypeEnum.year, "generate_time_range": TimeRangeYearEnum.sep,
                                      "retention_unit": RetentionTimeUnit.years, "retention_duration": 2},
                                     {"copy_type": CopyTypeEnum.month, "generate_time_range": TimeRangeMonthEnum.last,
                                      "retention_unit": RetentionTimeUnit.months, "retention_duration": 2},
                                     {"copy_type": CopyTypeEnum.week, "generate_time_range": TimeRangeWeekEnum.thu,
                                      "retention_unit": RetentionTimeUnit.weeks, "retention_duration": 2}]}}}
        self.service.deal_batch_copy = Mock(return_value=None)
        self.service.get_next_archive_copy = Mock(return_value={"key": "value"})
        mock_filter_copies.return_value = None
        self.service.specified_copy(params["policy"]["ext_parameters"], [COPY_INFO],
                                    {"resource_id": "123", "policy": json.dumps(params["policy"])}, "123", 3)
        mock_filter_copies.return_value = COPY_INFO
        self.service.specified_copy(params["policy"]["ext_parameters"], [COPY_INFO],
                                    {"resource_id": "123", "policy": json.dumps(params["policy"])}, "123", 3)
        self.service.get_next_archive_copy = Mock(return_value={"copy": "copy_value"})
        self.service.submit_archive_schedule = Mock(return_value=None)
        self.service.specified_copy(params["policy"]["ext_parameters"], [COPY_INFO],
                                    {"resource_id": "123", "policy": json.dumps(params["policy"])}, "123", 3)
        self.service.submit_archive_schedule(TriggerEnum.backup_complete,
                                             {"resource_id": "123", "policy": json.dumps(params["policy"])}, COPY_INFO)

    @unittest.skip
    @mock.patch("app.restore.client.copy_client.CopyClient.query_copies")
    @mock.patch("app.backup.client.protection_client.ProtectionClient.query_sla")
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    def test_interval_archive_copy_num_return(self, _mock_query_sla, _mock_query_copies):
        """
        验证场景：内部处理归档逻辑
        前置条件：NA
        验证点：未查到副本时，内部处理归档逻辑是否正常
        """
        request = Request()
        params = {"resource_id": "abc", "sla_id": "aa"}
        _mock_query_copies.return_value = {"total": 0}
        _mock_query_sla.return_value = None
        res = self.service.interval_archive(request, **params)
        self.assertIsNone(res)

    @unittest.skip
    @mock.patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list")
    def test_query_no_archive_copy_success(self, mock_no_archive_copy_list):
        """
        验证场景：验证允许归档的列表
        前置条件：NA
        验证点：2个副本，有一个副本允许归档，有一个副本不允许归档
        """
        copy_info1 = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
                      "generated_by": "Backup",
                      "generated_time": "2021-08-12T19:27:20",
                      "properties": "{\"format\":1 } ",
                      "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
                      "resource_name": "test1",
                      "resource_type": "VM",
                      "resource_sub_type": "vim.VirtualMachine",
                      "gn": 2
                      }
        copy_info2 = {"uuid": "8bc8df042-2894-4244-9446-9f8dfa8b65d0",
                      "generated_by": "Backup",
                      "generated_time": "2021-08-12T19:27:20",
                      "properties": "{\"format\":0 } ",
                      "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
                      "resource_name": "test2",
                      "resource_type": "VM",
                      "resource_sub_type": "vim.VirtualMachine",
                      "gn": 1
                      }
        copy_list = [copy_info1, copy_info2]
        mock_no_archive_copy_list.return_value = copy_list
        no_archive_copy_list = self.service.query_no_archive_copy("1a881ee6-69f8-4615-9b7c-da599402619d", "backup", {},
                                                                  None, {})
        self.assertTrue(len(no_archive_copy_list) == 1, "len equal 1")

    @mock.patch("app.common.redis_session.redis_session.hexists", mock.Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.hset", mock.Mock())
    @mock.patch("app.common.redis_session.redis_session.hdel", mock.Mock())
    @patch("app.base.db_base.database.session", mock.Mock())
    def test_all_copy(self):
        hget_mock = mock.Mock()
        hget_mock.return_value = "[]"
        redis_session.hget = hget_mock
        self.service.get_resource_latest_copy = mock.Mock(return_value="123")
        policy = {
            "ext_parameters":
                {
                    "archiving_scope": "all_no_archiving",
                    "auto_retry_wait_minutes": 5,
                    "resource_sub_type": ResourceSubTypeEnum.Common.value,
                    "storage_id": "123"
                },

            "archiving_scope": ""}
        self.service.all_copy(policy.get("ext_parameters"), [COPY_INFO],
                              {"resource_id": "123", "policy": json.dumps(policy)}, "123", 3)

        policy["ext_parameters"]["archiving_scope"] = "latest"
        self.service.all_copy(policy.get("ext_parameters"), [COPY_INFO],
                              {"resource_id": "123", "policy": json.dumps(policy)}, "123", 3)

    @mock.patch("app.archive.service.archive_scheduler.ArchiveScheduler.create_immediate_schedule", mock.Mock())
    def test_submit_archive_schedule(self):
        self.service.check_exist_copy_job = Mock(return_value=True)
        schedule_param = {"copy_id": "1", "resource_sub_type": "hdfs", "resource_type": "hadoop", "gn": "gn1",
                          "policy": "{\"format\":1 }"}
        self.service.submit_archive_schedule(TriggerEnum.backup_complete, schedule_param, COPY_INFO)

        self.service.check_exist_copy_job = Mock(return_value=False)
        COPY_INFO["gn"] = "gn1"
        self.service.submit_archive_schedule(TriggerEnum.backup_complete, schedule_param, COPY_INFO)

    @mock.patch("redis.client.Redis.hget")
    @mock.patch("app.common.toolkit.complete_job_center_task", mock.Mock())
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    @mock.patch("redis.client.Redis.delete", mock.Mock())
    @patch("app.backup.redis.context.Context.set", Mock(return_value=None))
    @patch("app.backup.redis.context.Context.get")
    @patch("app.backup.redis.context.Context.__init__", mock.Mock(return_value=None))
    @mock.patch("app.common.license.validate_license_by_resource_type")
    @mock.patch("app.backup.client.job_client.JobClient.update_job", Mock(return_value=None))
    @mock.patch("app.common.event_messages.event.EventBase", Mock(return_value=None))
    def test_start_archive_status_fail_return(self, _mock_license_result, _mock_ctx_get, _mock_hget):
        """
        验证场景：内部处理归档逻辑
        前置条件：NA
        验证点：归档状态为失败，内部处理归档逻辑是否正常
        """
        fake_ctx_map = {
            "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
            "object_type": ""
        }
        _mock_ctx_get.return_value = lambda x, t="": fake_ctx_map.get(x)
        _mock_hget.return_value = "123"
        request = Request()
        error_desc = "error message"
        status = "fail"
        res = self.service.start_archive(request, error_desc, status)
        self.assertIsNone(res)

        """
               验证场景：内部处理归档逻辑
               前置条件：NA
               验证点：归档状态为成功，内部处理归档逻辑是否正常
        """
        _mock_license_result.return_value = False
        status = "success"
        res = self.service.start_archive(request, error_desc, status)

        _mock_license_result.return_value = True
        status = "success"
        res = self.service.start_archive(request, error_desc, status)

    @unittest.skip
    @mock.patch("app.restore.client.copy_client.CopyClient.query_copies")
    @mock.patch("app.backup.client.protection_client.ProtectionClient.query_sla")
    @mock.patch("app.backup.client.resource_client.ResourceClient.query_protected_object")
    @mock.patch("app.archive.client.archive_client.ArchiveClient.get_no_archive_copy_list")
    @mock.patch("redis.client.Redis.hset")
    @mock.patch("app.common.redis_session.redis_session.hexists")
    @mock.patch("app.archive.service.service.get_next_archive_copy")
    @mock.patch("app.base.db_base.database.session")
    @mock.patch.object(ProtectionClient, "query_sla", Mock(return_value={"uuid": "123456", "enabled": True}))
    def test_interval_archive_specified_copy_last(self, _mock_db, _mock_get_next_archive_copy, _mock_hexists,
                                                  _mock_hset,
                                                  _mock_get_no_archive_copy_list,
                                                  _mock_query_protected_object, _mock_query_sla, _mock_query_copies):
        """
        验证场景：内部处理归档逻辑
        前置条件：NA
        验证点：最后一个副本，内部处理归档逻辑是否正常
        """
        date_format_nine = "2021-09-02T11:50:57"
        _mock_get_next_archive_copy.return_value = {
            "copy": {"uuid": str(uuid.UUID), "gn": 1, "generated_time": date_format_nine,
                     "backup_type": 1, "properties": "{}",
                     "resource_sub_type": "vim.VirtualMachine", "features": 2}}
        _mock_hexists.return_value = False
        _mock_hset.return_value = "123"
        request = Request()
        params = {"resource_id": "abc", "sla_id": "aa", "policy": {
            "ext_parameters":
                {"storage_id": str(uuid.UUID), "auto_retry_times": 1, "auto_retry": 2,
                 "archive_target_type": ArchiveTypeEnum.specified_copy,
                 "specified_scope": [{"copy_type": CopyTypeEnum.year, "generate_time_range": TimeRangeYearEnum.sep,
                                      "retention_unit": RetentionTimeUnit.years, "retention_duration": 2},
                                     {"copy_type": CopyTypeEnum.month, "generate_time_range": TimeRangeMonthEnum.last,
                                      "retention_unit": RetentionTimeUnit.months, "retention_duration": 2},
                                     {"copy_type": CopyTypeEnum.week, "generate_time_range": TimeRangeWeekEnum.thu,
                                      "retention_unit": RetentionTimeUnit.weeks, "retention_duration": 2}]}}}
        _mock_query_copies.return_value = {"total": 0}
        _mock_query_sla.return_value = {"application": ResourceSubTypeEnum.ImportCopy}
        _mock_query_protected_object.return_value = {"status": Status.Active.value}
        date_format_nine = "2021-09-02T11:50:57"
        date_format_five = "2021-05-09T01:24:17"
        date_format_twenty = "2020-05-26T20:04:17"
        date_format_one = "2021-09-01T21:57:37"
        copy_list = [
            {"uuid": str(uuid.uuid4()), "gn": 1, "generated_time": date_format_nine,
             "backup_type": 1, "properties": "{}", "resource_sub_type": "vim.VirtualMachine", "worm_status": 1},
            {"uuid": str(uuid.uuid4()), "gn": 2, "generated_time": date_format_five,
             "backup_type": 1, "properties": "{}", "resource_sub_type": "vim.VirtualMachine", "worm_status": 1},
            {"uuid": str(uuid.uuid4()), "gn": 3, "generated_time": date_format_twenty,
             "backup_type": 1, "properties": "{}", "resource_sub_type": "vim.VirtualMachine", "worm_status": 1},
            {"uuid": str(uuid.uuid4()), "gn": 4, "generated_time": date_format_one,
             "backup_type": 1, "properties": "{}", "resource_sub_type": "vim.VirtualMachine", "worm_status": 1}]

        _mock_get_no_archive_copy_list.return_value = copy_list
        self.service.CheckPolicy.filter_copies = Mock(return_value=copy_list)
        self.service.check_exist_copy_job = Mock(return_value=True)
        res = self.service.interval_archive(request, **params)
        self.assertIsNone(res)

        params['policy']['ext_parameters']['archive_target_type'] = ArchiveTypeEnum.all_copy
        self.service.get_resource_latest_copy = Mock(return_value=copy_list[-1]["uuid"])

        res = self.service.interval_archive(request, **params)
        self.assertIsNone(res)

    def test_is_allow_archive(self):
        """
        验证场景：是否允许归档
        前置条件：NA
        验证点：非原生不允许归档
        """
        res = self.service.is_allow_archive(COPY_INFO)
        self.assertFalse(res)

    def test_is_allow_archive_status_invalid(self):
        """
        验证场景：是否允许归档
        前置条件：NA
        验证点：无效副本不允许归档
        """
        copy_info = {"uuid": str(uuid.uuid4()), "gn": 1, "generated_time": "2021-09-01T21:57:37",
                     "backup_type": 1, "properties": "{}", "resource_sub_type": "vim.VirtualMachine", "worm_status": 1,
                     "status": "Invalid"}
        res = self.service.is_allow_archive(copy_info)
        self.assertFalse(res)


if __name__ == '__main__':
    unittest.main(verbosity=2)
