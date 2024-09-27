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

import ipaddress
import os
import re

from typing import Union

from common import common
from common.common import check_command_injection
from common.const import DELETING_PATH_WHITE_LIST


def is_ip_address(ip_addr: str) -> bool:
    """
    检查是否是IP地址
    :param ip_addr: IP地址
    :return: True:是，False:否
    """
    try:
        if ipaddress.ip_address(ip_addr):
            return True
        return False
    except ValueError:
        return False


def is_port(port: Union[int, str]) -> bool:
    """
    检查是否是端口
    :param port: 端口
    :return: True:是，False:否
    """
    pattern = re.compile(r"^\d$|^[1-9]\d$|^[1-9]\d{2}$|^[1-9]\d{3}$|"
                         r"^[1-5]\d{4}$|^6[0-4]\d{3}$|^65[0-4]\d{2}$|^655[0-2]\d$|^6553[0-5]$")
    return True if pattern.match(str(port)) else False


def is_domain(domain: str) -> bool:
    """
    校验是否域名
    校验规则：
    1.域名总长度则不能超过253个字符
    2.对于每一级域名长度的限制是63个字符，每一级域名可以包含字母、数字、中划线、下划线，但不能以中划线、下划线开头或结尾
    3.当每一级都使用单个字符时，限制为127个级别
    4.不能每一级域名都是数字
    :param domain: 输入的域名
    :return: True:是，False:否
    """
    if len(str(domain)) > 253:
        return False
    num_pattern = re.compile(r'^[0-9]{1,63}(\.[0-9]{1,63}){0,126}$')
    if num_pattern.match(str(domain)):
        return False
    pattern = re.compile(r'^([a-zA-Z0-9]{1}|[a-zA-Z0-9][-_a-zA-Z0-9]{0,61}[a-zA-Z0-9])'
                         r'(\.([a-zA-Z0-9]{1}|[a-zA-Z0-9][-_a-zA-Z0-9]{0,61}[a-zA-Z0-9])){0,126}$')
    return True if pattern.match(str(domain)) else False


def check_real_path(path):
    """
    校验文件路径是否是真实路径，覆盖相对路径、~与软链接等
    :param: 文件路径
    :return: True，真实路径；False，非真实路径
    """
    if path.endswith("/"):
        path = path[:-1]
    abs_path = os.path.realpath(path)
    if abs_path != path:
        return False
    return True


def check_file_path(file_path):
    """
    校验文件路径：1、是否是真实路径，2、不能包含特殊字符
    :param: 文件路径
    :return: True，合法文件；False，非法文件
    """
    if not file_path or not os.path.isfile(file_path):
        return False
    if not check_real_path(file_path):
        return False
    return not bool(re.search("[|;&$><`'\"!+\n]", str(file_path)))


def check_dir_path_without_check_mode(dir_path):
    """
    校验目录路径：1、是否是真实路径，2、不能包含特殊字符
    :param: 目录路径
    :return: True，合法目录；False，非法目录
    """
    if not dir_path or os.path.isfile(dir_path):
        return False
    if not check_real_path(dir_path):
        return False
    return not bool(re.search("[|;&$><`'\"!+\n]", str(dir_path)))


def check_dir_path(dir_path):
    """
    校验目录路径：1、是否是真实路径，2、不能包含特殊字符
    :param: 目录路径
    :return: True，合法目录；False，非法目录
    """
    if not dir_path or not os.path.isdir(dir_path):
        return False
    if not check_real_path(dir_path):
        return False
    return not bool(re.search("[|;&$><`'\"!+\n]", str(dir_path)))


def check_param_chars(param):
    """
    检查参数不能包含特殊字符
    :param: 参数
    :return: True，不包含特殊字符；False，包含特殊字符
    """
    return not bool(re.search("[|;&$><`!+\n]", str(param)))


def check_params_allow_arrowhead(param):
    """
    检查参数不能包含特殊字符, 允许右箭头符
    :param: 参数
    :return: True，不包含特殊字符；False，包含特殊字符
    """
    return not bool(re.search("[|;&$<`!+\n]", str(param)))


def check_param_chars_no_single_quote(param):
    """
    检查参数不能包含特殊字符，不允许单引号
    :param: 参数
    :return: True，不包含特殊字符；False，包含特殊字符
    """
    return not bool(re.search("[|;&$><`'!+\n]", str(param)))


def check_param_chars_no_quote(param):
    """
    检查参数不能包含特殊字符，不允许单、双引号
    :param: 参数
    :return: True，不包含特殊字符；False，包含特殊字符
    """
    return not bool(re.search("[|;&$><`'\"!+\n]", str(param)))


def check_repo_path(repo_path):
    """
    校验持久仓路径
    :param repo_path: 输入的持久仓路径
    """
    ret = common.check_path_legal(repo_path, "/mnt/databackup/")
    if not ret:
        return False
    return not bool(re.search(r'[^\/\w\-\.\:]', repo_path))


def check_repo_path_raise_ex(repo_path):
    """
    校验持久仓路径，失败抛异常
    :param repo_path: 输入的持久仓路径
    """
    ret = common.check_path_legal(repo_path, "/mnt/databackup/")
    if not ret:
        raise Exception("The input path is not repository path.")
    if re.search(r'[^\/\w\-\.\:]', repo_path):
        raise Exception("The input repository path is invalid.")


def is_valid_uuid(uuid_str):
    uuid4_reg = r"[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}$"
    return bool(re.match(uuid4_reg, uuid_str))


def check_path_in_white_list(path_):
    """
    检测白名单
    :param : shell 执行参数
    :return: bool
    """
    try:
        real_path = os.path.realpath(path_)
    except Exception as ex:
        return False

    if check_command_injection(real_path):
        return False

    for path in DELETING_PATH_WHITE_LIST:
        if real_path.find(path) == 0:
            return True
    if f"{real_path}/" in DELETING_PATH_WHITE_LIST:
        return True
    return False


def is_valid_id(uuid_str):
    id_reg = r"^[a-zA-Z_0-9-]{0,64}$"
    return bool(re.match(id_reg, uuid_str))
