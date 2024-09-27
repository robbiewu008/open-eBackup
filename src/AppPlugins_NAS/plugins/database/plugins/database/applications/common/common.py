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

import functools
import json
import os
import pathlib
import shlex
import shutil
import signal
import socket
import stat
import subprocess
import time
import re
import platform
import locale
from functools import wraps
from typing import List

import psutil

from common.const import ParamConstant, RpcParamKey, CMDResult, PathConstant, RepositoryDataTypeEnum
from common.schemas.thrift_base_data_type import Copy


def retry_when_exception(exceptions=Exception, retry_times=3, delay=1, logger=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            _times = retry_times
            while _times:
                try:
                    return func(*args, **kwargs)
                except exceptions as exception:
                    _times -= 1
                    if not _times:
                        raise exceptions(f'Retry failed. {str(exception)}') from exception
                    if logger is not None:
                        logger.error(f'retrying in {delay} seconds...')
                    time.sleep(delay)

        return wrapper

    return decorator


@retry_when_exception(retry_times=3, delay=3)
def read_result_file(file_name, encoding='UTF-8'):
    """
    从结果文件读取数据
    :param file_name: 结果文件路径
    :param encoding: 编码格式
    :return: 读取结果
    """
    with open(file_name, "r", encoding=encoding) as file_content:
        result = file_content.read()
        return result


def read_tmp_json_file(file_path):
    """
    读取临时json文件
    :param file_path: 文件路径
    :return: 文件内容
    """
    if not os.path.isfile(file_path):
        return {}
    with open(file_path, "r", encoding='UTF-8') as file_content:
        return json.load(file_content)


def execute_cmd(cmd, encoding=None, timeout=None, cmd_array_flag=False):
    """
    执行不包含管道符的命令
    :param cmd: 命令
    :param encoding: 使用自定义编码
    :param timeout: 执行命令超时
    :param cmd_array_flag:命令是否直接为数组格式
    :return: (字符串格式, 标准输出, 标准错误)
    """
    if platform.system().lower() == "windows" or not encoding:
        encoding = locale.getdefaultlocale()[1]
    if not cmd_array_flag:
        cmd = shlex.split(cmd)
    process = subprocess.Popen(cmd, stdin=None, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, encoding=encoding, errors='ignore')
    try:
        std_out, std_err = process.communicate(timeout=timeout)
    except TimeoutError as err:
        process.kill()
        ret_code = 1
        std_out = ""
        std_err = str(err)
    else:
        ret_code = process.returncode
    return str(ret_code), std_out, std_err


def execute_cmd_with_input(cmd, encoding=None, timeout=None, cmd_array_flag=False, is_force_utf8=False):
    """
    交互式执行不包含管道符的命令
    :param cmd: 命令
    :param encoding: 使用自定义编码
    :param timeout: 执行命令超时
    :param cmd_array_flag:命令是否直接为数组格式
    :param is_force_utf8: 是否启用utf8
    :return: (字符串格式, 标准输出, 标准错误)
    """
    if platform.system().lower() == "windows" or not encoding:
        encoding = locale.getdefaultlocale()[1]
    if not cmd_array_flag:
        cmd = shlex.split(cmd)
    # 取cmd list第一个元素，启动powershell
    new_powershell_cmds = [cmd[0], "-Command", "-"]
    process = subprocess.Popen(new_powershell_cmds, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, encoding=encoding, errors='ignore')
    try:
        if is_force_utf8:
            # 65001代表utf-8
            process.stdin.write("chcp 65001")
            process.stdin.write("\n")
        # 输入执行的命令
        process.stdin.write(cmd[-1])
        # 换行
        process.stdin.write("\n")
        std_out, std_err = process.communicate(timeout=timeout)
    except TimeoutError as err:
        process.kill()
        ret_code = 1
        std_out = ""
        std_err = str(err)
    else:
        ret_code = process.returncode
    return str(ret_code), std_out, std_err


def exec_rpc_tool_cmd(api_name, in_path, out_path):
    rpc_tool_path = os.path.realpath(os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh'))
    rpc_tool_cmd = f"{rpc_tool_path} {api_name} {in_path} {out_path}"
    return_code, out, err = execute_cmd(rpc_tool_cmd)
    if return_code != CMDResult.SUCCESS.value:
        return False
    return True


def execute_cmd_list(cmd_list, locale_encoding=False, timeout=None):
    """
    执行命令列表，前一条命令的输出是后一条命令的输入
    :param cmd_list: 命令列表
    :param locale_encoding: 是否使用本地编码
    :param timeout: 执行命令超时
    :return: (字符串格式返回码, 标准输出, 标准错误)
    """
    encoding = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        encoding = locale.getdefaultlocale()[1]
    ret_code = 1
    std_out = None
    std_err = None
    for tmp_cmd in cmd_list:
        process = subprocess.Popen(shlex.split(tmp_cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   encoding=encoding, errors='ignore')
        try:
            std_out, std_err = process.communicate(input=std_out, timeout=timeout)
        except TimeoutError as err:
            process.kill()
            ret_code = 1
            std_out = ""
            std_err = str(err)
            break
        else:
            ret_code = process.returncode
    return str(ret_code), std_out, std_err


def execute_cmd_list_with_communicate(cmd_list, locale_encoding=False):
    """
    执行命令列表，前一条命令的输出是后一条命令的输入
    :param cmd_list: 命令列表
    :param locale_encoding: 是否使用本地编码
    :return: (字符串格式返回码, 标准输出, 标准错误)
    """
    encoding = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        encoding = locale.getdefaultlocale()[1]
    process_list = []
    tmp_stdin = None
    tme_err = None
    for tmp_cmd in cmd_list:
        process = subprocess.Popen(shlex.split(tmp_cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, encoding=encoding)
        output, err = process.communicate(input=tmp_stdin)
        tmp_stdin = output
        tme_err = err
        process_list.append(process)
    return str(process_list[-1].returncode), tmp_stdin, tme_err


def execute_cmd_list_communicate(cmd_list, locale_encoding=False, timeout=None):
    """
    执行命令列表，前一条命令的输出是后一条命令的输入
    :param cmd_list: 命令列表
    :param locale_encoding: 是否使用本地编码
    :param timeout: 执行命令超时
    :return: (字符串格式返回码, 标准输出, 标准错误)
    """
    encoding = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        encoding = locale.getdefaultlocale()[1]
    ret_code = 1
    std_out = None
    std_err = None
    process = None
    for tmp_cmd in cmd_list:
        try:
            process = subprocess.Popen(shlex.split(tmp_cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE,
                                       encoding=encoding, errors='ignore')
            std_out, std_err = process.communicate(input=std_out, timeout=timeout)
        except (OSError, subprocess.CalledProcessError) as err:
            if process is not None:  # 检查process变量是否为None
                process.kill()  # 如果不是，再调用process.kill()方法
            ret_code = 1
            std_out = ""
            std_err = str(err)
            break
        else:
            ret_code = process.returncode

    return str(ret_code), std_out, std_err


def execute_cmd_list_out_to_file(cmd_list, file_ptah, locale_encoding=False, timeout=None):
    """
    执行命令列表，前一条命令的输出是后一条命令的输入
    :param cmd_list: 命令列表
    :param locale_encoding: 是否使用本地编码
    :param timeout: 执行命令超时
    :param file_ptah: 文件路径
    :return: (字符串格式返回码, 标准输出, 标准错误)
    """
    encoding = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        encoding = locale.getdefaultlocale()[1]
    process_list = []
    tmp_stdin = None
    for tmp_cmd in cmd_list[0:-1]:
        process = subprocess.Popen(shlex.split(tmp_cmd), stdin=tmp_stdin, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, encoding=encoding)
        process.wait(timeout=timeout)
        tmp_stdin = process.stdout
        process_list.append(process)
    with open(file_ptah, mode='w', encoding='utf-8') as out_file:
        process = subprocess.Popen(shlex.split(cmd_list[-1]), stdin=tmp_stdin, stdout=out_file,
                                   stderr=subprocess.PIPE, encoding=encoding)
        process.wait(timeout=timeout)
        process_list.append(process)
    return str(process_list[-1].returncode), process_list[-1].stderr.read()


def execute_cmd_with_expect(cmd, expect, time_out, locale_encoding=False):
    tmp_code = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        tmp_code = locale.getdefaultlocale()[1]
    """执行cmd命令"""
    # shell=False参数不支持管道
    process = subprocess.Popen(
        shlex.split(cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, encoding=tmp_code, shell=False, errors='ignore')
    try:
        std_out, std_err = process.communicate(expect + "\n", time_out)
    except TimeoutError as err:
        process.kill()
        ret_code = 1
        std_out = ""
        std_err = str(err)
    else:
        ret_code = process.returncode
    return ret_code, std_out, std_err


def output_result_file(pid, result_info):
    if not pid or not result_info:
        raise Exception("Param pid or result info is none")
    output_file_name = "{}{}".format("result", pid)
    if platform.system().lower() == "windows":
        output_file_path = f"{ParamConstant.WINDOWS_RESULT_PATH}/{output_file_name}"
    else:
        output_file_path = os.path.join(ParamConstant.RESULT_PATH, output_file_name)
    output_execution_result(output_file_path, result_info)
    if not os.path.exists(output_file_path):
        raise Exception(f"Result file: {output_file_path} can not create")


def output_execution_result(file_path, payload):
    """
    目标文件不存在，json方式写入
    :param file_path: 目标文件路径
    :param payload: 写入内容
    """
    json_str = json.dumps(payload)
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as f:
        f.write(json_str)


def write_content_to_file(file_path, content):
    """
    目标文件不存在，原内容写入
    :param file_path: 目标文件路径
    :param payload: 写入内容
    """
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as f:
        f.write(content)


def output_execution_result_ex(file_path, payload):
    """
    目标文件存在，先清空再写入
    :param file_path: 目标文件路径
    :param payload: 写入内容
    """
    json_str = json.dumps(payload)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as f:
        f.truncate(0)
        f.write(json_str)
        f.flush()


def change_dir_permission(dir_path: str, uid: int, gid: int):
    if not os.path.exists(dir_path):
        os.mkdir(dir_path)
    stat_info = os.stat(dir_path)
    if stat_info.st_uid != uid or stat_info.st_gid != gid:
        os.lchown(dir_path, uid, gid)


def check_dir_uid_gid(dir_path: str, uid: int, gid: int):
    stat_info = os.stat(dir_path)
    if stat_info.st_uid != uid or stat_info.st_gid != gid:
        return False
    return True


def check_dir_authority(dir_path: str, authority: int):
    auth = int(oct(os.stat(dir_path).st_mode)[-3:])
    return auth == authority


def convert_time_to_timestamp(time_str):
    time_array = time.strptime(time_str, "%Y-%m-%d %H:%M:%S")
    return int(time.mktime(time_array))


def convert_timestamp_to_time(time_int):
    return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time_int))


def clean_dir(dir_path):
    if platform.system().lower() == "windows":
        real_dir_path = os.path.realpath(dir_path)
        system_drive_up = os.getenv("SystemDrive").upper()
        system_drive_down = system_drive_up.lower()
        if real_dir_path.startswith(system_drive_up) or \
                real_dir_path.startswith(system_drive_down):
            return
    for path in os.listdir(dir_path):
        new_path = os.path.join(dir_path, path)
        if os.path.isfile(new_path):
            os.remove(new_path)
        elif os.path.isdir(new_path):
            shutil.rmtree(new_path)


def check_del_dir(target_dir_path):
    """
    查询目标目录是否存在，存在就删除
    :return:
    """
    if platform.system().lower() == "windows":
        system_drive_up = os.getenv("SystemDrive").upper()
        system_drive_down = system_drive_up.lower()
        if target_dir_path.startswith(system_drive_up) or \
                target_dir_path.startswith(system_drive_down):
            return
    if os.path.exists(target_dir_path):
        if os.path.isdir(target_dir_path):
            shutil.rmtree(target_dir_path)


def check_del_file(file_path):
    """
    查询目标文件是否存在，存在则删除
    :return:
    """
    if check_path_legal(file_path, "/mnt/databackup/"):
        if os.path.exists(file_path):
            os.remove(file_path)


def check_port_is_used(port):
    """
    判断指定端口是否已经被占用
    返回值: True：已被占用；False: 未被占用
    """
    addrs = socket.getaddrinfo(socket.gethostname(), None)
    for item in addrs:
        if len(item) >= 5 and len(item[4]) >= 2:
            if ":" in item[4][0]:
                continue
            is_used = False
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            except Exception:
                s.close()
                continue
            try:
                s.bind((item[4][0], port))  # 如果绑定成功，则表示该端口未被使用
            except Exception:
                is_used = True  # 如果绑定失败，则表示已被使用
            try:
                s.close()
            except Exception:
                continue
            if is_used:  # 只要有一个绑定失败，则任务已经被使用
                s.close()
                return True
            s.close()
    return False


def exter_attack(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        return func(*args, **kwargs)

    return wrapper


def check_command_injection(param):
    """
    检测是否含有特殊字符防止命令注入
    :param : shell 执行参数
    :return: bool
    """

    expression = "[|;&$><`!+\n]"
    if re.search(expression, param):
        return True
    return False


def check_command_injection_ex(param):
    """
    检测是否含有特殊字符防止命令注入添加'
    :param : shell 执行参数
    :return: bool
    """

    expression = "[|;&$><`'!+\n]"
    if re.search(expression, param):
        return True
    return False


def check_command_injection_ex_quote_space(param):
    """
    检测是否含有特殊字符防止命令注入添加', 空格
    :param : shell 执行参数
    :return: bool
    """

    expression = "[|;&$><`'!+\n\s]"
    if re.search(expression, param):
        return True
    return False


def check_command_injection_exclude_quote(param):
    """
    检测是否含有特殊字符防止命令注入，不允许单、双引号
    :param param: 参数
    :return: True，包含特殊字符；False，不包含特殊字符
    """
    expression = "[|;&$><`'\"!+\n]"
    return bool(re.search(expression, param))


def get_local_ips():
    """获取本机所有IP
    """
    local_ips = []
    ip_dict = psutil.net_if_addrs()
    for _, ele in ip_dict.items():
        for i in ele:
            if i[0] == 2 and i[1] != '127.0.0.1':
                local_ips.append(i[1])
    return local_ips


def check_path_legal(path, parent_dir):
    """
    将路径转为绝对路径，并校验起是否在指定父目录下,防止目录逃逸
    :path : 需要检验的目录
    :parent_dir: 父目录
    :return: bool
    """
    abs_path = os.path.realpath(path)
    abs_parent_dir = os.path.realpath(parent_dir)
    if abs_path.startswith(abs_parent_dir):
        return True
    return False


def get_previous_copy_info(protect_obj, backup_types, job_id):
    param_file_path = RpcParamKey.PARAM_FILE_PATH
    result_path = RpcParamKey.RESULT_PATH
    if platform.system().lower() == "windows":
        param_file_path = RpcParamKey.WINDOWS_PARAM_FILE_PATH
        result_path = RpcParamKey.WINDOWS_RESULT_PATH

    if not os.path.isdir(param_file_path) or not os.path.isdir(result_path):
        return {}
    input_file_name = RpcParamKey.INPUT_FILE_PREFFIX + job_id + str(int(time.time()))
    input_file_path = f"{param_file_path}/{input_file_name}"
    input_dict = {RpcParamKey.APPLICATION: protect_obj,
                  RpcParamKey.TYPES: backup_types,
                  RpcParamKey.COPY_ID: "",
                  RpcParamKey.JOB_ID: job_id}
    output_execution_result_ex(input_file_path, input_dict)
    output_file_name = RpcParamKey.OUTPUT_FILE_PREFFIX + job_id + str(int(time.time()))
    output_file_path = f"{result_path}/{output_file_name}"
    cmd = f"sh {RpcParamKey.RPC_TOOL} {RpcParamKey.QUERY_PREVIOUS_CPOY} {input_file_path} {output_file_path}"
    if platform.system().lower() == "windows":
        cmd = f"{RpcParamKey.WINDOWS_RPC_TOOL} {RpcParamKey.QUERY_PREVIOUS_CPOY} {input_file_path} {output_file_path}"
    ret, std_out, std_err = execute_cmd(cmd)
    if os.path.isfile(input_file_path):
        os.remove(input_file_path)
    if ret != CMDResult.SUCCESS.value or not os.path.isfile(output_file_path):
        return {}
    with open(output_file_path, "r", encoding='UTF-8') as file_content:
        output_data = file_content.read()
    os.remove(output_file_path)
    try:
        copy_info = json.loads(output_data)
    except Exception:
        return {}
    return copy_info


def invoke_rpc_tool_interface(unique_id: str, interface_name: str, input_dict: dict):
    # 因为是固定路径所以不用做校验
    def clear_file(path):
        if os.path.isfile(path):
            os.remove(path)

    if platform.system().lower() == "windows":
        input_file_path = f"{RpcParamKey.WINDOWS_PARAM_FILE_PATH}/{RpcParamKey.OUTPUT_FILE_PREFFIX}{unique_id}"
        output_file_path = f"{RpcParamKey.WINDOWS_RESULT_PATH}/{RpcParamKey.OUTPUT_FILE_PREFFIX}{unique_id}"
        output_execution_result(input_file_path, input_dict)
        cmd = f"{RpcParamKey.WINDOWS_RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    else:
        input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id)
        output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
        output_execution_result(input_file_path, input_dict)
        cmd = f"sh {RpcParamKey.RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    # 执行命令后不论结果都需要删除输入文件
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        clear_file(input_file_path)

    if ret != CMDResult.SUCCESS.value:
        err_info = f"Invoke rpc_tool script[{interface_name}] failed, std_err: {std_err}, ret:{ret}."
        raise Exception(err_info)

    # 不管读取文件是否成功都需要删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        clear_file(output_file_path)
    return result


@retry_when_exception(retry_times=3, delay=3)
def touch_file(file_path):
    pathlib.Path(file_path).touch()


def get_copy_info_by_type_list(copies: List[Copy], type_list: list) -> Copy:
    """
    从copies中获取任一在副本类型列表中的副本
    :param copies: 副本列表
    :param type_list: 类型列表
    :return: 副本信息
    """
    for copy_info in copies:
        if copy_info.type in type_list:
            return copy_info
    return None


def check_sql_cmd_param(param):
    """
    检测拼接sql命令的参数是否含有特殊字符，防止命令注入
    :param  : 拼接 sql 的参数
    :return : bool
    """
    if not param or not isinstance(param, str):
        return False
    # 特别注意，要匹配反斜杠(\)需要使用(\\\\)
    expression = "[|;&$><`!+%/\'\"\\\\:\[^]"
    if re.search(expression, param):
        return False
    return True


def execute_cmd_oversize_return_value(cmd):
    """
    执行shell命令，不支持管道和重定向, 针对返回值超大的情况使用此命令
    :param cmd: shell命令
    :return:执行返回码，正确信息，错误信息
    """
    exitcode = 0
    try:
        data = subprocess.check_output(shlex.split(cmd), encoding="utf-8", shell=False, text=True,
                                       stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as ex:
        data = ex.output
        exitcode = ex.returncode
    except Exception:
        # 存在utf-8 解析不了的返回信息。
        data = "cmd error"
        exitcode = 127
    if data[-1:] == '\n':
        data = data[:-1]
    return str(exitcode), data, data


def report_job_details(pid, job_detail):
    """
    功能描述：主动上报任务详情
    参数：无
    @pid: pid
    @job_detail: 任务详情
    返回值：无
    """
    input_file = ParamConstant.RESULT_PATH + pid + '_input'
    output_file = ParamConstant.RESULT_PATH + pid + '_output'
    json_str = json.dumps(job_detail.dict(by_alias=True))
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(input_file, flags, modes), 'w') as jsonfile:
        jsonfile.write(json_str)
    cmd = f'sh {ParamConstant.BIN_PATH}/rpctool.sh ReportJobDetails' \
          f' {input_file} {output_file}'
    execute_cmd(cmd)
    os.remove(input_file)
    os.remove(output_file)


def get_host_sn():
    if platform.system().lower() == "windows":
        encoding = locale.getdefaultlocale()[1]
        host_sn = read_result_file(PathConstant.WINDOWS_HOST_SN_FILE_PATH, encoding=encoding)
    else:
        host_sn = read_result_file(PathConstant.HOST_SN_FILE_PATH)
    return host_sn.strip("\n")


def str_is_none_or_empty(var_str):
    if var_str is None or len(var_str) == 0:
        return True
    return False


def check_size(src_path: str, size: int):
    """
    校验路径大小
    :param src_path: 路径信息
    :param size: 预期大小，单位为字节
    :return: bool
    """
    if not os.path.exists(src_path):
        return False
    try:
        path_size = os.path.getsize(src_path)
    except Exception as e_obj:
        return False
    if path_size > size:
        return False
    return True


def get_win_dir():
    """
    功能描述: 适配windows下特殊场景路径
    参数：无
    """
    head_path = os.environ['SystemRoot']
    if head_path:
        return head_path.replace("\\", "/")
    return ""


def is_clone_file_system(param):
    """
    是否是clonefilesystem
    :param param:入参
    :return: bool
    """
    repositories_infos = param.get('job').get("copies")[0].get("repositories", [])
    for rep in repositories_infos:
        if rep.get("repositoryType") == RepositoryDataTypeEnum.DATA_REPOSITORY:
            rep_extend_info = rep.get("extendInfo")
            return rep_extend_info.get("isCloneFileSystem", True)
    return True


def is_clone_file_system_for_kingbase(param):
    """
    是否是clonefilesystem
    :param param:入参
    :return: bool
    """
    repositories_infos = param.get("copies")[0].get("repositories", [])
    for rep in repositories_infos:
        if rep.get("repositoryType") == RepositoryDataTypeEnum.DATA_REPOSITORY:
            rep_extend_info = rep.get("extendInfo")
            return rep_extend_info.get("isCloneFileSystem", True)
    return True


def is_ubuntu():
    return 'Ubuntu' in platform.version()


def ismount_with_timeout(path, timeout=10):
    """
    带有超时的校验是否有挂载
    :param path:挂载点
    :param timeout:超时时间
    :return: bool
    """

    def handler(signum, frame):
        raise Exception("Timeout!")

    # 设置超时处理函数
    signal.signal(signal.SIGALRM, handler)
    # 设置一个 alarm，在指定的 timeout 秒数后触发 SIGALRM 信号
    signal.alarm(timeout)

    try:
        result = os.path.ismount(path)
    except Exception as e:
        # 如果超时，会抛出异常，我们重新设置 alarm 为0来取消之前的 alarm
        signal.alarm(0)
        raise e
    finally:
        # 取消 alarm
        signal.alarm(0)

    return result
