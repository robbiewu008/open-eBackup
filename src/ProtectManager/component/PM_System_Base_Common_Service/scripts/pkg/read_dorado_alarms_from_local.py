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
# coding: utf-8
import json
import os
import re
import sys

import grp
import pwd
import subprocess
import xml.etree.ElementTree as ET

zh_xml_path = "/OSM/conf/event_xve_zh.xml"
en_xml_path = "/OSM/conf/event_xve_en.xml"
alarm_zh_dir = "/tmp/BOOT-INF/classes/conf/alarmI18nZ"
alarm_en_dir = "/tmp/BOOT-INF/classes/conf/alarmI18nE"
zh_json_path = os.path.join(alarm_zh_dir, "DoradoV6AlarmZn.json")
en_json_path = os.path.join(alarm_en_dir, "DoradoV6AlarmEn.json")

# 前两个为告警，后面为上报给DM的事件，不需要转换
not_handle_alarms = {"0xF0C90001", "0xF0C90002", "0x2064032B0001", "0x2064032B0007", "0x2064032B0008", "0x2064032B0009",
                     "0x2064032B000A", "0x2064032B000B", "0x206400770002", "0x2064032B0016", "0x2064032B0017",
                     "0x2064032B0018", "0x2064032B000C", "0x2064032B000F", "0x2064032B000D", "0x2064032B0011",
                     "0x2064032B000E", "0x2064032B0010", "0x206400770001", "0x2064032B0012", "0x2064032B0015",
                     "0x2064032B0013", "0x2064032B0014", "0x2063032B0001", "0x2013E21C0001", "0x206500C80002",
                     "0x206403430001", "0x206403430002", "0x206403430003", "0x2064032B0019", "0x206403460002",
                     "0x206403460009", "0x20640346000A", "0x2064032C0019", "0x2064032C0017"}


def check_file_is_link(path):
    return os.path.islink(path)


def read_local_alarms_and_create_json(tree):
    root = tree.getroot()
    dom = root.findall("eventDefinition/param")
    alarms = {}
    for label in dom:
        alarm_id = hex_alarm_id(label.attrib["eventID"])

        if alarm_id in not_handle_alarms:
            continue

        name = label.attrib["name"]
        description = label.attrib["description"]
        detail = label.attrib["detail"]
        suggestion = label.attrib["suggestion"]
        location = label.attrib["location"]
        param_i18n = get_param_i18n(detail)

        name = replace_var(name)
        description = replace_var(description)
        detail = replace_var(detail)
        suggestion = suggestion.replace("&#10;", "<br>").replace(r"\\n", "<br>")
        location = parse_location(location)
        param = json.dumps(param_i18n, ensure_ascii=False)

        alarms[alarm_id + ".alarm.name"] = name
        alarms[alarm_id + ".alarm.advice"] = suggestion
        alarms[alarm_id + ".alarm.desc"] = description
        alarms[alarm_id + ".alarm.desc.detail"] = detail
        alarms[alarm_id + ".alarm.location"] = location
        if param != "{}":
            alarms[alarm_id + ".alarm.desc.param"] = param
    print(json.dumps(alarms, ensure_ascii=False, indent=1))


def replace_var(params):
    if re.search(r"##\d+", params):
        temp_re = re.findall(r"##\d+ *{.*?}", params)
        for temp in temp_re:
            num = int(temp.split("{")[0].split("##")[1])
            params = params.replace(temp, "{%d}" % num)
        temp_re = re.findall(r"##\d+", params)
        for temp in temp_re:
            params = params.replace(temp, "{%d}" % int(temp.split("##")[1]))
    return params


def hex_alarm_id(params):
    return "{:#X}".format(int(params)).replace(r"0X", "0x", 1)


def parse_location(params: str):
    if params.__len__() == 0:
        return params
    list = []
    for temp in params.split(","):
        list.append("[" + temp.split("=")[0] + "]")
    return "".join(list)


def get_param_i18n(params):
    res = {}
    if re.search(r"##\d+", params):
        temp_re = re.findall(r"##\d+ *{.*?}", params)
        for temp in temp_re:
            key = temp.split("{")[0].split("##")[1]
            value = temp.split("{")[1].split("}")[0]
            param = {}
            for per_param in value.split(";"):
                per_param = per_param.strip()
                split = per_param.split(":")
                if len(split) == 2:
                    param[split[0].strip()] = split[1].strip()
            res[str(int(key))] = param
    return res


def add_to_jar(path):
    try:
        if (not os.path.exists(path)) or check_file_is_link(path):
            return
        tree = ET.parse(path)
        read_local_alarms_and_create_json(tree)
    except Exception:
        return


if __name__ == '__main__':
    source_location = {"en": "/OSM/conf/event_xve_en.xml", "zh": "/OSM/conf/event_xve_zh.xml"}
    language = sys.argv[1]
    if language in ["en", "zh"]:
        add_to_jar(source_location.get(language))
