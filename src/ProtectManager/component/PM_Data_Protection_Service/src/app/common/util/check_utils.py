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
import re
import ipaddress


def is_ip_address(ip_addr: str):
    try:
        if ipaddress.ip_address(ip_addr):
            return True
        return False
    except ValueError:
        return False


def is_port(port):
    pattern = re.compile(r"^\d$|^[1-9]\d$|^[1-9]\d{2}$|^[1-9]\d{3}$|"
                         r"^[1-5]\d{4}$|^6[0-4]\d{3}$|^65[0-4]\d{2}$|^655[0-2]\d$|^6553[0-5]$")
    return True if pattern.match(str(port)) else False


def is_domain(domain):
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
