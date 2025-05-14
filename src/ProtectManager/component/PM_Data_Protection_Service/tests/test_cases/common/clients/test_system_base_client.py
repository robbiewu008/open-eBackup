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
from unittest.mock import patch

from tests.test_cases.common.mock_settings import fake_settings
from app.common.clients.system_base_client import SystemBaseClient


class TestSystemBaseClient(unittest.TestCase):
    def test_should_return_none_if_fs_id_is_none_when_query_filesystem(self):
        """
        *用例场景：检查空值传入是否有异常
        *前置条件：参数为空
        *检查点:检查逻辑是否正常
        """
        ret = SystemBaseClient.query_filesystem(None)
        self.assertIsNone(ret)

    @patch("app.common.clients.system_base_client.get_system_base_by_url")
    def test_should_return_fs_info_if_fs_exists_when_query_filesystem(self, _mock_get_base):
        """
        *用例场景：文件系统为空
        *前置条件：接口正常
        *检查点:返回文件系统为空
        """
        filesystem_id = "123"
        test_filesystem_info = {'hyperMetroPairIds': []}
        _mock_get_base.return_value = test_filesystem_info
        res = SystemBaseClient.query_filesystem(filesystem_id)
        self.assertEqual(res, test_filesystem_info)

    def test_should_return_none_if_repl_pair_id_is_none_when_query_replication_pair(self):
        """
        *用例场景：检查空值传入是否有异常
        *前置条件：参数为空
        *检查点:检查逻辑是否正常
        """
        replication_pair_id = None
        self.assertIsNone(SystemBaseClient.query_replication_pair(replication_pair_id))

    @patch("app.common.clients.system_base_client.get_system_base_by_url")
    def test_should_return_repl_pair_info_if_repl_pair_exists_when_query_replication_pair(self, _mock_get_base):
        """
        *用例场景：文件系统为空
        *前置条件：参数为空
        *检查点:检查逻辑是否正常
        """
        filesystem_id = "123"
        test_query_replication_pair = {}
        _mock_get_base.return_value = test_query_replication_pair
        res = SystemBaseClient.query_replication_pair(filesystem_id)
        self.assertEqual(res, test_query_replication_pair)

    @patch("app.common.clients.system_base_client.get_system_base_by_url")
    def test_should_return_none_if_para_is_none_when_query_fs_cyber_engine(self, _mock_get_base):
        """
        *用例场景：安全一体机部署形态下，查询文件系统时，传入文件系统或设备为空
        *前置条件：参数为空
        *检查点:检查逻辑是否正常
        """
        self.assertFalse(SystemBaseClient.query_remote_storage_filesystem("", ""))
        self.assertFalse(SystemBaseClient.query_remote_storage_filesystem("device1", ""))
        self.assertFalse(SystemBaseClient.query_remote_storage_filesystem("", "fs1"))

    @patch("app.common.clients.system_base_client.get_system_base_by_url")
    def test_should_return_file_system_if_fs_exists_when_query_hyper_metro_pair_cyber_engine(self, _mock_get_base):
        """
        *用例场景：安全一体机部署形态下，查询双活域时，传入双活pair或设备为空
        *前置条件：参数为空
        *检查点:检查逻辑是否正常
        """
        self.assertFalse(SystemBaseClient.query_remote_storage_hyper_metro_pair("", ""))
        self.assertFalse(SystemBaseClient.query_remote_storage_hyper_metro_pair("device1", ""))
        self.assertFalse(SystemBaseClient.query_remote_storage_hyper_metro_pair("", "pairid1"))
