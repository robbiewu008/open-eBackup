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

def find_all_value_by_key(data, key_name):
    """
    从传入的字典中获取所有的指定key的值
    """
    value_list = []
    if isinstance(data, dict):
        for key, value in data.items():
            if key == key_name:
                # object对象， 必须有"additionalProperties": False
                value_list.append(value)
            # object对象的properties中可能还有object，所以继续迭代检查。
            value_list.extend(find_all_value_by_key(value, key_name))
    elif isinstance(data, list):
        for item in data:
            value_list.extend(find_all_value_by_key(item, key_name))
    return value_list
