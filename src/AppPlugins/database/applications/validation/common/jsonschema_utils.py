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

from common.exception.common_exception import ErrCodeException


def __check_has_pattern(copy):
    if copy.get("pattern"):
        return True
    elif copy.get("enum"):
        # 如果没有pattern，先检查是否有enum
        return True
    elif copy.get("format"):
        # 如果没有pattern，再检查是否有format
        return True
    elif copy.get("anyOf"):
        # anyOf要求每个对象都要满足string的检查规则
        return __check_any_in_array(copy, "anyOf")
    elif copy.get("oneOf"):
        # oneOf要求每个对象都要满足string的检查规则
        return __check_any_in_array(copy, "oneOf")
    elif copy.get("allOf"):
        # allOf要求有任意一个对象都要满足string的检查规则
        return __check_all_in_array(copy)
    else:
        return False


def __check_has_item(data):
    if data.get("maxItems") == 0:
        return True
    elif data.get("items") and data.get("items").get("type"):
        # 如果没有pattern，先检查是否有enum
        return True
    elif data.get("items") and data.get("items").get("$ref"):
        return True
    else:
        return False


def __check_all_in_array(copy):
    all_of = copy["allOf"]
    if isinstance(all_of, list):
        for item in all_of:
            if __check_has_pattern(item):
                return True
        return False
    else:
        return False


def __check_any_in_array(copy, key):
    any_of = copy[key]
    if isinstance(any_of, list):
        for item in any_of:
            if not __check_has_pattern(item):
                return False
        return True
    else:
        return False


def check_string_has_pattern(json_schema):
    """
    对传入的所有string对象， 检查有pattern或enum或format
    """
    if isinstance(json_schema, dict):
        copy = json_schema.copy()
        for key, value in copy.items():
            if key == "type" and "string" in value and not __check_has_pattern(copy):
                # string对象， 必须有pattern或enum或format
                raise ErrCodeException(-1, message=f"jsonschema is invalid. {copy} has no pattern of string")

            # 继续迭代检查
            check_string_has_pattern(value)
    elif isinstance(json_schema, list):
        for item in json_schema:
            check_string_has_pattern(item)


def add_additional_properties(data2):
    """
    对传入的所有object对象，添加"additionalProperties": False
    """
    if isinstance(data2, dict):
        copy = data2.copy()
        for key, value in copy.items():
            if key == "type" and "object" in value:
                # object对象， 必须有"additionalProperties": False
                data2.update({"additionalProperties": False})
            # object对象的properties中可能还有object，所以继续迭代检查。
            add_additional_properties(data2[key])
    elif isinstance(data2, list):
        for item in data2:
            add_additional_properties(item)


def check_array_has_item(json_schema):
    """
    对传入的所有array对象， 检查有items或maxItems=0
    """
    if isinstance(json_schema, dict):
        copy = json_schema.copy()
        for key, value in copy.items():
            if key == "type" and "array" in value and not __check_has_item(copy):
                # string对象， 必须有pattern或enum或format
                raise ErrCodeException(-1, message=f"jsonschema is invalid. {copy} has no items or maxItems=0 of array")

            # 继续迭代检查
            check_array_has_item(value)
    elif isinstance(json_schema, list):
        for item in json_schema:
            check_array_has_item(item)
