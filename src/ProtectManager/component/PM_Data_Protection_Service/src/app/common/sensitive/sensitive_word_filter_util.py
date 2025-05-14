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
import copy
import json
import re
from app.common import logger
from pydantic import BaseModel
from app.common.sensitive.sensitive_word_list import sensitive_word_list

log = logger.get_logger(__name__)


# 参数：json格式的字符串
def _filter_json_string(json_string: str, all_sensitive_word_list: list):
    if json_string is None:
        return ""
    try:
        json_param = json.loads(json_string)
        json_data = _judge_value(json_param, all_sensitive_word_list)
        return json.dumps(json_data)
    except Exception:
        return json_string


# 参数：dict字典
def _filter_json(json_data: dict, all_sensitive_word_list: list):
    if json_data is None:
        return {}
    try:
        for key in list(json_data.keys()):
            # 判断key是不是敏感词，如果是敏感词，则直接从json_data里移除掉，否则就要判断value里的数据
            result = _point_data_if_sensitive_word(key, all_sensitive_word_list)
            if result:
                del json_data[key]
            else:
                # 处理value
                json_data[key] = _judge_value(json_data.get(key, ""), all_sensitive_word_list)
        return json_data
    except Exception:
        return json_data


# 参数：list
def _filter_list(list_data: list, all_sensitive_word_list: list):
    try:
        for i in range(len(list_data)):
            list_data[i] = _judge_value(list_data[i], all_sensitive_word_list)
        return list_data
    except Exception:
        return list_data


# 参数：BaseModel
def _filter_base_model(base_model: BaseModel, sensitive_list: list):
    if base_model is None:
        return BaseModel()
    try:
        return _filter_json(base_model.dict(), sensitive_list)
    except Exception:
        return base_model


# 对value判断
def _judge_value(json_data, all_sensitive_word_list):
    if isinstance(json_data, (list, set, tuple)):
        return type(json_data)(_filter_list(list(json_data), all_sensitive_word_list))
    elif isinstance(json_data, dict):
        return _filter_json(json_data, all_sensitive_word_list)
    elif isinstance(json_data, BaseModel):
        return _filter_base_model(json_data, all_sensitive_word_list)
    elif isinstance(json_data, (str, bytes)):
        return _filter_json_string(json_data, all_sensitive_word_list)
    else:
        return json_data


# 判断key是不是敏感词，返回一个bool值
def _point_data_if_sensitive_word(key, all_sensitive_word_list: list):
    for sensitive_word in all_sensitive_word_list:
        if sensitive_word.startswith("@"):
            if sensitive_word[1: len(sensitive_word)].upper() == key.upper():
                return True
        elif sensitive_word.find("%") != -1:
            result = re.search(sensitive_word.replace("%", ".*"), key, re.I)
            if result is not None:
                return True
        else:
            if sensitive_word.upper() == key.upper():
                return True
    return False


# 简易的入口方法
# 不修改参数里的数据，不新加敏感词
# 参数：要敏感过滤的数据.
def sensitive_word_filter(data):
    return sensitive_word_filter_extend_sensitive_word(data, False, [])


# 入口方法
# 参数一：要敏感过滤的数据.
# 参数二：是否允许直接在源数据上修改，如果False，则深复制一个数据出来处理.
# 参数三：支持额外新增敏感词，如果使用者的敏感词很特殊，那么可以在处理时额外把特殊敏感词加进处理逻辑中.
def sensitive_word_filter_extend_sensitive_word(data, is_affect_source_data: bool, add_sensitive_list: list):
    try:
        data_log = data
        if not is_affect_source_data:
            data_log = copy.deepcopy(data)
        all_sensitive_list = sensitive_word_list + add_sensitive_list
        return _judge_value(data_log, all_sensitive_list)
    except Exception as ex:
        log.exception(f"Remove Sensitive Words Fail:{ex}")
        return data
