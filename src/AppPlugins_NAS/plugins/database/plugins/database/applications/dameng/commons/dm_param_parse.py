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

from common.common import check_command_injection
from common.logger import Logger

log = Logger().get_logger("dameng.log")


def verifying_special_characters(param):
    """
    校验参数（字符）的合法性
    """
    if not param:
        return True
    if not isinstance(param, str):
        log.info(f"The param is not str. The type of the parameter is {type(param)}.")
        return False
    if check_command_injection(param):
        log.error("The param has special characters.")
        return False
    return True
