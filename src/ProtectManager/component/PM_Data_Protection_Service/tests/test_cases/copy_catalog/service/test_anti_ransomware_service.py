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
from unittest import mock
from unittest.mock import Mock

from app.common.deploy_type import DeployType
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock
from tests.test_cases.tools import jwt_utils_mock

common_mock()
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType
from app.common.schemas.common_schemas import BasePage
from app.resource.models.resource_models import ResourceTable

from app.common.enums.resource_enum import ResourceSubTypeEnum

from app.copy_catalog.common.common import IndexStatus

import uuid
import datetime
import json
from tests.test_cases import common_mocker  # noqa
from app.copy_catalog.common.copy_status import CopyStatus

mock.patch("app.copy_catalog.client.dee_client.alarm_handler", mock.Mock).start()

DeployType.is_x3000_type = Mock(return_value=False)

from app.copy_catalog.service.anti_ransomware_service import query_copy_anti_ransomware, create_detection_reports, \
    query_anti_ransomware_report, modify_anti_ransomware_copy, query_anti_ransomware_copies_resource, \
    query_anti_ransomware_copies_summary, query_anti_ransomware_copies, delete_copy_anti_ransomware_report, \
    get_deleting_anti_ransomware_report, query_anti_ransomware_detail, query_anti_ransomware_copies_resource_subquery, \
    query_anti_ransomware_copies_cyber_engine, refresh_resource_detect_table, refresh_resource_detect_table_with_lock, \
    update_resource_detect_table_with_lock, release_lock
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyAntiRansomwareTable, ResourceAntiRansomwareTable
from app.common.enums.sla_enum import BackupTypeEnum
from app.copy_catalog.schemas import CopyAntiRansomwareReq
from app.copy_catalog.schemas.copy_schemas import CopyAntiRansomwareQuery

mock.patch("app.copy_catalog.service.anti_ransomware_service.get_user_info_from_token", jwt_utils_mock.get_user).start()
mock.patch("app.copy_catalog.service.anti_ransomware_service.alarm_handler", mock.Mock).start()

COPY_DATA_ASC = [CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.full.value,
                           expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                           display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                           properties=json.dumps({"backup_type": "full"}), timestamp="1638780643176000", deletable=True,
                           status="Normal", generated_by="Backup", indexed="Indexing", generation=1, retention_type=2,
                           resource_id="f124bdf9-25b7-47bf-9dd4-5926f345b9e9", resource_name="test_resource",
                           resource_type="GaussDB", resource_location="8.40.106.11", resource_status="EXIST",
                           resource_properties=json.dumps({"name": "test_resource"}),
                           user_id="1",browse_mounted="Umount")
                 ]


class AntiRansomwareTest(unittest.TestCase):
    def setUp(self):
        from app.copy_catalog.service import anti_ransomware_service
        super(AntiRansomwareTest, self).setUp()
        self.anti_ransomware_service = anti_ransomware_service

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_undetect_resources(self, _mock_session):
        resource_id = "ea539f4d-0382-405b-b237-3c5916681c2d"
        generated_by_list = ["062a12c1-d03d-4e03-a185-ca9d59d10884",
                             "c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                             "5447556f-161b-4733-9047-48720c52b87a"]
        copy_start_time = "1638316604910"
        page_no = 0
        page_size = 20
        copies = [
            CopyTable(uuid="dd1acec4-9ac6-4727-a364-02dd38a6726b",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1635316604912"),
            CopyTable(uuid="32dc2de4-97fc-4fa5-a171-be06c19121ghy",
                      resource_id=resource_id,
                      generated_by="c34fb3c3-0669-4917-b1ef-3a8c5abd3009",
                      timestamp="1638316604912")]
        _mock_session().__enter__().query().outerjoin().filter().filter().filter().filter().count.return_value = 2
        _mock_session().__enter__().query().outerjoin().filter().filter().filter().filter() \
            .order_by().limit(page_size).offset(page_no * page_size).all.return_value = copies
        copy_anti_ransoware_query = CopyAntiRansomwareQuery(**{
            "resource_id": resource_id,
            "status": AntiRansomwareEnum.UNDETECTED.value,
            "generated_by_list": generated_by_list,
            "copy_start_time": copy_start_time,
            "page_no": page_no,
            "page_size": page_size
        })
        res = query_copy_anti_ransomware(copy_anti_ransoware_query)
        self.assertIsInstance(res, BasePage)
        self.assertEqual(2, len(res.items))

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_create_detection_reports_success(self, _mock_session):
        """
        用例场景：1.检测准备；2.检测开始；3.检测完成；
        前置条件：1.DEE可用、copy_catalog可用, 非安全一体机场景
        检 查 点 ：副本检测信息修改成功
        """
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        copy_id = "ea539f4d-0382-405b-b237-3c5916681c2d"
        detectionReport = CopyAntiRansomwareReq(status=0)

        copyAntiRansomware = CopyAntiRansomwareTable(copy_id=copy_id,
                                                     status=detectionReport.status,
                                                     model=detectionReport.model,
                                                     detection_start_time=detectionReport.detection_start_time,
                                                     detection_end_time=detectionReport.detection_end_time,
                                                     report=detectionReport.report
                                                     )
        _mock_session().__enter__().merge(copyAntiRansomware)
        create_detection_reports(copy_id, detectionReport)

        detectionReport = CopyAntiRansomwareReq(status=1)
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id=copy_id,
                                                     status=detectionReport.status,
                                                     model=detectionReport.model,
                                                     detection_start_time=detectionReport.detection_start_time,
                                                     detection_end_time=detectionReport.detection_end_time,
                                                     report=detectionReport.report
                                                     )
        _mock_session().__enter__().merge(copyAntiRansomware)
        create_detection_reports(copy_id, detectionReport)

        detectionReport = CopyAntiRansomwareReq(status=2,
                                                model='1.1version',
                                                detection_start_time='20211125110400',
                                                detection_end_time='20211125110400',
                                                report='report')
        detection_duration = 10
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id=copy_id,
                                                     status=detectionReport.status,
                                                     model=detectionReport.model,
                                                     detection_start_time="2021-11-25 11:04:00",
                                                     detection_end_time="2021-11-25 11:04:10",
                                                     detection_duration=detection_duration,
                                                     report=detectionReport.report
                                                     )
        _mock_session().__enter__().merge(copyAntiRansomware)
        create_detection_reports(copy_id, detectionReport)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_create_detection_reports_cyber_engine_success(self, _mock_session):
        """
        用例场景：DEE上报：1.检测中 2.检测有结果(结果含总文件数量/新增/修改/删除/可疑文件数量/备份软件类型)
        前置条件：1.DEE可用、copy_catalog可用, 安全一体机场景
        检 查 点 ：副本检测表/资源检测表修改成功
        """
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        copy_id = "ea539f4d-0382-405b-b237-3c5916681c2d"
        detectionReport = CopyAntiRansomwareReq(status=1)
        copy = CopyTable(
            uuid='ea539f4d-0382-405b-b237-3c5916681c2d',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value
        )
        _mock_session().__enter__().query(CopyTable).filter().one_or_none.return_value = copy
        _mock_session().__enter__().query(ResourceAntiRansomwareTable).filter().one_or_none.return_value = None
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id=copy_id,
                                                     status=detectionReport.status,
                                                     model=detectionReport.model,
                                                     detection_start_time=detectionReport.detection_start_time,
                                                     detection_end_time=detectionReport.detection_end_time,
                                                     report=detectionReport.report
                                                     )
        _mock_session().__enter__().merge(copyAntiRansomware)
        create_detection_reports(copy_id, detectionReport)
        detectionReport = CopyAntiRansomwareReq(status=2,
                                                model='1.1version',
                                                detection_start_time='20211125110400',
                                                detection_end_time='20211125110400',
                                                report='report',
                                                total_file_size=100,
                                                changed_file_count=20,
                                                added_file_count=30,
                                                deleted_file_count=10,
                                                infected_file_count=5,
                                                backup_software="veeam"
                                                )
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id=copy_id,
                                                     status=detectionReport.status,
                                                     model=detectionReport.model,
                                                     detection_start_time="2021-11-25 11:04:00",
                                                     detection_end_time="2021-11-25 11:04:10",
                                                     detection_duration=10,
                                                     report=detectionReport.report,
                                                     total_file_size=100,
                                                     changed_file_count=20,
                                                     added_file_count=30,
                                                     deleted_file_count=10,
                                                     infected_file_count=5,
                                                     backup_software="veeam",
                                                     generate_type="COPY_DETECT"
                                                     )
        _mock_session().__enter__().merge(copyAntiRansomware)
        create_detection_reports(copy_id, detectionReport)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_report_success(self, _mock_session):
        """
        用例场景：1：查看无感染或已感染副本检测报告 2：查看异常副本检测报告
        前置条件：副本检测报告存在
        检 查 点 ：是否正确查询出报告信息
        """
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.VirtualMachine,
            indexed=IndexStatus.INDEXING.value
        )
        copyAntiRansomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report'
        )
        _mock_session().__enter__().query().filter().one_or_none.return_value = copy
        _mock_session().__enter__().query().filter().first.return_value = copyAntiRansomware
        result = query_anti_ransomware_report("aaa99e395aad7502b875bb08d9d34a75fb9b2a7e5265ad16659d893726fc7a9", "1")
        self.assertEqual(result.copy_id, copy.uuid)
        self.assertEqual(result.timestamp, "2021-04-15 02:00:00")
        self.assertEqual(result.model, copyAntiRansomware.model)
        self.assertEqual(result.status, copyAntiRansomware.status)
        self.assertEqual(result.detection_duration, copyAntiRansomware.detection_duration)
        self.assertEqual(result.detection_time, copyAntiRansomware.detection_start_time)
        self.assertEqual(result.report, copyAntiRansomware.report)

        error_copyAntiRansomware = CopyAntiRansomwareTable(
            status=4,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='error report'
        )
        _mock_session().__enter__().query().filter().first.return_value = error_copyAntiRansomware

        result = query_anti_ransomware_report("aaa99e395aad7502b875bb08d9d34a75fb9b2a7e5265ad16659d893726fc7a9", "1")
        self.assertIsNone(result.model)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_the_copy_is_not_exists_when_query_copy_report(self,
                                                                                                _mock_session):
        """
        用例场景：；查看副本检测报告
        前置条件：副本不存在
        检 查 点 ：抛出异常
        """
        _mock_session().__enter__().query(CopyTable).filter(CopyTable.uuid == "1").first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            query_anti_ransomware_report("token", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_the_report_is_not_exists_when_query_report(self,
                                                                                             _mock_session):
        """
        用例场景：；查看副本检测报告
        前置条件：检测报告不存在
        检 查 点 ：抛出异常
        """
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.VirtualMachine,
            indexed=IndexStatus.INDEXING.value
        )
        _mock_session().__enter__().query().filter().one_or_none.return_value = copy
        _mock_session().__enter__().query().filter().first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            query_anti_ransomware_report("token", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_modify_anti_ransomware_copies_success(self, _mock_session):
        """
        用例场景：副本误报处理
        前置条件：非安全一体机场景
        检 查 点 ：正常处理误报
        """
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        copy_anti_ransomware = CopyAntiRansomwareTable(status=AntiRansomwareEnum.INFECTING.value)
        self.anti_ransomware_service.refresh_resource_detect_table_with_lock = Mock()
        _mock_session().__enter__().query().filter().one_or_none.return_value = COPY_DATA_ASC[0]
        _mock_session().__enter__().query().filter().first.return_value = copy_anti_ransomware
        _mock_session().__enter__().query().filter().update()
        modify_anti_ransomware_copy("aaa99e395aad7502b875bb08d9d34a75fb9b2a7e5265ad16659d893726fc7a9", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_copy_not_exist_when_modify_anti_ransomware_copies(self,
                                                                                                    _mock_session):
        """
        用例场景：副本误报处理
        前置条件：副本已被删除
        检 查 点 ：是否抛出异常
        """
        _mock_session().__enter__().query().filter().one_or_none.return_value = None
        with self.assertRaises(EmeiStorBizException):
            modify_anti_ransomware_copy("token", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_report_not_exist_when_modify_anti_ransomware_copies(self,
                                                                                                      _mock_session):
        """
        用例场景：副本误报处理
        前置条件：检测报告不存在
        检 查 点 ：是否抛出异常
        """
        _mock_session().__enter__().query().filter().one_or_none.return_value = COPY_DATA_ASC[0]
        _mock_session().__enter__().query().filter().first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            modify_anti_ransomware_copy("token", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_report_not_infecting_when_modify_anti_ransomware_copies(self,
                                                                                                          _mock_session):
        """
        用例场景：副本误报处理
        前置条件：副本不是已感染状态
        检 查 点 ：是否抛出异常
        """
        copy_anti_ransomware = CopyAntiRansomwareTable(status=AntiRansomwareEnum.UNINFECTING.value)
        _mock_session().__enter__().query().filter().one_or_none.return_value = COPY_DATA_ASC[0]
        _mock_session().__enter__().query().filter().first.return_value = copy_anti_ransomware
        with self.assertRaises(EmeiStorBizException):
            modify_anti_ransomware_copy("token", "1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_latest_resource_name(self, _mock_session):
        _mock_session().__enter__().query(ResourceTable.name).filter(
            ResourceTable.uuid == "123").first.return_value = ("name1",)
        res = self.anti_ransomware_service.query_latest_resource_name(["123", "name1"])
        self.assertEqual(res, "name1")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_resource_success(self, _mock_session):
        """
        用例场景：查询资源维度统计列表；
        前置条件：入参正确
        检 查 点 ：正确查询数据
        """
        page_no = 0
        page_size = 10
        resource_sub_type = "vim.VirtualMachine"
        # 无数据
        _mock_session().__enter__().query().count.return_value = 0
        resultPage = query_anti_ransomware_copies_resource(("token", resource_sub_type, page_no, page_size, None, None))
        self.assertEqual(0, resultPage.total)

        # 资源维度列表
        orders = "+total_copy_num"
        _mock_session().__enter__().query().count.return_value = 3
        query = [
            ("1", "name1", "location1", "policy1", 10000, 21, 1, 2, 3, 4, 5, 6, "tenant", "0", "2020-2-1 12:10:20", ""),
            ("2", "name2", "location2", "policy1", 10000, 4, 0, 0, 1, 1, 1, 1, "tenant", "0", "2020-2-1 12:10:20", ""),
            ("3", "name3", "location3", "policy1", 10000, 3, 0, 0, 0, 1, 1, 1, "tenant", "0", "2020-2-1 12:10:20", "")]
        _mock_session().__enter__().query().join().join().order_by() \
            .limit(page_size).offset(page_no * page_size).all.return_value = query
        self.anti_ransomware_service.query_latest_resource_name = Mock(return_value="name1")
        _mock_session().__enter__().query().join().join().count.return_value = 3
        resultPage = query_anti_ransomware_copies_resource(
            ("token", resource_sub_type, page_no, page_size, orders, None))
        self.assertEqual(3, resultPage.total)
        copyAntiRansomwareStatistics = resultPage.items[0]
        self.assertEqual(21, copyAntiRansomwareStatistics.total_copy_num)
        self.assertEqual(1, copyAntiRansomwareStatistics.uninspected_copy_num)
        self.assertEqual(2, copyAntiRansomwareStatistics.prepare_copy_num)
        self.assertEqual(3, copyAntiRansomwareStatistics.detecting_copy_num)
        self.assertEqual(4, copyAntiRansomwareStatistics.uninfected_copy_num)
        self.assertEqual(5, copyAntiRansomwareStatistics.infected_copy_num)
        self.assertEqual(6, copyAntiRansomwareStatistics.abnormal_copy_num)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_latest_detection_time_correspond_tenant_info(self, _mock_session):
        _mock_session().__enter__().query().join().filter().filter().filter().group_by().first.return_value = None
        res = self.anti_ransomware_service.query_latest_detection_time_correspond_tenant_info("123")
        self.assertEqual(res, ("", "", ""))

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_resource_when_tenant_is_none(self, _mock_session):
        """
        用例场景：查询资源维度统计列表；
        前置条件：入参正确
        检 查 点 ：正确查询数据
        """
        page_no = 0
        page_size = 10
        resource_sub_type = "vim.VirtualMachine"
        # 无数据
        _mock_session().__enter__().query().count.return_value = 0
        resultPage = query_anti_ransomware_copies_resource(("token", resource_sub_type, page_no, page_size, None, None))
        self.assertEqual(0, resultPage.total)

        # 资源维度列表
        orders = "+total_copy_num"
        _mock_session().__enter__().query().count.return_value = 3
        query = [("1", "name1", "location1", "policy1", 10000, 21, 1, 2, 3, 4, 5, 6, None, None, None, ""),
                 ("2", "name2", "location2", "policy1", 10000, 4, 0, 0, 1, 1, 1, 1, None, None, None, ""),
                 ("3", "name3", "location3", "policy1", 10000, 3, 0, 0, 0, 1, 1, 1, None, None, None, "")]
        _mock_session().__enter__().query().join().join().order_by() \
            .limit(page_size).offset(page_no * page_size).all.return_value = query

        self.anti_ransomware_service.query_latest_detection_time_correspond_tenant_info = Mock(
            return_value=("", "", ""))
        self.anti_ransomware_service.query_latest_resource_name = Mock(return_value="name1")
        _mock_session().__enter__().query().join().join().count.return_value = 3
        resultPage = query_anti_ransomware_copies_resource(
            ("token", resource_sub_type, page_no, page_size, orders, None))
        self.assertEqual(3, resultPage.total)
        copyAntiRansomwareStatistics = resultPage.items[0]
        self.assertEqual(21, copyAntiRansomwareStatistics.total_copy_num)
        self.assertEqual(1, copyAntiRansomwareStatistics.uninspected_copy_num)
        self.assertEqual(2, copyAntiRansomwareStatistics.prepare_copy_num)
        self.assertEqual(3, copyAntiRansomwareStatistics.detecting_copy_num)
        self.assertEqual(4, copyAntiRansomwareStatistics.uninfected_copy_num)
        self.assertEqual(5, copyAntiRansomwareStatistics.infected_copy_num)
        self.assertEqual(6, copyAntiRansomwareStatistics.abnormal_copy_num)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_orders_abnormal_when_query_copies_resource(self, _mock_session):
        """
        用例场景：查询资源维度统计列表；
        前置条件：抛出异常
        检 查 点 ：正确查询数据
        """
        orders = "+total_copy_num222"
        page_no = 0
        page_size = 10
        resource_sub_type = "vim.VirtualMachine"
        with self.assertRaises(EmeiStorBizException):
            query_anti_ransomware_copies_resource(("token", resource_sub_type, page_no, page_size, orders, None))

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_summary_success(self, _mock_session):
        """
        用例场景：查询检测总览；
        前置条件：入参正确
        检 查 点 ：正确查询数据
        """
        summary = [("vim.VirtualMachine", 21, 1, 2, 3, 4, 5, 6)]
        resource_sub_type = ["vim.VirtualMachine"]
        _mock_session().__enter__().query().join().outerjoin().filter().filter().group_by().all.return_value = summary
        result_summary = query_anti_ransomware_copies_summary("token", resource_sub_type, "")
        copyAntiRansomwareSummary = result_summary[0]
        self.assertEqual(21, copyAntiRansomwareSummary.total_copy_num)
        self.assertEqual(1, copyAntiRansomwareSummary.uninspected_copy_num)
        self.assertEqual(2, copyAntiRansomwareSummary.prepare_copy_num)
        self.assertEqual(3, copyAntiRansomwareSummary.detecting_copy_num)
        self.assertEqual(4, copyAntiRansomwareSummary.uninfected_copy_num)
        self.assertEqual(5, copyAntiRansomwareSummary.infected_copy_num)
        self.assertEqual(6, copyAntiRansomwareSummary.abnormal_copy_num)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_summary_by_period_success(self, _mock_session):
        """
        用例场景：按时间查询检测总览；
        前置条件：入参正确
        检 查 点 ：正确查询数据
        """
        summary = [(str(datetime.datetime.now().date()), 21, 1, 2, 3, 4, 5, 6)]
        resource_sub_type = ["vim.VirtualMachine"]
        _mock_session().__enter__().query().join().outerjoin().filter().filter().group_by().order_by().all.return_value = summary
        result_summary = query_anti_ransomware_copies_summary("token", resource_sub_type, "week")
        copyAntiRansomwareSummary = result_summary[-1]
        self.assertEqual(21, copyAntiRansomwareSummary.total_copy_num)
        self.assertEqual(1, copyAntiRansomwareSummary.uninspected_copy_num)
        self.assertEqual(2, copyAntiRansomwareSummary.prepare_copy_num)
        self.assertEqual(3, copyAntiRansomwareSummary.detecting_copy_num)
        self.assertEqual(4, copyAntiRansomwareSummary.uninfected_copy_num)
        self.assertEqual(5, copyAntiRansomwareSummary.infected_copy_num)
        self.assertEqual(6, copyAntiRansomwareSummary.abnormal_copy_num)
        self.assertEqual(7, len(result_summary))

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_success(self, _mock_session):
        """
        用例场景：副本维度检测列表
        前置条件：无
        检 查 点 ：正确查询数据
        """
        orders = "-display_timestamp"
        page_no = 0
        page_size = 10
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.VirtualMachine,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="aaa",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            browse_mounted="Umount"
        )
        copyAntiRansomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report'
        )
        _mock_session().__enter__().query().outerjoin().filter().filter().filter().filter().count.return_value = 1
        _mock_session().__enter__().query().outerjoin().filter().filter().filter().filter().order_by(
            CopyTable.display_timestamp.asc()) \
            .limit(page_size).offset(page_no * page_size).all.return_value = [(copy, copyAntiRansomware)]
        condition = json.dumps({"uuid": "1"})
        result = query_anti_ransomware_copies("token", "aaa", page_no, page_size, orders, condition)
        resultCopyAntiRansomware = result.items[0]
        self.assertEqual(copy.uuid, resultCopyAntiRansomware.uuid)
        self.assertEqual(copyAntiRansomware.status, resultCopyAntiRansomware.anti_status)
        self.assertEqual(copyAntiRansomware.detection_start_time, resultCopyAntiRansomware.detection_time)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_cyber_engine_success(self, _mock_session):
        """
        用例场景：副本维度检测列表, 子查询
        前置条件：安全一体机场景
        检 查 点 ：正确查询数据
        """
        resource_with_tenant = [{'c': {'tenant_id': '1', 'tenant_name': 'v1'}}]
        self.anti_ransomware_service.query_anti_ransomware_copies_resource_subquery_cyber_engine = Mock(
            resource_with_tenant)

        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        orders = "-display_timestamp"
        page_no = 0
        page_size = 10
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="aaa",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        copyAntiRansomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report',
            generate_type='COPY_DETECT',
            total_file_size=100,
            changed_file_count=20,
            added_file_count=30,
            deleted_file_count=10
        )
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter() \
            .count.return_value = 1
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter().order_by() \
            .limit(page_size).offset(page_no * page_size).all.return_value = [
            (copy, copyAntiRansomware, '1', 'v1', 1, "device1")]
        self.anti_ransomware_service.query_cyber_detect_devices = Mock(
            return_value={"device1": {"uuid": "device1", "name": "device", "endpoint": "127.0.0.1"}})
        result = query_anti_ransomware_copies_cyber_engine("token", "aaa", page_no, page_size, orders, None)
        resultCopyAntiRansomware = result.items[0]
        self.assertEqual(copy.uuid, resultCopyAntiRansomware.uuid)
        self.assertEqual(copyAntiRansomware.total_file_size, resultCopyAntiRansomware.total_file_size)
        self.assertEqual(copyAntiRansomware.changed_file_count, resultCopyAntiRansomware.changed_file_count)
        self.assertEqual(copyAntiRansomware.added_file_count, resultCopyAntiRansomware.added_file_count)
        self.assertEqual(copyAntiRansomware.deleted_file_count, resultCopyAntiRansomware.deleted_file_count)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_with_condition_cyber_engine_success(self, _mock_session):
        """
        用例场景：副本维度检测列表, 子查询, 带查询条件, uuid
        前置条件：安全一体机场景
        检 查 点 ：正确查询数据
        """
        resource_with_tenant = [{'c': {'tenant_id': '1', 'tenant_name': 'v1'}}]
        self.anti_ransomware_service.query_anti_ransomware_copies_resource_subquery_cyber_engine = Mock(
            resource_with_tenant)
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        orders = "-display_timestamp"
        page_no = 0
        page_size = 10
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="aaa",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        copy_anti_ransomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report',
            generate_type='COPY_DETECT',
            total_file_size=100,
            changed_file_count=20,
            added_file_count=30,
            deleted_file_count=10
        )
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter().filter() \
            .count.return_value = 1
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter().filter() \
            .order_by().limit(page_size).offset(page_no * page_size).all.return_value = [
            (copy, copy_anti_ransomware, '1', 'v1', 1, "device1")]
        self.anti_ransomware_service.query_cyber_detect_devices = Mock(
            return_value={"device1": {"uuid": "device1", "name": "device", "endpoint": "127.0.0.1"}})
        result = query_anti_ransomware_copies_cyber_engine("token", "aaa", page_no, page_size, orders,
                                                           '{"uuid":"1"}')
        result_copy_anti_ransomware = result.items[0]
        self.assertEqual(copy.uuid, result_copy_anti_ransomware.uuid)
        self.assertEqual(copy_anti_ransomware.total_file_size, result_copy_anti_ransomware.total_file_size)
        self.assertEqual(copy_anti_ransomware.changed_file_count, result_copy_anti_ransomware.changed_file_count)
        self.assertEqual(copy_anti_ransomware.added_file_count, result_copy_anti_ransomware.added_file_count)
        self.assertEqual(copy_anti_ransomware.deleted_file_count, result_copy_anti_ransomware.deleted_file_count)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_with_condition_status_cyber_engine_success(self, _mock_session):
        """
        用例场景：副本维度检测列表, 子查询, 带查询条件, anti_status/copy_status
        前置条件：安全一体机场景
        检 查 点 ：正确查询数据
        """
        resource_with_tenant = [{'c': {'tenant_id': '1', 'tenant_name': 'v1'}}]
        self.anti_ransomware_service.query_anti_ransomware_copies_resource_subquery_cyber_engine = Mock(
            resource_with_tenant)
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        orders = "-display_timestamp"
        page_no = 0
        page_size = 10
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="aaa",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        copy_anti_ransomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report',
            generate_type='COPY_DETECT',
            total_file_size=100,
            changed_file_count=20,
            added_file_count=30,
            deleted_file_count=10
        )
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter().filter() \
            .count.return_value = 1
        _mock_session().__enter__().query().outerjoin().join().join().filter().filter().filter().filter().filter() \
            .order_by().limit(page_size).offset(page_no * page_size).all.return_value = [
            (copy, copy_anti_ransomware, '1', 'v1', 1, "device1")]
        self.anti_ransomware_service.query_cyber_detect_devices = Mock(
            return_value={"device1": {"uuid": "device1", "name": "device", "endpoint": "127.0.0.1"}})
        result = query_anti_ransomware_copies_cyber_engine("token", "aaa", page_no, page_size, orders,
                                                           '{"anti_status":[2],"copy_status":["Deleting"]}')
        result_copy_anti_ransomware = result.items[0]
        self.assertEqual(copy.uuid, result_copy_anti_ransomware.uuid)
        self.assertEqual(copy_anti_ransomware.total_file_size, result_copy_anti_ransomware.total_file_size)
        self.assertEqual(copy_anti_ransomware.changed_file_count, result_copy_anti_ransomware.changed_file_count)
        self.assertEqual(copy_anti_ransomware.added_file_count, result_copy_anti_ransomware.added_file_count)
        self.assertEqual(copy_anti_ransomware.deleted_file_count, result_copy_anti_ransomware.deleted_file_count)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    @unittest.skip
    def test_query_anti_ransomware_resources_cyber_engine_success(self, _mock_session):
        """
        用例场景：副本维度检测列表
        前置条件：安全一体机场景
        检 查 点 ：正确查询数据
        """
        resource_with_tenant = [{'c': {'tenant_id': '1', 'tenant_name': 'v1'}}]
        self.anti_ransomware_service.query_anti_ransomware_copies_resource_subquery_cyber_engine = Mock(
            return_value=_mock_session().__enter__())
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        orders = "-display_timestamp"
        page_no = 0
        page_size = 10
        copy = CopyTable(
            uuid='bc659141-3594-458e-80af-cc2fb80c194b',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="aaa",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        copyAntiRansomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report',
            generate_type='COPY_DETECT',
            total_file_size=100,
            changed_file_count=10,
            added_file_count=20,
            deleted_file_count=30
        )
        _mock_session().__enter__().count.return_value = 1
        self.anti_ransomware_service.query_anti_ransomware_resources_data_cyber_engine = Mock(
            return_value=_mock_session().__enter__())
        copies_statistics = [["aaa", "gtest", "/root", "127.0.0.1", "dorado", "1", "tenant1",
                              "2023-03-31 02:00:00", 2, 100, 20, 10, 30, "2023-03-31 02:00:00",
                              "bc659141-3594-458e-80af-cc2fb80c194b", datetime.datetime(2023, 3, 31, 2, 0, 0),
                              datetime.datetime(2023, 3, 31, 2, 0, 0), 1, "device1", 0, "veeam", "IO_DETECT"]]
        _mock_session().__enter__().limit().offset().all.return_value = copies_statistics
        _mock_session().__enter__().query(ResourceTable.name).filter(
            ResourceTable.uuid == "123").first.return_value = ("qtest",)
        _mock_session().__enter__().query().join().group_by().all.return_value = [("aaa", 1, 0, 0, 1)]
        self.anti_ransomware_service.query_cyber_detect_devices = Mock(
            return_value={"device1": {"uuid": "device1", "name": "device", "endpoint": "127.0.0.1"}})
        result = self.anti_ransomware_service.query_anti_ransomware_resources_cyber_engine(("token", page_no,
                                                                                            page_size, orders, None))
        result_resource_anti_ransomware = result.items[0]
        self.assertEqual(copy.uuid, result_resource_anti_ransomware.latest_copy_id)
        self.assertEqual(copyAntiRansomware.total_file_size, result_resource_anti_ransomware.total_file_size)
        self.assertEqual(copyAntiRansomware.changed_file_count, result_resource_anti_ransomware.changed_file_count)
        self.assertEqual(copyAntiRansomware.added_file_count, result_resource_anti_ransomware.added_file_count)
        self.assertEqual(copyAntiRansomware.deleted_file_count, result_resource_anti_ransomware.deleted_file_count)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_resources_success(self, _mock_session):
        """
        用例场景：副本维度检测列表
        前置条件：非安全一体机场景
        检 查 点 ：正确查询数据
        """
        _mock_session.query = Mock(
            side_effect=[_mock_session.query(), _mock_session.query(), _mock_session.query(), ["resource"]])
        _mock_session.query.outerjoin.outerjoin = Mock(return_value=None)
        _mock_session.query.outerjoin.outerjoin.filter.filter = Mock(return_value=None)
        _mock_session.query.outerjoin.join.outerjoin.filter.filter.filter = Mock(return_value=None)
        self.anti_ransomware_service.check_user_is_admin_or_audit = Mock(return_value=True)
        res = self.anti_ransomware_service.query_anti_ransomware_copies_resource_subquery(_mock_session, ["FileSet"],
                                                                                          "", "")
        self.assertEqual(res, ["resource"])

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_refresh_resource_detect_table_success(self, _mock_session):
        """
        用例场景：刷新资源检测表
        前置条件：1. 原资源检测结果不存在(非安全一体机场景，升级) 2. 原资源检测结果存在, 当前删除的不是资源的最后一个副本
                3. 原资源检测结果存在, 当前删除的不是资源的最后一个副本
        检 查 点 ：1. 不报错, 不更新资源检测表  2. 成功更新资源检测表  3. 成功删除资源检测表记录
        """
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="ea539f4d-0382-405b-b237-3c5916681c2d",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        copy_anti_ransomware = CopyAntiRansomwareTable(
            model="1.1version",
            status=2,
            detection_duration=3600,
            detection_start_time='2021-11-29 00:00:00',
            report='report',
            generate_type='COPY_DETECT',
            total_file_size=100,
            changed_file_count=20,
            added_file_count=30,
            deleted_file_count=10
        )
        _mock_session().__enter__().query().join().filter().filter().order_by().limit().one_or_none.return_value = (
            copy, copy_anti_ransomware)

        # 1. 原资源检测结果不存在
        _mock_session().__enter__().query().filter().one_or_none.return_value = None
        refresh_resource_detect_table("ea539f4d-0382-405b-b237-3c5916681c2d")

        # 2. 原资源检测结果存在, 当前删除的不是资源的最后一个副本
        refresh_resource_detect_table("ea539f4d-0382-405b-b237-3c5916681c2d")

        # 3. 原资源检测结果存在, 当前删除的是资源的最后一个副本
        _mock_session().__enter__().query().join().filter().filter().order_by().limit().one_or_none.return_value = None
        refresh_resource_detect_table("ea539f4d-0382-405b-b237-3c5916681c2d")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_update_resource_detect_table_with_lock_success(self, _mock_session):
        """
        用例场景：带锁更新资源检测表
        前置条件：1. 无
        检 查 点 ：1. 不报错
        """
        self.anti_ransomware_service.update_resource_detect_table = Mock()
        from app.common.lock.lock_manager import lock_manager
        lock = lock_manager.get_lock('key')
        lock_manager.get_lock = Mock(return_value=lock)
        lock.lock = Mock(return_value=True)
        self.anti_ransomware_service.release_lock = Mock()
        copy = CopyTable(
            uuid='1',
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem,
            indexed=IndexStatus.INDEXING.value,
            deletable=True,
            generation=1,
            resource_id="ea539f4d-0382-405b-b237-3c5916681c2d",
            resource_type="VM",
            resource_location="/root",
            resource_status="1",
            resource_properties="properties",
            retention_type=True,
            worm_status=3,
            browse_mounted="Umount"
        )
        detection_report = CopyAntiRansomwareReq(status=1)
        update_resource_detect_table_with_lock(copy, detection_report, _mock_session)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_release_lock_success(self, _mock_session):
        """
        用例场景：测试释放锁
        前置条件：1. 已上锁 2.未上锁
        检 查 点 ：1/2均无异常
        """
        from app.common.lock.lock_manager import lock_manager
        lock = lock_manager.get_lock('key')
        lock_manager.get_lock = Mock(return_value=lock)
        lock.is_locked = Mock(return_value=False)
        release_lock(lock)

        lock.is_locked = Mock(return_value=True)
        lock.unlock = Mock()
        release_lock(lock)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_refresh_resource_detect_table_with_lock_success(self, _mock_session):
        """
        用例场景：带锁刷新资源检测表
        前置条件：1. 无
        检 查 点 ：1. 不报错
        """
        self.anti_ransomware_service.refresh_resource_detect_table = Mock()
        from app.common.lock.lock_manager import lock_manager
        lock = lock_manager.get_lock('key')
        lock_manager.get_lock = Mock(return_value=lock)
        lock.lock = Mock(return_value=True)
        self.anti_ransomware_service.release_lock = Mock()
        refresh_resource_detect_table_with_lock("ea539f4d-0382-405b-b237-3c5916681c2d")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_delete_copy_anti_ransomware_report_success(self, _mock_session):
        """
        用例场景: 刪除副本
        前置条件：有防勒索检测报告
        检 查 点 ：正常处理副本的检测报告
        """
        report = CopyAntiRansomwareTable(copy_id="adgag")
        _mock_session().__enter__().query().filter().first.return_value = report
        delete_copy_anti_ransomware_report("adgag")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_return_void_if_report_not_exists_when_delete_copy_report(self, _mock_session):
        """
        用例场景: 刪除副本
        前置条件：沒有防勒索检测报告
        检 查 点 ：正常处理副本的检测报告
        """
        _mock_session().__enter__().query().filter().first.return_value = None
        delete_copy_anti_ransomware_report("adgag")

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_get_deleting_anti_ransomware_report_success(self, _mock_session):
        """
        用例场景: 刪除副本
        前置条件： 无
        检 查 点 ：正常返回副本防勒索检测信息
        """
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id="1",
                                                     status=1,
                                                     model="1.1.version",
                                                     detection_start_time="2021-11-25 11:04:00",
                                                     detection_end_time="2021-11-25 11:04:10",
                                                     detection_duration=60,
                                                     report="roeport"
                                                     )
        _mock_session().__enter__().query().filter().one_or_none.return_value = copyAntiRansomware
        result = get_deleting_anti_ransomware_report("1")
        self.assertEqual(1, result.status)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_detail_success(self, _mock_session):
        """
        用例场景: 手动执行防勒索副本检测时，查询副本检测信息
        前置条件： 无
        检 查 点 ：正常返回副本防勒索检测信息
        """
        _mock_session().__enter__().query().filter().one_or_none.return_value = COPY_DATA_ASC[0]
        _mock_session().__enter__().query().filter().first.return_value = None
        result = query_anti_ransomware_detail("1")
        self.assertEqual(AntiRansomwareEnum.UNDETECTED.value, result.status)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_query_anti_ransomware_copies_resource_subquery(self, _mock_session):
        """
        用例场景: 根据租户名称查询数据
        前置条件： 无
        检 查 点 ：正常返回副本防勒索检测信息
        """
        self.anti_ransomware_service.check_user_is_admin_or_audit = Mock(return_value=True)
        copyAntiRansomware = CopyAntiRansomwareTable(copy_id="1",
                                                     status=1,
                                                     model="1.1.version",
                                                     detection_start_time="2021-11-25 11:04:00",
                                                     detection_end_time="2021-11-25 11:04:10",
                                                     detection_duration=60,
                                                     report="roeport",
                                                     tenant_id='123',
                                                     tenant_name="sys",
                                                     )
        conditions = '{"tenant_name": "sys"}'
        res = query_anti_ransomware_copies_resource_subquery(_mock_session, ['CloudBackupFileSystem'], conditions,
                                                             '123')
        self.assertIsNotNone(res)

        @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
        def test_sysadmin_query_anti_ransomware_copies_resource_subquery(self, _mock_session):
            """
            用例场景: 根据租户名称查询数据
            前置条件： 无
            检 查 点 ：正常返回副本防勒索检测信息
            """
            self.anti_ransomware_service.check_user_is_admin = Mock(return_value=True)
            copyAntiRansomware = CopyAntiRansomwareTable(copy_id="1",
                                                         status=1,
                                                         model="1.1.version",
                                                         detection_start_time="2021-11-25 11:04:00",
                                                         detection_end_time="2021-11-25 11:04:10",
                                                         detection_duration=60,
                                                         report="roeport",
                                                         tenant_id='123',
                                                         tenant_name="sys",
                                                         )
            conditions = '{"tenant_name": "sys"}'
            res = query_anti_ransomware_copies_resource_subquery(_mock_session, ['CloudBackupFileSystem'], conditions,
                                                                 '123')
            self.assertIsNotNone(res)

    def test_get_copy_is_security_snap(self):
        """
        用例场景: 判断是否安全快照
        前置条件： 主存防勒索
        检 查 点 ：根据base返回的快照信息判断是否安全快照
        """
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        date = {
            "id": "快照id",
            "name": "快照名称",
            "isSecuritySnap": True,
            "isInProtectionPeriod": True
        }

        self.anti_ransomware_service.SystemBaseClient.query_local_storage_fssnapshot = Mock(return_value=date)
        res = self.anti_ransomware_service.get_copy_is_security_snap(COPY_DATA_ASC[0])
        self.assertEqual(res, True)

    def test_get_copy_is_security_snap_cyber_engine(self):
        """
        用例场景: 判断是否安全快照
        前置条件： 安全一体机
        检 查 点 ：根据base返回的快照信息判断是否安全快照
        """
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        env_info = {
            "root_uuid": "123"
        }
        self.anti_ransomware_service.ResourceClient.query_resource = Mock(return_value=env_info)

        data_cannot_delete = {
            "id": "快照id",
            "name": "快照名称",
            "isSecuritySnap": True,
            "isInProtectionPeriod": True
        }
        self.anti_ransomware_service.SystemBaseClient.query_remote_storage_fssnapshot = Mock(
            return_value=data_cannot_delete)
        res = self.anti_ransomware_service.get_copy_is_security_snap(COPY_DATA_ASC[0])
        self.assertEqual(res, True)

        data_can_delete_expire = {
            "id": "快照id",
            "name": "快照名称",
            "isSecuritySnap": True,
            "isInProtectionPeriod": False
        }
        self.anti_ransomware_service.SystemBaseClient.query_remote_storage_fssnapshot = Mock(
            return_value=data_can_delete_expire)
        res = self.anti_ransomware_service.get_copy_is_security_snap(COPY_DATA_ASC[0])
        self.assertEqual(res, False)

        data_can_delete_not_security = {
            "id": "快照id",
            "name": "快照名称",
            "isSecuritySnap": False,
            "isInProtectionPeriod": False
        }
        self.anti_ransomware_service.SystemBaseClient.query_remote_storage_fssnapshot = Mock(
            return_value=data_can_delete_not_security)
        res = self.anti_ransomware_service.get_copy_is_security_snap(COPY_DATA_ASC[0])
        self.assertEqual(res, False)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    @mock.patch("app.common.deploy_type.DeployType.is_x3000_type", Mock(return_value=True))
    def test_should_raise_EmeiStorBizException_if_deploy_type_is_x3000_when_query_copies_resource(self, _mock_session):
        """
        用例场景：x3000部署方式下查询资源维度统计列表；
        前置条件：抛出异常
        检 查 点 ：抛出异常
        """
        orders = "+total_copy_num222"
        page_no = 0
        page_size = 10
        resource_sub_type = "vim.VirtualMachine"
        with self.assertRaises(EmeiStorBizException):
            query_anti_ransomware_copies_resource(("token", resource_sub_type, page_no, page_size, orders, None))
