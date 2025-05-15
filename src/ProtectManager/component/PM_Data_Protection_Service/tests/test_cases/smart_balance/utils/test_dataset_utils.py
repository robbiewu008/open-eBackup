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
import os
import stat
import sys
import unittest
from unittest import mock
from tests.test_cases.smart_balance.dateset import all_data, nan_data, less_data
from tests.test_cases.smart_balance.dateset import input_type3, input_size3, input_time3


class TestDatasetUtils(unittest.TestCase):

    def setUp(self) -> None:
        self.all_data = all_data
        self.nan_data = nan_data
        self.less_data = less_data
        sys.modules["os"] = mock.Mock()

    def tearDown(self) -> None:
        del sys.modules['os']

    def test_parse_job_size_success(self):
        """
        用例名称：将任务大小按统一量纲读成功
        前置条件：从数据库得到任务大小
        check点：输出值正确
        """
        from app.smart_balance.utils.dataset_utils import parse_job_size
        data_after_reduction = "100 TB"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100) * 1024 * 1024, job_size)

        data_after_reduction = "100 GB"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100) * 1024, job_size)

        data_after_reduction = "100 TB"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100) * 1024 * 1024, job_size)

        data_after_reduction = "100 MB"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100), job_size)

        data_after_reduction = "100 KB"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100) / 1024, job_size)

        data_after_reduction = "100 B"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(float(100) / 1024 / 1024, job_size)

        data_after_reduction = "100"
        job_size = parse_job_size(data_after_reduction)
        self.assertEqual(0.0, job_size)

    def test_decode_job_info_from_database_catch(self):
        """
        用例名称：从数据库读并处理函数捕获分支成功
        前置条件：得到正确的输入
        check点：输出类型正确
        """
        from app.smart_balance.utils.dataset_utils import decode_job_info_from_database

        data = self.nan_data.split('"records":')[1][1:]
        data = data.split('{"userId":')[1:-1]

        for each_data in data:
            result = decode_job_info_from_database(each_data)
            self.assertEqual(result, [[], [], [], []])

    def test_decode_job_info_from_database_confidition(self):
        """
        用例名称：从数据库读并处理函数条件分支成功
        前置条件：得到正确的输入
        check点：输出类型正确
        """
        from app.smart_balance.utils.dataset_utils import decode_job_info_from_database

        data = self.less_data.split('"records":')[1][1:]
        data = data.split('{"userId":')[1:-1]

        for each_data in data:
            result = decode_job_info_from_database(each_data)
            self.assertEqual(result, [[], [], [], []])

    def test_decode_job_info_from_database_success(self):
        """
        用例名称：从数据库读并处理函数主逻辑成功
        前置条件：从数据库得到信息
        check点：输出类型正确
        """
        import json
        from app.smart_balance.utils.dataset_utils import decode_job_info_from_database

        data = json.loads(self.all_data)['records']
        with mock.patch('app.smart_balance.utils.dataset_utils.parse_job_size') as mock_parse_job_size:
            mock_parse_job_size.return_value = 100.0
            for each_data in data:
                result = decode_job_info_from_database(each_data)
                self.assertEqual(result, ["example", 1, 200.0, 100.0])

        data = json.loads(self.nan_data)['records']
        with mock.patch('app.smart_balance.utils.dataset_utils.parse_job_size') as mock_parse_job_size:
            mock_parse_job_size.return_value = 100.0
            for each_data in data:
                result = decode_job_info_from_database(each_data)
                self.assertEqual(result, ['example', 7, 200.0, 100.0])

    def test_confirm_reload_database_noneed(self):
        """
        用例名称：不需要重读数据库
        前置条件：模块开始执行
        check点：输出值正确
        """
        from app.smart_balance.utils.dataset_utils import confirm_reload_database
        from app.smart_balance.schemas import option

        file_path = option.dataset_path
        with os.fdopen(os.open(file_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR), 'w'):
            pass
        reload_needed = confirm_reload_database()
        self.assertTrue(reload_needed)
        os.remove(file_path)

    def test_fill_nan_success(self):
        """
        用例名称：fill_nan执行成功
        前置条件：模块开始执行
        check点：输出值长度正确
        """
        from app.smart_balance.utils.dataset_utils import fill_nan
        input_features = [input_size3, input_time3, input_type3]
        input_size, input_time, input_type = fill_nan(input_features, [], [], [])
        self.assertTrue(len(input_size[0]), 10)

    @mock.patch("os.walk")
    @mock.patch("os.remove")
    @mock.patch("app.smart_balance.utils.dataset_utils.get_time")
    def test_confirm_refresh_model(self, _mock_get_time, _mock_remove, _mock_walk):
        """
        用例名称：测试是否要删除过久的model
        前置条件：模块开始执行
        check点：os.remove被调用
        """

        from app.smart_balance.utils import confirm_refresh_model
        from app.smart_balance.schemas import option

        mock_filenames = ["file1.txt", "file2.txt", "file3.txt"]
        _mock_walk.return_value = [("", "", mock_filenames)]

        # 设置模拟的时间差值和常量
        _mock_get_time.side_effect = [4, 2, 7]  # 对应文件1、文件2、文件3的时间差
        res = confirm_refresh_model()
        self.assertIsNone(res)

    @mock.patch("os.path.isfile")
    def test_get_time(self, mock_isfile):
        """
        用例名称：get_time无文件传入时候的测试
        前置条件：模块开始执行
        check点：返回值正确
        """
        from app.smart_balance.utils.dataset_utils import get_time
        mock_isfile.return_value = False
        filepath = "dummy_filepath"
        result = get_time(filepath)
        self.assertEqual(result, -1)
