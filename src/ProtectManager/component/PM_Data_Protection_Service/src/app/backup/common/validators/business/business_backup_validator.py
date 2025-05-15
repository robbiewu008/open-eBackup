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
from app.backup.common.validators.business.trigger.interval_trigger_validator import IntervalTriggerValidator
from app.backup.common.validators.business.trigger.customize_trigger_validator import CustomizeTriggerValidator
from app.backup.common.validators.sla_validator import ParamsValidator
from app.common.enums.sla_enum import PolicyTypeEnum, RetentionTypeEnum, PolicyActionEnum, BackupTimeUnit, \
    TriggerEnum, IntervalUnit
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class BusinessBackupValidator:

    @staticmethod
    def do_validate(policy):
        if policy.type != PolicyTypeEnum.backup:
            return
        BusinessBackupValidator._validate_policy(policy)

    @staticmethod
    def _validate_policy(policy):
        schedule = policy.schedule
        retention = policy.retention
        action = policy.action

        BusinessBackupValidator._check_action_not_empty(action)
        BusinessBackupValidator._check_schedule_trigger_limit(schedule)
        BusinessBackupValidator._check_schedule_interval_unit(schedule)
        BusinessBackupValidator._check_full_backup_unit_not_minutes(action, schedule)
        BusinessBackupValidator._check_retention_by_type(retention)
        BusinessBackupValidator._check_schedule_by_trigger_type(schedule, retention)
        BusinessBackupValidator._check_log_interval_minutes_unit_greater_or_equal_to_five(action, schedule)
        BusinessBackupValidator._check_start_time_and_window_start_end_not_empty(action, schedule)

    @staticmethod
    def _check_action_not_empty(action):
        ParamsValidator.check_param_not_empty(action, "action")

    @staticmethod
    def _check_schedule_trigger_limit(schedule):
        if schedule.trigger not in [TriggerEnum.interval, TriggerEnum.customize_interval]:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["trigger"])

    @staticmethod
    def _check_schedule_interval_unit(schedule):
        if schedule.interval_unit == IntervalUnit.weeks:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["interval_unit"])

    @staticmethod
    def _check_full_backup_unit_not_minutes(action, schedule):
        ParamsValidator.check_backup_unit_by_action(action, schedule)

    @staticmethod
    def _check_schedule_by_trigger_type(schedule, retention):
        if schedule.trigger == TriggerEnum.interval:
            IntervalTriggerValidator.do_validate(schedule, retention)
        else:
            CustomizeTriggerValidator.do_validate(schedule)

    @staticmethod
    def _check_retention_by_type(retention):
        if retention.retention_type == RetentionTypeEnum.permanent:
            BusinessBackupValidator._check_permanent_retention_must_empty(retention)
        if retention.retention_type == RetentionTypeEnum.temporary:
            BusinessBackupValidator._check_temporary_retention_not_empty(retention)
            BusinessBackupValidator._check_retention_unit_not_out_of_range(retention)

    @staticmethod
    def _check_permanent_retention_must_empty(retention):
        ParamsValidator.check_param_empty(
            retention.retention_duration, "retention_duration")
        ParamsValidator.check_param_empty(
            retention.duration_unit, "duration_unit")

    @staticmethod
    def _check_temporary_retention_not_empty(retention):
        ParamsValidator.check_param_not_empty(
            retention.retention_duration, "retention_duration")
        if retention.retention_duration <= 0:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["retention_duration"])
        ParamsValidator.check_param_not_empty(
            retention.duration_unit, "duration_unit")

    @staticmethod
    def _check_retention_unit_not_out_of_range(retention):
        ParamsValidator.check_retention_value_and_unit(retention.duration_unit,
                                                       retention.retention_duration,
                                                       ["retention_duration"])

    @staticmethod
    def _check_log_interval_minutes_unit_greater_or_equal_to_five(action, schedule):
        if action == PolicyActionEnum.log and schedule.interval_unit == BackupTimeUnit.minutes:
            if schedule.interval < 5:
                raise IllegalParamException(
                    CommonErrorCodes.ILLEGAL_PARAMS, ["interval"])

    @staticmethod
    def _check_start_time_and_window_start_end_not_empty(action, schedule):
        if action == PolicyActionEnum.log:
            return
        start_datetime = schedule.start_time
        window_start = schedule.window_start
        window_end = schedule.window_end
        ParamsValidator.check_param_not_empty(window_start, "window_start")
        ParamsValidator.check_param_not_empty(window_end, "window_end")
        if schedule.trigger == TriggerEnum.customize_interval:
            return
        ParamsValidator.check_param_not_empty(start_datetime, "start_time")
