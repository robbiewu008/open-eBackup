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
from app.backup.common.validators.sla_validator import ParamsValidator
from app.backup.service.backup_policy_service import get_start_time_by_action
from app.common import logger
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum, TriggerEnum
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

log = logger.get_logger(__name__)


class BusinessPoliciesValidator:

    @staticmethod
    def do_validate(policies):
        backup_policies = [policy for policy in policies
                           if policy.type == PolicyTypeEnum.backup and policy.action != PolicyActionEnum.log]
        replication_policies = [policy for policy in policies if policy.type == PolicyTypeEnum.replication]
        if len(backup_policies) > 1:
            BusinessPoliciesValidator._check_backup_full_start_time_before_increment(backup_policies)
        if len(replication_policies) != 0:
            BusinessPoliciesValidator._check_backup_shortest_retention_greater_than_replication_longest_frequency(
                backup_policies, replication_policies)

    @staticmethod
    def _check_backup_full_start_time_before_increment(backup_policies):
        increment_backup_policies = [policy for policy in backup_policies if policy.action != PolicyActionEnum.full]
        if len(increment_backup_policies) == 0:
            return
        start_time_dict = get_start_time_by_action(backup_policies)
        full_start_time = start_time_dict.get(PolicyActionEnum.full.value)
        increment_start_time = start_time_dict.get(increment_backup_policies[0].action)
        if full_start_time is not None and increment_start_time is not None:
            log.info(f"full back up start time = {full_start_time}, "
                     f"increment back up start time = {increment_start_time}")
            if increment_start_time <= full_start_time:
                raise EmeiStorBizException(error=ProtectionErrorCodes.START_TIME_INVALID_OPERATION,
                                           parameters=[],
                                           message="The start time of different "
                                                   "or cumulative backup must be later than full backup")

    @staticmethod
    def _check_backup_shortest_retention_greater_than_replication_longest_frequency(backup_policies,
                                                                                    replication_policies):
        backup_type = {PolicyActionEnum.full.value, PolicyActionEnum.difference_increment.value,
                       PolicyActionEnum.cumulative_increment.value}
        backup_policy_list = [policy for policy in backup_policies if policy.action in backup_type]
        backup_shortest_retention = ParamsValidator.get_backup_shortest_retention(backup_policy_list)
        log.debug(f"backup_min_retention={backup_shortest_retention}, "
                  f"compare size between retention and frequency")
        replication_longest_frequency = ParamsValidator.get_replication_longest_frequency(replication_policies)
        log.debug(f"replication_longest_frequency={replication_longest_frequency}, "
                  f"compare size between retention and frequency")
        # 备份保留时间中的最小值 必须大于 复制周期中的最大值
        if backup_shortest_retention and replication_longest_frequency and (
                backup_shortest_retention <= replication_longest_frequency):
            raise EmeiStorBizException(error=ProtectionErrorCodes.OPERATION_INTERVAL_CAN_NOT_MORE_THAN_RETENTION)
