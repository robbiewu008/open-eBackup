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
import re

from app.backup.common.validators.sla_validator import ParamsValidator
from app.common.enums.sla_enum import TriggerActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class CustomizeTriggerValidator:
    @staticmethod
    def do_validate(schedule):
        CustomizeTriggerValidator._check_schedule_trigger(schedule)

    @staticmethod
    def check_schedule_by_trigger_action(schedule):
        trigger_action = schedule.get("trigger_action")
        if trigger_action == TriggerActionEnum.year and not schedule.get("days_of_year"):
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["days_of_year"])
        if trigger_action == TriggerActionEnum.month and not schedule.get("days_of_month"):
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["days_of_month"])
        if trigger_action == TriggerActionEnum.week and not schedule.get("days_of_week"):
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, ["days_of_week"])

    @staticmethod
    def _check_schedule_trigger(schedule):
        ParamsValidator.check_param_not_empty(
            schedule.trigger_action, "trigger_action")
        CustomizeTriggerValidator._check_backup_customize_year_unusual_param(schedule, schedule.trigger_action)
        if schedule.trigger_action == TriggerActionEnum.week:
            CustomizeTriggerValidator._check_backup_customize_days_of_week_trigger(
                schedule)
        # 对按月选择的时间进行校验，不能重、不能超过当月的最大值
        if schedule.trigger_action == TriggerActionEnum.month:
            CustomizeTriggerValidator._check_backup_customize_days_of_month_trigger(
                schedule)
        if schedule.trigger_action == TriggerActionEnum.year:
            CustomizeTriggerValidator._check_backup_customize_days_of_year_trigger(
                schedule)

    @staticmethod
    def _check_backup_customize_days_of_week_trigger(schedule):
        ParamsValidator.check_param_not_empty(schedule.days_of_week, "days_of_week")
        list_days_of_week = schedule.days_of_week.split(",")
        ParamsValidator.check_list_param_not_duplicated(list_days_of_week, "days_of_week")

    @staticmethod
    def _check_backup_customize_days_of_month_trigger(schedule):
        # 先校验None 再校验 格式 再校验值
        ParamsValidator.check_param_not_empty(schedule.days_of_month, "days_of_month")
        pattern_res = re.search(r'(\d+)(,\d+)*', schedule.days_of_month)
        if not pattern_res:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, [
                "days_of_month format is wrong"])
        list_days_of_month = schedule.days_of_month.split(",")
        ParamsValidator.check_list_param_not_duplicated(list_days_of_month, "days_of_month")
        ParamsValidator.check_last_day_of_each_month_not_create_with_others(list_days_of_month, "days_of_month")
        for i in list_days_of_month:
            if int(i) not in range(1, 33):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, [
                    "days_of_month has illegal element"])

    @staticmethod
    def _check_backup_customize_days_of_year_trigger(schedule):
        ParamsValidator.check_param_not_empty(schedule.days_of_year, "days_of_year")

    @staticmethod
    def _check_backup_customize_year_unusual_param(schedule, trigger_action):
        ParamsValidator.check_param_empty(schedule.interval, "interval")
        ParamsValidator.check_param_empty(schedule.interval_unit, "interval_unit")
        if trigger_action != TriggerActionEnum.year:
            # 年周期性备份会生成start_time，导致克隆时校验不过，因此跳过此项检查。
            ParamsValidator.check_param_empty(schedule.start_time, "start_time")
