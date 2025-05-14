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
from unittest.mock import MagicMock, Mock, patch
from tests.test_cases import common_mocker # noqa
# mock处理“from app.resource.service.common import resource_service”中“from app.common.events import producer”
from tests.test_cases.common.events import mock_producer
from tests.test_cases.common.mock_settings import fake_settings
_mock_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_db_init.start()
from app.common.enums.db_desesitization_enum import DesesitizationStatusEnum, IdentificationStatusEnum
from app.common.enums import db_desesitization_enum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.resource.schemas.db_desesitization_schema import DbDesestitationTaskSchema
from tests.test_cases.backup.common.context import mock_context  # noqa


class DbDesesitizationServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        super(DbDesesitizationServiceTest, self).setUp()
        from app.resource.service.common import db_desesitization_service
        self.db_desesitization_service = db_desesitization_service

    @patch("app.resource.service.common.db_desesitization_service.process_update_status", Mock(return_value=None))
    @patch("app.resource.service.common.db_desesitization_service.process_start_status", Mock(return_value=None))
    @patch("app.resource.db.db_desestitation_db_api.update_desesitization_info", Mock)
    @patch("app.resource.db.db_desestitation_db_api.create_desesitization_info", Mock)
    @patch("app.resource.db.db_desestitation_db_api.query_desesitization_info")
    @patch("app.resource.service.common.resource_service.query_resource_by_id")
    @patch("app.base.db_base.database.session", MagicMock)
    def test_desestitation_db(self, _mock_query_resource_by_id, _mock_query_desesitization_info):
        """
        测试场景：执行脱敏数据库
        前提条件: 一：资源信息不存在；二：资源信息存在，脱敏信息不存在；
        检查点: 一：抛出异常；二：执行成功返回None
        """
        params = DbDesestitationTaskSchema(resource_id='00001')
        # 1 资源信息不存在
        _mock_query_resource_by_id.return_value = None
        self.assertRaises(EmeiStorBizException, self.db_desesitization_service.desestitation_db, params)

        # 2 资源信息存在，脱敏信息不存在
        from app.resource.models.resource_models import ResourceTable
        _mock_query_resource_by_id.return_value = ResourceTable()
        _mock_query_desesitization_info.return_value = None
        self.db_desesitization_service.process_update_status = Mock(result_value=None)
        result = self.db_desesitization_service.desestitation_db(params)
        self.assertIsNone(result)

        params.task_status = db_desesitization_enum.TaskStatusEnum.Start
        result = self.db_desesitization_service.desestitation_db(params)
        self.assertIsNone(result)

        params.task_type = db_desesitization_enum.TaskTypeEnum.ConfirmPolicy
        result = self.db_desesitization_service.desestitation_db(params)
        self.assertIsNone(result)

    @unittest.skip
    @patch("app.resource.db.db_desestitation_db_api.update_desesitization_info", Mock)
    @patch("app.common.toolkit.modify_task_log", Mock)
    @patch("app.resource.rpc.job_rpc.JobClient.create_job")
    @patch("app.resource.service.common.resource_service.query_resource_by_id")
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_process_start_status(self, _mock_query_resource_by_id, _mock_create_job):
        """
        测试场景：处理开始脱敏任务状态
        前提条件: 一：环境资源不存在；
                 二：环境资源存在，task_type为"identification"；
                 三：环境资源存在，task_type为None；
        检查点: 一：抛出异常；二：执行成功返回None；三：执行成功返回None
        """
        from app.resource.models.resource_models import ResourceDesesitizationTable
        from app.resource.models.resource_models import ResourceTable
        desesitization_info = ResourceDesesitizationTable()
        params = DbDesestitationTaskSchema(
            resource_id='00001', task_status=db_desesitization_enum.TaskStatusEnum.Start)
        resource_info = ResourceTable()
        _mock_query_resource_by_id.return_value = None
        with self.assertRaises(EmeiStorBizException):
            self.db_desesitization_service.process_start_status(desesitization_info, params, resource_info)

        _mock_query_resource_by_id.return_value = ResourceTable()
        params.task_type = db_desesitization_enum.TaskTypeEnum.Identification
        _mock_create_job.return_value = '00001'
        result = self.db_desesitization_service.process_start_status(desesitization_info, params, resource_info)
        self.assertIsNone(result)

        params.task_type = None
        result = self.db_desesitization_service.process_start_status(desesitization_info, params, resource_info)
        self.assertIsNone(result)

    @patch("app.resource.db.db_desestitation_db_api.update_desesitization_info", Mock)
    @patch("app.common.toolkit.modify_task_log", Mock)
    def test_process_update_status(self):
        """
        测试场景：处理更新脱敏任务状态
        前提条件: 一：task_type为"identification"；
                 二：环境资源存在，task_type为None；
        检查点: 一：执行成功返回None；二：执行成功返回None
        """
        from app.resource.models.resource_models import ResourceDesesitizationTable
        desesitization_info = ResourceDesesitizationTable()
        params = DbDesestitationTaskSchema(
            resource_id='00001', task_status=db_desesitization_enum.TaskStatusEnum.Start)
        params.task_type = db_desesitization_enum.TaskTypeEnum.Identification
        result = self.db_desesitization_service.process_update_status(desesitization_info, params)
        self.assertIsNone(result)

        params.task_type = None
        result = self.db_desesitization_service.process_update_status(desesitization_info, params)
        self.assertIsNone(result)

    @patch("app.resource.db.db_desestitation_db_api.query_desesitization_info")
    def test_check_can_delete(self, _mock_query_desesitization_info):
        """
        测试场景：检查脱敏资源是否可以删除
        前提条件: 一：脱敏信息不存在；
                 二：脱敏信息存在，状态为“DESESITIZING”或者“IDENTIFING”；
                 三：脱敏信息存在，状态不为“DESESITIZING”和“IDENTIFING”；
        检查点: 一：返回True；二：返回False；二：返回True
        """
        resource_id = '00001'
        _mock_query_desesitization_info.return_value = None
        result = self.db_desesitization_service.check_can_delete(resource_id)
        self.assertTrue(result)

        from app.resource.models.resource_models import ResourceDesesitizationTable
        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable(
            desesitization_status=DesesitizationStatusEnum.Desesitizing)
        result = self.db_desesitization_service.check_can_delete(resource_id)
        self.assertFalse(result)

        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable(
            identification_status=IdentificationStatusEnum.Identifing)
        result = self.db_desesitization_service.check_can_delete(resource_id)
        self.assertFalse(result)

        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable()
        result = self.db_desesitization_service.check_can_delete(resource_id)
        self.assertTrue(result)

    @patch("app.resource.db.db_desestitation_db_api.update_desesitization_info", Mock)
    @patch("app.resource.db.db_desestitation_db_api.create_desesitization_info", Mock)
    @patch("app.resource.db.db_desestitation_db_api.query_desesitization_info")
    @patch("app.resource.service.common.resource_service.query_resource_by_id")
    @patch("app.base.db_base.database.session", MagicMock)
    def test_add_desestitation_info(self, _mock_query_resource_by_id, _mock_query_desesitization_info):
        """
        测试场景：保存脱敏信息
        前提条件: 一：资源信息不存在；二：资源存在且类型是“MySQL”或者“Oracle”或者“SQLServer”；
        检查点: 一：抛出异常；二：执行成功返回None
        """
        resource_id = '00001'
        from app.resource.models.resource_models import ResourceTable
        _mock_query_resource_by_id.return_value = ResourceTable(sub_type=ResourceSubTypeEnum.MySQL)
        _mock_query_desesitization_info.return_value = None
        result = self.db_desesitization_service.add_desestitation_info(resource_id)
        self.assertIsNone(result)

        _mock_query_resource_by_id.return_value = ResourceTable(sub_type=ResourceSubTypeEnum.Oracle)
        from app.resource.models.resource_models import ResourceDesesitizationTable
        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable()
        result = self.db_desesitization_service.add_desestitation_info(resource_id)
        self.assertIsNone(result)

        _mock_query_resource_by_id.return_value = ResourceTable(sub_type=ResourceSubTypeEnum.SQLServer)
        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable()
        result = self.db_desesitization_service.add_desestitation_info(resource_id)
        self.assertIsNone(result)

    @patch("app.resource.db.db_desestitation_db_api.update_desesitization_info", Mock)
    @patch("app.resource.db.db_desestitation_db_api.query_desesitization_info")
    @patch("app.base.db_base.database.session", MagicMock)
    def test_clean_desestitation_info(self, _mock_query_desesitization_info):
        """
        测试场景：清理脱敏信息
        前提条件: 一：脱敏信息不存在；二：脱敏信息存在
        检查点: 一：返回None；二：执行成功返回None
        """
        resource_id = '00001'
        _mock_query_desesitization_info.return_value = None
        result = self.db_desesitization_service.clean_desestitation_info(resource_id)
        self.assertIsNone(result)

        from app.resource.models.resource_models import ResourceDesesitizationTable
        _mock_query_desesitization_info.return_value = ResourceDesesitizationTable()
        result = self.db_desesitization_service.clean_desestitation_info(resource_id)
        self.assertIsNone(result)


if __name__ == '__main__':
    unittest.main()
