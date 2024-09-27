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
from dameng.commons.const import DamengStrFormat, ExecCmdResult
from dameng.resource.damengsource import DamengSource

from common.logger import Logger
from common.common import execute_cmd, check_command_injection

LOGGER = Logger().get_logger("dameng.log")


class DmCtlcvt:

    @staticmethod
    def run_ctlcvt_tool(src='', dest='', fun_type=1):
        """
        ctl文件与ini文件转换
        :param dest: 转换源文件路径
        :param type: 转换目标文件路径
        :return: bool
        """
        return_code, username, usergroup = DamengSource.discover_application()
        if not return_code:
            return ExecCmdResult.UNKNOWN_CMD, '', ''
        bin_path = DamengSource.get_bin_path(username)
        if not os.path.exists(src):
            return ExecCmdResult.UNKNOWN_CMD, '', ''
        cmd = DamengStrFormat.DMCTLCVT.format(username, bin_path, fun_type, src, dest)
        if check_command_injection(cmd):
            return ExecCmdResult.UNKNOWN_CMD, '', ''
        return_code, out_info, err_info = execute_cmd(cmd)
        return return_code, out_info, err_info