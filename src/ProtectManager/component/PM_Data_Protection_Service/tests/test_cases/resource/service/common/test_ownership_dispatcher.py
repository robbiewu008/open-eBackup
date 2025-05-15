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
import uuid

import unittest
from unittest import mock
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases.backup.common.context import mock_context  # noqa


class OwnerShipDispatcherTest(unittest.TestCase):
    def setUp(self) -> None:
        super(OwnerShipDispatcherTest, self).setUp()
        from tests.test_cases.common.mock_settings import fake_settings
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        from app.resource.service.common.ownership_dispatcher import OwnershipDispatcher
        self.ownership_dispatcher_obj = OwnershipDispatcher(1)

    @patch("app.base.db_base.database.session")
    def test_common_resource(self, _mock_session):
        """
        *用例场景：测试查询通用资源场景
        *前置条件：无
        *检查点: 返回结果非空，无异常信息
        """
        from app.resource.models.resource_models import ResourceTable
        resource = ResourceTable(uuid=str(uuid.uuid4()))
        from app.base.db_base import database
        res = ResourceTable(**{
            'uuid': 'res',
            'name': 'res_name'
        })
        _mock_session().__enter__().query().filter().join().first.return_value = res
        _mock_session().__enter__().query().filter().count.return_value = 1
        with database.session() as session:
            self.assertRaises(EmeiStorBizException, self.ownership_dispatcher_obj.common_resource, resource, session)

        _mock_session().__enter__().query().filter().join().first.return_value = None
        _mock_session().__enter__().query().filter().count.return_value = 1
        with database.session() as session:
            self.assertRaises(EmeiStorBizException, self.ownership_dispatcher_obj.common_resource, resource, session)

        _mock_session().__enter__().query().filter().count.return_value = 0
        with database.session() as session:
            result = self.ownership_dispatcher_obj.common_resource(resource, session)
            self.assertIsInstance(result, list)
            self.assertEqual(0, len(result))

        resource = ResourceTable(uuid=str(uuid.uuid4()), children_uuids=[str(uuid.uuid4()), str(uuid.uuid4())])
        with database.session() as session:
            result = self.ownership_dispatcher_obj.common_resource(resource, session)
            self.assertIsInstance(result, list)
            self.assertEqual(2, len(result))

    @unittest.skip
    @patch("app.base.db_base.database.session")
    def test_vm_ware_resource(self, _mock_session):
        """
        *用例场景：测试查询VMware资源场景成功
        *前置条件：无
        *检查点: 返回结果非空，无异常信息
        """
        from app.base.db_base import database
        from app.resource.models.resource_models import ResourceTable
        resource = ResourceTable(uuid=str(uuid.uuid4()), name="res_name")
        _mock_session().__enter__().query().filter().filter().all.return_value = [(resource.uuid,)]
        _mock_session().__enter__().query().filter().join().first.return_value = 0
        with database.session() as session:
            result = self.ownership_dispatcher_obj.vm_ware_resource(resource, session)
            self.assertIsInstance(result, list)
            self.assertEqual(1, len(result))

    @patch("app.base.db_base.database.session")
    def test_all_sub_resource(self, _mock_session):
        """
        *用例场景：测试查询所有子资源场景成功
        *前置条件：无
        *检查点: 返回结果非空，无异常信息
        """
        from app.base.db_base import database
        from app.resource.models.resource_models import ResourceTable
        resource = ResourceTable(uuid=str(uuid.uuid4()))
        _mock_session().__enter__().execute().fetchall.return_value = [("15b1d5de-7e8e-4386-b0a5-ebc3eaa5ebf7",)]
        _mock_session().__enter__().query().filter().first.return_value = []
        with database.session() as session:
            result = self.ownership_dispatcher_obj.all_sub_resource(resource, session)
            self.assertIsInstance(result, list)
            self.assertEqual(1, len(result))

if __name__ == '__main__':
    unittest.main(verbosity=2)
