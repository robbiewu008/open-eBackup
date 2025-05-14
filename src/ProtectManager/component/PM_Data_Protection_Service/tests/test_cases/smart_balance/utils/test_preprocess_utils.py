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

import torch

from tests.test_cases.smart_balance.dateset import input_type, input_size, input_time, input_type2, \
    input_time2, input_size2




class TestPreprocessUtils(unittest.TestCase):

    def test_seq_preprocess_success(self):
        """
        用例名称：seq_preprocess预处理函数主逻辑成功
        前置条件：得到正确的输入
        check点：输出类型正确
        """
        from app.smart_balance.utils.preprocess_utils import preprocessing_for_seq

        train_set, validate_set = preprocessing_for_seq(input_type2, input_time2,
                                                                                  input_size2)
        self.assertIsInstance(train_set, torch.utils.data.TensorDataset)
        self.assertIsInstance(validate_set, torch.utils.data.TensorDataset)


    def test_not_split_seq_dataset_success(self):
        """
        用例名称：seq数据太少就不划分数据集
        前置条件：得到正确的输入
        check点：数据集大小正确
        """
        from app.smart_balance.utils.preprocess_utils import preprocessing_for_seq
        train_set, validate_set = preprocessing_for_seq(input_type, input_time, input_size)
        self.assertEqual(len(train_set), len(validate_set))