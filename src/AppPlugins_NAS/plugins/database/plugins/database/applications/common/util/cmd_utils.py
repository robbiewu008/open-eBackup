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

import platform
import shlex

from common.err_code import CommErrCode
from common.exception.common_exception import ErrCodeException
from common.util.validators import ALL_VALIDATOR_NAMES, NAME_VALIDATOR_MAP

if platform.system().lower() == "windows":
    from common.logger_wins import Logger
else:
    from common.logger import Logger
LOGGER = Logger().get_logger()


class CmdParam:
    def __init__(self, cmd: str, fmt_params_list: list):
        """
        :param cmd: 命令字符创，例："python3 {zctl_file} -t {t_p}"
        :param fmt_params_list: 命令格式化参数列表的列表，例：
            [("zctl_file", "/opt/gs/app/bin/zctl.py", ValidatorEnum.PATH_CHK_FILE),
             ("t_p", "status", ValidatorEnum.CHAR_CHK_EXCLUDE_QUOTE)],
        """
        self.cmd = cmd
        self.fmt_params_list = fmt_params_list


def cmd_format(cmd, *args):
    """
    格式化命令，用于应用二进制命令
    :param cmd: 命令字符创
    :param args: 命令参数列表
    :return: 格式化后的命令
    """
    params = list()
    for param in args:
        if param:
            param = shlex.quote(str(param))
        params.append(param)
    return cmd.format(*params)


def cmd_format_with_validators(param: CmdParam):
    """
    命令格式化，带参数校验器
    :param param: 执行命令函数参数实例
    :return: 格式化后的命令
    """
    _check_cmd_fmt_param(param.cmd, param.fmt_params_list)
    fmt_param_dict = {i[0]: i[1] for i in param.fmt_params_list}
    fmt_cmd = param.cmd.format(**fmt_param_dict)
    return fmt_cmd


def _check_cmd_fmt_param(cmd, params_list):
    """校验命令格式化参数"""
    for key, val, valid_name in params_list:
        brace_key = "{%s}" % key
        # 检查格式化参数是否存在
        if brace_key not in cmd:
            LOGGER.error("The format param %s not exist in command: %s.", key, cmd)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The format param not exist in command.")
        # 检查校验器是否存在
        if valid_name not in ALL_VALIDATOR_NAMES:
            LOGGER.error("The ParamValidator does not contain validator: %s.", valid_name)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="Unsupported command param validator.")
        # 参数校验失败
        validator = NAME_VALIDATOR_MAP.get(valid_name)
        if not callable(validator) or not validator(val):
            LOGGER.error("The command param is invalid, key: %s, value: %s, validator name: %s, func: %s.",
                         key, val, valid_name, validator)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The command param is invalid.")
