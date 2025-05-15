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
from unittest.mock import Mock, MagicMock

from app.copy_catalog.common.copy_status import CopyStatus
from tests.test_cases import common_mocker # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock
from app.common.exception.unified_exception import EmeiStorBizException

common_mock()

from tests.test_cases.copy_catalog.util.mock_data import COPY_DATA_ASC, NEW_SLA, OLD_SLA
from app.copy_catalog.service.curd import copy_update_service

from app.copy_catalog.service.curd.copy_update_service import update_copy_worm_status_by_id


class CopyQueryServiceTest(unittest.TestCase):

    def setUp(self) -> None:
        super(CopyQueryServiceTest, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        self.update_copy_feature_by_id = update_copy_worm_status_by_id

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_should_raise_EmeiStorBizException_when_update_copy_feature_if_copy_not_exist(self, mock_one_or_none):
        """
        用例场景：更新副本feature时，如果副本不存在，则抛出异常
        前置条件: 副本不存在
        检查点：抛出异常
        """
        from app.copy_catalog.schemas.copy_schemas import CopyWormStatusUpdate
        req = CopyWormStatusUpdate(worm_status=2)
        copy_id = str(uuid.uuid4())
        mock_one_or_none.return_value = None
        with self.assertRaises(EmeiStorBizException) as ex:
            self.update_copy_feature_by_id(copy_id, req.worm_status)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_should_raise_EmeiStorBizException_when_update_copy_feature_if_feature_not_valid(self, mock_one_or_none):
        """
        用例场景：更新副本worm_status时，如果修改的worm_status不合法，则抛出异常
        前置条件: worm_status不合法
        检查点：抛出异常
        """
        from app.copy_catalog.schemas.copy_schemas import CopyWormStatusUpdate
        from pydantic import ValidationError
        with self.assertRaises(ValidationError) as error:
            req = CopyWormStatusUpdate(worm_status=6)

    @mock.patch("sqlalchemy.orm.query.Query.update")
    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_update_copy_feature_by_id_success_when_copy_is_not_exist1(self, mock_one_or_none, mock_update):
        """
        用例场景：更新副本feature成功
        前置条件: 副本存在
        检查点：不抛异常
        """
        from app.copy_catalog.schemas.copy_schemas import CopyWormStatusUpdate
        req = CopyWormStatusUpdate(worm_status=3)
        copy_id = str(uuid.uuid4())
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=copy_id, chain_id=uuid.uuid4(), timestamp="12345678", gn=1, features=1)
        mock_one_or_none.return_value = copy
        mock_update.return_value = None
        self.update_copy_feature_by_id(copy_id, req.worm_status)

    @mock.patch("app.common.database.Database.session", MagicMock)
    @mock.patch("sqlalchemy.orm.query.Query.all")
    def test_resource_sla_name_of_copy_update_handle(self, mock_query_all):
        """
        用例场景：修改sla名称
        前置条件：无
        检查点：同步修改副本properties字段sla_name成功
        """
        mock_query_all.return_value = COPY_DATA_ASC
        copy_update_service.resource_sla_name_of_copy_update_handle(str(uuid.uuid4()), json.dumps(OLD_SLA),
                                                                    json.dumps(NEW_SLA))

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_copy_protection_sla_name_by_sla_id_success(self):
        """
        用例场景：修改sla名称，同步修改副本保护对象sla_name
        前置条件：无
        检查点：同步修改副本保护对象sla_name成功
        :return:
        """
        copy_update_service.update_copy_protection_sla_name_by_sla_id(str(uuid.uuid4()), json.dumps(OLD_SLA),
                                                                      json.dumps(NEW_SLA))

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_status_to_invalid_by_device_esn_success(self):
        """
        用例场景：根据集群esn，更新集群中所有副本状态为无效
        前置条件：无
        检查点：更新状态成功
        :return:
        """
        copy_update_service.update_status_by_device_esn(str(uuid.uuid4()), CopyStatus.INVALID)

    def test_check_value_is_changed(self):
        """
        用例场景：判断扩展参数中key对应的value是否变化
        前置条件：无
        检查点：value发生了变化
        返回:False
        """
        result = copy_update_service.check_value_is_changed("1","1",{"1":"1"})
        self.assertFalse(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_activated_copy_protection(self):
        """
        用例场景：激活副本保护
        前置条件：无
        检查点：成功激活副本保护
        返回:None
        """
        result = copy_update_service.activated_copy_protection(["123"])
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_revoke_copy_user_id(self):
        """
        用例场景：撤销副本用户ID
        前置条件：无
        检查点：成功撤销副本用户ID
        返回:None
        """
        result = copy_update_service.revoke_copy_user_id("123")
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    @mock.patch("sqlalchemy.orm.query.Query.all")
    def test_resource_sla_name_of_copy_update_handle_when_old_name_equals_new_name(self, mock_query_all):
        """
        用例场景：修改sla名称
        前置条件：无
        检查点：新的副本properties字段sla_name值与之前一样，直接返回
        """
        mock_query_all.return_value = COPY_DATA_ASC
        result = copy_update_service.resource_sla_name_of_copy_update_handle(str(uuid.uuid4()), json.dumps(OLD_SLA),
                                                                             json.dumps(OLD_SLA))
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_deactivate_copy_protection(self):
        """
        用例场景：停用副本保护
        前置条件：无
        检查点：成功停用副本保护
        返回:None
        """
        result = copy_update_service.deactivate_copy_protection(["123"])
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    @mock.patch("app.copy_catalog.service.curd.copy_update_service.refresh_resource_detect_table_with_lock", MagicMock)
    def test_handle_resource_added(self):
        """
        用例场景：添加资源
        前置条件：资源原本存在于环境上, 并且有副本存在的场景
        检查点：需要初始化资源防勒索检测表，成功添加资源
        返回:None
        """
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=True)
        result = copy_update_service.handle_resource_added("123", "321")
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_copy_status_when_status_is_Normal(self):
        """
        用例场景：更新副本状态
        前置条件：存在副本
        检查点：成功更新副本状态
        返回:None
        """
        result = copy_update_service.update_copy_status("123", CopyStatus.NORMAL)
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_resource_copy_index_status_when_index_status_is_Indexed(self):
        """
        用例场景：更新资源副本索引状态
        前置条件：存在副本，索引字符串正常
        检查点：成功更新副本状态
        返回:None
        """
        result = copy_update_service.update_resource_copy_index_status("123", "Indexed", "123")
        self.assertIsNone(result)

    @mock.patch("app.common.database.Database.session", MagicMock)
    def test_update_copy_status_by_id(self):
        """
        用例场景：根据副本ID更新副本状态
        前置条件：存在副本，可以根据提供的ID查到对应的副本
        检查点：成功更新副本状态
        返回:None
        """
        from app.copy_catalog.schemas import CopyStatusUpdate
        copyStatusUpdate = CopyStatusUpdate(status=CopyStatus.NORMAL,
                                            deletable=False,
                                            timestamp="20231010 18:18",
                                            display_timestamp=datetime.datetime(2023, 10, 10, 2, 0, 0),
                                            is_archived=True,
                                            is_replicated=True,
                                            expiration_time=datetime.datetime(2023, 11, 5, 2, 0, 0),
                                            generated_time=datetime.datetime(2023, 10, 5, 2, 0, 0))
        result = copy_update_service.update_copy_status_by_id("123", copyStatusUpdate)
        self.assertIsNone(result)
