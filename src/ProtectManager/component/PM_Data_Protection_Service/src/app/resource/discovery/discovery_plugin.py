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
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema


class DiscoveryPlugin:
    def do_delete_env(self, params: str):
        pass

    def do_scan_env(self, params: ScanEnvSchema, is_rescan=False, is_session_connect=False):
        """
        扫描环境
        :param params:环境参数
        :param is_rescan: 是否重写扫描
        :param is_session_connect: 是否使用session连接
        :return:
        """
        pass

    def do_modify_env(self, params: UpdateEnvSchema):
        """
        修改环境参数
        :param params:环境参数
        :return:
        """
        pass

    def do_fetch_resource(self, params: ScanEnvSchema):
        pass

    def pre_check(self, params: ScanEnvSchema):
        """
        接入环境前的预检查，比如，网络联通性，用户名、密码的正确性检查
        :param params:
        :return:
        """
        pass
