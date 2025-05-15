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
from unittest.mock import patch
from unittest.mock import MagicMock
from unittest.mock import Mock
from tests.test_cases import common_mocker # noqa
from tests.test_cases.common.events import mock_producer
from tests.test_cases.common.mock_settings import fake_settings
_mock_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_db_init.start()
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.resource.schemas.database_schema import DBRestoreCreateResourceSchema
from tests.test_cases.backup.common.context import mock_context  # noqa


class DbRestoreCreateResServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        super(DbRestoreCreateResServiceTest, self).setUp()
        from app.resource.service.common import db_restore_create_res_service
        self.db_restore_create_res_service = db_restore_create_res_service

    @patch("app.resource.service.common.db_desesitization_service.add_desestitation_info", Mock)
    @patch("app.resource.db.db_res_db_api.query_host_info_by_uuid")
    @patch("app.base.db_base.database.session", MagicMock)
    def test_create_res(self, _mock_query_host_info_by_uuid):
        """
        测试场景：增加资源脱敏信息
        前提条件: 资源主机信息不存在
        检查点: 抛出异常
        """
        params = DBRestoreCreateResourceSchema(
            host_id='00001',
            sub_type=ResourceSubTypeEnum.SQLServer,
            instance_name='sqlserver01',
            database_name='sqlserver01'
        )

        _mock_query_host_info_by_uuid.return_value = None
        self.assertRaises(EmeiStorBizException, self.db_restore_create_res_service.create_res, params)


if __name__ == '__main__':
    unittest.main()
