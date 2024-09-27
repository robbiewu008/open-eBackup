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

import platform

from common.common import check_command_injection
from common.err_code import CommErrCode
from common.exception.common_exception import ErrCodeException
from common.util.check_user_utils import check_os_user, check_path_owner
from common.util.check_utils import check_repo_path

if platform.system().lower() == "windows":
    from common.logger_wins import Logger
else:
    from common.logger import Logger
LOGGER = Logger().get_logger()


def check_common_params(chk_users: list = None, chk_char_params: list = None, chk_path_owners: list = None,
                        chk_repo_paths: list = None):
    """
    检查参数合法性
    :param chk_users: 待校验用户名列表
    :param chk_char_params: 待校验特殊字符的参数列表
    :param chk_path_owners: 待校验路径和所属用户信息列表，如[("/home/rdadmin", ["rdadmin"])]
    :param chk_repo_paths: 待校验持久仓路径列表
    :return:
    """

    def handle_default_list(input_list):
        return input_list or list()

    chk_users = handle_default_list(chk_users)
    chk_char_params = handle_default_list(chk_char_params)
    chk_path_owners = handle_default_list(chk_path_owners)
    chk_repo_paths = handle_default_list(chk_repo_paths)
    for user in chk_users:
        if not check_os_user(user):
            LOGGER.error("The os user(%s) is invalid.", user)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The os user is invalid.")
    for tmp_param in chk_char_params:
        if check_command_injection(tmp_param):
            LOGGER.error("The parameter(%s) contains special characters.", tmp_param)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The parameter contains special characters.")
    for path, owners in chk_path_owners:
        if not check_path_owner(path, owners):
            LOGGER.error("The owners(%s) of path(%s) not match.", owners, path)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The owners of path not match.")
    for path in chk_repo_paths:
        if not check_repo_path(path):
            LOGGER.error("The repository path(%s) is invalid.", path)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message="The repository path is invalid.")
