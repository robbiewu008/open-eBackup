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
from datetime import datetime
# from tests.test_cases import common_mocker # noqa
# from tests.test_cases.backup.common.context import mock_context # noqa
from app.backup.common.validators.sla_validator import ParamsValidator
from app.common.exception.unified_exception import IllegalParamException


class SlaMock:
    def __init__(self, name):
        self.name = name


class TestParamsValidator(unittest.TestCase):
    def test_check_sla_name_success(self):
        legal_sla = SlaMock("test_sla")
        result = ParamsValidator.check_name_has_no_pre_and_tail_space(legal_sla.name, "sla_name")
        self.assertIsNone(result)

    def test_should_raise_IllegalParamException_if_sla_name_has_pre_tail_space_when_create_sla(self):
        illegal_sla = SlaMock(" test_sla ")
        with self.assertRaises(IllegalParamException):
            ParamsValidator.check_name_has_no_pre_and_tail_space(illegal_sla.name, "sla_name")

    def test_check_time_window_1(self):
        """
        时间窗: 00:00:00 ~ 00:30:00   非跨天时间窗

        校验时间 2024-08-31 23:59:00，期望：True
        校验时间 2024-08-31 23:58:59，期望：False
        校验时间 2024-09-01 00:00:00，期望：True
        校验时间 2024-09-01 00:30:00，期望：False
        校验时间 2024-09-01 00:31:00，期望：False
        """
        check_time_1 = datetime.strptime('2024-08-31 23:59:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:30:00', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:31:00', '%Y-%m-%d %H:%M:%S')
        window_start = '00:00:00'
        window_end = '00:30:00'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_5))

    def test_check_time_window_2(self):
        """
        时间窗: 23:59:00 ~ 23:59:30

        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：False
        校验时间 2024-09-01 23:59:00，期望：True
        校验时间 2024-09-01 23:59:31，期望：False
        校验时间 2024-09-01 23:59:29，期望：True
        """
        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:00', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 23:59:31', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 23:59:29', '%Y-%m-%d %H:%M:%S')
        window_start = '23:59:00'
        window_end = '23:59:30'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_5))

    def test_check_time_window_3(self):
        """
        时间窗: 23:59:00 ~ 00:59:00

        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：False
        校验时间 2024-09-01 23:59:00，期望：True
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 00:59:01，期望：False
        """
        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:00', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:59:01', '%Y-%m-%d %H:%M:%S')
        window_start = '23:59:00'
        window_end = '00:59:00'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_5))

    def test_check_time_window_4(self):
        """
        时间窗: 00:00:00 ~ 00:00:00 全天都能执行

        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：True
        校验时间 2024-09-01 23:59:00，期望：True
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 00:59:01，期望：True
        校验时间 2024-09-01 00:00:00，期望：True
        """

        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:00', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:59:01', '%Y-%m-%d %H:%M:%S')
        check_time_6 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        window_start = '00:00:00'
        window_end = '00:00:00'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_5))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_6))

    def test_check_time_window_5(self):
        """
        时间窗: 00:00:00 ~ 23:59:59 全天都能执行

        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：True
        校验时间 2024-09-01 23:59:59，期望：True
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 00:59:01，期望：True
        校验时间 2024-09-01 00:00:00，期望：True
        """

        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:59', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:59:01', '%Y-%m-%d %H:%M:%S')
        check_time_6 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        window_start = '00:00:00'
        window_end = '23:59:59'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_5))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_6))

    def test_check_time_window_6(self):
        """
        时间窗: 14:00:00 ~ 14:00:00 全天都能执行
        时间窗容忍度 1min
        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：True
        校验时间 2024-09-01 23:59:59，期望：True
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 14:00:00，期望：True
        校验时间 2024-09-01 00:00:00，期望：True
        """

        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:59', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 14:00:00', '%Y-%m-%d %H:%M:%S')
        check_time_6 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        window_start = '14:00:00'
        window_end = '14:00:00'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_5))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_6))

    def test_check_time_window_7(self):
        """
        时间窗: 00:00:06 ~ 00:00:00
        时间窗容忍度 1min，全天都可以执行
        校验时间 2024-08-31 23:58:00，期望：True
        校验时间 2024-08-31 23:57:59，期望：True
        校验时间 2024-09-01 23:59:59，期望：True
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 00:00:06，期望：True
        校验时间 2024-09-01 00:00:00，期望：True
        """

        check_time_1 = datetime.strptime('2024-08-31 23:58:00', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:59', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:00:06', '%Y-%m-%d %H:%M:%S')
        check_time_6 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        window_start = '00:00:06'
        window_end = '00:00:00'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_5))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_6))

    def test_check_time_window_8(self):
        """
        时间窗: 00:01:10 ~ 23:59:59
        时间窗容忍度 1min，
        校验时间 2024-08-31 00:01:02，期望：True
        校验时间 2024-08-31 23:57:59，期望：True
        校验时间 2024-09-01 23:59:59，期望：False
        校验时间 2024-09-01 00:58:59，期望：True
        校验时间 2024-09-01 00:00:00，期望：False
        校验时间 2024-09-01 00:01:09，期望：True
        校验时间 2024-09-01 00:01:10，期望：True
        校验时间 2024-09-01 00:00:09，期望：False
        """

        check_time_1 = datetime.strptime('2024-08-31 00:01:02', '%Y-%m-%d %H:%M:%S')
        check_time_2 = datetime.strptime('2024-08-31 23:57:59', '%Y-%m-%d %H:%M:%S')
        check_time_3 = datetime.strptime('2024-09-01 23:59:59', '%Y-%m-%d %H:%M:%S')
        check_time_4 = datetime.strptime('2024-09-01 00:58:59', '%Y-%m-%d %H:%M:%S')
        check_time_5 = datetime.strptime('2024-09-01 00:00:00', '%Y-%m-%d %H:%M:%S')
        check_time_6 = datetime.strptime('2024-09-01 00:01:09', '%Y-%m-%d %H:%M:%S')
        check_time_7 = datetime.strptime('2024-09-01 00:01:10', '%Y-%m-%d %H:%M:%S')
        check_time_8 = datetime.strptime('2024-09-01 00:00:09', '%Y-%m-%d %H:%M:%S')
        window_start = '00:01:10'
        window_end = '23:59:59'
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_1))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_2))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_3))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_4))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_5))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_6))
        self.assertTrue(ParamsValidator.check_time_window(window_start, window_end, check_time_7))
        self.assertFalse(ParamsValidator.check_time_window(window_start, window_end, check_time_8))


