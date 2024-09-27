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
import time

from dameng.commons.const import BACKUP_MOUNT_PATH
from dameng.commons.const import DamengStrFormat
from dameng.commons.common import check_path_in_white_list, mkdir_set_permissions, cmd_grep
from dameng.resource.damengsource import DamengSource


from common.logger import Logger
from common.common import execute_cmd

LOGGER = Logger().get_logger("dameng.log")


def backup_mount_bind(path_, job_id_, new_path=''):
    """
    日志挂载目录mount bind
    :return: bind结果,'新路径'
    """
    if not new_path:
        new_path = f"{BACKUP_MOUNT_PATH}/{job_id_}"
    ret, realpath = check_path_in_white_list(path_)
    if not ret:
        return False, ''
    ret, new_realpath = check_path_in_white_list(new_path)
    if not ret:
        return False, ''
    if not dameng_user_mkdir(new_realpath):
        return False, ''
    cmd = DamengStrFormat.MOUNT_BIND.format(realpath, new_realpath)
    return_code, _, _ = execute_cmd(cmd)
    if return_code == "0":
        LOGGER.info("Mount succ.")
        return True, new_path
    LOGGER.info("Mount fail.")
    return False, ''


def umount(path_):
    ret, realpath = check_path_in_white_list(path_)
    if not ret:
        return False
    if not os.path.ismount(realpath):
        return True
    cmd = f"umount {realpath}"
    return_code, out_info, _ = execute_cmd(cmd)
    if return_code == "0":
        return True
    return False


def dameng_user_mkdir(path_):
    result_type, username, _ = DamengSource.discover_application()
    if not result_type:
        LOGGER.error("Get username fail.")
        return False
    if mkdir_set_permissions(path_, username):
        LOGGER.info(f"Mkdir file path {path_} succ.")
        return True
    LOGGER.error(f"Mkdir file path fail.")
    return False