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

from app.copy_catalog.es_doc import some_time_value

class EsDocTest(unittest.TestCase):
    def test_some_time_value_when_timestamp_is_str(self):
        """
        时间戳格式为string，数据为微秒级
        期望：返回秒级数据格式时间戳
        return:以秒为单位的时间戳
        """
        timestamp = "1657074770"
        result = some_time_value(timestamp)
        self.assertEqual(1657, result)

    def test_some_time_value_when_timestamp_is_str_2(self):
        """
        时间戳格式为string，数据为秒级
        期望：返回秒级数据格式时间戳
        return:以秒为单位的时间戳
        """
        timestamp = "-1657"
        result = some_time_value(timestamp)
        self.assertEqual(1657, result)

    def test_some_time_value_when_timestamp_is_float(self):
        """
        时间戳格式为float，数据为秒级
        期望：返回秒级数据格式时间戳
        return:以秒为单位的时间戳
        """
        timestamp = 1559.2867742953627
        result = some_time_value(timestamp)
        self.assertEqual(1559, result)

    def test_some_time_value_when_timestamp_is_int(self):
        """
        时间戳格式为int，数据为微秒级
        期望：返回秒级数据格式时间戳
        return:以秒为单位的时间戳
        """
        timestamp = 1657074770
        result = some_time_value(timestamp)
        self.assertEqual(1657, result)

    def test_some_time_value_when_timestamp_is_other_type(self):
        """
        时间戳格式为其他类型
        期望：返回默认值585225380
        return:585225380
        """
        timestamp = [1657074770]
        result = some_time_value(timestamp)
        self.assertEqual(585225380, result)

    def test_some_time_value_when_timestamp_is_str_and_has_value_error(self):
        """
        时间戳格式为string，但是存在数据问题
        期望：返回默认值585225380
        return:585225380
        """
        timestamp = "2019-05-31 15:12:54"
        result = some_time_value(timestamp)
        self.assertEqual(585225380, result)