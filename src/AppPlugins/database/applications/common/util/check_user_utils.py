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
import platform

if platform.system().lower() != 'windows':
    import pwd
    from common.util.common_utils import get_uid_by_os_user


def check_os_user_raise_ex(os_user):
    """
    检查是否操作系统用户名，不是抛异常（支持Linux、Unix系统）
    :param os_user: 操作系统用户名
    """
    try:
        pwd.getpwnam(os_user)
    except TypeError as ex:
        raise Exception(f"Check os user: {os_user} type error.") from ex
    except KeyError as ex:
        raise Exception(f"Check os user: {os_user} does not exist.") from ex
    except Exception as ex:
        raise Exception(f"Check os user: {os_user} exception.") from ex


def check_os_user(os_user):
    """
    检查是否操作系统用户名（支持Linux、Unix系统）
    :param os_user: 操作系统用户名
    :return: True，是；False：不是
    """
    try:
        pwd.getpwnam(os_user)
    except Exception:
        return False
    return True


def check_path_owner(input_path: str, owners: list):
    """
    检查路径的所属用户是否满足
    :param input_path: filename参数
    :param owners: 用户列表
    :return: True，路径所属用户满足；False，路径所属用户不满足
    """
    if not os.path.exists(input_path) or not owners or not isinstance(owners, list):
        return False
    st_uid = os.stat(input_path).st_uid
    for owner in owners:
        if not check_os_user(owner):
            continue
        uid = get_uid_by_os_user(owner)
        if uid == st_uid:
            return True
    return False
