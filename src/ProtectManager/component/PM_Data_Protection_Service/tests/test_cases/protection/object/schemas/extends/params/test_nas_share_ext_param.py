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
from unittest import TestCase

from app.protection.object.common.protection_enums import SmallFileAggregation
from app.protection.object.schemas.extends.params.nas_share_ext_param import NasShareExtParam
from app.common.exception.unified_exception import IllegalParamException


class NasShareExtValuesMock:
    def __init__(self, small_file_aggregation, aggregation_file_size, aggregation_file_max_size):
        self.small_file_aggregation = small_file_aggregation
        self.aggregation_file_size = aggregation_file_size
        self.aggregation_file_max_size = aggregation_file_max_size


class TestNasShareExtParam(TestCase):
    def test_check_nas_share_ext_param_success(self):
        """
        验证场景：测试Nas共享保护扩展参数
        前置条件：参数正常
        验证点：无异常抛出
        """
        values = NasShareExtValuesMock(SmallFileAggregation.AGGREGATE, 128, 128)
        result = NasShareExtParam.check_ext_params(values.__dict__)
        self.assertEqual(result, values.__dict__)

    def test_should_raise_IllegalParamException_if_aggregation_file_size_is_illegal_when_check_ext_params(
            self):
        """
        验证场景：测试聚合文件大小是否合法
        前置条件：聚合文件大小超过范围
        验证点：抛出异常
        """
        values = NasShareExtValuesMock(SmallFileAggregation.AGGREGATE, 8000, 128)
        with self.assertRaises(IllegalParamException):
            NasShareExtParam.check_ext_params(values.__dict__)

    def test_should_raise_IllegalParamException_if_aggregation_file_size_is_smaller_than_aggregation_file_max_size(
            self):
        """
        验证场景：测试聚合文件大小是否小于待聚合文件最大大小
        前置条件：聚合文件大小小于待聚合文件最大大小
        验证点：抛出异常
        """
        values = NasShareExtValuesMock(SmallFileAggregation.AGGREGATE, 128, 256)
        with self.assertRaises(IllegalParamException):
            NasShareExtParam.check_ext_params(values.__dict__)


if __name__ == '__main__':
    unittest.main(verbosity=2)
