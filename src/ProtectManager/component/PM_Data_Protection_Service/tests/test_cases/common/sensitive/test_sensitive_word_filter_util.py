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
import json
import unittest

from app.common.sensitive.sensitive_word_filter_util import sensitive_word_filter, \
    sensitive_word_filter_extend_sensitive_word
from pydantic import BaseModel


class TestSensitiveWordFilter(unittest.TestCase):

    # 测试set
    def test_set(self):
        sets = set()
        dict_str = json.dumps({"password": "p1", "session": "se1", "type": "oracle"})
        sets.add(dict_str)
        dict_filet_str = sensitive_word_filter(sets)
        value = dict_filet_str.pop()
        json_param = json.loads(value)
        keys = json_param.keys()
        self.assertFalse('password' in keys)
        self.assertFalse('session' in keys)

    # 测试dict
    def test_dict(self):
        dict_value = {"a": "a1", "param": {"b": "b1", "user_password": "123456"},
                      "params": [{"to": "from", "token": "eqe"}]}
        sensitive_word_filter_extend_sensitive_word(dict_value, True, [])
        self.assertTrue(str(dict_value) == "{'a': 'a1', 'param': {'b': 'b1'}, 'params': [{'to': 'from'}]}")

    # 测试list
    def test_list(self):
        list_value = TestSensitiveWordFilter.get_list_value()
        sensitive_word_filter_extend_sensitive_word(list_value, True, [])
        self.assertTrue(str(list_value) == "[{'a': 'a1', 'param': {'b': 'b1'}, 'params': "
                                           "[{'to': 'from'}]}, {'name': 'eqwe'}, {'name': 'eqwe'}]")

    # 测试tuple
    def test_tuple(self):
        tuple_value = ({"password": "p", "list_param": TestSensitiveWordFilter.get_list_value()})
        sensitive_word_filter_extend_sensitive_word(tuple_value, True, [])
        self.assertTrue(str(tuple_value) == "{'list_param': [{'a': 'a1', 'param': {'b': 'b1'}, 'params': "
                                            "[{'to': 'from'}]}, {'name': 'eqwe'}, {'name': 'eqwe'}]}")

    # 测试str
    def test_str(self):
        str_value = json.dumps(TestSensitiveWordFilter.get_list_value())
        str_filter_value = sensitive_word_filter(str_value)
        self.assertTrue(str_filter_value == "[{\"a\": \"a1\", \"param\": {\"b\": \"b1\"}, \"params\": [{\"to\": "
                                            "\"from\"}]}, {\"name\": \"eqwe\"}, {\"name\": \"eqwe\"}]")

    # 测试BaseModel
    def test_base_model(self):
        class UserModel(BaseModel):
            user_name: str
            password: str
        user_model = UserModel(user_name="xiaoming", password="my")
        user_filter_model = sensitive_word_filter(user_model)
        self.assertTrue(user_filter_model == {})

    # 获取测试的list数据
    @staticmethod
    def get_list_value():
        list_value = [{"a": "a1", "param": {"b": "b1", "user_password": "123456"},
                       "params": [{"to": "from", "token": "eqe"}]}, {"privateKey": "prik", "name": "eqwe"},
                      {"publicKey": "pubk", "name": "eqwe"}]
        return list_value


if __name__ == '__main__':
    unittest.main(verbosity=2)
