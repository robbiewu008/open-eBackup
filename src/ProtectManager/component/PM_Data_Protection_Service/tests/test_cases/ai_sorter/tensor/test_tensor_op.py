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


class TestTensorOp(unittest.TestCase):
    def setUp(self) -> None:
        super(TestTensorOp, self).setUp()
        sys.modules["app.common.database"] = mock.Mock()

    def tearDown(self) -> None:
        super(TestTensorOp, self).tearDown()
        del sys.modules["app.common.database"]

    def test_pre_process_success(self):
        """
        用例名称：测试pre_process预处理函数成功
        前置条件：输入正确的job_speed
        check点：未抛出异常
        """
        from app.ai_sorter.tensor import preprocessing

        job_speed_dataset = []
        for _ in range(100):
            job_speed_dataset.append(1.)

        train_set, validate_set, norm_paras = \
            preprocessing(job_speed_dataset, 10)
        self.assertIsInstance(train_set, torch.utils.data.TensorDataset)
        self.assertIsInstance(train_set, torch.utils.data.TensorDataset)
        self.assertIsInstance(validate_set, torch.utils.data.TensorDataset)
        self.assertEqual(norm_paras, [1, 0])

    @unittest.skip
    def test_torch_train_success(self):
        """
        用例场景： 训练模型，训练完成之后会保存一个model
                 当前训练model的测试会比较消耗时间，请耐心等待
        前置条件： AISorter运行正常
        检 查 点 ：返回有效序列值
        """
        import app.ai_sorter.tensor as tensor
        import app.ai_sorter.tensor.config as config

        job_speed_dataset = [1., 2., 3., 4., 5., 6., 7., 8., 9., 10.]
        train_set, validate_set, norm_paras = tensor.preprocessing(job_speed_dataset, 10)

        # 临时修改配置，减小训练时间
        config.opt.batch_sizes = [8]
        config.opt.hidden_sizes = [64]
        config.opt.lstm_layers = [1]
        tensor.train(train_set, validate_set, norm_paras)

        self.assertTrue(os.path.isfile(config.opt.final_model_path))
        self.assertFalse(os.path.isfile(config.opt.tmp_model_path))

        os.remove(config.opt.final_model_path)
        os.removedirs(config.opt.save_dir)
        config.opt = config.Options()