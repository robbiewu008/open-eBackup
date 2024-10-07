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

from enum import Enum

from common.util.check_user_utils import check_os_user
from common.util.check_utils import check_repo_path, check_param_chars, check_param_chars_no_single_quote, \
    check_param_chars_no_quote, is_valid_uuid, check_file_path, check_dir_path, check_params_allow_arrowhead


class ValidatorEnum(str, Enum):
    """校验器名称定义类"""
    # 操作系统用户检查函数
    USER_CHK = "user_chk"
    # 通用特殊字符检查函数
    CHAR_CHK_COMMON = "char_chk_common"
    # 允许右箭头符
    CHAR_CHK_INCLUDE_ARROWHEAD = "char_chk_include_arrowhead"
    # 特殊字符检查函数，不允许单引号
    CHAR_CHK_EXCLUDE_SINGLE_QUOTE = "char_chk_exclude_single_quote"
    # 特殊字符检查函数，不允许单引号、双引号
    CHAR_CHK_EXCLUDE_QUOTE = "char_chk_exclude_quote"
    # 正则校验
    REGEX_CHK_UUID4 = "regex_chk_uuid4"
    # 挂载路径检查函数
    PATH_CHK_REPO = "path_chk_repo"
    # 文件路径检查函数
    PATH_CHK_FILE = "path_chk_file"
    # 目录路径检查函数
    PATH_CHK_DIR = "path_chk_dir"


ALL_VALIDATOR_NAMES = [e.value for e in ValidatorEnum]
NAME_VALIDATOR_MAP = {
    ValidatorEnum.USER_CHK: check_os_user,
    ValidatorEnum.CHAR_CHK_COMMON: check_param_chars,
    ValidatorEnum.CHAR_CHK_INCLUDE_ARROWHEAD: check_params_allow_arrowhead,
    ValidatorEnum.CHAR_CHK_EXCLUDE_SINGLE_QUOTE: check_param_chars_no_single_quote,
    ValidatorEnum.CHAR_CHK_EXCLUDE_QUOTE: check_param_chars_no_quote,
    ValidatorEnum.REGEX_CHK_UUID4: is_valid_uuid,
    ValidatorEnum.PATH_CHK_REPO: check_repo_path,
    ValidatorEnum.PATH_CHK_FILE: check_file_path,
    ValidatorEnum.PATH_CHK_DIR: check_dir_path,
}
