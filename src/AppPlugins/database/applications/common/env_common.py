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


def get_install_head_path():
    """
    功能描述: 通过环境变量获取agent安装目录
    参数：无
    linux路径: /opt/DataBackup/ProtectClient/ProtectClient-E/log/Plugins/GeneralDBPlugin  
    windows路径:C:\DataBackup\ProtectClient\ProtectClient-E\log\Plugins\GeneralDBPlugin
    默认返回值: 1.Linux:/opt  2.Windows: C:
    """
    return os.getenv("DATA_BACKUP_AGENT_HOME")


def get_informix_install_path():
    return os.getenv("INFORMIXDIR")


def get_gbase_install_path():
    gbase_dir = os.getenv("GBASEDBTDIR")
    if gbase_dir and not os.path.exists(gbase_dir):
        gbase_dir = "/opt/GBASE/gbase"
    return gbase_dir


def adaptation_win_path():
    """
    功能描述: 适配windows下特殊场景路径
    参数：无
    """
    head_path = os.getenv("DATA_BACKUP_AGENT_HOME")
    if head_path:
        return head_path.replace("\\", "/")
    return ""
