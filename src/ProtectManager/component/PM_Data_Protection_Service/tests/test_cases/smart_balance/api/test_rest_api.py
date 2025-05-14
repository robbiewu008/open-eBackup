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
import asyncio
import unittest
from unittest import mock
from pydantic import ValidationError

from app.smart_balance.api.rest_api import sort_clusters, retrain
from app.smart_balance.schemas import ExecuteCluster


class TestRestApi(unittest.TestCase):

    def test_should_raise_validationerror_when_request_sort_clusters_if_missing_required_field(self):
        """
        用例场景：对集群进行排序时，如果缺少必传字段，则抛出ValidationError异常
        前置条件：网络连接正常
        检查点：捕获ValidationError异常并输出
        """
        try:
            ExecuteCluster(esn=1)
        except ValidationError as e:
            pass

    @mock.patch("app.smart_balance.api.rest_api.sort_clusters")
    def test_should_success_when_request_sort_clusters_if_execute_clusters_is_empty(self, mock_sort_clusters):
        """
        用例场景：对集群进行排序时，如果传入空节点列表，返回空列表
        前置条件：网络连接正常
        检查点：输出类型正确
        """
        mock_sort_clusters.return_value = []

        sorted_nodes_list = asyncio.run(sort_clusters([]))
        self.assertEqual(sorted_nodes_list, [])

    def test_access_retrain_port_success(self):
        """
        用例场景：重训模型接口触发正常
        前置条件：网络连接正常
        检查点：输出类型正确
        """
        result = asyncio.run(retrain())
        self.assertEqual(result, None)
