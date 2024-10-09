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
import psutil

from dameng.commons.dm_rman_tool import DmRmanTool
from dameng.commons.query_information import get_progress
from dameng.resource.damengsource import DamengSource
from common.common import execute_cmd
from dameng.commons.common import check_path_in_white_list, dameng_execute_cmd
from dameng.commons.const import DM_FILE_PATH, RestoreProgress
from dameng.commons.const import ExecCmdResult, ArrayIndex
from common.const import SubJobStatusEnum

from common.logger import Logger

LOGGER = Logger().get_logger("dameng.log")


def cheak_backupset_info(backupset_path, file_name_id):
    """
    检查备份集信息
    :param backupset_path:
    :return:
    """
    # 下发检查备份集命令
    dmrman_cmd = f"CHECK BACKUPSET \'{backupset_path}\'"
    rman_ret = DmRmanTool().run_rman_tool((dmrman_cmd,), DM_FILE_PATH, redirect_output=True, file_name_id=file_name_id)
    if not rman_ret.get("result"):
        LOGGER.error(f"Check backupset path fail rman_ret:{rman_ret}.")
        return False, rman_ret.get("out_info")
    return True, ''


def read_cheak_backupset_progress(file_name_id):
    progress_file_path = os.path.join(DM_FILE_PATH, f"dmrman_cmd_{file_name_id}.log")
    if not os.path.exists(progress_file_path):
        LOGGER.error(f"Get check backupset progress file fail.")
        return True, RestoreProgress.start_progress
    ret, progress_file_path = check_path_in_white_list(progress_file_path)
    if not ret:
        return False, RestoreProgress.failed_progress
    # 查看检查备份集进度文件
    return_type, install_user, _ = DamengSource.discover_application()
    if not return_type:
        LOGGER.error("Get install user fail.")
        return False, RestoreProgress.failed_progress
    bin_path = DamengSource.get_bin_path(install_user)
    if not bin_path:
        LOGGER.error("Get bin_path user fail.")
        return False, RestoreProgress.failed_progress
    if install_user == '':
        return False, RestoreProgress.failed_progress
    cmd = f"su - {install_user} -c \'cat {progress_file_path}\'"
    ret_type, ret_info, err_info = dameng_execute_cmd(cmd)
    if ret_type != ExecCmdResult.SUCCESS:
        LOGGER.error(f"Get check backupset progress fail.")
        return False, RestoreProgress.failed_progress
    if "successfully" in ret_info:
        return True, RestoreProgress.failed_progress
    re_rule = "Percent:(.*?)%]"
    slot_list = re.findall(re_rule, ret_info)
    if slot_list:
        progress = int(float(slot_list[ArrayIndex.INDEX_LAST_1]))
        return True, progress
    LOGGER.error("Parses check backupset progress fail.")
    return False, RestoreProgress.failed_progress


def cheak_instance_status(dmini_path):
    """
    通过进程查看数据库实例是否运行
    :return:
    """
    result_type, user, group = DamengSource.discover_application()
    if not result_type:
        LOGGER.error("Discover dameng application fail.")
        return False
    bin_path = DamengSource.get_bin_path(user)
    if not bin_path:
        LOGGER.error("Get dameng bin path fail.")
        return False
    pids = psutil.pids()
    dmserver_str = f"{bin_path}/dmserver"
    for pid in pids:
        try:
            process = psutil.Process(pid)
        except Exception as e_info:
            LOGGER.error("Pid does not exist.")
            continue
        try:
            cmd = process.cmdline()
        except Exception as e_info:
            LOGGER.error("Pid does not exist.")
            continue
        #2:预期结果['/dm8/bin/dmserver', '/dm8/dm.ini' ]
        if len(cmd) < 2:
            continue
        if dmserver_str == cmd[ArrayIndex.INDEX_FIRST_0] and dmini_path in cmd[ArrayIndex.INDEX_FIRST_1]:
            return True
    return False


def check_dmapserver_status():
    ret, uname, ugroup = DamengSource.discover_application()
    if not ret:
        LOGGER.error(f"Get dameng install user fail.")
        return False
    bin_path = DamengSource.get_bin_path(uname)
    if not bin_path:
        LOGGER.error("Get dameng bin path fail.")
        return False
    pids = psutil.pids()
    for pid in pids:
        try:
            process = psutil.Process(pid)
        except Exception as e_info:
            LOGGER.error("Pid does not exist.")
            continue
        try:
            cmd = process.cmdline()
        except Exception as e_info:
            LOGGER.error("Pid does not exist.")
            continue
        #预期结果为["/dm8/bin/dmap", '', '']
        if len(cmd) < 1:
            continue
        str_key = os.path.join(bin_path, "dmap")
        if str_key == cmd[0]:
            return True
    LOGGER.error(f"DmAPService status not running.")
    return False