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
import uuid
from unittest import mock
from unittest.mock import Mock, patch
from tests.test_cases import common_mocker # noqa
from tests.test_cases.common.mock_settings import fake_settings
_mock_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_db_init.start()


class ResourceCatalogServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        from app.resource.service.common import resource_catalog_service
        self.resource_catalog_service = resource_catalog_service

    @patch("app.base.db_base.database.session")
    def test_list_resource_catalog_success(self, _mock_session):
        """
        测试场景：查询副本目录
        前提条件: 存在parent_id为“-1”的副本目录
        检查点: 返回list类型且数目正确
        """
        from app.resource.models.resource_models import ResourceCatalogTable
        res_catalog_inst1 = ResourceCatalogTable(catalog_id=str(uuid.uuid4()), catalog_name="c1", display_order=0,
                                                 parent_id="-1", label="l1")
        res_catalog_inst2 = ResourceCatalogTable(catalog_id=str(uuid.uuid4()), catalog_name="c2", display_order=0,
                                                 parent_id="-1", label="l2")
        _mock_session().__enter__().query(ResourceCatalogTable).filter().all.return_value = [
            res_catalog_inst1, res_catalog_inst2]
        ret = self.resource_catalog_service.list_resource_catalog()
        self.assertIsInstance(ret, list)
        self.assertEqual(2, len(ret))

    @patch("app.resource.service.common.resource_catalog_service._update_hidden_field", Mock(return_value=None))
    def test_hidden_catalog_success(self):
        """
        测试场景：更新副本目录显示状态为隐藏
        前提条件: 存在副本目录ID在列表中的副本目录
        检查点: 更新成功返回None
        """
        catalog_ids = [str(uuid.uuid4())]
        self.assertIsNone(self.resource_catalog_service.hidden_catalog(catalog_ids))

    @patch("app.resource.service.common.resource_catalog_service._update_hidden_field", Mock(return_value=None))
    def test_show_catalog_success(self):
        """
        测试场景：更新副本目录显示状态为显示
        前提条件: 存在副本目录ID在列表中的副本目录
        检查点: 更新成功返回None
        """
        catalog_ids = [str(uuid.uuid4())]
        self.assertIsNone(self.resource_catalog_service.show_catalog(catalog_ids))

    @patch("app.base.db_base.database.session")
    def test_update_hidden_field_success(self, _mock_session):
        """
        测试场景：根据副本目录ID列表更新副本目录的显示状态
        前提条件: 存在副本目录ID在列表中的副本目录
        检查点: 更新成功返回None
        """
        catalog_ids = [str(uuid.uuid4()), str(uuid.uuid4())]
        from app.resource.models.resource_models import ResourceCatalogTable
        _mock_session().__enter__().query(ResourceCatalogTable).filter().update.return_value = None
        ret = self.resource_catalog_service._update_hidden_field(catalog_ids, True)
        self.assertIsNone(ret)