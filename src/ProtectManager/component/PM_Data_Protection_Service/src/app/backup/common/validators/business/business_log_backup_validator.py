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
import time

from app.backup.common.constant import ProtectionConstant
from app.backup.service.backup_policy_service import get_start_time_by_action
from app.common import logger
from app.common.enums.sla_enum import time_interval_switcher, TriggerEnum, BackupTimeUnit, PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException

log = logger.get_logger(__name__)

BACKUP_ACTIONS = [PolicyActionEnum.full, PolicyActionEnum.cumulative_increment,
                  PolicyActionEnum.permanent_increment, PolicyActionEnum.difference_increment]


class BusinessLogBackupValidator:

    @staticmethod
    def check_log_interval_minutes_unit_greater_or_equal_to_five(policies):
        log_policies = [policy for policy in policies if policy.action == PolicyActionEnum.log]
        if len(log_policies) == 0:
            return
        for policy in log_policies:
            if policy.schedule.interval_unit == BackupTimeUnit.minutes and policy.schedule.interval < 5:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["interval"])

    @staticmethod
    def check_policy_backup_start_time_before_log(policies):
        log_policies = [policy for policy in policies if policy.action == PolicyActionEnum.log]
        backup_policies = [policy for policy in policies if policy.action in BACKUP_ACTIONS]
        if len(log_policies) == 0:
            return

        log_start_time = str(log_policies[0].schedule.start_time)
        log_timestamp = time.mktime(time.strptime(log_start_time, ProtectionConstant.DATE_TIME_FORMATTER))
        log.debug(f"log backup validator, log start time={log_timestamp}")

        start_time_dict = get_start_time_by_action(backup_policies)
        backup_start_time = [
            start_time_dict.get(policy.action) for policy in backup_policies]
        log.debug(f"log backup validator, backup start time={backup_start_time}")
        if max(backup_start_time) >= int(log_timestamp):
            raise EmeiStorBizException(error=ProtectionErrorCodes.START_TIME_INVALID_OPERATION,
                                       parameters=[],
                                       message="The start time of log backup must be later than other backups")

    @staticmethod
    def check_policy_backup_schedule_interval_greater_than_log(policies):
        log_policies = [policy for policy in policies if policy.action == PolicyActionEnum.log]

        backup_policies = [policy for policy in policies if policy.action in BACKUP_ACTIONS]
        # 指定时间策略schedule无interval字段，无需比较
        for policy in backup_policies:
            if policy.schedule.trigger == TriggerEnum.customize_interval:
                return
        if len(log_policies) == 0:
            return
        log_second = time_interval_switcher[log_policies[0].schedule.interval_unit.value](
            log_policies[0].schedule.interval)
        backup_seconds = [time_interval_switcher[policy.schedule.interval_unit.value](
            policy.schedule.interval) for policy in backup_policies]
        if min(backup_seconds) <= log_second:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["policy.interval"],
                                        "The log backup frequency must be less than other backup frequencies")
