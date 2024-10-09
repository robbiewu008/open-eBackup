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

import json
import os

from common.parse_parafile import ParamFileUtil
from validation.common.json_util import find_all_value_by_key
from validation.validator import ParamValidator


class ConfSplit:
    EQUAL_SPLIT = "="


class ReadFile(object):
    @staticmethod
    def read_param_file(file_path):
        """
        解析参数文件
        :return:
        """
        if not os.path.isfile(file_path):
            raise Exception(f"File:{file_path} not exist")
        try:
            with open(file_path, "r", encoding='UTF-8') as f:
                json_dict = json.loads(f.read())
        except Exception as e:
            raise Exception("parse param file failed") from e
        return json_dict

    @staticmethod
    def read_conf(path):
        """
        读取conf文件
        :return: 
        """
        conf = {}
        if not os.path.exists(path):
            return conf
        with open(path, 'r', encoding='utf-8') as file_read:
            for line in file_read.readlines():
                if "=" in line:
                    key_value = line.strip('\n').split(ConfSplit.EQUAL_SPLIT)
                    key = key_value[0].strip()
                    value = key_value[1].strip()
                    conf[key] = value
        return conf


class ParseParaFile(object):
    @staticmethod
    def parse_para_file(pid):
        """
        解析参数文件
        :return:
        """
        file_content = ParamFileUtil.parse_param_file_and_valid_by_schema(pid,
                                                                          "mysql/src/jsonschema/mysql_base_define.json")
        ParseParaFile.check_master_info(file_content)
        return file_content

    @staticmethod
    def check_master_info(file_content):
        master_info_jsonschema_path = "mysql/src/jsonschema/master_info_define.json"

        # 校验clusterInfo
        master_info_list = find_all_value_by_key(file_content, "master_info")
        for master_info in master_info_list:
            ParamValidator.valid_data_by_schema(json.loads(master_info), master_info_jsonschema_path)


class RestoreParseParam:
    def __init__(self, restore_log, sub_type, cluster_type, role_type):
        self.restore_log = restore_log
        self.sub_type = sub_type
        self.cluster_type = cluster_type
        self.role_type = role_type


class MasterInfoParseParam:
    def __init__(self, master_host, master_user, master_password, master_port):
        self.master_host = master_host
        self.master_user = master_user
        self.master_password = master_password
        self.master_port = master_port


class ExecSQLParseParam:
    def __init__(self, host_ip, port, user, passwd_env, sql_str="", database_name="", passwd='', charset=""):
        self.host_ip = host_ip
        self.port = port
        self.user = user
        self.passwd_env = passwd_env
        self.sql_str = sql_str
        self.database_name = database_name
        self.passwd = passwd
        self.charset = charset


class BaseConnectParam:
    def __init__(self, host_ip, port, charset, cmd):
        self.host_ip = host_ip
        self.port = port
        self.charset = charset
        self.cmd = cmd
