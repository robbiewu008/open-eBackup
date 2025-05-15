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
import sys
import unittest
from unittest import mock
from unittest.mock import patch, MagicMock, Mock

from app.common.clients.client_util import ProtectionServiceHttpsClient
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.security.kmc_util import Kmc
from app.backup.client.job_client import JobClient
from tests.test_cases.common.mock_settings import fake_settings  # noqa


class KafkaProducerMock:
    def produce(self, *args, **kwargs):
        pass

    def flush(self, *args, **kwargs):
        pass


class DataBaseMock:
    def add(self, *args, **kwargs):
        pass


def get_mock_admin_client_mock():
    from confluent_kafka.admin import AdminClient
    return AdminClient({
        "bootstrap.servers": "infrastructure:9092",
        "security.protocol": "SASL_SSL",
        "sasl.mechanism": "PLAIN",
        "sasl.username": "kafka_usr",
        "sasl.password": "123456789",
        "ssl.ca.location": "mock",
    })


producer_patcher = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
producer_patcher.start()
patch("app.common.rpc.system_base_rpc.encrypt", MagicMock(return_value='')).start()

sys.modules['app.resource_lock.kafka.messaging_utils'] = Mock()


def get_mock_copy_info():
    from app.copy_catalog.models.tables_and_sessions import CopyTable
    return CopyTable(uuid='21d07c5d-f647-4d39-9d8d-08c7c86678f4',
                     timestamp='1658838898000000',
                     display_timestamp='2022-07-26T20:34:58', deletable=True,
                     status='Normal',
                     generated_by='Backup',
                     backup_type="full",
                     properties="{\"format\": 0}",
                     resource_id='501ff036-d852-ab32-bbac-df5d15609e5c',
                     resource_properties='{}', indexed='Unindexed', generation=1,
                     retention_type=1,
                     resource_name='syb_centos_8.40.97.233', resource_type='VM',
                     resource_sub_type='vim.VirtualMachine',
                     resource_location='8.40.97.171', resource_status='EXIST')


class TestReplicationService(unittest.TestCase):
    def setUp(self):
        super(TestReplicationService, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        sys.modules['app.archive.service.service'] = Mock()
        from app.replication.service import replication_service
        self.replication_service = replication_service
        self.replication_service.query_target_cluster_by_id = Mock(return_value=TargetClusterMock())

    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(ProtectionServiceHttpsClient, "request", autospec=True)
    def test_should_raise_EmeiStorBizException_when_reverse_replicate_if_copy_is_null(self, mock_request):
        """
        测试场景：执行反向复制时，如果副本为空，则抛出异常
        前提条件: 副本信息不存在
        检查点: 抛出异常
        """
        from app.replication.schemas.replication_request import ReplicationRequest
        from app.replication.service.replication_service import reverse_replicate
        self.replication_service.query_copy_by_id = Mock(return_value=None)
        user_id = "123456"
        req = ReplicationRequest(copy_id="123456", external_system_id="1", retention_type=1, storage_type="1",
                                 storage_id="1", user_id="1", username="1", password="1")
        with self.assertRaises(EmeiStorBizException) as ex:
            reverse_replicate(user_id, req)
        message = "copy is null."
        self.assertEqual(CommonErrorCodes.SYSTEM_ERROR.get("code"), str(ex.exception.error_code))
        self.assertEqual(message, ex.exception._error_message)

    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    def test_return_empty_list_when_get_same_chain_copies_if_copy_is_not_directory_format(self):
        """
        测试场景：执行反向复制时，如果副本格式不为非原生格式，则返回空列表
        前提条件: 存在副本信息
        检查点: 非原生格式副本才需要获取副本链
        """
        from app.replication.service.replication_service import get_directory_copy_same_chain_copies
        copy_info = get_mock_copy_info()
        copy_list = get_directory_copy_same_chain_copies(copy_info=copy_info)
        self.assertEqual(0, len(copy_list))

    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    def test_get_same_chain_copies_success_if_copy_is_directory_format(self):
        """
        测试场景：执行反向复制时，如果副本格式为非原生格式，则返回对应链的所有副本id
        前提条件: 存在非原生格式副本信息
        检查点: 非原生格式副本需要获取副本链
        """
        from app.replication.service.replication_service import get_directory_copy_same_chain_copies
        copy_info = get_mock_copy_info()
        copy_info.properties = "{\"format\": 1}"
        mock_copy_list = [copy_info.uuid]
        self.replication_service.get_same_chain_copies = Mock(return_value=mock_copy_list)
        copy_list = get_directory_copy_same_chain_copies(copy_info=copy_info)
        self.assertEqual(1, len(copy_list))
        self.assertEqual(copy_info.uuid, copy_list[0])

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(ProtectionServiceHttpsClient, "request", autospec=True)
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @unittest.skip
    def test_reverse_replication_success(self, mock_request):
        """
        测试场景：执行反向复制成功
        前提条件: 无
        检查点: 执行反向复制成功
        """
        from app.replication.schemas.replication_request import ReplicationRequest
        from app.replication.service.replication_service import reverse_replicate
        mock_copy_info = get_mock_copy_info()
        mock_copy_info.resource_properties = '{"root_uuid":"48f7dc0f-80c1-3bcd-97ed-89fd4cbfb33c","parent_name":"OpenGauss_single2","sub_type":"OpenGauss-instance","uuid":"135d0449-d2d1-5ec0-b274-82c89adc8f79"}'
        self.replication_service.query_copy_by_id = Mock(return_value=mock_copy_info)
        user_id = "123456"
        req = ReplicationRequest(copy_id="123456", external_system_id="1", retention_type=1, storage_type="1",
                                 storage_id="1", user_id="1", username="1", password="1")
        reverse_replicate(user_id, req)
        self.assertEqual(JobClient.create_job.call_count, 1)

    @patch.object(JobClient, "create_job", Mock(return_value=None))
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(ProtectionServiceHttpsClient, "request", autospec=True)
    @mock.patch("app.base.db_base.database.session", MagicMock)
    def test_reverse_replication_throw_exception(self, mock_request):
        """
        测试场景：执行反向复制，校验所选集群不是复制目标集群
        前提条件: 无
        检查点: 执行反向复制失败
        """
        from app.replication.schemas.replication_request import ReplicationRequest
        from app.replication.service.replication_service import reverse_replicate
        target = TargetClusterMock()
        target.role = 2
        self.replication_service.query_target_cluster_by_id = Mock(return_value=target)
        mock_copy_info = get_mock_copy_info()
        mock_copy_info.resource_properties = '{"root_uuid":"48f7dc0f-80c1-3bcd-97ed-89fd4cbfb33c","parent_name":"OpenGauss_single2","sub_type":"OpenGauss-instance","uuid":"135d0449-d2d1-5ec0-b274-82c89adc8f79"}'
        self.replication_service.query_copy_by_id = Mock(return_value=mock_copy_info)
        user_id = "123456"
        req = ReplicationRequest(copy_id="123456", external_system_id="1", retention_type=1, storage_type="1",
                                 storage_id="1", user_id="1", username="1", password="122")
        with self.assertRaises(EmeiStorBizException) as ex:
            reverse_replicate(user_id, req)
        message = "The target cluster role is not a replication cluster."
        self.assertEqual(CommonErrorCodes.ERR_PARAM.get("code"), str(ex.exception.error_code))
        self.assertEqual(message, ex.exception._error_message)


class TargetClusterMock:
    def __init__(self):
        self.status = "127.0.0.1"
        self.role = 1
