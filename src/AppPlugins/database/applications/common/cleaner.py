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

import ctypes
import os
import platform
import shutil

import sys


def clear(secret):
    """
    遗留待删除方法，安全已有结论，对于python语言无需清理
    :param secret: 变量
    :return: 无
    """
    pass


def clear_repository_dir(file_path):
    """
    清空仓库文件
    :param file_path: 文件路径
    """
    if platform.system().lower() == "windows":
        realpath = file_path
    else:
        realpath = os.path.realpath(file_path)
    for path in os.listdir(realpath):
        new_path = os.path.join(realpath, path)
        if '.snapshot' in new_path:
            continue
        # 排除的文件不删除
        if os.path.isfile(new_path):
            try:
                os.remove(new_path)
            except FileNotFoundError:
                pass
        elif os.path.isdir(new_path):
            shutil.rmtree(new_path, ignore_errors=True)
