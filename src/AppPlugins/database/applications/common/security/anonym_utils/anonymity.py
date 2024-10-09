#
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
#

import copy
import json
import inspect
import re
from pydantic import BaseModel


# 用于对结构化数据（Json格式）进行脱敏
class Anonymity:
    """
    脱敏基本规则：
    1.关键字不区分大小写
    2.以下关键字作包含匹配: pass, pwd, key, crypto, session, token, fingerprint, auth, enc, dec, tgt, iqn, initiator,
                        secret, cert, salt, private, user_info, verfiycode, rand, safe, PKCS1, base64, AES128, AES256,
                        RSA, SHA1, SHA256, SHA384, SHA512, algorithm
    3.以下关键字作全词匹配: sk, mk, iv
    4.业务中有2、3场景不能覆盖的其他敏感字段, 自行加入匹配关键字中。
    """
    sensitive_words = ["%pass%", "%pwd%", "%key%", "%crypto%", "%session%", "%token%", "%fingerprint%", "%auth%",
                       "%enc%", "%dec%", "%tgt%", "%iqn%", "%initiator%", "%secret%", "%cert%",  "%salt%", "%private%",
                       "%user_info%", "%verfiycode%", "%rand%", "%safe%", "%PKCS1%", "%base64%", "%AES128%", "%AES256%",
                       "%RSA%", "%SHA1%", "%SHA256%", "%SHA384%", "%SHA521%", "%algorithm%", "@mk", "@sk", "@iv"]

    @classmethod
    def process(cls, data):
        """
        通用方式，不修改参数里的数据，不新加敏感词
        :param data: 要脱敏处理的数据
        :return: 脱敏后的数据
        """
        return cls.process_ext(data, False, [])

    @classmethod
    def process_ext(cls, data, is_affect_source_data: bool, add_sensitive_list: list):
        """
        扩展方式，支持自定义的敏感词以及是否修改源数据
        :param data: 要脱敏处理的数据
        :param is_affect_source_data: 是否允许直接在源数据上修改，如果False，则深复制一个数据出来处理.
        :param add_sensitive_list: 支持额外新增敏感词，如果使用者的敏感词很特殊，那么可以在处理时额外把特殊敏感词加进处理逻辑中.
        :return:
        """
        data_log = data
        if not is_affect_source_data:
            data_log = copy.deepcopy(data)
        all_sensitive_list = cls.sensitive_words + add_sensitive_list
        try:
            data_result = cls._process_data(data_log, all_sensitive_list)
        except Exception as ex:
            return data
        return data_result

    @classmethod
    def is_sensitive_word(cls, key, sensitive_word_list: list):
        """
        判断key是否为敏感词
        """
        for sens_word in sensitive_word_list:
            if sens_word.startswith("@"):
                if sens_word[1: len(sens_word)].upper() == key.upper():
                    return True, sens_word
            elif sens_word.find("%") != -1:
                result = re.search(sens_word.replace("%", ".*"), key, re.I)
                if result is not None:
                    return True, sens_word
            else:
                if sens_word.upper() == key.upper():
                    return True, sens_word
        return False, ''

    @classmethod
    def find_first_sens_word_index(cls, key: str, sensitive_word_list: list):
        """
        判断key是否为敏感词
        :param key: 要检查的字符串
        :param sensitive_word_list: 敏感信息关键字列表
        :return: 返回字符串中第一个的敏感字符串的索引
        """
        first_index = len(key)
        for sens_word in sensitive_word_list:
            # 全字匹配字符串等于敏感词本身，不需要脱敏,只需匹配关键字
            if sens_word.find("%") == -1:
                continue
            result = re.search(sens_word.replace("%", ""), key, re.I)
            if not result:
                continue
            if result.end() < first_index:
                first_index = result.end()
        if first_index == len(key):
            return False, 0
        return True, first_index

    @classmethod
    def _process_json_string(cls, json_str: str, sensitive_word_list: list):
        if json_str is None:
            return ""
        try:
            json_param = json.loads(json_str)
        except Exception as ex:
            result, sens_word_index = cls.find_first_sens_word_index(json_str, sensitive_word_list)
            if not result:
                return json_str
            return json_str[:sens_word_index] + ':******'
        else:
            json_data = cls._process_data(json_param, sensitive_word_list)
            return json.dumps(json_data)


    @classmethod
    def _process_json(cls, json_data: dict, sensitive_word_list: list):
        if json_data is None:
            return {}
        try:
            for key in list(json_data.keys()):
                # 判断key是不是敏感词，如果是敏感词，则直接从json_data里移除掉，否则就要判断value里的数据
                result, sens_word = cls.is_sensitive_word(key, sensitive_word_list)
                if result:
                    del json_data[key]
                else:
                    # 处理value
                    json_data[key] = cls._process_data(json_data.get(key, ""), sensitive_word_list)
            return json_data
        except Exception as e_info:
            return json_data

    @classmethod
    def _process_list(cls, list_data: list, sensitive_word_list: list):
        try:
            for index, value in enumerate(list_data):
                list_data[index] = cls._process_data(value, sensitive_word_list)
            return list_data
        except Exception as e_info:
            return list_data

    @classmethod
    def _process_base_model(cls, base_model: BaseModel, sensitive_word_list: list):
        if base_model is None:
            return BaseModel()
        try:
            return cls._process_json(base_model.dict(), sensitive_word_list)
        except Exception as e_info:
            return base_model

    @classmethod
    def _process_data(cls, json_data, sensitive_word_list: list):
        if isinstance(json_data, (list, set, tuple)):
            return type(json_data)(cls._process_list(list(json_data), sensitive_word_list))
        elif isinstance(json_data, dict):
            return cls._process_json(json_data, sensitive_word_list)
        elif isinstance(json_data, BaseModel):
            return cls._process_base_model(json_data, sensitive_word_list)
        elif isinstance(json_data, (str, bytes)):
            return cls._process_json_string(json_data, sensitive_word_list)
        else:
            return json_data

