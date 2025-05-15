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
from app.common.enums.sla_enum import PolicyTypeEnum, RetentionTypeEnum, TriggerEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class BusinessReplicationValidator:

    @staticmethod
    def do_validate(policy):
        # 只针对单个复制策略进行校验
        if policy.type != PolicyTypeEnum.replication:
            return
        BusinessReplicationValidator._validate_policy(policy)

    @staticmethod
    def _validate_policy(policy):
        ext_parameters = policy.ext_parameters
        schedule = policy.schedule
        retention = policy.retention

        BusinessReplicationValidator._check_schedule_trigger_limit(schedule)
        BusinessReplicationValidator._check_trigger_by_type(schedule)
        BusinessReplicationValidator._check_external_system_id_exists_and_not_empty(ext_parameters)
        BusinessReplicationValidator._check_retention_by_type(retention, schedule)

    @staticmethod
    def _check_schedule_trigger_limit(schedule):
        if schedule.trigger not in [TriggerEnum.backup_complete,
                                    TriggerEnum.interval]:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["trigger"],
                                        f"param [schedule.trigger={schedule.trigger}] in archive policy is not support")

    @staticmethod
    def _check_trigger_by_type(schedule):
        if schedule.trigger == TriggerEnum.interval:
            BusinessReplicationValidator._check_interval_trigger(schedule)

    @staticmethod
    def _check_interval_trigger(schedule):
        """校验周期复制

        校验周期复制时：1. 开始时间不能为空；2. 校验周期单位与周期

        Args:
            schedule: 复制策略的调度方法
        """
        ParamsValidator.check_param_not_empty(
            schedule.start_time, "start_time")
        ParamsValidator.check_backup_value_and_unit(
            schedule.interval_unit, schedule.interval, ["interval"])

    @staticmethod
    def _check_external_system_id_exists_and_not_empty(ext_parameters):
        ParamsValidator.check_param_not_empty(
            ext_parameters.external_system_id, "external_system_id")

    @staticmethod
    def _check_retention_by_type(retention, schedule):
        if retention.retention_type == RetentionTypeEnum.permanent:
            BusinessReplicationValidator._check_permanent_retention(retention)
        if retention.retention_type == RetentionTypeEnum.temporary:
            BusinessReplicationValidator._check_temporary_retention(retention, schedule)

    @staticmethod
    def _check_permanent_retention(retention):
        ParamsValidator.check_param_empty(
            retention.retention_duration, "retention_duration")
        ParamsValidator.check_param_empty(
            retention.duration_unit, "duration_unit")

    @staticmethod
    def _check_temporary_retention(retention, schedule):
        ParamsValidator.check_param_not_empty(
            retention.retention_duration, "retention_duration")
        ParamsValidator.check_param_not_empty(
            retention.duration_unit, "duration_unit")
        if retention.retention_duration <= 0:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["retention_duration"])
        ParamsValidator.check_retention_value_and_unit(retention.duration_unit,
                                                       retention.retention_duration,
                                                       ["retention_duration"])
        # 校验备份周期不能大于保留时间
        BusinessReplicationValidator._check_frequency_and_retention(schedule, retention)

    @staticmethod
    def _check_frequency_and_retention(schedule, retention):
        ParamsValidator.check_frequency_and_retention(schedule.interval,
                                                      schedule.interval_unit,
                                                      retention.retention_duration,
                                                      retention.duration_unit)
