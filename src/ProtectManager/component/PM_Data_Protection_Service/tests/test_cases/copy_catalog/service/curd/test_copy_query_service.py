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
from unittest.mock import Mock

from app.common.enums.copy_enum import GenerationType
from tests.test_cases import common_mocker # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()

from app.copy_catalog.service.curd.copy_query_service import get_deleting_copy, query_all_copy_ids_by_resource_id, \
    get_log_data_cyber, get_protect_resource_by_id


class CopyQueryServiceTest(unittest.TestCase):

    def setUp(self) -> None:
        self.get_deleting_copy = get_deleting_copy
        self.query_copy_ids_by_resource_id = query_all_copy_ids_by_resource_id

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_get_deleting_copy_success_when_copy_is_not_exist(self, mock_one_or_none):
        """
        用例场景：获取待删除副本
        前置条件: 副本不存在
        检查点：返回None
        """
        copy_id = str(uuid.uuid4())
        mock_one_or_none.return_value = None
        self.assertIsNone(self.get_deleting_copy(copy_id, False))

    @mock.patch("sqlalchemy.orm.query.Query.all")
    def test_get_copy_ids_by_resource_id(self, mock_all):
        """
        用例场景：根据资源id获取副本id成功
        前置条件: 无
        检查点：返回不为空
        """
        resource_id = str(uuid.uuid4())
        mock_all.return_value = ["123456"]
        self.assertIsNotNone(self.query_copy_ids_by_resource_id(resource_id, "Backup"))

    @mock.patch("app.copy_catalog.service.curd.copy_query_service.load_resource")
    def test_get_log_data_cyber(self, mock_load_resource):
        """
        验证场景：组装安全一体机告警所需资源设备信息
        前置条件：无
        验证点：返回信息成功
        """
        resource = {}
        resource.update({"path": "CyberEngine OceanProtect/op28/System_vStore/testj"})
        resource.update({"root_uuid": "2102353GTH10L8000008"})
        resource.update({"parent_name": "System_env"})
        resource.update({"parent_uuid": "0"})
        resource.update({"parent_uuid": "0"})
        resource.update({"uuid": "34b147bc-038a-399d-b3b8-72650de2cb3c"})
        resource.update({"name": "test_resource"})
        mock_load_resource.return_value = [resource]
        self.assertEqual("op28", get_log_data_cyber("test")[0])

    @mock.patch("app.copy_catalog.service.curd.copy_query_service.get_log_data_cyber")
    @mock.patch("app.copy_catalog.service.curd.copy_query_service.query_copy_by_id")
    def test_get_protect_resource_by_id(self, mock_copy, mock_log):
        """
        验证场景：组装安全一体机告警所需资源设备信息,增加标记位cyber-array-true
        前置条件：无
        验证点：返回信息成功,数组末尾cyber-array-true
        """
        mock_log.return_value = []
        self.assertEqual("cyber-array-true", get_protect_resource_by_id("test").pop())

    @mock.patch("app.common.database.Database.session", mock.MagicMock)
    def test_query_last_copy_by_resource_id_when_generated_by_is_None(self):
        """
        验证场景：根据资源id查询最后一个副本
        前置条件：副本generated_by字段为None
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_last_copy_by_resource_id
        result = query_last_copy_by_resource_id("123", None)
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock)
    def test_query_last_copy_by_resource_id_when_generated_by_is_not_None(self):
        """
        验证场景：根据资源id查询最后一个副本
        前置条件：副本generated_by字段值不为空
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_last_copy_by_resource_id
        result = query_last_copy_by_resource_id("123", "Backup")
        self.assertTrue(result)

    def test_get_same_chain_copies(self):
        """
        验证场景：获取副本依赖链上所有副本id
        前置条件：副本为全量副本
        验证点：返回当前副本id
        """
        from app.copy_catalog.service.curd.copy_query_service import get_same_chain_copies
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid='57d97940-5246-4650-ab1e-3ecc06a94b37', backup_type=1)
        result = get_same_chain_copies(copy)
        self.assertEqual(['57d97940-5246-4650-ab1e-3ecc06a94b37'], result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_get_same_chain_copies_2(self):
        """
        验证场景：获取副本依赖链上所有副本id
        前置条件：副本为增量副本
        验证点：返回副本依赖链上所有副本id
        """
        from app.copy_catalog.service.curd.copy_query_service import get_same_chain_copies
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid='2cc7c3dd-3c86-4e29-833c-b8604b5b55bf', chain_id=str(uuid.uuid4()),
                         resource_id=str(uuid.uuid4()), generated_by="Backup", origin_copy_time_stamp=1, backup_type=2)
        result = get_same_chain_copies(copy)
        self.assertEqual(['2cc7c3dd-3c86-4e29-833c-b8604b5b55bf'], result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_count_copy_by_parent_id(self):
        """
        验证场景：根据父副本id查询副本数量
        前置条件：无
        验证点：返回副本数量
        """
        from app.copy_catalog.service.curd.copy_query_service import count_copy_by_parent_id
        result = count_copy_by_parent_id("123")
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_query_first_last_copy_id(self):
        """
        验证场景：查询月的最新或第一个副本
        前置条件：无
        验证点：返回副本id
        """
        from app.copy_catalog.service.curd.copy_query_service import query_first_last_copy_id
        result = query_first_last_copy_id("123", "Backup", 3, 4, None)
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_count_copies_by_condition(self):
        """
        验证场景：根据条件查询副本数量
        前置条件：无
        验证点：返回副本数量
        """
        from app.copy_catalog.service.curd.copy_query_service import count_copies_by_condition
        result = count_copies_by_condition("123", "Backup")
        self.assertTrue(result)

    def test_raise_duplicate_copy_or_snapshot_name_exception(self):
        """
        验证场景：快照名称重复时抛出异常
        前置条件：设备是安全一体机
        验证点：抛出EmeiStorBizException异常
        """
        from app.copy_catalog.service.curd.copy_query_service import raise_duplicate_copy_or_snapshot_name_exception
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = mock.MagicMock(return_value=True)
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        with self.assertRaises(EmeiStorBizException) as ex:
            raise_duplicate_copy_or_snapshot_name_exception("1")
        self.assertEqual(ex.exception.error_code, CopyErrorCode.FORBID_DUPLICATE_SNAPSHOT_NAME.get("code"))

    def test_raise_duplicate_copy_or_snapshot_name_exception_2(self):
        """
        验证场景：副本名称重复时抛出异常
        前置条件：设备不是安全一体机
        验证点：抛出EmeiStorBizException异常
        """
        from app.copy_catalog.service.curd.copy_query_service import raise_duplicate_copy_or_snapshot_name_exception
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = mock.MagicMock(return_value=False)
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        with self.assertRaises(EmeiStorBizException) as ex:
            raise_duplicate_copy_or_snapshot_name_exception("1")
        self.assertEqual(ex.exception.error_code, CopyErrorCode.FORBID_DUPLICATE_COPY_NAME.get("code"))

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_check_copy_name_valid(self):
        """
        验证场景：根据副本名查副本，检查副本名称是否合规
        前置条件：已存在同名副本，设备不是安全一体机
        验证点：抛出EmeiStorBizException异常
        """
        from app.copy_catalog.service.curd.copy_query_service import check_copy_name_valid
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = mock.MagicMock(return_value=False)
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.copy_catalog.copy_error_code import CopyErrorCode
        with self.assertRaises(EmeiStorBizException) as ex:
            check_copy_name_valid("copy")
        self.assertEqual(ex.exception.error_code, CopyErrorCode.FORBID_DUPLICATE_COPY_NAME.get("code"))

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_copy_info_by_uuid_and_esn(self, mock_first):
        """
        验证场景：根据副本uuid和esn查询副本
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_info_by_uuid_and_esn
        from app.copy_catalog.schemas import CopySchema
        from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC
        mock_first.return_value = COPY_DATA_ASC[0]
        result = query_copy_info_by_uuid_and_esn("123", "123")
        self.assertEqual(CopySchema(**COPY_DATA_ASC[0].as_dict()), result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    @mock.patch("app.common.deploy_type.DeployType.is_dependent", Mock(return_value=False))
    def test_query_copy_info_by_backup_id(self, mock_first):
        """
        验证场景：根据副本backup_id和esn查询副本
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_info_by_backup_id
        from app.copy_catalog.schemas import CopySchema
        from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC
        mock_first.return_value = COPY_DATA_ASC[0]
        result = query_copy_info_by_backup_id("123", "123", [GenerationType.BY_BACKUP,
                                                             GenerationType.BY_COMMON_INTERFACE_BACKUP])
        self.assertEqual(CopySchema(**COPY_DATA_ASC[0].as_dict()), result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_copy_by_resource_properties(self, mock_first):
        """
        验证场景：根据副本resource_properties查询副本
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_resource_properties
        from app.copy_catalog.schemas import CopySchema
        from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC
        mock_first.return_value = COPY_DATA_ASC[0]
        result = query_copy_by_resource_properties("123", "Backup", "Mysql")
        self.assertEqual(CopySchema(**COPY_DATA_ASC[0].as_dict()), result)

    @mock.patch("sqlalchemy.orm.query.Query.scalar")
    def test_query_archive_copy_count_by_storage_id(self, mock_scalar):
        """
        验证场景：根据storage_id查询副本数量
        前置条件：无
        验证点：返回副本数量
        """
        from app.copy_catalog.service.curd.copy_query_service import query_archive_copy_count_by_storage_id
        mock_scalar.return_value = 2
        result = query_archive_copy_count_by_storage_id("123")
        self.assertEqual(2, result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_query_replicated_copies(self):
        """
        验证场景：查询复制副本
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_replicated_copies
        result = query_replicated_copies("123", "123")
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_query_delete_copy_list(self):

        """
        验证场景：根据副本链id查询删除副本列表
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_delete_copy_list
        result = query_delete_copy_list("123")
        self.assertTrue(result)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_query_copy_backup_id_1(self, mock_one_or_none):
        """
        验证场景：根据copy_id查询副本backup_id
        前置条件：copy_id没有对应的copy
        验证点：返回""
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_backup_id
        mock_one_or_none.return_value = None
        result = query_copy_backup_id("123")
        self.assertEqual("", result)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_query_copy_backup_id_2(self, mock_one_or_none):
        """
        验证场景：根据copy_id查询副本backup_id
        前置条件：无
        验证点：返回副本backup_id
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_backup_id
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), properties=json.dumps({"backup_id": "1234"}))
        mock_one_or_none.return_value = copy
        result = query_copy_backup_id("123")
        self.assertEqual("1234", result)

    @mock.patch("sqlalchemy.orm.query.Query.count")
    def test_query_copy_count_by_resource_id(self, mock_count):
        """
        验证场景：根据资源id查询副本数量
        前置条件：无
        验证点：返回副本数量
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_count_by_resource_id
        mock_count.return_value = 3
        result = query_copy_count_by_resource_id("123")
        self.assertEqual(3, result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_replicated_copy_by_resource_id(self, mock_first):
        """
        验证场景：根据资源id查询副本数量
        前置条件：无
        验证点：返回副本数量
        """
        from app.copy_catalog.service.curd.copy_query_service import query_replicated_copy_by_resource_id
        mock_first.return_value = None
        result = query_replicated_copy_by_resource_id("123")
        self.assertEqual(None, result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_copy_by_resource_id(self, mock_first):
        """
        验证场景：根据资源id查询副本信息
        前置条件：无
        验证点：返回副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_resource_id
        from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC
        mock_first.return_value = COPY_DATA_ASC[0]
        result = query_copy_by_resource_id("123", "Backup")
        self.assertTrue(result)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_query_copy_by_id(self, mock_one_or_none):
        """
        验证场景：根据copy_id查询副本
        前置条件：无
        返回：副本信息
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_id
        mock_one_or_none.return_value = None
        result = query_copy_by_id("123")
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.session.Session")
    def test_check_copy_whether_exist_1(self, _mock_session):
        """
        验证场景：检查副本是否存在
        前置条件：无
        返回：抛出ValidationError异常
        """
        from app.copy_catalog.service.curd.copy_query_service import check_copy_whether_exist
        from app.copy_catalog.schemas import CopyInfoSchema
        copy_info = CopyInfoSchema(uuid=str(uuid.uuid4()),
                                   generated_by="Backup",
                                   resource_sub_type="GaussDB",
                                   resource_id=str(uuid.uuid4()),
                                   timestamp="1638780643176000",
                                   display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
                                   deletable=True,
                                   status="Normal",
                                   indexed="Indexing",
                                   generation=1,
                                   retention_type=1,
                                   resource_name="test_resource",
                                   resource_type="GaussDB",
                                   resource_location="8.40.106.11",
                                   resource_status="EXIST",
                                   resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}),
                                   browse_mounted="Umount")
        _mock_session.query().filter().one_or_none.return_value = copy_info
        self.assertEqual(copy_info, check_copy_whether_exist(copy_info, _mock_session))

    @mock.patch("sqlalchemy.orm.session.Session")
    def test_check_copy_whether_exist_2(self, _mock_session):
        """
        验证场景：检查副本是否存在
        前置条件：无
        返回：抛出ValidationError异常
        """
        from app.copy_catalog.service.curd.copy_query_service import check_copy_whether_exist
        from app.copy_catalog.schemas import CopyInfoSchema
        copy_info = CopyInfoSchema(uuid=None,
                                   generated_by="CloudArchive",
                                   resource_sub_type="GaussDB",
                                   resource_id=str(uuid.uuid4()),
                                   timestamp="1638780643176000",
                                   display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
                                   deletable=True,
                                   status="Normal",
                                   indexed="Indexing",
                                   generation=1,
                                   retention_type=1,
                                   resource_name="test_resource",
                                   resource_type="GaussDB",
                                   resource_location="8.40.106.11",
                                   resource_status="EXIST",
                                   resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}),
                                   browse_mounted="Umount")
        _mock_session.query().filter().first.return_value = copy_info
        self.assertEqual(copy_info, check_copy_whether_exist(copy_info, _mock_session))

    @mock.patch("sqlalchemy.orm.session.Session")
    def test_check_copy_whether_exist_3(self, _mock_session):
        """
        验证场景：检查副本是否存在
        前置条件：无
        返回：抛出ValidationError异常
        """
        from app.copy_catalog.service.curd.copy_query_service import check_copy_whether_exist
        from app.copy_catalog.schemas import CopyInfoSchema
        copy_info = CopyInfoSchema(uuid=None,
                                   generated_by="replication",
                                   resource_sub_type="Fileset",
                                   resource_id=str(uuid.uuid4()),
                                   timestamp="1638780643176000",
                                   display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0),
                                   deletable=True,
                                   status="Normal",
                                   indexed="Indexing",
                                   generation=1,
                                   retention_type=1,
                                   resource_name="test_resource",
                                   resource_type="GaussDB",
                                   resource_location="8.40.106.11",
                                   resource_status="EXIST",
                                   resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}),
                                   browse_mounted="Umount")
        _mock_session.query().filter().first.return_value = copy_info
        self.assertEqual(copy_info, check_copy_whether_exist(copy_info, _mock_session))

    def test_verify_copy_ownership_1(self):
        """
        验证场景：验证副本所有权
        前置条件：copy_uuid_list为None
        返回：直接返回
        """
        from app.copy_catalog.service.curd.copy_query_service import verify_copy_ownership
        result = verify_copy_ownership("123", None)
        self.assertIsNone(result)

    def test_verify_copy_ownership_2(self):
        """
        验证场景：验证副本所有权
        前置条件：user_id为None
        返回：抛出EmeiStorBizException异常
        """
        from app.copy_catalog.service.curd.copy_query_service import verify_copy_ownership
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.common.exception.user_error_codes import UserErrorCodes
        with self.assertRaises(EmeiStorBizException) as ex:
            verify_copy_ownership(None, ["123"])
        self.assertEqual(ex.exception.error_code, UserErrorCodes.ACCESS_DENIED.get("code"))

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_verify_copy_ownership_3(self):
        """
        验证场景：验证副本所有权
        前置条件：无
        返回：抛出EmeiStorBizException异常
        """
        from app.copy_catalog.service.curd.copy_query_service import verify_copy_ownership
        from app.common.exception.unified_exception import EmeiStorBizException
        from app.common.exception.user_error_codes import UserErrorCodes
        with self.assertRaises(EmeiStorBizException) as ex:
            verify_copy_ownership("123", ["123"])
        self.assertEqual(ex.exception.error_code, UserErrorCodes.ACCESS_DENIED.get("code"))

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    @mock.patch("app.copy_catalog.service.curd.copy_query_service.handle_copy_info_by_condition",
                mock.Mock(return_value=True))
    def test_query_copy_by_condition_1(self):
        """
        验证场景：根据条件查询副本
        前置条件：sorted_key存在
        返回：True
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_condition
        conditions = {"condition_key": ["value_condition"]}
        result = query_copy_by_condition("123", True, conditions)
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    @mock.patch("app.copy_catalog.service.curd.copy_query_service.handle_copy_info_by_condition",
                mock.Mock(return_value=True))
    def test_query_copy_by_condition_2(self):
        """
        验证场景：根据条件查询副本
        前置条件：sorted_key存在
        返回：True
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_condition
        conditions = {"%condition_key": "value_condition"}
        result = query_copy_by_condition("123", False, conditions)
        self.assertTrue(result)

    @mock.patch("app.common.database.Database.session", mock.MagicMock())
    def test_handle_copy_info_by_condition_1(self):
        """
        验证场景：根据条件查询副本
        前置条件：condition值为{}
        返回：直接返回
        """
        from app.copy_catalog.service.curd.copy_query_service import query_copy_by_condition
        result = query_copy_by_condition("123", False, {})
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_get_full_copy_by_increment_copy(self, mock_first):
        """
        验证场景：根据增量副本查询全量副本
        前置条件：condition值不正确
        返回：抛出ValidationError异常
        """
        from app.copy_catalog.service.curd.copy_query_service import get_full_copy_by_increment_copy
        mock_first.return_value = True
        from pydantic import ValidationError
        with self.assertRaises(ValidationError):
            get_full_copy_by_increment_copy({"gn":1}, mock_first)


