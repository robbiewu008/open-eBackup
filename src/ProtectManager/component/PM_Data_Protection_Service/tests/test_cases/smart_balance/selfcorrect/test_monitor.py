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
import stat
import unittest
from unittest import mock
from unittest.mock import patch
from app.smart_balance.schemas import ModelName
from app.smart_balance.schemas import option
from tests.test_cases.smart_balance.dateset import all_data


class TestDatasetUtils(unittest.TestCase):

    def test_catch_threshold_success(self):
        """
        用例名称：判断阈值函数所有case成功
        前置条件：正确输入
        check点：输出值正确
        """
        from app.smart_balance.selfcorrect.monitor import catch_threshold
        predicted_value, real_value = 1, 1.5
        flag_value = catch_threshold(predicted_value, real_value)
        self.assertEqual(-1, flag_value)

        predicted_value, real_value = 1, 0.5
        flag_value = catch_threshold(predicted_value, real_value)
        self.assertEqual(1, flag_value)

        predicted_value, real_value = 1, 0.8
        flag_value = catch_threshold(predicted_value, real_value)
        self.assertEqual(0, flag_value)

    def test_check_threshold_success(self):
        """
        用例名称：判断finetune方向
        前置条件：正确输入
        check点：输出值正确
        """
        from app.smart_balance.selfcorrect.monitor import check_threshold
        history_amount = [2, 0, 1.1]
        row = [100, 1]

        count = 1
        flag_value = check_threshold(row, history_amount, count)
        self.assertEqual(0, flag_value)

        count = 2
        flag_value = check_threshold(row, history_amount, count)
        self.assertEqual(1, flag_value)

        count = 3
        flag_value = check_threshold(row, history_amount, count)
        self.assertEqual(-1, flag_value)

    @mock.patch("app.smart_balance.selfcorrect.monitor.query_job_list")
    def test_read_history_online_success(self, _mock_query_job_list):
        """
        用例名称：判断在线读历史数据的格式
        前置条件：正确输入
        check点：输出值格式正确
        """

        from app.smart_balance.selfcorrect.monitor import read_history_online

        _mock_query_job_list.return_value = all_data
        previous_seq_data, previous_ml_data = read_history_online("example", 1)
        self.assertEqual(len(previous_seq_data), 3)
        self.assertEqual(len(previous_ml_data), 3)

    def test_get_record_file(self):
        """
        用例名称：判断存在哪个模型返回正确的路径
        前置条件：正确输入
        check点：输出值格式正确
        """
        from app.smart_balance.selfcorrect.monitor import get_record_file

        with patch.object(option, 'seq_record_path', "seq_record_path"), \
                patch.object(option, 'gbrt_record_path', "gbrt_record_path"), \
                patch.object(option, 'rf_record_path', "rf_record_path"), \
                patch.object(option, 'adabr_record_path', "adabr_record_path"):
            result = get_record_file(ModelName.anet)
            self.assertEqual(result, "seq_record_path")

            result = get_record_file(ModelName.gbrt)
            self.assertEqual(result, "gbrt_record_path")

            result = get_record_file(ModelName.rf)
            self.assertEqual(result, "rf_record_path")

            result = get_record_file(ModelName.adabr)
            self.assertEqual(result, "adabr_record_path")

    @mock.patch('app.smart_balance.selfcorrect.monitor.get_record_file')
    def test_online_finetune_output_no_finetune(self, _mock_get_record_file):
        """
        用例名称：测试online_finetune不需要finetune的情况
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        import os
        from app.smart_balance.selfcorrect.monitor import online_finetune

        os.makedirs(option.save_dir, exist_ok=True)
        with os.fdopen(os.open(option.gbrt_model_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR),
                       'w') as txt_file:
            txt_file.write("1 1" + '\n')
            txt_file.write("1 3" + '\n')
        _mock_get_record_file.return_value = option.gbrt_model_path
        result = online_finetune("1", ModelName.gbrt, [[], [], ["1", "3.2", "0.0"]], 0.1)
        self.assertEqual(result, 0.1)
        os.remove(option.gbrt_model_path)
        os.removedirs(option.save_dir)

        _mock_get_record_file.return_value = "fake_path"
        result = online_finetune("1", ModelName.gbrt, [[], [], ["1", "3.2", "0.2"]], 0.1)
        self.assertEqual(result, 0.1)

    @mock.patch('app.smart_balance.selfcorrect.monitor.get_record_file')
    def test_online_finetune_output_finetune(self, _mock_get_record_file):
        """
        用例名称：测试online_finetune需要finetune的情况
        前置条件：底层成功
        check点：返回和预计输出一致
        """
        import os
        from app.smart_balance.selfcorrect.monitor import online_finetune

        os.makedirs(option.save_dir, exist_ok=True)
        with os.fdopen(os.open(option.gbrt_model_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR),
                       'w') as txt_file:
            txt_file.write("1 0.7" + '\n')
            txt_file.write("1 0.8" + '\n')
            txt_file.write("1 0.9" + '\n')
            txt_file.write("1 1.0" + '\n')
            txt_file.write("1 0.5" + '\n')
            txt_file.write("1 0.6" + '\n')
            txt_file.write("2 0.7" + '\n')
            txt_file.write("2 0.8" + '\n')
            txt_file.write("2 0.9" + '\n')
            txt_file.write("1 0.7" + '\n')
            txt_file.write("1 0.8" + '\n')
            txt_file.write("1 0.9" + '\n')
        _mock_get_record_file.return_value = option.gbrt_model_path
        result = online_finetune("1", ModelName.gbrt, [[], [], [3, 3, 3, 3, 3, 3, 3, 3, 3]], 0.5)
        self.assertEqual(result, 0.5 * 1.3)
        os.remove(option.gbrt_model_path)
        os.removedirs(option.save_dir)

        os.makedirs(option.save_dir, exist_ok=True)
        with os.fdopen(os.open(option.gbrt_model_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR),
                       'w') as txt_file:
            txt_file.write("1 9.7" + '\n')
            txt_file.write("1 9.8" + '\n')
            txt_file.write("1 9.9" + '\n')
            txt_file.write("1 9.0" + '\n')
            txt_file.write("1 9.5" + '\n')
            txt_file.write("1 9.6" + '\n')
            txt_file.write("2 0.7" + '\n')
            txt_file.write("2 0.8" + '\n')
            txt_file.write("2 0.9" + '\n')
            txt_file.write("1 0.7" + '\n')
            txt_file.write("1 0.8" + '\n')
            txt_file.write("1 0.9" + '\n')
        _mock_get_record_file.return_value = option.gbrt_model_path
        result = online_finetune("1", ModelName.gbrt, [[], [], [3, 3, 3, 3, 3, 3, 3, 3, 3]], 9)
        self.assertEqual(result, 9 * 0.7)
        os.remove(option.gbrt_model_path)
        os.removedirs(option.save_dir)

        os.makedirs(option.save_dir, exist_ok=True)
        with os.fdopen(os.open(option.gbrt_model_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR),
                       'w') as txt_file:
            txt_file.write("1 3.7" + '\n')
            txt_file.write("1 3.8" + '\n')
            txt_file.write("1 3.9" + '\n')
            txt_file.write("1 3.0" + '\n')
            txt_file.write("1 3.5" + '\n')
            txt_file.write("1 3.6" + '\n')
            txt_file.write("2 0.7" + '\n')
            txt_file.write("2 0.8" + '\n')
            txt_file.write("2 0.9" + '\n')
            txt_file.write("1 0.7" + '\n')
            txt_file.write("1 0.8" + '\n')
            txt_file.write("1 0.9" + '\n')
        _mock_get_record_file.return_value = option.gbrt_model_path
        result = online_finetune("1", ModelName.gbrt, [[], [], [3, 3, 3, 3, 3, 3, 3, 3, 3]], 3)
        self.assertEqual(result, 3)
        os.remove(option.gbrt_model_path)
        os.removedirs(option.save_dir)
