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
import threading
import unittest
from unittest import mock
from app.smart_balance.schemas import option
from app.smart_balance.service import train_start
from tests.test_cases.smart_balance.dateset import input_type, input_size, input_time
from tests.test_cases.smart_balance.dateset import input_type2, input_time2, input_size2


class TestTrainOnline(unittest.TestCase):

    def test_train_start_success(self):
        """
        用例名称：测试train_start服务成功
        前置条件：底层成功
        check点：线程执行一次
        """
        thread_mock = mock.Mock(spec=threading.Thread)

        def mock_train_model():
            pass

        thread_mock.target = mock_train_model

        with mock.patch('threading.Thread', return_value=thread_mock):
            train_start()

            thread_mock.start.assert_called_once()

    @mock.patch("app.smart_balance.ai_train.train_attention.run_attention", mock.Mock())
    @mock.patch(
        "app.smart_balance.model.StaticContainer.training_lock", mock.Mock())
    @mock.patch("app.smart_balance.service.train_online.preprocessing_for_ml")
    @mock.patch("app.smart_balance.utils.read_txt_for_seq_train")
    def test_train_model_success(self, _mock_read_txt_for_seq_train, _mock_preprocessing_for_ml):
        """
        用例名称：测试train_model服务成功
        前置条件：底层成功
        check点：解锁成功执行一次
        """
        from app.smart_balance.service.train_online import train_model
        from app.smart_balance.model import StaticContainer
        import pandas as pd

        _mock_read_txt_for_seq_train.return_value = [input_type2, input_time2, input_size2]
        features = pd.DataFrame([[1.0000000e+00, 2.1512800e-01, 3.0000000e+00, 7.2601600e-01, 4.0000000e+00],
                                 [4.0000000e+00, 7.9898000e-01, 2.0000000e+00, 1.0400000e-01, 3.0000000e+00],
                                 [1.0000000e+00, 2.1526100e-01, 3.0000000e+00, 7.2704000e-01, 4.0000000e+00],
                                 [1.0000000e+00, 4.1935400e-01, 3.0000000e+00, 7.2499200e-01, 4.0000000e+00],
                                 [2.0000000e+00, 2.2912900e-01, 3.0000000e+00, 1.4966000e-01, 3.0000000e+00],
                                 [2.0000000e+00, 1.7805300e-01, 3.0000000e+00, 1.6330000e-01, 2.0000000e+00],
                                 [4.0000000e+00, 1.2859300e-01, 3.0000000e+00, 6.8359375e-04, 1.0000000e+00],
                                 [1.0000000e+00, 2.3649400e-01, 3.0000000e+00, 7.1987200e-01, 4.0000000e+00],
                                 [1.0000000e+00, 1.9294600e-01, 3.0000000e+00, 7.1884800e-01, 4.0000000e+00],
                                 [4.0000000e+00, 1.3155300e-01, 3.0000000e+00, 6.8359375e-04, 1.0000000e+00]])
        labels = pd.DataFrame([7.2704000e+03, 6.8359375e-03, 7.2499200e+03, 7.1987200e+03, 1.6330000e+01,
                               1.6450000e+01, 6.8359375e-03, 7.1884800e+03, 2.7545600e+03, 6.8359375e-03])
        _mock_preprocessing_for_ml.return_value = [features, labels]
        train_model()
        os.remove(option.gbrt_model_path)
        os.remove(option.rf_model_path)
        os.remove(option.adabr_model_path)
        # assert StaticContainer.training_lock.release.call_count == 2

    @mock.patch("app.smart_balance.ai_train.train_attention.run_attention", mock.Mock())
    @mock.patch(
        "app.smart_balance.model.StaticContainer.training_lock", mock.Mock())
    @mock.patch("app.smart_balance.utils.read_txt_for_seq_train")
    def test_train_model_lack_data_and_false(self, _mock_read_txt_for_seq_train):
        """
        用例名称：测试train_model因数据不足未启动
        前置条件：底层成功
        check点：解锁不成功
        """
        from app.smart_balance.service.train_online import train_model
        from app.smart_balance.model import StaticContainer

        _mock_read_txt_for_seq_train.return_value = [input_type, input_time, input_size]
        train_model()
        StaticContainer.training_lock.release.assert_not_called()
