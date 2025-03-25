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

def merge_schema(target_schema: dict, source_schema: dict):
    """
    合并2个jsonschema, 将source_schema的配置合入target_schema, 相同的key，以source_schema为准
    主要合并各properties中定义的属性。
    合并规则：
    1、在source中key但是不在target中， 直接复制到target中。
    2、definitions字段，调用本方法单独合并
    3、不合并$schema， $id 和description字段。
    @param target_schema: 公共参数
    @param source_schema: 应用定制
    """
    if not target_schema:
        return source_schema
    if not source_schema:
        return target_schema
    # 获取data1的key
    target_keys = target_schema.keys()
    source_schema_keys = source_schema.keys()
    for key in source_schema_keys:
        # 在source中key但是不在target中， 直接复制到target中。
        if key not in target_keys:
            target_schema[key] = source_schema[key]
        elif key == 'definitions':
            # definitions字段，调用本方法单独合并
            target_schema[key] = merge_schema(target_schema[key], source_schema[key])
        elif key != '$schema' and key != '$id' and key != 'description':
            # 不合并$schema， $id 和description字段。
            target_schema[key] = __merger_object(target_schema[key], source_schema[key])

    return target_schema


def __type_not_exist(type_str):
    if not type_str or type_str == "null" or type_str == ["null"]:
        return True
    return False


def __merger_object(target_obj: dict, source_obj: dict):
    """
    合并2个object对象, 将source_obj的配置合入target_obj, 相同的key，以source为准
    合并规则如下：
    1、source或target中有一个没有type属性，或都没有type属性，则obj以source为准
    2、source与target的type值不同， 则obj以source为准
    3、type值相同时
        3.1 type值为boolean/integer/number/string，没有需要合并的属性，直接以source为准
        3.2 type值为object，将source的properties和patternProperties的值合入target, 相同的key，以source为准
        3.3 type值为array，如有source或target没有items属性，直接以source为准， 都有items，则将source的合入master

    @param target_obj: 公共参数
    @param source_obj: 应用定制
    """
    target_type = target_obj.get("type")
    source_type = source_obj.get("type")
    if __type_not_exist(target_type) or __type_not_exist(source_type):
        # source或target中有一个没有type属性，或都没有type属性，则obj以source为准
        return source_obj

    ret, type_str = __is_same_type(target_obj, source_obj)
    if not ret:
        # 2、source与target的type值不同， 则obj以source为准
        return source_obj

    if type_str in ["boolean", "integer", "number", "string"]:
        # 3.1 type值为boolean/integer/number/string，没有需要合并的属性， 直接以source为准
        return source_obj

    if type_str == "object":
        # 3.2 type值为object，将source_obj的配置合入target_obj, 相同的key，以source_obj为准
        properties_1 = target_obj.get("properties")
        properties_2 = source_obj.get("properties")

        ret = merge_schema(properties_1, properties_2)
        if ret:
            target_obj["properties"] = ret

        pattern_properties_1 = target_obj.get("patternProperties")
        pattern_properties_2 = source_obj.get("patternProperties")

        ret = merge_schema(pattern_properties_1, pattern_properties_2)
        if ret:
            target_obj["patternProperties"] = ret
        return target_obj

    if type_str == "array":
        # 3.3 type值为array，如有source或target没有items属性，直接以source为准， 都有items，则将source的合入master
        items_1 = target_obj.get("items")
        items_2 = source_obj.get("items")
        if not items_1 or not items_2:
            return source_obj
        target_obj["items"] = __merger_object(items_1, items_2)
        return target_obj
    return source_obj


def __is_same_type(obj1, obj2):
    """
    检查2个obj的type是否相同。
    例：
    "string" == ["string"]
    "string" == ["string", null]
    ["string"] == ["string", null]
    ["string", null] == ["string", null]

    注意：type除了null之外，都没有其他值， 认为type不相同。
    例：
    ["null"] != ["null"]
    ["null"] != []
    """
    type1 = obj1.get("type")
    type2 = obj2.get("type")
    # 如果type是string对象， 则转换为list
    if isinstance(type1, str):
        type1 = [type1]
    if isinstance(type2, str):
        type2 = [type2]
    # 去除list中的null对象
    if "null" in type1:
        # 操作copy，以免影响源对象
        type1 = type1.copy()
        type1.remove("null")
    if "null" in type2:
        # 操作copy，以免影响源对象
        type2 = type2.copy()
        type2.remove("null")
    len1 = type1.__len__()
    len2 = type2.__len__()
    if len1 > 1 or len2 > 1:
        # type中除null之外的类型只允许有一种。
        raise Exception("type is too many")
    if len1 == 0 or len2 == 0:
        # type除了null之外，都没有其他值， 也认为type不相同。
        return False, ""
    return type1 == type2, type2.pop(0)
