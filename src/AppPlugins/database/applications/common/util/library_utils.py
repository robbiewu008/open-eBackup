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
from typing import List


def add_library(library_path: str) -> None:
    """
    将指定的 library_path 添加到 LD_LIBRARY_PATH 环境变量中，
    如果 library_path 不存在于当前的 LD_LIBRARY_PATH 中。

    参数:
        library_path (str): 要添加的库路径。
    """
    if not library_path:
        return

    ld_library_path = os.environ.get('LD_LIBRARY_PATH', '')
    # 使用 clean_ld_library_path 过滤掉指定的路径 这里需要过滤掉原xtrabackup的两个lib库，否则会报错
    new_ld_library_path = clean_ld_library_path(ld_library_path,
                                                ["/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib/3rd",
                                                 "/opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/lib"])
    if not new_ld_library_path or (library_path in new_ld_library_path.split(':')):
        os.environ['LD_LIBRARY_PATH'] = new_ld_library_path
        return
    os.environ['LD_LIBRARY_PATH'] = f"{library_path}:{new_ld_library_path}"


def clean_ld_library_path(ld_library_path: str, remove_paths: List[str]) -> str:
    """
    移除LD_LIBRARY_PATH字符串中指定的路径。

    参数:
        ld_library_path (str): 原始的LD_LIBRARY_PATH字符串，以冒号分隔。
        remove_paths (List[str]): 需要移除的路径列表。

    返回:
        str: 更新后的LD_LIBRARY_PATH字符串。
    """
    if not ld_library_path:
        return ld_library_path

    # 将原始字符串按冒号分割成列表
    paths = ld_library_path.split(':')

    # 过滤掉需要移除的路径
    filtered_paths = [path for path in paths if path not in remove_paths]

    # 重新用冒号连接成新的字符串
    new_ld_library_path = ':'.join(filtered_paths)

    return new_ld_library_path
