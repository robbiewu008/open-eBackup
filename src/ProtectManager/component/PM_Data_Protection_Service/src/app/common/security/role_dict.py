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


class RoleEnum(Enum):
    ROLE_SYS_ADMIN = "Role_SYS_Admin"
    ROLE_DP_ADMIN = "Role_DP_Admin"
    ROLE_AUDITOR = "Role_Auditor"
    ROLE_DR_ADMIN = "Role_DR_Admin"
    ROLE_RD_ADMIN = "Role_RD_Admin"
    ROLE_DEVICE_MANAGER = "Role_Device_Manager"

    @staticmethod
    def is_builtin_role_name(role_name: str):
        return any(role_name == role.value for role in RoleEnum)