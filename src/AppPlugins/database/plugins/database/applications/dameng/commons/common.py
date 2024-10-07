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
import re

import pwd
import grp
import stat
import time
import configparser
import shlex
import subprocess
import json
import shutil
import locale

from dameng.commons.const import DamengStrConstant, SubJobType, SubJobPolicy, \
    ExecCmdResult, DELETING_PATH_WHITE_LIST, SysData, RpcParamKey
from common.common import execute_cmd, output_execution_result, check_command_injection, execute_cmd_list, \
    get_local_ips, check_size
from common.logger import Logger

LOGGER = Logger().get_logger("dameng.log")


class IniParses:

    def __init__(self, ini_path='', encoding=None):
        self.ini_path = ini_path
        if not encoding:
            encoding = locale.getdefaultlocale()[1]
        self.encoding = encoding
        self.config = configparser.ConfigParser()
        self.config.read(self.ini_path, encoding=self.encoding)

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

    def get_all_ep(self):

        section_list = self.config.sections()
        return section_list

    def get_ep_allitems(self, ep_name):

        result = self.config.items(ep_name)
        return result

    def get_ep_value(self, ep_name, key):

        result = self.config.get(ep_name, key)
        if result:
            result = result.replace(' ', '').split("#")[0]
        return result

    def check_ep(self, ep_name):

        result = self.config.get(ep_name)
        return result


def check_ip_in_local(ip: str):
    all_ip = get_local_ips()
    if ip in all_ip:
        return True
    return False


def del_space(str_):
    """
    倒序循环删除空字符串
    :type s: str
    :rtype: list
    """
    if str_.isspace():
        return []
    elif str_ == "":
        return []
    else:
        temp = str_.split(" ")
    for i in range(len(temp) - 1, -1, -1):
        if temp[i] == "":
            del temp[i]
    return temp


def del_space_in_list(list_):

    for i in range(len(list_) - 1, -1, -1):
        if list_[i] == "":
            del list_[i]
    return list_


def del_file(path):
    ret, realpath = check_path_in_white_list(path)
    if not ret:
        return False
    if os.path.exists(realpath):
        os.remove(realpath)
        return True
    return False


def get_path_user_and_group(path):
    """
    获取指定目录的用户名和组名
    """
    if not path:
        return False, '', ''
    stat_info = os.stat(path)
    uid = stat_info.st_uid
    gid = stat_info.st_gid
    user = pwd.getpwuid(uid)[0]
    group = grp.getgrgid(gid)[0]
    return True, user, group


def read_file_by_utf_8(file_path):
    if not file_path or not os.path.exists(file_path):
        return ""
    with open(file_path, "r", encoding="utf-8") as f:
        text = f.read()
    return text


def get_hostsn():
    hostsn = read_file_by_utf_8(DamengStrConstant.HOSTSN_FILE_PATH)
    hostsn = hostsn.strip("\n")
    return hostsn


def build_sub_job(job_id, job_priority, exec_node_id, job_info, job_name):
    """
    填充子任务信息
    :return:
    """
    sub_job_info = dict()
    sub_job_info["jobId"] = job_id
    sub_job_info["subJobId"] = ""
    sub_job_info["jobType"] = SubJobType.BUSINESS_SUB_JOB.value
    sub_job_info["jobName"] = job_name
    sub_job_info["jobPriority"] = job_priority
    sub_job_info["policy"] = SubJobPolicy.FIXED_NODE.value
    sub_job_info["ignoreFailed"] = False
    sub_job_info["execNodeId"] = exec_node_id
    sub_job_info["jobInfo"] = job_info
    return sub_job_info


def clean_dir(dir_path):
    for path in os.listdir(dir_path):
        new_path = os.path.join(dir_path, path)
        ret, realpath = check_path_in_white_list(new_path)
        if not ret:
            return False
        if os.path.isfile(realpath):
            os.remove(realpath)
        elif os.path.isdir(realpath):
            shutil.rmtree(realpath)
    return True


def mkdir_set_permissions(path_, username_):
    """
    设置目录及其父目录所属用户
    :param path_: 目录路径
    :param username_: 用户名
    :return:
    """
    ret, realpath = check_path_in_white_list(path_)
    if not ret:
        return False
    if not os.path.exists(realpath):
        try:
            os.makedirs(realpath)
        except Exception as e:
            return False
    #获取uid和gid
    cmd = f"id {username_}"
    result_type, out_info, err_info = execute_cmd(cmd)
    if result_type != ExecCmdResult.SUCCESS:
        return False
    id_info = out_info.replace('(', ' ')
    id_info = id_info.replace('=', ' ')
    id_info = id_info.split(' ')
    uid_info_local = 1
    uid = id_info[uid_info_local]
    gid_info_local = 4
    gid = id_info[gid_info_local]
    os.lchown(path_, int(uid), int(gid))
    p_path_info = path_.split('/')[0:-1]
    p_path = '/'.join(p_path_info)
    os.lchown(p_path, int(uid), int(gid))
    return True


def check_path_in_white_list(path_):
    try:
        real_path = os.path.realpath(path_)
    except Exception as e:
        return False, ''

    if check_command_injection(real_path):
        return False, ''

    for path in DELETING_PATH_WHITE_LIST:
        if real_path.find(path) == 0:
            return True, real_path
    if f"{real_path}/" in DELETING_PATH_WHITE_LIST:
        return True, real_path
    return False, ''


def dameng_execute_cmd(cmd, encoding=None):
    """
    执行shell命令，不支持管道和重定向
    :param cmd: shell命令
    :return:执行返回码，正确信息，错误信息
    """
    exitcode = 0
    if not encoding:
        encoding = locale.getdefaultlocale()[1]

    try:
        data = subprocess.check_output(shlex.split(cmd), encoding=encoding, \
            shell=False, text=True, stderr=subprocess.STDOUT, errors='ignore')
    except subprocess.CalledProcessError as ex:
        data = ex.output
        exitcode = ex.returncode
    if data[-1:] == '\n':
        data = data[:-1]
    return str(exitcode), data, data


def get_env_value(data):
    input_str = json.loads(SysData.SYS_STDIN)
    value = input_str.get(data, '')
    return value


def overwrite_file(file_path, payload):
    json_str = json.dumps(payload)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as f_object:
        f_object.write(json_str)


def invoke_rpc_tool_interface(unique_id: str, interface_name: str, param_dict: dict):

    def clear_file(path):
        if os.path.isfile(path):
            os.remove(path)
    if check_command_injection(unique_id):
        return {}
    input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id)
    output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
    output_execution_result(input_file_path, param_dict)

    cmd = f"sh {RpcParamKey.RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        # 执行命令后删除输入文件
        clear_file(input_file_path)

    if ret != ExecCmdResult.SUCCESS:
        return {}

    # 读取文件成功后删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        clear_file(output_file_path)

    return result


def timestamp(shijian):
    s_t = time.strptime(shijian, "%Y-%m-%d %H:%M:%S")
    mkt = int(time.mktime(s_t))
    return mkt


def matching_field(key, file):
    """
    匹配以关键字开头的字符串
    :param key:匹配关键字
    :param file: 匹配内容
    :return: 匹配结果
    """
    file = file.strip(" ")
    find_str = file.replace(' ', '')
    find_str = find_str.replace('\t', '')
    result = re.findall(rf"^{key}", find_str, re.I)
    return result


def parse_matching_output(key, file):
    result = matching_field(key, file)
    if not result:
        return ''
    file = file.replace(' ', '')
    file = file.replace('\t', '')
    # 0:预期结果为["SYStEM_PATH= /dm8/data/test0109/test0109name", "sadasd"]取第一个
    file = file.split("#")[0]
    file_list = file.strip('=').split("=")
    # 2：预期结果为["SYStEM_PATH", "/dm8/data/test0109/test0109name"]
    if len(file_list) != 2:
        return ''
    # 1：预期结果为["SYStEM_PATH", "/dm8/data/test0109/test0109name"]取第2个
    return file_list[1]


def matching_dameng_field(key, path):
    """
    关键字忽略大小写匹配达梦数据库配置文件参数并解析所需结果
    :param key: 关键字
    :param file: 匹配内容
    :return: 所需结果
    """
    out_put = []
    ret_info = open_grep(key, path, ignore_case=True)
    if not ret_info:
        return []
    for file in ret_info:
        out_info = parse_matching_output(f"{key}=", file)
        if not out_info:
            continue
        out_put.append(out_info)
    return out_put


def remove_file_dir(path_):
    ret, realpath = check_path_in_white_list(path_)
    if not ret:
        return False
    if not os.path.exists(realpath):
        return True
    if os.path.isfile(realpath):
        try:
            os.remove(realpath)
        except Exception:
            return False
    else:
        try:
            shutil.rmtree(realpath)
        except Exception:
            return False
    return True


def check_user_password(user, password):
    """
    校验用户名密码是否包含/或者\
    """
    if "/" in user or "\\" in user:
        LOGGER.error("The user name is not compliant.")
        return False
    if "/" in password or "\\" in password:
        LOGGER.error("The password name is not compliant.")
        return False
    return True


def check_port(port):
    """
    校验数据库端口
    """
    if isinstance(port, int):
        return True
    if not isinstance(port, str):
        return False
    if not port.isdigit():
        return False
    return True


def open_grep(key, path: str, ignore_case=False):
    result = []
    path = path.strip('"').strip("'")
    if not os.path.exists(path):
        return result
    if os.path.islink(path):
        return result
    file_size = 1073741824
    if not check_size(path, file_size):
        return result
    with open(path, 'r') as f_obj:
        for line in f_obj:
            line = line.strip('\n')
            tmp_line = line
            tmp_key = key
            if ignore_case:
                tmp_line = line.lower()
                tmp_key = key.lower()
            if tmp_key in tmp_line:
                result.append(line)
    return result


def cmd_grep(key, cmd, ignore_case=False):
    result = []
    ret, out, err = execute_cmd(cmd)
    if ret != ExecCmdResult.SUCCESS:
        return result
    out_list = out.split('\n')
    for line in out_list:
        tmp_line = line
        tmp_key = key
        if ignore_case:
            tmp_line = line.lower()
            tmp_key = key.lower()
        if tmp_key in tmp_line:
            result.append(line)
    return result


def check_path_user(path, user):
    try:
        if pwd.getpwuid(os.stat(path).st_uid).pw_name != user:
            return False
    except Exception as e_obj:
        return False
    return True
