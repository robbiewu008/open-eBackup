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
import os
import re
import sys
import xml.etree.ElementTree as ET


def check_file_is_link(path):
    return os.path.islink(path)


def xml_to_json(tree, language):
    root = tree.getroot()
    dom = root.findall("errors/error")
    errors = {}
    for label in dom:
        error_code = label.attrib["id"]
        reason = label.findall("reason")[0].attrib["description"]
        suggestion = label.findall("suggestions/suggestion")[0].attrib["description"]
        reason = replace_var(reason)
        suggestion = replace_var(suggestion)
        if language == "zh":
            errors[error_code] = ("原因:{0}<br>建议:{1}".format(reason, suggestion)).replace(r"\\n", "<br>")
        else:
            errors[error_code] = ("Cause:{0}<br>Suggestion:{1}".format(reason, suggestion)).replace(r"\\n", "<br>")
    print(json.dumps(errors, ensure_ascii=False, indent=1))


def replace_var(params):
    if re.search(r"##\d+", params):
        temp_re = re.findall(r"##0\d*", params)
        for temp in temp_re:
            params = params.replace(temp, "{" + temp.split("##0")[1] + "}")
    return params


def add_gui_jar(path, language):
    try:
        if check_file_is_link(path):
            return
        tree = ET.parse(path)
        xml_to_json(tree, language)
    except Exception:
        return


if __name__ == '__main__':
    source_location = {"zh": "/OSM/conf/error_zh.xml", "en": "/OSM/conf/error_en.xml"}
    language = sys.argv[1]
    if language in ["en", "zh"]:
        add_gui_jar(source_location.get(language), language)
