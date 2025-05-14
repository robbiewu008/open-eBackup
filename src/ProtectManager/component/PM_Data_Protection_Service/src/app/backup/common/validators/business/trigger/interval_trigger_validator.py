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
from app.common.enums.sla_enum import RetentionTypeEnum


class IntervalTriggerValidator:
    @staticmethod
    def do_validate(schedule, retention):
        IntervalTriggerValidator._check_schedule_trigger(schedule, retention)

    @staticmethod
    def _check_schedule_trigger(schedule, retention):
        IntervalTriggerValidator._check_backup_schedule_unit_not_out_of_range(schedule)
        IntervalTriggerValidator._check_backup_schedule_interval_less_than_retention(schedule, retention)

    @staticmethod
    def _check_backup_schedule_unit_not_out_of_range(schedule):
        ParamsValidator.check_param_not_empty(schedule.interval_unit, "interval unit")
        ParamsValidator.check_param_not_empty(schedule.interval, "interval")
        # customize调度没有interval
        ParamsValidator.check_backup_value_and_unit(
            schedule.interval_unit, schedule.interval, ["interval"])

    @staticmethod
    def _check_backup_schedule_interval_less_than_retention(schedule, retention):
        if retention.retention_type == RetentionTypeEnum.temporary:
            ParamsValidator.check_frequency_and_retention(schedule.interval, schedule.interval_unit,
                                                          retention.retention_duration, retention.duration_unit)
