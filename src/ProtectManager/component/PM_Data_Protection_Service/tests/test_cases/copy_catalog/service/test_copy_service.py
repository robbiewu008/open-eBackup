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
from unittest.mock import MagicMock

from app.common.deploy_type import DeployType
from tests.test_cases.copy_catalog.util.mock_util import common_mock
from tests.test_cases import common_mocker # noqa
common_mock()
from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC
from app.copy_catalog.util.copy_util import check_copy_status
from app.protection.object.models.projected_object import ProtectedObject
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType

from app.common.enums.resource_enum import ResourceSubTypeEnum

from app.copy_catalog.common.common import GenIndexType, IndexStatus

import uuid
import datetime
import json

from tests.test_cases import common_mocker # noqa
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.service.curd.copy_delete_service import check_copy_can_be_deleted
from app.copy_catalog.service.curd.copy_update_service import update_copy_detail, update_resource_copy_index_status, \
    update_copy_index_status
from app.copy_catalog.service import copy_delete_workflow
from app.copy_catalog.service.curd import copy_update_service, copy_delete_service
from app.copy_catalog.service.copy_expire_service import query_recent_expiring_copies
from app.copy_catalog.service.curd.copy_create_service import send_copy_save_event,send_copy_save_event_if_need_forward
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyAntiRansomwareTable
from app.common.enums.sla_enum import BackupTypeEnum
from app.copy_catalog.schemas import CopySchema
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_resource_properties, check_copy_name_valid, \
    query_resource_type_name, get_resource_latest_copy

from pydantic import BaseModel

from unittest import mock
from unittest.mock import Mock


copy_anti_ransomware_table = CopyAntiRansomwareTable(
    copy_id=str(uuid.uuid4()),
    status=AntiRansomwareEnum.UNDETECTED)


class MockBaseExtParam(BaseModel):
    ext_parameters = {}


protected_object = ProtectedObject(uuid="34926913-b6a6-4c3b-812e-4fba893cbe55",
                                   resource_id="d8893969-a01e-4b8c-be8d-f09a56d74e33",
                                   sub_type="Nas",
                                   chain_id="34926913-b6a6-4c3b-812e-4fba893cbe66")


class CopyServiceTest(unittest.TestCase):
    copy_service_prefix_for_test = "app.copy_catalog.service.copy_service."

    def setUp(self):
        super(CopyServiceTest, self).setUp()
        self.copy_service = copy_delete_service
        self.copy_delete_workflow = copy_delete_workflow
        self.check_copy_status = check_copy_status
        self.query_recent_expiring_copies = query_recent_expiring_copies
        self.get_resource_latest_copy = get_resource_latest_copy
        self.query_copy_by_resource_properties = query_copy_by_resource_properties
        self.check_copy_can_be_deleted = check_copy_can_be_deleted
        copy_delete_service.get_deleting_anti_ransomware_report = Mock(return_value=copy_anti_ransomware_table)
        self.update_copy_detail = update_copy_detail

    def test_check_copy_restore_status(self, copy_id=str(uuid.uuid4()), status=CopyStatus.RESTORING):
        """
            副本状态为恢复时，删除副本失败

           期望：
            抛出EmeiStorBizException异常，无法删除副本
            :return:
        """
        self.assertRaises(EmeiStorBizException, self.check_copy_status, copy_id, status, True)

    @mock.patch("sqlalchemy.orm.Query.all")
    def test_query_recent_expiring_copies(self, mock_query_all):
        """
                副本状态为恢复时，删除副本失败

               期望：
                抛出EmeiStorBizException异常，无法删除副本
                :return:
        """
        mock_query_all.return_value = []
        copies = self.query_recent_expiring_copies()
        self.assertEqual(0, len(copies))

    @mock.patch("sqlalchemy.orm.Query.one_or_none")
    def test_get_resource_latest_copy(self, mock_query_one_or_more):
        """
                副本状态为恢复时，删除副本失败

               期望：
                抛出EmeiStorBizException异常，无法删除副本
                :return:
        """
        mock_query_one_or_more.return_value = None
        result = self.get_resource_latest_copy('123', 'Backup')
        self.assertEqual(result, None)

    @mock.patch("pydantic.BaseModel.from_orm")
    @mock.patch("sqlalchemy.orm.query.Query.all")
    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_send_copy_save_event(self, mock_query_first, mock_query_all, mock_from_orm):
        copy_id = "9b17382f-7164-4f5b-8d77-2910a0be348c"
        res = send_copy_save_event(copy_id)
        copy_obj = CopyTable(uuid=copy_id, backup_type=BackupTypeEnum.cumulative_increment.value,
                             expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                             display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                             properties=json.dumps({"backup_type": "cumulative_increment"}),
                             generated_by=GenerationType.BY_BACKUP.value,
                             resource_sub_type=ResourceSubTypeEnum.NasFileSystem.value)
        mock_query_first.return_value = copy_obj
        generation_type = copy_obj.generated_by
        resource_sub_type = copy_obj.resource_sub_type
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        self.assertEqual(resource_sub_type, ResourceSubTypeEnum.NasFileSystem.value)
        self.assertEqual(generation_type, "Backup")
        self.assertIsNone(res)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("app.copy_catalog.service.curd.copy_create_service.check_is_need_to_forward",  Mock(return_value=True))
    @mock.patch("app.copy_catalog.service.curd.copy_create_service.send_create_index_forward_request")
    def test_send_copy_save_event_forward(self, mock_query_first, mock_send_create_index_forward_request):
        copy_id = "9b17382f-7164-4f5b-8d77-2910a0be348c"
        copy_obj = CopyTable(uuid=copy_id, backup_type=BackupTypeEnum.cumulative_increment.value,
                             expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                             display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                             properties=json.dumps({"backup_type": "cumulative_increment"}),
                             generated_by=GenerationType.BY_BACKUP.value,
                             resource_sub_type=ResourceSubTypeEnum.NasFileSystem.value)
        mock_query_first.return_value = copy_obj
        send_copy_save_event_if_need_forward(copy_id)
        self.assertTrue(mock_send_create_index_forward_request.called)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_copy_by_resource_properties(self, mock_query_first):
        root_uuid = "ac03060fd4e111ebaf32286ed488d337"
        mock_query_first.return_value = COPY_DATA_ASC[0]
        result = self.query_copy_by_resource_properties(root_uuid, "Backup", ResourceSubTypeEnum.GaussDB)
        copy_data = CopySchema(**COPY_DATA_ASC[0].as_dict())
        self.assertEqual(copy_data, result)

    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_should_raise_EmeiStorBizException_if_update_copy_detail_when_copy_not_exist(self, _mock_session):
        """
        用例场景：更新副本详情detail字段
        前置条件：副本不存在
        检查点：更新不存在的副本抛异常
        :return:
        """
        copy_id = str(uuid.uuid4())
        detail = "test"
        _mock_session().__enter__().query().filter(CopyTable.uuid == copy_id).first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            update_copy_detail(copy_id, detail)

    def test_should_raise_EmeiStorBizException_if_update_resource_copy_index_status_when_status_illegal(self):
        """
        用例场景：更新资源副本索引
        前置条件：索引状态为非法的
        检查点：抛出异常
        """
        resource_id = str(uuid.uuid4())
        index_status = "index"
        with self.assertRaises(EmeiStorBizException):
            update_resource_copy_index_status(resource_id, index_status, "")

    @mock.patch("pydantic.BaseModel.from_orm")
    @mock.patch("sqlalchemy.orm.query.Query.all")
    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("app.common.license.validate_license_by_resource_type")
    def test_send_copy_save_event_should_success_when_manual_create_HDFSFileSetSubType(self, mock_query_first,
                                                                                       mock_query_all, mock_from_orm,
                                                                                       mock_license):
        """
        用例场景：发送手动创建索引命令
        前置条件：copy_id对应的sub_type为 HDFS_FileSet
        检查点：任务成功
        """
        copy_id = "9b17382f-7164-4f5b-8d77-2910a0be348c"
        copy_obj = CopyTable(uuid=copy_id, backup_type=BackupTypeEnum.cumulative_increment.value,
                             expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                             display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                             properties=json.dumps({"backup_type": "cumulative_increment"}),
                             generated_by=GenerationType.BY_BACKUP.value,
                             resource_sub_type=ResourceSubTypeEnum.HDFSFileset,
                             status=CopyStatus.NORMAL
                             )
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=False)
        mock_query_first.return_value = copy_obj
        res = send_copy_save_event(copy_id, gen_index=GenIndexType.MANUAL)
        self.assertIsNone(res)

    def test_should_raise_EmeiStorBizException_if_update_copy_index_status_when_status_illegal(self):
        """
        用例场景：更新副本索引
        前置条件：索引状态为非法的
        检查点：抛出异常
        """
        resource_id = str(uuid.uuid4())
        index_status = "index"
        with self.assertRaises(EmeiStorBizException):
            update_copy_index_status(resource_id, index_status, "")

    @mock.patch("sqlalchemy.orm.Query.first")
    def test_should_raise_EmeiStorBizException_if_update_copy_index_status_when_copy_illegal(self, mock_query_first):
        """
        用例场景：更新副本索引
        前置条件: 副本不存在
        检查点：抛出异常
        """
        resource_id = str(uuid.uuid4())
        index_status = IndexStatus.UNINDEXED.value
        mock_query_first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            update_copy_index_status(resource_id, index_status, "")

    @mock.patch("sqlalchemy.orm.Query.first")
    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_copy_index_status_and_send_alarm_event(self, mock_query_first):
        """
        用例场景：更新副本索引
        前置条件: 合法的状态
        检查点：更新成功
        """
        resource_id = str(uuid.uuid4())
        index_status = IndexStatus.INDEX_FAIL.value
        mock_query_first.return_value = COPY_DATA_ASC[0]
        update_copy_index_status(resource_id, index_status, "")

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_copy_resource_type_name_when_resource_exist(self, mock_query_resource_first):
        """
        用例场景：获取资源类型和名称
        前置条件: 资源存在
        检查点：返回正确的类型和名称
        """
        from app.resource.models.resource_models import ResourceTable
        resource_id = str(uuid.uuid4())
        mock_query_resource_first.return_value = ResourceTable(uuid=resource_id, sub_type='NasShare', name='test')
        params = {"resource_id": resource_id}
        sub_type, name = query_resource_type_name(params)
        self.assertEqual("NasShare", sub_type)
        self.assertEqual("test", name)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_resource_type_name_when_copy_exist_(self, mock_query_resource_first):
        """
        用例场景：获取资源类型和名称
        前置条件: 资源不存在，副本不存在
        检查点：返回空的类型和名称
        """
        resource_id = str(uuid.uuid4())
        mock_query_resource_first.return_value = None
        params = {"resource_id": resource_id}
        sub_type, name = query_resource_type_name(params)
        self.assertEqual("--", sub_type)
        self.assertEqual("--", name)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_check_copy_name_valid_success_when_copy_name_is_none(self, mock_query_resource_first):
        """
        用例场景：副本名为空，校验副本名通过
        前置条件：副本名为空
        检查点：不抛异常
        :return:
        """
        mock_query_resource_first.return_value = None
        copy_name = None
        self.assertIsNone(check_copy_name_valid(copy_name))

    def test_check_copy_name_valid_fail_when_copy_name_include_invalid_character(self):
        """
        用例场景：副本名有特殊字符校验不过
        前置条件：副本名有特殊字符
        检查点：校验不过抛异常
        :return:
        """
        copy_name = '<copy_name>'
        with self.assertRaises(EmeiStorBizException):
            check_copy_name_valid(copy_name)

    @mock.patch("sqlalchemy.orm.Query.first")
    def test_check_copy_exist_clone_file_system_success_when_copy_is_None(self, _mock_first):
        copy_id = str(uuid.uuid4())
        _mock_first.return_value = None
        self.assertFalse(self.copy_service.check_copy_exist_clone_file_system(copy_id))

    def test_resource_has_copy_link_when_copy_is_cloud_backup_file_system(self):
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         resource_sub_type="CloudBackupFileSystem",
                         backup_type=BackupTypeEnum.full.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                         properties=json.dumps({"backup_type": "full"}), timestamp="1638780643176000",
                         deletable=True,
                         status="Normal", generated_by="Backup", indexed="Indexing", generation=1,
                         retention_type=2, browse_mounted="Umount",
                         resource_id="f124bdf9-25b7-47bf-9dd4-5926f345b9e9", resource_name="test_resource",
                         resource_type="GaussDB", resource_location="8.40.106.11", resource_status="EXIST",
                         resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))
        res = self.copy_service.resource_has_copy_link(copy)
        self.assertTrue(res)

    def test_resource_has_copy_link_when_copy_is_snapshot(self):
        copy = CopyTable(uuid=str(uuid.uuid4()),
                         resource_sub_type="CloudBackupFileSystem",
                         backup_type=BackupTypeEnum.snapshot.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                         properties=json.dumps({"backup_type": "full"}), timestamp="1638780643176000",
                         deletable=True,
                         status="Normal", generated_by="Backup", indexed="Indexing", generation=1,
                         retention_type=2, browse_mounted="Umount",
                         resource_id="f124bdf9-25b7-47bf-9dd4-5926f345b9e9", resource_name="test_resource",
                         resource_type="GaussDB", resource_location="8.40.106.11", resource_status="EXIST",
                         resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))
        res = self.copy_service.resource_has_copy_link(copy)
        self.assertFalse(res)

    @mock.patch("app.common.database.Database.session")
    def test_update_copy_properties_by_key_success_when_properties_not_contain_key(self, mock_session):
        """s
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.full.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=4,
                         properties=json.dumps({"backup_type": "full"}),
                         resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))
        mock_session().__enter__().query().filter().first.return_value = copy
        copy_update_service.update_copy_properties_by_key("f124bdf9-25b7-47bf-9dd4-5926f345b9e9", "key1", "9999")
        updated_properties = json.loads(copy.properties)
        self.assertEqual(updated_properties.get("key1"), "9999")

    @mock.patch("app.common.database.Database.session")
    @mock.patch("app.copy_catalog.service.curd.copy_update_service.check_value_is_changed", Mock(return_value=False))
    def test_update_copy_properties_by_key_success_when_properties_not_contain_key_2(self, mock_session):
        """
        用例场景：根据副本中的key，更新副本扩展参数
        前置条件：值没有变化
        return:直接返回，不做修改
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.full.value,
                         expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                         display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=5,
                         properties=json.dumps({"backup_type": "full"}),
                         resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))
        mock_session().__enter__().query().filter().first.return_value = copy
        result = copy_update_service.update_copy_properties_by_key("f124bdf9-25b7-47bf-9dd4-5926f345b9e9",
                                                                   "key1", "9999")
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session")
    def test_update_copy_properties_by_key_should_raise_exception_when_copy_id_not_existed(self, mock_session):
        """s
        """
        mock_session().__enter__().query().filter().first.return_value = None
        with self.assertRaises(EmeiStorBizException):
            copy_update_service.update_copy_properties_by_key("f124bdf9-25b7-47bf-9dd4-5926f345b9e9", "key1", "9999")
