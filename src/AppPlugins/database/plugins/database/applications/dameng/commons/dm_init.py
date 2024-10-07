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

import os

from dameng.commons.const import DamengStrConstant
from dameng.commons.const import ExecCmdResult
from dameng.resource.damengsource import DamengSource

from common.common import execute_cmd
from common.logger import Logger

LOGGER = Logger().get_logger("dameng.log")


class DmInitTool:
    """
    执行具体的dmini命令
    :param cmd:命令
    :return: 执行命令返回结果列表
    """
    @staticmethod
    def run_dm_init(cmd):
        """
        执行具体的dmini命令
        :param cmd:命令
        :return: 执行命令返回结果列表
        """
        return_type, install_user, _ = DamengSource.discover_application()
        if not return_type:
            LOGGER.error("Get install user fail.")
            return False, ""
        bin_path = DamengSource.get_bin_path(install_user)
        if not bin_path:
            LOGGER.error("Get bin_path user fail.")
            return False, ""
        init_path = os.path.join(bin_path, DamengStrConstant.DM_INIT)
        bin_cmd = f"su - {install_user} -c \"{init_path} {cmd}\""
        ret, out, err = execute_cmd(bin_cmd)
        if ret != ExecCmdResult.SUCCESS:
            return False, err
        return True, out