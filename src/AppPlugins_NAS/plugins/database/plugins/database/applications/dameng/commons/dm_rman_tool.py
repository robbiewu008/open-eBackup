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
import stat
import time

from dameng.commons.const import DamengStrFormat, ExecCmdResult
from dameng.commons.common import del_file, check_path_in_white_list, dameng_execute_cmd
from dameng.commons.path_operation import dameng_user_mkdir
from dameng.resource.damengsource import DamengSource

from common.logger import Logger
from common.common import execute_cmd
from common.const import JobData

LOGGER = Logger().get_logger("dameng.log")


class DmRmanTool:

    @staticmethod
    def execute_run_rman_tool_cmd(cmd, redirect_output):
        result_info = {"result": False, "out_info": ''}
        fun = dameng_execute_cmd
        if redirect_output:
            fun = execute_cmd
        return_code, out_info, err_info = fun(cmd)
        if return_code == ExecCmdResult.SUCCESS:
            result_info["result"] = True
            result_info["out_info"] = out_info
        else:
            LOGGER.info(f"Failed to run the dmrman command.")
            result_info["out_info"] = err_info
        return result_info

    def run_rman_tool(self, dmrman_cmd=(), path_='', redirect_output=False, file_name_id=""):
        """
        执行具体的dmrman命令
        :param dmrman_cmd:命令列表
        :return: 执行命令返回结果列表
        """
        if file_name_id == "":
            file_name_id = JobData.PID
        result_info = {"result": False, "out_info": ''}
        if not dameng_user_mkdir(path_):
            return result_info
        return_type, install_user, _ = DamengSource.discover_application()
        if not return_type:
            LOGGER.error("Get install user fail.")
            return result_info
        bin_path = DamengSource.get_bin_path(install_user)
        if not bin_path:
            LOGGER.error("Get bin_path user fail.")
            return result_info
        if install_user == '':
            return result_info
        file_name = f"{path_}/dmrman_cmd_{JobData.PID}.txt"
        ret, file_name = check_path_in_white_list(file_name)
        if not ret:
            return result_info
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        modes = stat.S_IWUSR | stat.S_IRUSR + stat.S_IROTH
        with os.fdopen(os.open(file_name, flags, modes), 'w') as fout:
            for rman in dmrman_cmd:
                rman = rman.strip(";")
                fout.write(f"{rman};\n")
        os.chmod(file_name, stat.S_IRWXU + stat.S_IRWXG + stat.S_IRWXO)
        if redirect_output:
            redirect_output_path = f"{path_}/dmrman_cmd_{file_name_id}.log"
            ret, redirect_output_path = check_path_in_white_list(redirect_output_path)
            if not ret:
                return result_info
            cmd = DamengStrFormat.DMRMAN_REDIRECT_OUTPUT.format(install_user, bin_path,
                                                                file_name, redirect_output_path)
            result_info = self.execute_run_rman_tool_cmd(cmd, redirect_output)
            del_file(file_name)
            return result_info
        else:
            cmd = DamengStrFormat.DMRMAN.format(install_user, bin_path, file_name)
        result_info = self.execute_run_rman_tool_cmd(cmd, redirect_output)
        del_file(file_name)
        return result_info
