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
import uuid
from unittest import mock
from unittest.mock import Mock, patch

from urllib3 import HTTPResponse

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, ReplicationModeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.clients import client_util
from app.common.security import kmc_util
from app.common.security.kmc_util import Kmc
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()

from app.replication.client.replication_client import ReplicationClient
from app.common.clients.client_util import ProtectionServiceHttpsClient


class TargetClusterMock:
    def __init__(self):
        self.status = "127.0.0.1"
        self.role = 1
        self.cluster_name = "cluster_name"


class DistributionStorageMock:
    def __init__(self):
        self.uuid = "ecf16f64-72de-4383-b73b-f61561718fe1"
        self.name = "distribution_name"


class ReplicationClientTest(unittest.TestCase):

    def setUp(self):
        super(ReplicationClientTest, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        sys.modules['app.common.events.producer'] = Mock()
        from app.replication.client import replication_client
        self.replication_client = replication_client
        self.replication_client.query_target_cluster_by_id = Mock(return_value=TargetClusterMock())
        self.replication_client.query_distribution_storage_by_id = Mock(
            return_value=DistributionStorageMock())

    def tearDown(self) -> None:
        del sys.modules['app.common.database']
        del sys.modules['app.common.config']
        del sys.modules['app.protection.object.common.db_config']
        del sys.modules['app.common.events.producer']

    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=HTTPResponse(status=500)))
    def test_should_raise_ex_if_query_copies_ex_when_query_copy_statistic_by_resource_id(self):
        """
        测试场景：根据资源ID获取副本数目时调用"/v1/internal/copies"接口响应错误
        前提条件: 调用"/v1/internal/copies"接口响应错误
        检查点: 抛出异常
        """
        from app.replication.client.replication_client import ReplicationClient
        with self.assertRaises(EmeiStorBizException) as ex:
            ReplicationClient.query_copies_by_resource_id(None)
        self.assertEqual(ex.exception._error_message, "query backup copies failed")

    @patch.object(ProtectionServiceHttpsClient, "request", autospec=True)
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    def test_query_copy_statistic_by_resource_id_success(self, _mock_req):
        """
        测试场景：根据资源ID获取副本数目时调用"/v1/internal/copies"接口响应正确
        前提条件: 调用"/v1/internal/copies"接口响应正确
        检查点: 返回副本数目正确
        """
        fake_resource_id = str(uuid.uuid4())
        copies_num = 1
        resp_data = {"total": copies_num, "items": [
            "{\"backup_type\": 0, \"chain_id\": \"string\", \"deletable\": true, \"display_timestamp\": \"2023-01-28T12:28:16\",\"duration_unit\": \"d\", \"expiration_time\": \"2023-01-28T08:33:07.252Z\", \"features\": 0,\"generated_by\": \"Backup\", \"generated_time\":\"2023-01-28 12:28:16\", \"generation\": 0,\"indexed\": \"Unindexed\", \"is_archived\": true, \"is_replicated\": true, \"location\": \"Local\"}"]}
        _mock_req.return_value = HTTPResponse(body=json.dumps(resp_data).encode("utf-8"), status=200)
        from app.replication.client.replication_client import ReplicationClient
        ret = ReplicationClient.query_copies_by_resource_id(fake_resource_id)
        self.assertEqual(len(ret), copies_num)

    @unittest.skip("skipping this test")
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[{"uuid":"a55c3800-9e25-439d-b70a-5fa996440fc8","timestamp":"1678434059000000","display_timestamp":"2023-03-10T15:40:59", "generated_time":"2023-03-10T15:40:59", "deletable":True,"status":"Normal","retention_type":2,"retention_duration":30,"duration_unit":"d","expiration_time":"2023-04-09T15:40:59","properties":"{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}","resource_id":"868a5c250bc6499cac77604be56998bd", "device_esn":"38457854"}]))
    def test_query_copy_statistic(self):
        """
        测试场景：根据资源ID获取生成时间最晚的待复制副本
        前提条件: 有副本
        检查点: 正确获取副本
        """
        last_copy = ReplicationClient.query_copy_statistic("123", {"qos_id":"","external_system_id":"2","link_deduplication":True,"link_compression":True,"alarm_after_failure":True,"start_replicate_time":"2023-03-09 00:27:00"})
        self.assertTrue(last_copy)

    @unittest.skip("skipping this test")
    @mock.patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "a55c3800-9e25-439d-b70a-5fa996440fc8", "timestamp": "1678434059000000",
         "display_timestamp": "2023-01-10T15:40:59", "generated_time": "2023-01-10T15:40:59", "deletable": True,
         "status": "Normal", "retention_type": 2, "retention_duration": 30, "duration_unit": "d",
         "expiration_time": "2023-04-09T15:40:59",
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}",
         "resource_id": "868a5c250bc6499cac77604be56998bd"}]))
    def test_query_copy_statistic_no_copy(self):
        """
		测试场景：根据资源ID获取生成时间最晚的待复制副本
		前提条件: 没有符合条件的副本
		检查点: 返回空
		"""
        last_copy, device_esn = ReplicationClient.query_copy_statistic("123", {"qos_id": "", "external_system_id": "2",
                                                                   "link_deduplication": True, "link_compression": True,
                                                                   "alarm_after_failure": True,
                                                                   "start_replicate_time": "2023-03-09 00:27:00"})
        self.assertTrue(last_copy == [])
        self.assertTrue(device_esn == [])

    @unittest.skip("skipping this test")
    @patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[
        {"uuid": "acedff55-35ea-49a6-991b-b52ed172c42a", "chain_id": "5edad7c8-6cdb-4a24-a85f-a1440c3e2467",
         "timestamp": "1674880096000000", "display_timestamp": "2023-01-28T12:28:16", "deletable": True,
         "status": "Normal", "generated_time": "2023-01-28T12:28:16","device_esn":"939985943",
         "user_id": "", "is_archived": False, "is_replicated": False,
         "properties": "{\"format\":1,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}"
         }, {"uuid": "acedff55-35ea-49a6-991b-b52ed172c42b", "chain_id": "5edad7c8-6cdb-4a24-a85f-a1440c3e2467",
         "timestamp": "1674880096000000", "display_timestamp": "2023-01-28T12:28:16", "deletable": True,
         "status": "Normal", "generated_time": "2023-01-29T12:28:16","device_esn":"939985943",
         "user_id": "", "is_archived": False, "is_replicated": False,
         "properties": "{\"format\":3,\"verifyStatus\":\"3\",\"size\":480,\"backup_id\":\"a55c3800-9e25-439d-b70a-5fa996440fc8\",\"replicate_count\":1}"
         }]))
    def test_query_specified_time_copy_statistic(self):
        """
		测试场景：根据资源ID及复制策略查询是否存在指定时间副本
		前提条件: 1.该资源存在指定时间的副本;2.复制策略为复制复制指定时间副本
		检查点: 返回True
		"""
        ext_parameters = {
            "specified_scope": [
                {
                    "copy_type": "year",
                    "generate_time_range": "1",
                    "retention_unit": "y",
                    "retention_duration": 1
                },
                {
                    "copy_type": "month",
                    "generate_time_range": "last",
                    "retention_unit": "MO",
                    "retention_duration": 2
                },
                {
                    "copy_type": "week",
                    "generate_time_range": "mon",
                    "retention_unit": "w",
                    "retention_duration": 3
                }
            ],
            "qos_id": "",
            "external_system_id": "3",
            "link_deduplication": True,
            "link_compression": True,
            "alarm_after_failure": False,
            "replication_target_type": 2,
            "start_replicate_time": "2023-01-28 00:16:02"
        }
        last_copy = ReplicationClient.query_copy_statistic("123", ext_parameters)
        self.assertTrue(len(last_copy) == 2)

    @unittest.skip("skipping this test")
    @patch.object(ReplicationClient, "query_copies_by_resource_id", Mock(return_value=[]))
    def test_query_specified_time_copy_statistic_when_resource_without_copy(self):
        """
		测试场景：根据资源ID及复制策略查询是否存在指定时间副本
		前提条件: 1.资源下没有副本;2.复制策略为复制复制指定时间副本
		检查点: 返回False
		"""
        ext_parameters = {
            "specified_scope": [
                {
                    "copy_type": "month",
                    "generate_time_range": "first",
                    "retention_unit": "MO",
                    "retention_duration": 2
                }
            ],
            "start_replicate_time": "2023-01-28 00:16:02"
        }
        last_copy, device_esn = ReplicationClient.query_copy_statistic("123", ext_parameters)
        self.assertTrue(last_copy == [])
        self.assertTrue(device_esn == [])

