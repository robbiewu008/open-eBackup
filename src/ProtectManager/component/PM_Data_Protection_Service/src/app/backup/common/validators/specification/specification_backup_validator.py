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
from app.common.enums.sla_enum import PolicyTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class SpecificationBackupValidator:

    @staticmethod
    def do_validate(policies):
        backup_policies = [policy for policy in policies
                           if policy.type == PolicyTypeEnum.backup.value]

        if len(backup_policies) == 0:
            return

        SpecificationBackupValidator._check_backup_policies_count_not_empty(backup_policies)

    @staticmethod
    def _check_backup_policies_count_not_empty(backup_policies):
        if len(backup_policies) <= 0:
            raise IllegalParamException(
                error=CommonErrorCodes.ILLEGAL_PARAMS,
                parameters=["sla.policy"],
                message="backup policy is sla can not be null")
