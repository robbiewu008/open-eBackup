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
from collections import OrderedDict
from unittest.mock import patch
import torch
from sklearn.ensemble import RandomForestRegressor, AdaBoostRegressor, GradientBoostingRegressor
from app.smart_balance.model import StaticContainer
from app.smart_balance.selfcorrect import load_model


class TestDatasetUtils(unittest.TestCase):

    def setUp(self) -> None:
        self.torch_return = {
            'model': OrderedDict([('multihead_attn.in_proj_weight', torch.Tensor([[0.1133] * 64] * 192)),
                                  ('multihead_attn.in_proj_bias', torch.Tensor([1] * 192)),
                                  ('multihead_attn.out_proj.weight', torch.Tensor([[1] * 64] * 64)),
                                  ('multihead_attn.out_proj.bias', torch.Tensor([0] * 64)),
                                  ('pre_node.0.weight', torch.Tensor([[0.1133] * 45] * 64)),
                                  ('pre_node.0.bias', torch.Tensor([0] * 64)),
                                  ('pre_node.2.weight', torch.Tensor([[1] * 64] * 64)),
                                  ('pre_node.2.bias', torch.Tensor([0] * 64)),
                                  ('linear.0.weight', torch.Tensor([[0] * 64] * 32)),
                                  ('linear.0.bias', torch.Tensor([0] * 32)),
                                  ('linear.3.weight', torch.Tensor([[0] * 32] * 1)),
                                  ('linear.3.bias', torch.Tensor([0]))
                                  ])
        }

    def test_has_built_model_all_models_exist(self):
        """
           用例名称：仅4个模型均存在才返回True
           前置条件：开启智能均衡模式
           check点：返回值正确
        """
        with patch.object(StaticContainer, 'seq_model', 'dummy_seq_model'), \
                patch.object(StaticContainer, 'gbrt_model', 'dummy_gbrt_model'), \
                patch.object(StaticContainer, 'rf_model', 'dummy_rf_model'), \
                patch.object(StaticContainer, 'ada_model', 'dummy_ada_model'):
            result = load_model()

            self.assertTrue(result)

    def test_has_built_model_missing_models(self):
        """
           用例名称：只要一个模型不存在就返回False
           前置条件：开启智能均衡模式
           check点：返回值正确
        """
        with patch.object(StaticContainer, 'seq_model', None), \
                patch.object(StaticContainer, 'gbrt_model', 'dummy_gbrt_model'), \
                patch.object(StaticContainer, 'rf_model', None), \
                patch.object(StaticContainer, 'ada_model', 'dummy_ada_model'):
            result = load_model()

            self.assertFalse(result)

    @patch("os.path.isfile", side_effect=[False, False, False, True])
    @patch("torch.load")
    def test_reload_model_when_seq_model_format_wrong(self, _mock_torch_load, _mock_isfile):
        """
        用例名称：当其他模型均不存在，seq模型存在但不符合预设存储格式
        前置条件：开启智能均衡模式
        check点：返回值正确
        """
        with patch.object(StaticContainer, 'seq_model', "1"), \
                patch.object(StaticContainer, 'gbrt_model', None), \
                patch.object(StaticContainer, 'rf_model', None), \
                patch.object(StaticContainer, 'ada_model', None):
            _mock_torch_load.return_value = {
                'nomodel': 1
            }
            result = load_model()

            self.assertFalse(result)

    @patch('os.path.isfile')
    @patch("joblib.load")
    @patch("torch.load")
    def test_need_reload_model_if_trained_model_more_than_one(self, _mock_torch_load, _mock_joblib_load, _mock_isfile):
        """
        用例名称：只要有一个训练好的模型就可以开启ai_policy,否则不行
        前置条件：开启智能均衡模式
        check点：返回值正确
        """
        _mock_isfile.side_effect = [False, False, False, False]
        result = load_model()
        self.assertFalse(result)

        _mock_isfile.side_effect = [True, False, False, False]
        _mock_joblib_load.return_value = [GradientBoostingRegressor()]
        result = load_model()
        self.assertTrue(result)

        _mock_isfile.side_effect = [False, True, False, False]
        _mock_joblib_load.return_value = [RandomForestRegressor()]
        result = load_model()
        self.assertTrue(result)

        _mock_isfile.side_effect = [False, False, True, False]
        _mock_joblib_load.return_value = [AdaBoostRegressor()]
        result = load_model()
        self.assertTrue(result)

        _mock_isfile.side_effect = [False, False, False, True]
        _mock_torch_load.return_value = self.torch_return
        result = load_model()
        self.assertTrue(result)

    @patch("os.path.isfile", side_effect=[True, True, True, True])
    @patch("joblib.load", side_effect=[GradientBoostingRegressor(), RandomForestRegressor(), AdaBoostRegressor()])
    @patch("torch.load")
    def test_reload_model_success(self, _mock_torch_load, _mock_joblib_load, _mock_isfile):
        """
        用例名称：当所有ai模型为None时,寻找训练路径下是否存在模型，找到模型并且模型格式正确
        前置条件：开启智能均衡模式
        check点：返回值正确
        """
        StaticContainer.ai_model = None
        _mock_torch_load.return_value = self.torch_return
        result = load_model()

        self.assertTrue(result)
        self.assertIsNotNone(StaticContainer.seq_model)
        self.assertIsNotNone(StaticContainer.gbrt_model)
        self.assertIsNotNone(StaticContainer.rf_model)
        self.assertIsNotNone(StaticContainer.ada_model)
