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
from unittest.mock import Mock, patch

from app.common.deploy_type import DeployType
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.backup.common.context import mock_context  # noqa
from tests.test_cases.tools import http, env, redis_mock  # noqa

patch("app.backup.common.validators.sla_validator.manager").start()
DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
from app.common.events import producer
from app.backup.api import backup_api
from app.backup.client.resource_client import ResourceClient
from app.common.schemas.job_schemas import JobSchema


@mock.patch.object(producer, "produce", Mock(return_value=None))
class TestBackupApi(unittest.TestCase):
    def setUp(self):
        super(TestBackupApi, self).setUp()

    def test_update_backup_result_should_success_when_resource_id_exist(self):
        """
        测试备份取消回调,验证资源id存在时,更新遵从度接口调用到
        :return:
        """
        ResourceClient.update_protected_object_compliance = Mock(return_value=None)
        job = JobSchema()
        job.job_id = 'jdsdsafdsf2r2waq2'
        job.source_id = '2sf4rgtr56sfds'
        job.data = '{"callback.data.backup.execute.type": "AUTOMATIC"}'
        backup_api.update_backup_result(job)
        self.assertEqual(ResourceClient.update_protected_object_compliance.call_count, 1)

    def test_update_backup_result_should_failed_when_resource_id_not_exist(self):
        """
        测试备份取消回调,验证资源id不存在时，更新遵从度接口未调用到
        :return:
        """
        ResourceClient.update_protected_object_compliance = Mock(return_value=None)
        job = JobSchema()
        job.job_id = 'jdsdsafdsf2r2waq2'
        job.data = '{"callback.data.backup.execute.type": "AUTOMATIC"}'
        backup_api.update_backup_result(job)
        self.assertEqual(ResourceClient.update_protected_object_compliance.call_count, 0)

    def test_update_backup_result_should_failed_when_job_data_not_exist(self):
        """
        测试备份取消回调,验证资源id不存在时，更新遵从度接口未调用到
        :return:
        """
        ResourceClient.update_protected_object_compliance = Mock(return_value=None)
        job = JobSchema()
        job.job_id = 'jdsdsafdsf2r2waq2'
        backup_api.update_backup_result(job)
        self.assertEqual(ResourceClient.update_protected_object_compliance.call_count, 0)

    def test_update_backup_result_should_not_execute_when_manual_backup(self):
        """
        测试备份取消回调,验证手动备份时，更新遵从度接口未调用到
        :return:
        """
        ResourceClient.update_protected_object_compliance = Mock(return_value=None)
        job = JobSchema()
        job.job_id = 'jdsdsafdsf2r2waq2'
        job.data = '{"callback.data.backup.execute.type": "MANUAL"}'
        backup_api.update_backup_result(job)
        self.assertEqual(ResourceClient.update_protected_object_compliance.call_count, 0)


if __name__ == '__main__':
    unittest.main(verbosity=2)
