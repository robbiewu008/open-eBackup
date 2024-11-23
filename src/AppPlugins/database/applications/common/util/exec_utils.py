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

__all__ = (
    "check_path_valid", "exec_append_file", "exec_append_newline_file", "exec_cat_cmd", "exec_cp_cmd",
    "exec_cp_cmd_no_user", "exec_ln_cmd", "exec_mkdir_cmd", "exec_mount_cmd", "exec_mv_cmd", "exec_overwrite_file",
    "exec_umount_cmd", "exec_write_file", "exec_write_file_without_x_permission", "exec_write_new_file",
    "exec_write_new_file_without_x_permission", "ExecFuncParam", "get_exe_abs_path", "get_user_env_param_by_key",
    "get_user_home", "su_exec_cat_cmd", "su_exec_cmd_list", "su_exec_rm_cmd", "su_exec_touch_cmd",
    "exec_mount_cmd_with_aix", "exec_cp_dir_no_user", "read_lines_cmd"
)

import json
import os
import platform
import re
import shlex
import stat
from shutil import move, copy
from typing import List
import uuid

import pexpect

from common.common import execute_cmd, check_size
from common.const import EnumPathType, CMDResult, FilePath
from common.env_common import get_install_head_path
from common.err_code import CommErrCode
from common.exception.common_exception import ErrCodeException
from common.file_common import check_file_or_dir, get_user_info, exec_lchown
from common.number_const import NumberConst
from common.security.anonym_utils.anonymity import Anonymity
from common.util.check_user_utils import check_path_owner, check_os_user
from common.util.check_utils import check_param_chars_no_quote, check_dir_path, check_file_path, check_real_path, \
    check_dir_path_without_check_mode
from common.util.param_check_utils import check_common_params
from common.util.validators import ALL_VALIDATOR_NAMES, NAME_VALIDATOR_MAP

if platform.system().lower() == "windows":
    from common.logger_wins import Logger
else:
    from common.logger import Logger

# Linux系统默认命令提示符
DEFAULT_PROMPT = '[#\$>]'
LINUX_CMD_WHITE_LIST = (
    "cd",
    "cp",
    "echo",
    "ll",
    "ls",
    f"{get_install_head_path()}/DataBackup/ProtectClient/ProtectClient-E/bin/agentcli"
)
LINUX_PROFILE_WHITE_LIST = ("/etc/profile",)
LINUX_SHELL_WHITE_LIST = ("/bin/bash", "/bin/sh")
LOGGER = Logger().get_logger()
# Linux系统临时命令提示符
TEMP_PROMPT = r"\[PEXPECT\][\$\#]"
SIZE_1_G = 1073741824


class ExecFuncParam:
    def __init__(self, os_user: str, cmd_list: List[str], fmt_params_list: List[List], timeout: int = None,
                 env_file_list: List[str] = None, shell_file: str = None, chk_exe_owner: bool = True,
                 treat_as_error_echo_list: List[str] = None, need_input_password_echo_list: List[str] = None,
                 password: str = None):
        """
        :param os_user: 操作系统用户名
        :param cmd_list: 命令列表，例：["python3 {zctl_file} -t {t_p}", "ls -l"]
        :param fmt_params_list: 命令格式化参数列表的列表，例：
        [
            [("zctl_file", "/opt/gs/app/bin/zctl.py", ValidatorEnum.PATH_CHK_FILE),
             ("t_p", "status", ValidatorEnum.CHAR_CHK_EXCLUDE_QUOTE)],
            []
        ]
        :param timeout: 命令超时时间，单位：秒
        :param env_file_list: 环境变量文件列表
        :param shell_file: shell文件
        :param chk_exe_owner: 是否校验可执行文件的用户
        :param treat_as_error_echo_list: 视为错误的回显，出现就视为命令执行失败
        :param need_input_password_echo_list: 需要输入密码场景的回显
        :param password: 需要输入密码场景下的键入密码
        """
        self.os_user = os_user
        self.cmd_list = cmd_list
        self.fmt_params_list = fmt_params_list
        self.timeout = timeout
        self.env_file_list = env_file_list or list()
        self.shell_file = shell_file
        self.chk_exe_owner = chk_exe_owner
        self.treat_as_error_echo_list = treat_as_error_echo_list
        self.need_input_password_echo_list = need_input_password_echo_list
        self.password = password


class WriteFileParam:
    def __init__(self, file_path, data, json_flag: bool = True, flags=os.O_WRONLY | os.O_CREAT | os.O_EXCL,
                 modes=stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR, write_mode="w", check_whitelist_flag: bool = True):
        """
        :param file_path: 目标路径文件
        :param data: 数据
        :param flags: flags
        :param modes: modes
        :param json_flag: 是否json格式化
        :param write_mode: write_mode
        :param check_whitelist_flag: 是否检查路径白名单
        """
        self.file_path = file_path
        self.data = data
        self.json_flag = json_flag
        self.flags = flags
        self.modes = modes
        self.write_mode = write_mode
        self.check_whitelist_flag = check_whitelist_flag


def _check_cmd_fmt_param(cmd_list, params_list):
    """校验命令格式化参数"""
    if len(cmd_list) != len(params_list):
        LOGGER.error("The command list length %s not equal with param list length %s.", len(cmd_list), len(params_list))
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The command format params number incorrect.")
    for idx, params in enumerate(params_list):
        cmd = cmd_list[idx]
        for key, val, valid_name in params:
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


def _check_exe_owner(os_user, cmd_list, shell_file: str = None):
    """校验命令可执行文件所属用户"""
    for cmd in cmd_list:
        exe = shlex.split(cmd)[0]
        if exe in LINUX_CMD_WHITE_LIST:
            continue
        if os.path.isfile(exe):
            check_common_params(chk_path_owners=[(exe, [os_user])])
            continue
        if not check_param_chars_no_quote(exe):
            LOGGER.error("The executable file(%s) is invalid.", exe)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The executable file is invalid.")
        exe_abs_path = get_exe_abs_path(os_user, exe, shell_file=shell_file)
        LOGGER.debug("Get the absolute path(%s) of the executable file(%s) success.", exe_abs_path, exe)
        check_common_params(chk_path_owners=[(exe_abs_path, [os_user])])


def _check_su_exec_cmd_list_params(param: ExecFuncParam):
    """检查执行命令列表函数的参数"""
    # 检查用户、环境变量文件
    check_common_params(
        chk_users=[param.os_user],
        chk_path_owners=[(_replace_user_home(param.os_user, i), [param.os_user]) for i in param.env_file_list
                         if i not in LINUX_PROFILE_WHITE_LIST]
    )
    if param.timeout is not None and not str(param.timeout).isdigit():
        LOGGER.error("The timeout(%s) parameter is invalid.", param.timeout)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The timeout parameter is invalid.")
    if param.shell_file is not None and param.shell_file not in LINUX_SHELL_WHITE_LIST:
        LOGGER.error("The shell file(%s) is invalid.", param.shell_file)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The shell file is invalid.")
    _check_cmd_fmt_param(param.cmd_list, param.fmt_params_list)
    # 不检查可执行命令所属用户
    if not param.chk_exe_owner:
        return
    _check_exe_owner(param.os_user, param.cmd_list, shell_file=param.shell_file)


def _temp_change_prompt(ssh):
    """临时修改命令提示符"""
    ssh.sendline("unset PROMPT_COMMAND")
    change_ps_cmd = r"PS1='[PEXPECT]\$ '"
    ssh.sendline(change_ps_cmd)
    index = ssh.expect([TEMP_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.TEN)
    if index != 0:
        out = ssh.before
        LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", change_ps_cmd, index, out)
        raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[change_ps_cmd, out])


def _remove_ansi_escape_code(text):
    """移除ANSI转义码"""
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    return ansi_escape.sub('', text)


def _handle_output(out, cmd):
    """处理输出信息"""
    out_lines = _remove_ansi_escape_code(str(out)).split(os.linesep)
    if out_lines:
        # 裁剪命令行
        if cmd is not None and str(cmd).strip() and cmd in out_lines[0]:
            out_lines = out_lines[1:]
        # 裁剪多余的最后一行
        if out_lines and not str(out_lines[-1]).strip():
            out_lines = out_lines[:-1]
    return os.linesep.join(out_lines)


def _handle_cat_output_(out):
    """处理输出信息"""
    out_lines = _remove_ansi_escape_code(str(out)).split(os.linesep)
    if out_lines:
        # 裁剪命令行
        out_lines = out_lines[1:]
        # 裁剪多余的最后一行
        if out_lines and not str(out_lines[-1]).strip():
            out_lines = out_lines[:-1]
    return os.linesep.join(out_lines)


def _replace_user_home(os_user, input_file):
    """替换路径文件中的用户根路径"""
    if input_file.startswith("~/"):
        user_home = get_user_home(os_user)
        input_file = str(input_file).replace("~", user_home, 1)
    return input_file


def _init_tmp_file(des_path, data, append_str=""):
    """
    初始化临时文件
    :param des_path: 文件路径
    :param data: 内容
    :param append_str: 追加字符
    :return: bool
    """
    file_extension = os.path.splitext(des_path)
    prefix = file_extension[0]
    suffix = file_extension[1]
    tmp = f"{prefix}{str(uuid.uuid4())}{suffix}"
    if not su_exec_touch_cmd(tmp, "root"):
        return str()
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(tmp, flags, modes), 'w+') as tmp_file:
        tmp_file.write(data)
        if append_str == "\n":
            tmp_file.write(append_str)
    return tmp


def _check_read_file(input_file):
    if not check_file_path(input_file):
        LOGGER.error("The input_file param(%s) is invalid or not exist.", input_file)
        return False

    if not check_size(input_file, SIZE_1_G):
        LOGGER.error("The input_file size is too large.")
        return False

    real_path = os.path.realpath(input_file)
    if re.match(FilePath.READ_BLACK_LIST, real_path):
        LOGGER.error(f"Path: {real_path} is in the black list.")
        return False
    return True


def _write_to_file(write_file_param: WriteFileParam):
    """
    向目标文件写数据，
    :param write_file_param: 写文件的参数
    :return: bool
    """
    data = write_file_param.data
    file_path = write_file_param.file_path
    json_flag = write_file_param.json_flag
    flags = write_file_param.flags
    modes = write_file_param.modes
    write_mode = write_file_param.write_mode

    if json_flag:
        input_data = json.dumps(data)
    else:
        input_data = data

    if os.path.exists(file_path):
        path_type = check_file_or_dir(file_path)
        if path_type != EnumPathType.FILE_TYPE:
            LOGGER.error(f"Check des path not file type: {path_type}")
            return False

    try:
        with os.fdopen(os.open(file_path, flags, modes), write_mode) as f:
            f.write(input_data)
        return True
    except Exception as e:
        LOGGER.error(f"write file failed, exception: {e}")
        return False


def get_user_home(os_user):
    """获取用户根路径"""
    su_cmd = f"su - {os_user}"
    ssh = pexpect.spawn(su_cmd, encoding='utf-8')
    try:
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", su_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[su_cmd, out])
        user_root_cmd = "echo ~"
        ssh.sendline(user_root_cmd)
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", user_root_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[user_root_cmd, out])
        before_out = ssh.before
        out_lines = str(before_out).split(os.linesep)
        if len(out_lines) < 2:
            LOGGER.error("Execute command(%s) failed, index: %s, out lines: %s.", user_root_cmd, index, out_lines)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[user_root_cmd, before_out])
        user_root_path = out_lines[1].strip()
        LOGGER.debug("Get root path(%s) of user(%s) success.", user_root_path, os_user)
        return user_root_path
    finally:
        if ssh:
            ssh.close()


def get_exe_abs_path(os_user: str, exe: str, shell_file: str = None):
    """获取可执行文件的绝对路径"""
    su_cmd = f"su - {os_user}" if shell_file is None else f"su - {os_user} -s {shell_file}"
    ssh = pexpect.spawn(su_cmd, encoding='utf-8')
    try:
        # 切换用户可能的命令提示符
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", su_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[su_cmd, out])
        # 临时修改命令提示符
        which_cmd = f"which {exe}"
        ssh.sendline(which_cmd)
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", which_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[which_cmd, out])
        before_out = ssh.before
        out_lines = str(before_out).split(os.linesep)
        if len(out_lines) < 2:
            LOGGER.error("Execute command(%s) failed, index: %s, out lines: %s.", which_cmd, index, out_lines)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[which_cmd, before_out])
        exe_abs_path = out_lines[1].strip()
    finally:
        if ssh:
            ssh.close()
    exe_abs_path = _replace_user_home(os_user, str(exe_abs_path))
    if not os.path.isfile(exe_abs_path):
        LOGGER.error("The executable file(%s) obtained is invalid.", exe_abs_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The executable file obtained is invalid.")
    return exe_abs_path


def handle_extended_scenarios(ssh, param, expect_list):
    if param.treat_as_error_echo_list:
        expect_list = expect_list[:1] + param.treat_as_error_echo_list + expect_list[1:]
    if param.need_input_password_echo_list:
        index = ssh.expect(expect_list + param.need_input_password_echo_list,
                           timeout=param.timeout or NumberConst.THIRTY)
        if index in range(len(expect_list), len(expect_list) + len(param.need_input_password_echo_list)):
            if param.password:
                ssh.sendline(param.password)
            else:
                ssh.sendline()
            index = ssh.expect(expect_list, timeout=param.timeout or NumberConst.THIRTY)
    else:
        index = ssh.expect(expect_list, timeout=param.timeout or NumberConst.THIRTY)
    return index


def su_exec_cmd_list(param: ExecFuncParam):
    """
    切换用户交互式执行命令
    :param param: 执行命令函数参数实例
    :return: (exitstatus, output)，例("0", "output")
    """
    _check_su_exec_cmd_list_params(param)
    su_cmd = f"su - {param.os_user}" if param.shell_file is None else f"su - {param.os_user} -s {param.shell_file}"
    ssh = pexpect.spawn(su_cmd, encoding='utf-8')
    try:
        # 切换用户可能的命令提示符
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", su_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[su_cmd, out])
        _temp_change_prompt(ssh)
        expect_list = [TEMP_PROMPT, pexpect.EOF, pexpect.TIMEOUT]
        # 存在环境变量，先导环境变量
        if param.env_file_list:
            source_cmd = " && ".join([f"source {i}" for i in param.env_file_list])
            ssh.sendline(source_cmd)
            index = ssh.expect(expect_list, timeout=NumberConst.THIRTY)
            if index != 0:
                out = ssh.before
                LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", source_cmd, index, out)
                raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[source_cmd, out])
        fmt_cmd_list = list()
        for idx, cmd in enumerate(param.cmd_list):
            params = param.fmt_params_list[idx]
            fmt_param_dict = {i[0]: i[1] for i in params}
            fmt_cmd = cmd.format(**fmt_param_dict)
            fmt_cmd_list.append(fmt_cmd)
        cmd_str = " && ".join(fmt_cmd_list)
        ssh.sendline(cmd_str)
        index = handle_extended_scenarios(ssh, param, expect_list)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", Anonymity.process(cmd_str), index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[cmd_str, out])
        last_before = _handle_output(ssh.before, cmd_str)
        ssh.close()
        return str(ssh.exitstatus), last_before
    finally:
        if ssh:
            ssh.close()


def su_exec_cat_cmd(input_file: str, os_user: str):
    """
    执行cat命令
    :param input_file: 输入文件绝对路径
    :param os_user: 操作系统用户名
    :return: (exitstatus, output)，例("0", "output")
    """
    check_common_params(chk_users=[os_user])
    input_file = _replace_user_home(os_user, input_file)
    if not check_file_path(input_file):
        LOGGER.error("The input_file param(%s) is invalid.", input_file)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The input_file is invalid.")
    check_common_params(chk_path_owners=[(input_file, [os_user])])
    file_size = 1073741824
    if not check_size(input_file, file_size):
        LOGGER.error("The input_file size is too large.")
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The input_file size is too large.")
    su_cmd = f"su - {os_user}"
    ssh = pexpect.spawn(su_cmd, encoding='utf-8')
    try:
        # 切换用户可能的命令提示符
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", su_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[su_cmd, out])
        _temp_change_prompt(ssh)
        cat_cmd = f"cat {input_file}"
        ssh.sendline(cat_cmd)
        index = ssh.expect([TEMP_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", cat_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[cat_cmd, out])
        last_before = _handle_cat_output_(ssh.before)
        ssh.close()
        return str(ssh.exitstatus), last_before
    finally:
        if ssh:
            ssh.close()


def exec_cat_cmd(input_file: str, encoding: str = None):
    """
    执行cat命令
    :param input_file: 输入文件绝对路径
    :param encoding: 编码格式
    :return: (成功or失败, output)，例("True", "output")
    """
    if not _check_read_file(input_file):
        LOGGER.error(f"Path: {input_file} is invalid.")
        return False, ""

    try:
        with open(input_file, "r", encoding=encoding) as f:
            result = f.read()
        return True, result
    except Exception:
        return False, ""


def read_lines_cmd(input_file: str, encoding: str = None):
    """
    按行读取文件，删除末尾的空白符，包括空格、换行符、回车符、制表符
    :param input_file: 输入文件绝对路径
    :param encoding: 编码格式
    :return: (成功or失败, output)，例("True", "output")
    """
    if not _check_read_file(input_file):
        LOGGER.error(f"Path: {input_file} is invalid.")
        return False, []
    result = []
    try:
        with open(input_file, "r", encoding=encoding) as f:
            lines = f.readlines()
            for line_str in lines:
                result.append(line_str.rstrip())
        return True, result
    except Exception:
        return False, []


def get_user_env_param_by_key(os_user, env_key):
    """
    获取环境参数
    :param os_user: 操作系统用户名
    :param env_key: 环境变量key
    :return: 环境变量key的值
    """
    val = None
    su_cmd = f"su - {os_user}"
    ssh = pexpect.spawn(su_cmd, encoding='utf-8')
    try:
        # 切换用户可能的命令提示符
        index = ssh.expect([DEFAULT_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", su_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[su_cmd, out])
        _temp_change_prompt(ssh)
        env_cmd = "env"
        ssh.sendline(env_cmd)
        index = ssh.expect([TEMP_PROMPT, pexpect.EOF, pexpect.TIMEOUT], timeout=NumberConst.THIRTY)
        if index != 0:
            out = ssh.before
            LOGGER.error("Execute command(%s) failed, index: %s, out: %s.", env_cmd, index, out)
            raise ErrCodeException(CommErrCode.FAILED_EXECUTE_COMMAND, *[env_cmd, out])
        before_out = ssh.before
        out_lines = str(before_out).split(os.linesep)
        for i in out_lines:
            tmp_splits = i.split("=", 1)
            if len(tmp_splits) == 2 and str(tmp_splits[0]).strip() == env_key:
                val = str(tmp_splits[1]).strip()
                LOGGER.info("Get env key(%s) success, value: %s.", env_key, val)
                break
        return val
    finally:
        if ssh:
            ssh.close()


def exec_mount_cmd(src_path: str, dst_path: str):
    """
    执行mount命令，当前改指令不存在切换用户场景
    :param src_path: 被挂载路径
    :param dst_path: 目标路径
    :return: ret, std_out, std_err
    """
    if not check_dir_path(src_path):
        LOGGER.error("The mount src_path param is invalid: %s.", src_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The mount src_path param is invalid.")
    if not check_dir_path(dst_path):
        LOGGER.error("The mount dst_path param is invalid: %s.", dst_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The mount dst_path param is invalid.")
    cmd = f"mount --bind {src_path} {dst_path}"
    return execute_cmd(cmd)


def exec_mount_cmd_with_aix(src_path: str, dst_path: str):
    """
    执行mount命令，当前改指令不存在切换用户场景
    :param src_path: 被挂载路径
    :param dst_path: 目标路径
    :return: ret, std_out, std_err
    """
    if not check_dir_path(src_path):
        LOGGER.error("The mount src_path param is invalid: %s.", src_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The mount src_path param is invalid.")
    if not check_dir_path(dst_path):
        LOGGER.error("The mount dst_path param is invalid: %s.", dst_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The mount dst_path param is invalid.")
    cmd = f"mount {src_path} {dst_path}"
    return execute_cmd(cmd)


def exec_umount_cmd(mount_path: str, option: str = None):
    """
    执行umount 命令，当前改指令不存在切换用户场景
    :param option: 解挂参数，比如强制卸载-l，-f
    :param mount_path: 挂载路径
    :return: ret, std_out, std_err
    """
    if not check_dir_path_without_check_mode(mount_path):
        LOGGER.error("The umount path param is invalid: %s.", mount_path)
        raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The umount path param is invalid.")
    if not option:
        cmd = f"umount {mount_path}"
    else:
        cmd = f"umount {option} {mount_path}"
    return execute_cmd(cmd)


def check_path_valid(check_path, is_check_white_list: bool = True):
    """
    对指定路径进行校验
    1、如果有特殊字符直接返回失败
    2、如果在路径在黑名单里并且不在黑名单开放列表中返回失败
    3、如果路径没有在白名单里返回失败
    """
    expression = "[|;&$><`'\"!+\n]"
    if re.search(expression, check_path):
        LOGGER.error(f"Path: {check_path} contains special characters.")
        return False
    if not check_real_path(check_path):
        LOGGER.error(f"Path: {check_path} is not real Path")
        return False
    if is_check_white_list:
        if re.match(FilePath.PATH_BLACK_LIST, check_path):
            LOGGER.error(f"Path: {check_path} is in the black list.")
            return False
        if re.match(FilePath.PATH_WHITE_LIST, check_path):
            return True
        LOGGER.error(f"Path: {check_path} is not in the white list.")
        return False
    return True


def exec_mkdir_cmd(dir_path: str, os_user: str = None, mode=None, is_check_white_list: bool = True):
    """
    执行mkdir 命令
    :param dir_path: 目录路径
    :param os_user: 用户名
    :param mode: chmod 模式
    :param is_check_white_list: 是否做白名单检查
    :return: bool
    """
    if not check_path_valid(dir_path, is_check_white_list):
        LOGGER.error("The mkdir path param is invalid: %s.", dir_path)
        return False
    if not os_user:
        LOGGER.info("Use default user mkdir dir")
        try:
            if mode:
                os.makedirs(dir_path, mode)
            else:
                os.makedirs(dir_path)
        except Exception as e:
            LOGGER.error(f"Default mkdir failed, exception: {e}")
            return False
        return True
    if not check_os_user(os_user):
        LOGGER.error("The os user(%s) is invalid.", os_user)
        return False
    cmd = f"su - {os_user} -c 'mkdir -p {dir_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Use user mkdir failed dir path: {dir_path}, error: {std_err}")
        return False
    if mode:
        try:
            os.chmod(dir_path, mode)
        except Exception as e:
            LOGGER.error(f"Change dest path permission failed, exception: {e}")
            return False
    return True


def exec_cp_cmd(src_path, dest_path, user_name=None, option='-rp', is_check_white_list: bool = True):
    """
    复制文件到指定路径 默认指定路径存在文件会覆盖
    :param src_path: 源文件路径 文件夹或着文件均可以
    :param dest_path: 目标路径 文件夹或这文件均可以
    :param user_name: 用户名
    :param option: 复制参数选项
    :param is_check_white_list: 是否校验黑白名单
    :return: bool
    """
    if not check_path_valid(src_path, is_check_white_list) or not check_path_valid(dest_path, is_check_white_list):
        return False
    path_type = check_file_or_dir(src_path)
    if path_type not in (EnumPathType.DIR_TYPE, EnumPathType.FILE_TYPE):
        LOGGER.error(f"Src path is invalid type: {path_type} can not copy ")
        return False
    if not user_name:
        LOGGER.info("Use default user copy file")
        if not option:
            cmd = f"cp {src_path} {dest_path}"
        else:
            cmd = f"cp {option} {src_path} {dest_path}"
        return_code, std_out, std_err = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            LOGGER.error(f"Use user copy file failed src path: {src_path} des path: {dest_path}, error: {std_err}")
            return False
        return True
    if not check_path_owner(src_path, [user_name]):
        LOGGER.error("The owner of src_path: %s is not %s.", src_path, user_name)
        return False
    if os.path.exists(dest_path) and not check_path_owner(dest_path, [user_name]):
        LOGGER.error("The owner of desc_path: %s is not %s.", dest_path, user_name)
        return False
    if not option:
        cmd = f"su - {user_name} -c 'cp {src_path} {dest_path}'"
    else:
        cmd = f"su - {user_name} -c 'cp {option} {src_path} {dest_path}'"
    return_code, std_out, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Use user copy file failed src path: {src_path} des path: {dest_path}, error: {std_err}")
        return False
    return True


def exec_cp_dir_no_user(src_path, dest_path, option='-rp', is_check_white_list: bool = True,
                        is_overwrite: bool = False):
    """
    复制文件到指定路径 默认指定路径存在文件会覆盖
    :param src_path: 源文件路径 文件夹或着文件均可以
    :param dest_path: 目标路径 文件夹或这文件均可以
    :param option: 复制参数选项
    :param is_check_white_list: 是否校验黑白名单
    :param is_overwrite: 是否进行覆盖复制
    :return: bool
    """
    if not check_path_valid(src_path, is_check_white_list) or not check_path_valid(dest_path, is_check_white_list):
        return False
    path_type = check_file_or_dir(src_path)
    if path_type not in (EnumPathType.DIR_TYPE, EnumPathType.FILE_TYPE):
        LOGGER.error(f"Src path is invalid type: {path_type} can not copy ")
        return False

    LOGGER.info("Use default user copy file")
    if is_overwrite:
        cp_file = "/bin/cp"
    else:
        cp_file = "cp"

    if not option:
        cmd = f"{cp_file} {src_path} {dest_path}"
    else:
        cmd = f"{cp_file} {option} {src_path} {dest_path}"
    return_code, std_out, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Use user copy file failed src path: {src_path} des path: {dest_path}, error: {std_err}")
        return False
    return True


def exec_cp_cmd_no_user(src_path, dest_path):
    """
    复制文件到指定路径 默认指定路径存在文件会覆盖
    :param src_path: 源文件路径 文件夹或着文件均可以
    :param dest_path: 目标路径 文件夹或这文件均可以
    :return: bool
    """
    if not check_path_valid(src_path) or not check_path_valid(dest_path):
        return False
    path_type = check_file_or_dir(src_path)
    if path_type not in (EnumPathType.DIR_TYPE, EnumPathType.FILE_TYPE):
        LOGGER.error(f"Src path is invalid type: {path_type} can not copy ")
        return False
    LOGGER.info("Use default user copy file")
    try:
        copy(src_path, dest_path)
    except Exception as e:
        LOGGER.error(f"Default Copy file to des path failed, exception: {e}")
        return False
    return True


def exec_ln_cmd(src_path, dest_path, user_name=None, check_src_whitelist=True, check_dest_whitelist=True):
    """
    创建符号链接
    :param src_path: 源文件路径 文件夹或着文件均可以
    :param dest_path: 目标路径 文件夹或这文件均可以
    :param user_name: 用户名
    :param check_src_whitelist: 是否检查src_path是否在黑白名单中
    :param check_dest_whitelist: 是否检查dest_path是否在黑白名单中
    :return: bool
    """
    if not check_path_valid(src_path, check_src_whitelist) or not check_path_valid(dest_path, check_dest_whitelist):
        return False
    if not user_name:
        LOGGER.info("Use default user copy file")
        try:
            os.symlink(src_path, dest_path)
        except Exception as e:
            LOGGER.error(f"Default link file to des path failed, exception: {e}")
            return False
        return True
    if not check_path_owner(src_path, [user_name]):
        LOGGER.error("The owner of src_path: %s is not %s.", src_path, user_name)
        return False
    cmd = f"su - {user_name} -c 'ln -s {src_path} {dest_path}'"
    return_code, std_out, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Use user link file failed src path: {src_path} des path: {dest_path}, error: {std_err}")
        return False
    return True


def su_exec_rm_cmd(dest_path, user_name=None, check_white_black_list_flag: bool = True):
    """
    删除指定用户的文件或者文件夹
    :param dest_path: 指定路径
    :param user_name: 用户名
    :param check_white_black_list_flag: 是否检查黑白名单
    :return: bool
    """
    if not check_path_valid(dest_path, check_white_black_list_flag):
        return False

    if not os.path.exists(dest_path):
        LOGGER.info(f"The file[{dest_path}] want to delete is not exist.")
        return True

    path_type = check_file_or_dir(dest_path)
    if path_type in (EnumPathType.INVALID_TYPE, EnumPathType.LINK_TYPE):
        LOGGER.error(f"Des path not file or dir type: {path_type}, delete failed")
        return False
    if not user_name:
        LOGGER.info("Use default user remove file")
        cmd = f"rm -rf {dest_path}"
        return_code, _, std_err = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            LOGGER.error(f"Fail to delete file or dir, err: {std_err}")
            return False
        return True
    if not check_path_owner(dest_path, [user_name]):
        LOGGER.error("The owner of src_path: %s is not %s.", dest_path, user_name)
        return False
    cmd = f"su - {user_name} -c 'rm -rf {dest_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Fail to delete file or dir, err: {std_err}")
        return False
    return True


def exec_mv_cmd(src_path, dest_path, user_name=None, check_white_black_list_flag: bool = True):
    """
    移动/重命令文件到指定路径 默认指定路径存在文件会覆盖
    :param src_path: 源文件路径 文件夹或着文件均可以
    :param dest_path: 目标路径 文件夹
    :param user_name: 用户名
    :param check_white_black_list_flag: 路径是否执行黑白名单校验
    :return: bool
    """
    if not check_path_valid(src_path, check_white_black_list_flag) or not check_path_valid(dest_path,
                                                                                           check_white_black_list_flag):
        return False
    path_type = check_file_or_dir(src_path)
    if path_type in (EnumPathType.INVALID_TYPE, EnumPathType.LINK_TYPE):
        LOGGER.error(f"Source path is invalid type: {path_type} can not move ")
        return False
    if not user_name:
        LOGGER.info("Use default user move file")
        try:
            move(src_path, dest_path)
        except Exception as e:
            LOGGER.error(f"Default move file to des path failed, exception: {e}")
            return False
        return True
    if not check_path_owner(src_path, [user_name]):
        LOGGER.error("The owner of src_path: %s is not %s.", src_path, user_name)
        return False
    if os.path.exists(dest_path) and not check_path_owner(dest_path, [user_name]):
        LOGGER.error("The owner of desc_path: %s is not %s.", dest_path, user_name)
        return False
    cmd = f"su - {user_name} -c 'mv {src_path} {dest_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Use user move file failed src path: {src_path} des path: {dest_path}, error: {std_err}")
        return False
    return True


def exec_append_file(file_path, data, user_name=None, json_flag=False):
    """
    向目标文件追加写入数据
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :param user_name: 用户名
    :return: bool
    """
    if json_flag:
        input_data = json.dumps(data)
    else:
        input_data = data

    if os.path.exists(file_path):
        path_type = check_file_or_dir(file_path)
        if path_type != EnumPathType.FILE_TYPE:
            LOGGER.error(f"Check des path not file type: {path_type}")
            return False
    if not user_name:
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'a+') as out_file:
            out_file.write(input_data)
        return True
    if not check_path_owner(file_path, [user_name]):
        LOGGER.error("The owner of file_path: %s is not %s.", file_path, user_name)
        return False
    tmp = _init_tmp_file(file_path, input_data)
    if tmp == str():
        return False
    cmd = f"su - {user_name} -c 'cat {tmp} >> {file_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if os.path.isfile(tmp):
        LOGGER.info("Remove temp copy info file: %s.", tmp)
        os.remove(tmp)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Append to file failed, des_path: {file_path}")
        return False
    return True


def exec_append_newline_file(file_path, data, user_name=None, json_flag=False):
    """
    向目标文件追加写入数据后换行
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :param user_name: 用户名
    :return: bool
    """
    if json_flag:
        input_data = json.dumps(data)
    else:
        input_data = data
    if os.path.exists(file_path):
        path_type = check_file_or_dir(file_path)
        if path_type != EnumPathType.FILE_TYPE:
            LOGGER.error(f"Check des path not file type: {path_type}")
            return False
    if not user_name:
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'a+') as out_file:
            out_file.write(input_data)
            out_file.write('\n')
        return True
    if not check_path_owner(file_path, [user_name]):
        LOGGER.error("The owner of file_path: %s is not %s.", file_path, user_name)
        return False
    tmp = _init_tmp_file(file_path, input_data, "\n")
    if tmp == str():
        return False
    cmd = f"su - {user_name} -c 'cat {tmp} >> {file_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if os.path.isfile(tmp):
        LOGGER.info("Remove temp copy info file: %s.", tmp)
        os.remove(tmp)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Append line break to file failed, des_path: {file_path}")
        return False
    return True


def exec_write_new_file_without_x_permission(file_path, data, json_flag=True):
    """
    向目标文件写数据，
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :return: bool
    """
    return _write_to_file(WriteFileParam(file_path, data, json_flag,
                                         flags=os.O_WRONLY | os.O_CREAT | os.O_EXCL,
                                         modes=stat.S_IWUSR | stat.S_IRUSR,
                                         write_mode="w"))


def exec_write_file_without_x_permission(file_path, data, json_flag=True):
    """
    向目标文件写数据，
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :return: bool
    """
    return _write_to_file(WriteFileParam(file_path, data, json_flag,
                                         flags=os.O_WRONLY | os.O_CREAT,
                                         modes=stat.S_IWUSR | stat.S_IRUSR,
                                         write_mode="w"))


def exec_write_new_file(file_path, data, json_flag=True):
    """
    向目标文件写数据，
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :return: bool
    """
    return _write_to_file(WriteFileParam(file_path, data, json_flag,
                                         flags=os.O_WRONLY | os.O_CREAT | os.O_EXCL,
                                         modes=stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR,
                                         write_mode="w"))


def exec_write_file(file_path, data, json_flag=True):
    """
    向目标文件写数据，
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :return: bool
    """
    return _write_to_file(WriteFileParam(file_path, data, json_flag,
                                         flags=os.O_WRONLY | os.O_CREAT,
                                         modes=stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR,
                                         write_mode="w"))


def exec_overwrite_file(file_path, data, user_name=None, json_flag=True):
    """
    向目标文件覆盖写入数据
    :param file_path: 目标路径文件
    :param data: 数据
    :param json_flag: 是否json格式化
    :param user_name: 用户名
    :return: bool
    """
    if json_flag:
        input_data = json.dumps(data)
    else:
        input_data = data
    if os.path.exists(file_path):
        path_type = check_file_or_dir(file_path)
        if path_type != EnumPathType.FILE_TYPE:
            LOGGER.error(f"Check des path not file type: {path_type}")
            return False
    if not user_name:
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(file_path, flags, modes), 'w') as f:
            f.truncate(0)
            f.write(input_data)
        return True
    if not check_path_owner(file_path, [user_name]):
        LOGGER.error("The owner of file_path: %s is not %s.", file_path, user_name)
        return False
    tmp = _init_tmp_file(file_path, input_data)
    if tmp == str():
        return False
    cmd = f"su - {user_name} -c 'cat {tmp} > {file_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if os.path.isfile(tmp):
        LOGGER.info("Remove temp copy info file: %s.", tmp)
        os.remove(tmp)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Overwrite to file failed, des_path: {file_path}")
        return False
    return True


def su_exec_touch_cmd(file_path, user_name, mode=None):
    """
    指定用户创建文件
    :param file_path: 文件路径
    :param user_name: 用户名
    :param mode: chmod 模式
    :return: bool
    """
    if not check_os_user(user_name):
        LOGGER.error("The touch user_name(%s) is invalid.", user_name)
        return False
    if not check_path_valid(file_path):
        LOGGER.error("The touch file_path is invalid: %s.", file_path)
        return False
    cmd = f"su - {user_name} -c 'touch {file_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        LOGGER.error(f"Touch file failed, file path: {file_path}")
        return False
    if mode:
        try:
            os.chmod(file_path, mode)
        except Exception as e:
            LOGGER.error(f"Change dest path permission failed, exception: {e}")
            return False
    return True
