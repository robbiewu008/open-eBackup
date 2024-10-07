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

from openGauss.common.common import safe_get_environ
from openGauss.common.const import EXTEND_INFO, APPENV, OPEN_GAUSS_USER, DEPLOY_TYPE, \
    APPLICATION, EVEN_PATH, CLUSTER_VERSION, GUI_NODES


class ParamStruct:

    def __init__(self, param_dict, pid=None):
        if pid is not None:
            self.pid = pid
        for field, value in param_dict.items():
            if isinstance(value, dict):
                value = ParamStruct(value)
            setattr(self, field, value)

    @property
    def app_extend(self):
        extend = None
        application = getattr(self, APPLICATION, None)
        if application:
            extend = getattr(application, EXTEND_INFO, "")
        return extend

    @property
    def app_version(self):
        version = ""
        if self.app_extend:
            version = getattr(self.app_extend, CLUSTER_VERSION, "")
        return version

    @property
    def env_auth(self):
        auth_key = ""
        if getattr(self, "pid", None):
            auth_key = safe_get_environ(f'{OPEN_GAUSS_USER}_{self.pid}')
        return auth_key

    @property
    def app_env(self):
        app_env = getattr(self, APPENV, None)
        return app_env

    @property
    def app_deploy_type(self):
        deploy_type = ""
        if self.app_extend:
            deploy_type = getattr(self.app_extend, DEPLOY_TYPE, "")
        return deploy_type

    @property
    def env_extend(self):
        extend = None
        if self.app_env:
            extend = getattr(self.app_env, EXTEND_INFO, None)
        return extend

    @property
    def app_env_path(self):
        if not self.app_extend:
            return ""
        return getattr(self.app_extend, EVEN_PATH, "")

    @property
    def app_env_version(self):
        if not self.env_extend:
            return ""
        return getattr(self.env_extend, CLUSTER_VERSION, "")

    @property
    def gui_nodes(self):
        node_ips = []
        if not self.app_extend:
            return node_ips
        nodes = getattr(self.app_extend, GUI_NODES, [])
        if isinstance(nodes, str):
            nodes = json.loads(nodes)
        if not isinstance(nodes, list):
            return node_ips

        for node in nodes:
            try:
                host_ip = node.get("ip", "")
            except AttributeError:
                host_ip = ""
            node_ips.append(host_ip)
        return node_ips
