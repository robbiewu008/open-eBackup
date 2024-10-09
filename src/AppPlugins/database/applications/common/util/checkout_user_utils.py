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

import pexpect
import psutil


def checkout_user_and_execute_cmd(os_username, cmd):
    # 登录
    child = pexpect.spawn(f"su - {os_username}", encoding="utf-8", timeout=20)
    index = child.expect([pexpect.TIMEOUT, pexpect.EOF, r'\$'])
    if index in (0, 1):
        child.close()
        raise Exception("Checkout user failed!")
    child.sendline(cmd)
    index = child.expect([pexpect.TIMEOUT, pexpect.EOF, r'\$'])
    if index in (0, 1):
        child.close()
        raise Exception("Execute cmd failed!")
    child.close()


def get_path_owner(check_path):
    """
    获取路径用户
    """
    if not os.path.exists(check_path):
        raise Exception("Get path owner failed!")
    return psutil.pwd.getpwuid(os.stat(check_path).st_uid).pw_name
