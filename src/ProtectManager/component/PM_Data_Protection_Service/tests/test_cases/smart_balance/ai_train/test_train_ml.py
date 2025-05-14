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
import unittest


class TestTrainML(unittest.TestCase):
    def setUp(self):
        super(TestTrainML, self).setUp()
        import pandas as pd
        self.features = pd.DataFrame([[1.0000000e+00, 2.1512800e-01, 3.0000000e+00, 7.2601600e-01, 4.0000000e+00],
                                      [4.0000000e+00, 7.9898000e-01, 2.0000000e+00, 1.0400000e-01, 3.0000000e+00],
                                      [1.0000000e+00, 2.1526100e-01, 3.0000000e+00, 7.2704000e-01, 4.0000000e+00],
                                      [1.0000000e+00, 4.1935400e-01, 3.0000000e+00, 7.2499200e-01, 4.0000000e+00],
                                      [2.0000000e+00, 2.2912900e-01, 3.0000000e+00, 1.4966000e-01, 3.0000000e+00],
                                      [2.0000000e+00, 1.7805300e-01, 3.0000000e+00, 1.6330000e-01, 2.0000000e+00],
                                      [4.0000000e+00, 1.2859300e-01, 3.0000000e+00, 6.8359375e-04, 1.0000000e+00],
                                      [1.0000000e+00, 2.3649400e-01, 3.0000000e+00, 7.1987200e-01, 4.0000000e+00],
                                      [1.0000000e+00, 1.9294600e-01, 3.0000000e+00, 7.1884800e-01, 4.0000000e+00],
                                      [4.0000000e+00, 1.3155300e-01, 3.0000000e+00, 6.8359375e-04, 1.0000000e+00]]
                                     )
        self.labels = pd.DataFrame([7.2704000e+03, 6.8359375e-03, 7.2499200e+03, 7.1987200e+03, 1.6330000e+01,
                                    1.6450000e+01, 6.8359375e-03, 7.1884800e+03, 2.7545600e+03, 6.8359375e-03])

    def test_ml_model_run_success(self):
        """
        用例场景： 训练模型，训练完成之后会保存一个model
        前置条件： AI建模运行正常
        检查点:   生成AI模型
        """
        from app.smart_balance.ai_train import train_ml
        from app.smart_balance.schemas import option, ModelName

        train_ml.train_ml_model(option.gbrt_model_path, self.features, self.labels, ModelName.gbrt)
        self.assertTrue(os.path.isfile(option.gbrt_model_path))
        os.remove(option.gbrt_model_path)

        train_ml.train_ml_model(option.rf_model_path, self.features, self.labels, ModelName.rf)
        self.assertTrue(os.path.isfile(option.rf_model_path))
        os.remove(option.rf_model_path)

        train_ml.train_ml_model(option.adabr_model_path, self.features, self.labels, ModelName.adabr)
        self.assertTrue(os.path.isfile(option.adabr_model_path))
        os.remove(option.adabr_model_path)
        os.removedirs(option.save_dir)
