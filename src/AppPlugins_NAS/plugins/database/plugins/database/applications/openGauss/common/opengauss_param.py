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

import configparser
import json

from common.logger import Logger
from common.parse_parafile import ParamFileUtil
from validation.common.json_util import find_all_value_by_key
from validation.validator import ParamValidator

LOGGER = Logger().get_logger(filename="openGauss.log")


def parse_ini_to_dict(data: str):
    """
    用于将str类型的ini格式数据转换成dict类型
    :param data: 输入字符串类型的ini格式数据
    :return: 转化成字典类型的数据
    """
    config = configparser.ConfigParser()
    config.read_string('[default]\n' + data)

    ini_dict = dict()
    for section in config.sections():
        for key, value in config.items(section):
            ini_dict[key] = value

    return ini_dict


class JsonParam:

    @staticmethod
    def parse_param_with_jsonschema(pid):
        # openGauss的参数校验路径
        path = "openGauss/jsonschema/openGauss_base_define.json"
        # 需要二次单独校验的参数路径
        agent_applications_jsonschema_path = "openGauss/jsonschema/openGauss_agent_applications_define.json"
        gui_nodes_jsonschema_path = "openGauss/jsonschema/openGauss_guiNodes_define.json"
        pg_probackup_conf_jsonschema_path = "openGauss/jsonschema/openGauss_pg_probackup_conf_define.json"

        file_content = ParamFileUtil.parse_param_file_and_valid_by_schema(pid, path)

        # 校验agent_applications
        agent_applications_list = find_all_value_by_key(file_content, "agent_applications")
        for agent_applications in agent_applications_list:
            ParamValidator.valid_data_by_schema(json.loads(agent_applications), agent_applications_jsonschema_path)

        # 校验guiNodes
        gui_nodes_list = find_all_value_by_key(file_content, "guiNodes")
        for gui_nodes in gui_nodes_list:
            ParamValidator.valid_data_by_schema(json.loads(gui_nodes), gui_nodes_jsonschema_path)

        # 校验pg_probackup_conf
        pg_probackup_conf_list = find_all_value_by_key(file_content, "pg_probackup.conf")
        for pg_probackup_conf in pg_probackup_conf_list:
            ParamValidator.valid_data_by_schema(parse_ini_to_dict(pg_probackup_conf), pg_probackup_conf_jsonschema_path)

        LOGGER.info("Param is valid!")

        return file_content
