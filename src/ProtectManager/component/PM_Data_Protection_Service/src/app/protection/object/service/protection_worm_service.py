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
from app.common.clients.anti_ransomware_client import AntiRansomwareClient
from app.common.enums.sla_enum import PolicyTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger

log = get_logger(__name__)


def has_mutual_exclusion_policies(sla):
    policies = sla.get("policy_list")
    for policy in policies:
        if policy.get("type") == PolicyTypeEnum.archiving.value:
            return True
    return False

