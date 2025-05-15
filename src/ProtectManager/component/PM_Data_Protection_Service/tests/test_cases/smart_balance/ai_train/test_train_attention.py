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
import sys
import unittest
from unittest import mock
import torch.utils.data
from tests.test_cases.smart_balance.dateset import input_type_dataset, input_size_dataset, input_time_dataset

from app.smart_balance.utils.dataset_utils import dimensional_normalization


class TestTrainAttention(unittest.TestCase):
    def setUp(self):
        super(TestTrainAttention, self).setUp()
        sys.modules["app.common.database"] = mock.Mock()
        sys.modules["os"] = mock.Mock()

    def tearDown(self) -> None:
        del sys.modules['os']

    @unittest.skip
    def test_airun_success(self):
        """
        用例场景： 训练模型，训练完成之后会保存一个model
        前置条件： AI建模运行正常
        检查点:   生成AI模型
        """
        from app.smart_balance.ai_train import train_attention
        from app.smart_balance.utils import preprocessing_for_seq
        from app.smart_balance.schemas import option

        input_time = []
        input_size = []
        data_size = len(input_size_dataset)
        feature_size = len(input_size_dataset[0])
        for i in range(data_size):
            input_time_tmp = []
            input_size_tmp = []
            for j in range(feature_size):
                input_time_tmp.extend(dimensional_normalization(input_time_dataset[i][j]))
                input_size_tmp.extend(dimensional_normalization(input_size_dataset[i][j]))
            input_time.append(input_time_tmp)
            input_size.append(input_size_tmp)

        train_set, validate_set = preprocessing_for_seq(input_type_dataset, input_size,
                                                        input_time)

        training_dataloader = \
            torch.utils.data.DataLoader(train_set,
                                        batch_size=option.batch_sizes,
                                        shuffle=True,
                                        num_workers=0,
                                        drop_last=True)
        validate_dataloader = \
            torch.utils.data.DataLoader(validate_set,
                                        batch_size=option.batch_sizes,
                                        shuffle=False,
                                        num_workers=0,
                                        drop_last=True)

        train_attention.run_attention(training_dataloader, validate_dataloader)

        self.assertTrue(os.path.isfile(option.seq_model_path))

        os.remove(option.seq_model_path)
        os.removedirs(option.save_dir)
