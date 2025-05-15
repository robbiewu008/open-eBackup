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
import sys
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, patch

from app.common.clients.scheduler_client import SchedulerClient
from app.common.deploy_type import DeployType
from app.common.schemas.common_schemas import BasePage
from app.copy_catalog.schemas import CopyAntiRansomwareReport
from tests.test_cases.tools import http, env, functiontools, timezone
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.copy_catalog.common.copy_status import CopyStatus
from tests.test_cases.copy_catalog.util.mock_util import common_mock

#common_mock()
sys.modules['app.common.events.producer'] = Mock()
sys.modules['app.common.database.Database.initialize'] = Mock()
sys.modules['app.common.config'] = Mock()
sys.modules['app.common.database'] = Mock()
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
from app.copy_catalog.service.curd.copy_delete_service import check_copy_expire_fail_last_for_one_month, \
    update_copy_info_when_delete_copy_fail, update_copy_protection_info_when_delete_copy_success
from app.copy_catalog.service.curd import copy_delete_service
from app.common.enums.job_enum import JobType
from app.common.toolkit import query_job_list
from app.copy_catalog.common.common import CopyConstants
from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyProtectionTable
from app.common.redis_session import redis_session

class CopyDeleteServiceTest(unittest.TestCase):

    def setUp(self) -> None:
        self.query_job_list = query_job_list

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_job_list")
    def test_copy_expire_last_for_one_month_success(self, mock_query_job_list):
        """
        用例场景：检查副本过期时间是否超过1个月
        前置条件：副本过期时间超过1个月
        检查点：检查副本过期超过1一个成功
        :param mock_query_job_list:
        :return:
        """
        timezone = None
        now = datetime.datetime.now(tz=timezone).timestamp()
        expire_job_response = {
            "totalCount": 1,
            "records": [
                {
                    "startTime": now * 1000 - CopyConstants.MONTH_IN_SECOND * 2000
                }
            ]
        }
        copy_id = str(uuid.uuid4())
        mock_query_job_list.return_value = json.dumps(expire_job_response)
        self.assertTrue(check_copy_expire_fail_last_for_one_month(copy_id))

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_job_list")
    def test_check_copy_expire_fail_last_for_one_month_2(self, mock_query_job_list):
        """
        验证场景：检查副本过期时间是否超过1个月
        前置条件：无
        返回：查询副本列表返回值为None，返回False
        """
        copy_id = str(uuid.uuid4())
        mock_query_job_list.return_value = None
        result = check_copy_expire_fail_last_for_one_month(copy_id)
        self.assertFalse(result)

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_job_list")
    def test_check_copy_expire_fail_last_for_one_month_3(self, mock_query_job_list):
        """
        验证场景：检查副本过期时间是否超过1个月
        前置条件：这次任务之前不存在副本过期失败任务
        返回：返回False
        """
        timezone = None
        now = datetime.datetime.now(tz=timezone).timestamp()
        expire_job_response = {
            "totalCount": -1,
            "records": [
                {
                    "startTime": now * 1000 - CopyConstants.MONTH_IN_SECOND * 2000
                }
            ]
        }
        copy_id = str(uuid.uuid4())
        mock_query_job_list.return_value = json.dumps(expire_job_response)
        result = check_copy_expire_fail_last_for_one_month(copy_id)
        self.assertFalse(result)

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.check_copy_expire_fail_last_for_one_month",
                Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.send_copy_expire_failed_alarm", Mock())
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.update_copy_status", Mock())
    def test_send_alarm_msg_when_copy_expire_time_longer_than_one_month(self, _mock_session):
        """
        用例场景：检查副本过期时间超过1个月
        前置条件：副本过期超过1一个月还是失败, 非安全一体机
        检点点：功能正常，调用副本过期超过1一个月发送告警接口
        :return:
        """
        hget_mock = mock.Mock()
        hget_mock.return_value = JobType.COPY_EXPIRE.value
        redis_session.hget = hget_mock
        copy_id = str(uuid.uuid4())
        copy = CopyTable(
            uuid=copy_id,
            resource_id=str(uuid.uuid4())
        )
        request_id = str(uuid.uuid4())
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        update_copy_info_when_delete_copy_fail(copy, request_id, _mock_session)
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        update_copy_info_when_delete_copy_fail(copy, request_id, _mock_session)
        self.assertEqual(copy_delete_service.send_copy_expire_failed_alarm.call_count, 2)

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_anti_ransomware_detail")
    @mock.patch("app.copy_catalog.service.anti_ransomware_service.query_copy_anti_ransomware")
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.batch_query_job_list")
    def test_check_copy_anti_ransomware_success_when_one_count(self, mock_batch_query_job_list,
                                                               mock_query_copy_anti_ransomware,
                                                               mock_query_anti_ransomware_detail):
        """
        测试场景：查询所有未感染副本数量 如果是1 且资源下没有过期任务，则自动过期不执行
        前置条件：感染副本数量为1
        检查点：直接返回True
        :return:
        """
        copy_id = "dd1acec4-9ac6-4727-a364-02dd38a6726b"
        resource_id = "dd1acec4-9ac6-4727-a364-02dd38a6726b"
        copy = CopyTable(
            uuid=copy_id,
            resource_id=resource_id,
            status=CopyStatus.NORMAL.value,
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
        )
        copies = [
            CopyTable(uuid="dd1acec4-9ac6-4727-a364-02dd38a6726b",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1635316604912")]
        mock_query_anti_ransomware_detail.return_value = CopyAntiRansomwareReport(
                                            status=AntiRansomwareEnum.UNINFECTING.value,
                                            copy_id="dd1acec4-9ac6-4727-a364-02dd38a6726b"
                                            )
        mock_query_copy_anti_ransomware.return_value = BasePage(
            items=copies,
            total=1,
            pages=1,
            page_no=0,
            page_size=10)
        mock_batch_query_job_list.return_value = []
        self.assertTrue(copy_delete_service.check_copy_anti_ransomware(copy))

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_anti_ransomware_detail")
    @mock.patch("app.copy_catalog.service.anti_ransomware_service.query_copy_anti_ransomware")
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.batch_query_job_list")
    def test_check_copy_anti_ransomware_fail_when_not_one_count(self, mock_batch_query_job_list,
                                                                mock_query_copy_anti_ransomware,
                                                                mock_query_anti_ransomware_detail):
        """
           测试场景：查询所有未感染副本数量 如果是2 且资源下没有过期任务，则自动过期可以执行
           前置条件：感染副本数量为2,资源下没有过期任务
           检查点：直接返回False
           :return:
           """
        copy_id = "32dc2de4-97fc-4fa5-a171-be06c19121ghy"
        resource_id = "dd1acec4-9ac6-4727-a364-02dd38a6726b"
        copy = CopyTable(
            uuid=copy_id,
            resource_id=resource_id,
            status=CopyStatus.NORMAL.value,
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
        )
        copies = [
            CopyTable(uuid="dd1acec4-9ac6-4727-a364-02dd38a6726b",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1635316604912"),
            CopyTable(uuid="32dc2de4-97fc-4fa5-a171-be06c19121ghy",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1638316604912")]
        mock_query_copy_anti_ransomware.return_value = BasePage(
            items=copies,
            total=2,
            pages=1,
            page_no=0,
            page_size=10)
        mock_query_anti_ransomware_detail.return_value = CopyAntiRansomwareReport(
            status=AntiRansomwareEnum.UNINFECTING.value,
            copy_id="dd1acec4-9ac6-4727-a364-02dd38a6726b"
        )
        mock_batch_query_job_list.return_value = []
        self.assertFalse(copy_delete_service.check_copy_anti_ransomware(copy))

    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.query_anti_ransomware_detail")
    @mock.patch("app.copy_catalog.service.anti_ransomware_service.query_copy_anti_ransomware")
    @mock.patch("app.copy_catalog.service.curd.copy_delete_service.batch_query_job_list")
    def test_check_copy_anti_ransomware_fail_when_two_count_and_one_deleting(self, mock_batch_query_job_list,
                                                                             mock_query_copy_anti_ransomware,
                                                                             mock_query_anti_ransomware_detail):
        """
        测试场景：查询所有未感染副本数量 如果是2 且资源下有一个是过期任务中，则自动过期不执行
        前置条件：感染副本数量为2 且资源下有一个是过期任务中
        检查点：直接返回True
        :return:
        """
        copy_id = "dd1acec4-9ac6-4727-a364-02dd38a6726b"
        resource_id = "dd1acec4-9ac6-4727-a364-02dd38a6726b"
        copy = CopyTable(
            uuid=copy_id,
            resource_id=resource_id,
            status=CopyStatus.NORMAL.value,
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
        )
        copies = [
            CopyTable(uuid="dd1acec4-9ac6-4727-a364-02dd38a6726b",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1635316604912"),
            CopyTable(uuid="32dc2de4-97fc-4fa5-a171-be06c19121ghy",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1638316604912")]
        mock_query_anti_ransomware_detail.return_value = CopyAntiRansomwareReport(
                                            status=AntiRansomwareEnum.UNINFECTING.value,
                                            copy_id="dd1acec4-9ac6-4727-a364-02dd38a6726b"
                                            )
        mock_query_copy_anti_ransomware.return_value = BasePage(
            items=copies,
            total=1,
            pages=1,
            page_no=0,
            page_size=10)
        mock_batch_query_job_list.return_value = [{"copyId": "32dc2de4-97fc-4fa5-a171-be06c19121ghy"}]
        self.assertTrue(copy_delete_service.check_copy_anti_ransomware(copy))

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.batch_delete_schedules", Mock())
    def test_update_copy_protection_info_success_when_when_delete_copy_success(self, _mock_session):
        """
        用例场景：检查 更新副本保护对象数据
        前置条件：mock完整
        检查点：代码运行成功
        :return:
        """
        test_id = str(uuid.uuid4())
        copy = CopyTable(
            uuid=test_id,
            resource_id=test_id
        )
        copyProtection = CopyProtectionTable(
            protected_resource_id=test_id
        )
        _mock_session.query().filter().filter().count.return_value = 0
        _mock_session.query().filter().first.return_value = copyProtection
        update_copy_protection_info_when_delete_copy_success(copy, _mock_session)
        self.assertEqual(SchedulerClient.batch_delete_schedules.call_count, 1)
