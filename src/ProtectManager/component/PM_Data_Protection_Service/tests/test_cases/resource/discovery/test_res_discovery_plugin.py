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
import json
import sys
import unittest
from unittest import mock
from unittest.mock import MagicMock, patch

from app.common.enums.resource_enum import DeployTypeEnum
from tests.test_cases import common_mocker  # noqa


class DiscoveryManagerTest(unittest.TestCase):
    def setUp(self) -> None:
        self._mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
        self._mock_common_db_init.start()
        from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock
        self._mock_kafka_producer = mock.patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
        self._mock_kafka_producer.start()
        mock.patch("app.common.deploy_type.DeployType.get_deploy_type",
                   MagicMock(return_value=DeployTypeEnum.X8000)).start()
        sys.modules['pyVmomi.Iso8601'] = MagicMock()
        sys.modules['app.resource.service.vmware.service_instance_manager'] = MagicMock()

    def tearDown(self) -> None:
        del sys.modules['pyVmomi.Iso8601']

    @patch("app.resource.service.common.resource_service.query_environment", MagicMock(return_value=None))
    @patch("app.common.toolkit.modify_task_log", MagicMock())
    @patch("app.common.toolkit.create_job_center_task", MagicMock(return_value={'job_id': '1'}))
    @patch("app.common.toolkit.complete_job_center_task", MagicMock())
    def test_register_env_success(self):
        """
        *用例场景：测试注册环境成功
        *前置条件：环境信息参数正确
        *检查点: 无异常信息返回
        """
        from app.common.enums.resource_enum import ResourceSubTypeEnum
        from app.resource.schemas.env_schemas import ScanEnvSchema
        from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
        from app.resource.discovery.res_discovery_plugin import DiscoveryManager
        from app.common.clients.scheduler_client import SchedulerClient

        params = ScanEnvSchema(
            verify_cert=1,
            sub_type=ResourceSubTypeEnum.vCenter.value,
            uuid="12345",
            endpoint="1.1.1.1",
            port=443,
            extend_context={
                'cert_name': 'cert_name1',
                'crl_name': 'crl_name1',
                'certification': 'certification1',
                'revocation_list': 'revocation_list1',
            },
            job_id="11111111"
        )
        with mock.patch.object(VMwareDiscoveryPlugin, "do_scan_env", MagicMock()), \
                mock.patch.object(SchedulerClient, "submit_interval_job", MagicMock()), \
                mock.patch.object(DiscoveryManager, "heartbeat", MagicMock()):
            DiscoveryManager(ResourceSubTypeEnum.vCenter.value).register_env(params)
            self.assertIsNotNone(params)

    @patch("app.resource.service.common.resource_service.query_environment", MagicMock(return_value=[{}]))
    @patch("app.common.toolkit.query_job_list", MagicMock(return_value=json.dumps({"totalCount": 0})))
    @patch("app.common.toolkit.modify_task_log", MagicMock())
    @patch("app.common.toolkit.complete_job_center_task", MagicMock())
    def test_manual_scan_env_success(self):
        """
        *用例场景：测试手动扫描环境成功
        *前置条件：环境信息参数正确
        *检查点: 无异常信息返回
        """
        from app.common.enums.resource_enum import ResourceSubTypeEnum
        from app.resource.schemas.env_schemas import ScanEnvSchema
        from app.resource.discovery.plugins.vmware_discovery_plugin import VMwareDiscoveryPlugin
        from app.resource.discovery.res_discovery_plugin import DiscoveryManager

        params = ScanEnvSchema(
            verify_cert=1,
            sub_type=ResourceSubTypeEnum.vCenter.value,
            uuid="12345",
            endpoint="1.1.1.1",
            port=443,
            job_id='job_1',
            extend_context={
                'cert_name': 'cert_name1',
                'crl_name': 'crl_name1',
                'certification': 'certification1',
                'revocation_list': 'revocation_list1',
            }
        )
        with mock.patch.object(VMwareDiscoveryPlugin, "do_scan_env", MagicMock()), \
                mock.patch.object(DiscoveryManager, "heartbeat", MagicMock()):
            self.assertIsNone(DiscoveryManager(ResourceSubTypeEnum.vCenter.value).manual_scan_env('123', '123'))


if __name__ == '__main__':
    unittest.main(verbosity=2)
