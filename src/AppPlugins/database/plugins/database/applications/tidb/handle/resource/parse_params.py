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
import logging

from common.parse_parafile import ParamFileUtil
from tidb.logger import log


class ResourceParam:

    def __init__(self, pid):
        self.pid = pid
        try:
            self._body_param = ParamFileUtil.parse_param_file(self.pid)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not self._body_param:
            raise Exception(f"Failed to parse job param file is none")

    def get_param(self):
        """
        获取参数
        :return:
        """
        return self._body_param

    def get_register_info(self):
        """
        获取集群注册时用户填写的参数
        """
        conditions = self._body_param.get("condition", {}).get("conditions", {})
        action_type = conditions.get("action_type", "")
        return action_type, conditions

    def get_list_application_extend_info(self):
        """
        获取extendInfo下的操作系统用户
        :param
        :return: PM下发的操作系统用户
        """
        extend_info = {}
        param = self._body_param.get("applications", [])
        if param:
            extend_info = param[0].get("extendInfo", {})
        return extend_info

    def get_extend_info(self):
        """
        获取一个节点的认证信息
        :param
        :return 一个节点的认证信息
        """
        return self._body_param.get("application", {}).get("extendInfo", {})

    def get_app_env_info(self):
        return self._body_param.get("appEnv", {}).get("extendInfo", {})
