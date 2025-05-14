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
        return sub_type == ResourceSubTypeEnum.HBaseBackupSet.value

    @staticmethod
    def do_validate(policies):
        backup_policies = [policy for policy in policies if policy.type == PolicyTypeEnum.backup.value]

        for backup_policy in backup_policies:
            ext_parameters = backup_policy.get("backup_policy", {})
            if backup_policy.get("action", "") == PolicyActionEnum.full.value:
                if ext_parameters.get("hbase_backup_type", "") != HbaseBackupMode.Snapshot.value:
                    raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["hbase_backup_type"])
                if not (ext_parameters.get("is_reserved_latest_snapshot", "") is True):
                    raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["reserved_snapshot"])

            if backup_policy.get("action", "") == PolicyActionEnum.difference_increment.value:
                if (ext_parameters.get("hbase_backup_type", "") == HbaseBackupMode.Snapshot.value) or \
                        ("is_reserved_latest_snapshot" in ext_parameters):
                    raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["is_reserved_latest_snapshot"])
