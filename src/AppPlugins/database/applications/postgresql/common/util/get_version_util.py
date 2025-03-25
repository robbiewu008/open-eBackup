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

from common.common import execute_cmd
from common.const import CMDResult
from common.logger import Logger
from common.util import check_user_utils
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import check_path_valid
from postgresql.common.error_code import ErrorCode

LOGGER = Logger().get_logger("postgresql.log")


def get_version(pid, client_path, os_username, enable_root=0):
    LOGGER.info(f"Get pgsql version!pid: {pid}")
    if not check_user_utils.check_os_user(os_username):
        LOGGER.error(f"Get pgsql version fail, os user {os_username} is invalid.")
        return False, ErrorCode.GET_VERSION_FAILED
    if not enable_root:
        if not check_user_utils.check_path_owner(client_path, [os_username]):
            LOGGER.error("Client path and os user is not matching.")
            return False, ErrorCode.GET_VERSION_FAILED
    if not check_path_valid(client_path, False, False):
        LOGGER.error(f"pg_ctl path[{client_path}] is invalid")
        return False, ErrorCode.GET_VERSION_FAILED
    cmd = cmd_format("su - {} -c '{} --version'", os_username, client_path)
    return_code, std_out, err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS.value:
        LOGGER.error(f"Get pgsql version error!pid: {pid}, error: {err}")
        return False, ErrorCode.GET_VERSION_FAILED
    version = std_out.split(" ")[-1]
    LOGGER.info(f"Success get pgsql version version:{version}!pid: {pid}")
    return True, version
