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
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, HbaseBackupMode
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException

from app.backup.common.validators.validator import Validator


class HbaseValidator(Validator):

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.DWSCluster.value

    @staticmethod
    def do_validate(policies):
        # 增量备份和差异增量无法同时存在
        backup_policies = [policy for policy in policies if policy.type == PolicyTypeEnum.backup.value]
        backup_type = list(backup_policy.get("action", "") for backup_policy in backup_policies)
        if (PolicyActionEnum.difference_increment.value in backup_type) and (PolicyActionEnum.cumulative_increment.value
                                                                             in backup_type):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["action"])
