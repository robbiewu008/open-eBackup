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
import sys
import unittest
import uuid
from unittest import mock
from unittest.mock import Mock, patch, MagicMock

from app.copy_catalog.schemas.copy_schemas import DeleteExcessCopiesRequest
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()
from app.copy_catalog.service.delete_excess_copy_service import delete_excess_copy
from app.copy_catalog.models.tables_and_sessions import CopyTable
from tests.test_cases.tools import env, functiontools, http
from tests.test_cases.tools.kafka_producer_mock import KafkaProducerMock

from app.copy_catalog.service.curd.copy_query_service import get_deleting_copy, query_all_copy_ids_by_resource_id

sys.modules['app.common.events.producer'] = Mock()
sys.modules['app.common.database.Database.initialize'] = Mock()
sys.modules['app.common.config'] = Mock()
sys.modules['app.common.database'] = Mock()
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("pydantic.validator", functiontools.mock_decorator).start()

producer_patcher = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))


class CopyCreateServiceTest(unittest.TestCase):

    def setUp(self) -> None:
        self.get_deleting_copy = get_deleting_copy
        self.query_copy_ids_by_resource_id = query_all_copy_ids_by_resource_id

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.query_all_copy_ids_by_resource_id")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.request_delete_copy_by_id")
    def test_should_call_request_delete_copy_by_id_zero_time_when_delete_excess_copy_if_current_copy_size_equal_to_max_count(
            self,
            request_delete_copy_by_id,
            _mock_query_copy_ids_by_resource_id,
            mock_session):
        """
        用例场景：如果当前副本数量为2，最大保留数量为2，则不调用删除副本函数
        前置条件: 副本保留类型为按数量保留
        检查点：当前副本数未超过最大保留数量，不删除副本
        """
        copy = CopyTable(uuid=str(uuid.uuid4()), timestamp="123456", display_timestamp=datetime.datetime.now(),
                         deletable=True,
                         status=1, generated_by=2, indexed=0, generation=0, retention_type=3,
                         resource_id=str(uuid.uuid4()),
                         resource_name="test", resource_type="Database", resource_location="test",
                         resource_status="exist",
                         resource_properties="{\"user_id\":\"test id\",\"type\":\"Database\","
                                             "\"sub_type\":\"Oracle\"}")
        _mock_query_copy_ids_by_resource_id.return_value = ["1", "2"]
        request = DeleteExcessCopiesRequest(resource_id=str(uuid.uuid4()), retentionQuantity=2, generatedBy="Backup",
                                            user_id=str(uuid.uuid4()))
        delete_excess_copy(request=request, resource_id=str(uuid.uuid4()))
        self.assertEqual(request_delete_copy_by_id.call_count, 0)

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.query_all_copy_ids_by_resource_id")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.request_delete_copy_by_id")
    def test_should_call_request_delete_copy_by_id_one_time_when_delete_excess_copy_if_current_copy_size_3_max_count_2(
            self,
            request_delete_copy_by_id,
            _mock_query_copy_ids_by_resource_id,
            mock_session):
        """
        用例场景：如果当前副本数量为3，最大保留数量为2，则调用1次删除副本函数
        前置条件: 副本保留类型为按数量保留
        检查点： 能够删除多余的副本
        """
        request = DeleteExcessCopiesRequest(resource_id=str(uuid.uuid4()), retentionQuantity=2, generatedBy="Backup",
                                            user_id=str(uuid.uuid4()))
        _mock_query_copy_ids_by_resource_id.return_value = ["1", "2", "3"]
        delete_excess_copy(request=request, resource_id=str(uuid.uuid4()))
        self.assertEqual(request_delete_copy_by_id.call_count, 1)

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.query_all_copy_ids_by_resource_id")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.request_delete_copy_by_id")
    def test_should_call_request_delete_copy_by_id_two_times_when_delete_excess_copy_if_current_copy_size_4_max_count_2(
            self,
            request_delete_copy_by_id,
            _mock_query_copy_ids_by_resource_id,
            mock_session):
        """
        用例场景：如果当前副本数量为4，最大保留数量为2，则调用2次删除副本函数
        前置条件: 副本保留类型为按数量保留
        检查点： 能够删除多余的副本
        """
        _mock_query_copy_ids_by_resource_id.return_value = ["1", "2", "3", "4"]
        request = DeleteExcessCopiesRequest(resource_id=str(uuid.uuid4()), retentionQuantity=2, generatedBy="Backup",
                                            user_id=str(uuid.uuid4()))
        delete_excess_copy(request=request, resource_id=str(uuid.uuid4()))
        self.assertEqual(request_delete_copy_by_id.call_count, 2)

    @mock.patch("sqlalchemy.orm.session.Session")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.query_all_copy_ids_by_resource_id")
    @mock.patch("app.copy_catalog.service.delete_excess_copy_service.request_delete_copy_by_id")
    def test_should_return_when_delete_excess_copy_if_retention_quantity_is_0(
            self,
            request_delete_copy_by_id,
            _mock_query_copy_ids_by_resource_id,
            mock_session):
        """
        用例场景：如果当前副本数量为4，最大保留数量为0，则直接返回
        前置条件: 副本保留类型为按数量保留
        检查点： 当策略中最大保留数量为0时是否直接返回
        """
        _mock_query_copy_ids_by_resource_id.return_value = ["1", "2", "3", "4"]
        request = DeleteExcessCopiesRequest(resource_id=str(uuid.uuid4()), retentionQuantity=0, generatedBy="Backup",
                                            user_id=str(uuid.uuid4()))
        delete_excess_copy(request=request, resource_id=str(uuid.uuid4()))
        self.assertEqual(request_delete_copy_by_id.call_count, 0)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_create_replicated_copies(self, mock_one_or_none):
        """
        验证场景：创建复制副本
        前置条件：无
        返回：副本创建成功
        """
        from app.copy_catalog.schemas import ReplicatedCopiesSchema
        from app.copy_catalog.service.curd.copy_create_service import create_replicated_copies
        replicated_copies_schema = ReplicatedCopiesSchema(copy_id="123", resource_id="321", esn=1)
        mock_one_or_none.return_value = True
        result = create_replicated_copies(replicated_copies_schema)
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    def test_generate_copy_archive_map(self, mock_one_or_none):
        """
        验证场景：生成副本归档映射
        前置条件：无
        返回：副本归档映射生成成功
        """
        from app.copy_catalog.schemas import CopyArchiveMapSchema
        from app.copy_catalog.service.curd.copy_create_service import generate_copy_archive_map
        copy_archive_map = CopyArchiveMapSchema(copy_id="123", storage_id="321", resource_id="231")
        mock_one_or_none.return_value = True
        result = generate_copy_archive_map(copy_archive_map)
        self.assertIsNone(result)
