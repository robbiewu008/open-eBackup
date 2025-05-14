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
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, time_interval_switcher
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException

from app.backup.common.validators.validator import Validator


class OracleValidator(Validator):

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.Oracle.value

    @staticmethod
    def do_validate(policies):
        backup_policies = [policy for policy in policies
                           if
                           policy.type == PolicyTypeEnum.backup.value and policy.action != PolicyActionEnum.log.value]
        log_policy = [
            policy for policy in policies if policy.action == PolicyActionEnum.log]
        if len(log_policy) == 0:
            return

        # 校验 日志备份首次开始时间必须晚于数据备份首次开始时间
        log_start_time = log_policy[0].schedule.start_time
        backup_start_time = [
            policy.schedule.start_time for policy in backup_policies]
        if max(backup_start_time) >= log_start_time:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["policy.schedule"],
                                        "The start time of log backup must be later than other backups")
        # 校验 日志备份频率必须小于数据备份频率
        log_second = time_interval_switcher[log_policy[0].schedule.interval_unit.value](
            log_policy[0].schedule.interval)
        backup_seconds = [time_interval_switcher[policy.schedule.interval_unit.value](
            policy.schedule.interval) for policy in backup_policies]
        if min(backup_seconds) <= log_second:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["policy.interval"],
                                        "The log backup frequency must be less than other backup frequencies")
