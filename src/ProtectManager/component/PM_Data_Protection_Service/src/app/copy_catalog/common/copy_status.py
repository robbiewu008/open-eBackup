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
from enum import Enum


class CopyStatus(str, Enum):
    """
    deleting: 副本正在删除中
    Normal: 副本有效
    Invalid: 副本无效
    Mounting: 副本正在挂载
    Restoring: 副本正在恢复
    Mounting: 副本正在挂载
    Mounted: 副本已挂载
    Unmounting: 副本正在解挂载
    Verifying: 副本正在校验
    DeleteFailed: 副本删除失败，副本过期一个月还没有过期成功，置为删除失败
    Sharing: 共享中
    Downloading: 下载中
    """
    DELETING = "Deleting"
    NORMAL = "Normal"
    INVALID = "Invalid"
    RESTORING = "Restoring"
    MOUNTING = "Mounting"
    MOUNTED = "Mounted"
    UNMOUNTING = "Unmounting"
    VERIFYING = "Verifying"
    DELETEFAILED = "DeleteFailed"
    SHARING = "Sharing"
    DOWNLOADING = "Downloading"

    @classmethod
    def get(cls, name: str):
        return cls[name.upper()]


class CopyWormStatus(int, Enum):
    # 未设置
    UNSET = 1,
    # 设置中
    SETTING = 2,
    # 已设置
    SET_SUCCESS = 3,
    # 设置失败
    SET_FAILED = 4,
    # 已过期
    EXPIRED = 5
