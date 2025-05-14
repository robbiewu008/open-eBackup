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
from datetime import datetime, timedelta
from typing import List

from app.common import logger
from app.common.enums.sla_enum import BackupTimeUnit, PolicyTypeEnum, RetentionTimeUnit, WindowTimeUnit, \
    time_interval_switcher, PolicyActionEnum, RetentionTypeEnum, MonthDayEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException

from app.backup.common.validators.validator_manager import ValidatorManager

log = logger.get_logger(__name__)


def convert_time_to_second(value) -> int:
    return value.hour * 60 * 60 + value.minute * 60 + value.second


def check_unit_minutes(value, *params):
    if value < 1 or value > 59:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_log_minutes(value, *params):
    if value < 5:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_unit_hours(value, *params):
    if value < 1 or value > 23:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_time_window_unit_hours(value, *params):
    if value < 1 or value > 24:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_unit_days(value, *params):
    if value < 1 or value > 365:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_unit_weeks(value, *params):
    if value < 1 or value > 4:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_unit_months(value, *params):
    if value < 1 or value > 12:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_unit_years(value, *params):
    if value < 1 or value > 5:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_retention_unit_days(value, *params):
    if value < 1 or value > 365:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_retention_unit_weeks(value, *params):
    if value < 1 or value > 54:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_retention_unit_months(value, *params):
    if value < 1 or value > 24:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


def check_retention_unit_years(value, *params):
    if value < 1 or value > 10:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)


backup_time_check_switcher = {
    BackupTimeUnit.minutes: check_unit_minutes,
    BackupTimeUnit.hours: check_unit_hours,
    BackupTimeUnit.days: check_unit_days,
    BackupTimeUnit.weeks: check_unit_weeks,
}

time_window_check_switcher = {
    WindowTimeUnit.hours: check_time_window_unit_hours,
    WindowTimeUnit.minutes: check_unit_minutes
}

retention_time_check_switcher = {
    RetentionTimeUnit.days: check_retention_unit_days,
    RetentionTimeUnit.weeks: check_retention_unit_weeks,
    RetentionTimeUnit.months: check_retention_unit_months,
    RetentionTimeUnit.years: check_retention_unit_years
}


class ParamsValidator(object):

    @staticmethod
    def parse_time_str(str_time: str, date_time: datetime) -> datetime:
        str_hour, str_minute, str_second = str_time.split(':')
        current_date = date_time
        return current_date.replace(hour=int(str_hour), minute=int(str_minute), second=int(str_second))

    @staticmethod
    def check_backup_unit_by_action(action, schedule):
        if action == PolicyActionEnum.full and schedule.interval_unit == BackupTimeUnit.minutes:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["interval_unit"])

    @staticmethod
    def check_after_backup_complete_value(value, *params):
        if value < 1 or value > 365:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, *params)

    @staticmethod
    def check_backup_value_and_unit(time_unit, value, *params):
        if time_unit.value in backup_time_check_switcher:
            backup_time_check_switcher[time_unit.value](value, *params)

    @staticmethod
    def check_window_value_and_unit(time_unit, value, *params):
        if time_unit.value in time_window_check_switcher:
            time_window_check_switcher[time_unit.value](value, *params)

    @staticmethod
    def check_retention_value_and_unit(time_unit, value, *params):
        if time_unit.value in retention_time_check_switcher:
            retention_time_check_switcher[time_unit.value](value, *params)

    @staticmethod
    def check_frequency_and_retention(interval, interval_unit, retention, retention_unit):
        """
        校验操作频率和保留时间，备份/归档/复制周期必须小于保留时间
        :param interval:执行周期
        :param interval_unit:执行周期单位
        :param retention:保留周期
        :param retention_unit:保留周期单位
        :return:
        """
        frequency_of_seconds = time_interval_switcher[interval_unit.value](
            interval)
        retention_of_seconds = time_interval_switcher[retention_unit.value](
            retention)
        if frequency_of_seconds >= retention_of_seconds:
            raise EmeiStorBizException(error=ProtectionErrorCodes.OPERATION_INTERVAL_CAN_NOT_MORE_THAN_RETENTION,
                                       parameters=[])

    @staticmethod
    def check_param_empty(value, param):
        if value:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, [param])

    @staticmethod
    def check_name_has_no_pre_and_tail_space(name, param_name: str):
        """ 检查名字的前后是否有空格

        Args:
            name: 被检查的SLA名字或Policy名字
            param_name: 传入参数的名字，小写加下划线的格式，如：“sla_name”，"policy_name"
        """
        if not name:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        [param_name])
        if name.strip() != name:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        [param_name])

    @staticmethod
    def check_param_not_empty(value, param):
        if not value:
            raise IllegalParamException(
                CommonErrorCodes.ILLEGAL_PARAMS, [param])

    @staticmethod
    def check_list_param_not_duplicated(value: List, param):
        if len(value) != len(set(value)):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, [param],
                                        message="The list param has duplicated value")

    @staticmethod
    def check_last_day_of_each_month_not_create_with_others(value: List, param):
        if len(value) == 1:
            return
        for days in value:
            if days == MonthDayEnum.last_day_of_each_month:
                raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, [param])

    @staticmethod
    def check_time_window(window_start, window_end, check_time) -> bool:
        if ParamsValidator.check_time_window_inner(window_start, window_end, check_time):
            return True
        # 给校验时间加 1min 容忍度
        return ParamsValidator.check_time_window_inner(window_start, window_end, check_time + timedelta(minutes=1))

    @staticmethod
    def check_time_window_inner(window_start, window_end, check_time):
        if window_start == window_end:
            return True
        current_time = check_time
        # 时间窗口开始时间添加日期
        windows_start_date = ParamsValidator.parse_time_str(window_start, current_time)
        windows_end_date = ParamsValidator.parse_time_str(window_end, current_time)
        log.info(
            f'before check time window windows_start_date={windows_start_date},'
            f'check_time={current_time},windows_end_date={windows_end_date}')
        # 如果check_time比windows_start_date和windows_end_date，则check_time+1天
        if current_time < windows_end_date and current_time < windows_start_date:
            current_time += timedelta(days=1)
        # 若开始时间晚于或等于结束时间，按跨天处理
        if windows_end_date <= windows_start_date:
            windows_end_date = windows_end_date + timedelta(days=1)
        if windows_start_date <= current_time < windows_end_date:
            return True
        else:
            log.info(
                f'after check time window windows_start_date={windows_start_date},'
                f'check_time={current_time},windows_end_date={windows_end_date}')
            return False

    @staticmethod
    def check_time_window_and_start_time(window_start, window_end, start_time):
        """
        校验首次开始时间是否在时间窗范围内
        1.跨天：结束时间小于开始时间：首次执行时间小于开始时间 且 大于等于结束时间，一定校验不通过
        2.不跨天：结束时间大于开始时间：首次执行时间小于开始时间 或 大于等于结束时间，一定校验不通过
        :param window_start: 时间窗开始
        :param window_end: 时间窗结束
        :param start_time: 首次开始时间
        :return:
        """
        if window_end <= start_time.time() < window_start:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["window_start", "window_end", "start_time"])
        elif window_end > window_start:
            if start_time.time() < window_start or start_time.time() >= window_end:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["window_start", "window_end", "start_time"])

    @staticmethod
    def check_retention_and_frequency_by_action(policy_list):
        """
        检查增量备份或差异备份或全量备份的保留时间必须大于复制的周期时间
        :param policy_list:
        :return:
        """
        backup_type = {PolicyActionEnum.full.value, PolicyActionEnum.difference_increment.value,
                       PolicyActionEnum.cumulative_increment.value}
        replication_type = PolicyActionEnum.replication
        backup_policy_list = [policy for policy in policy_list if policy.action in backup_type]
        replication_policy_list = [policy for policy in policy_list if policy.action is replication_type]
        # 是否有的策略里面没有复制策略，没有就不用比较
        if replication_policy_list is not None and len(replication_policy_list) > 0:
            min_retention = ParamsValidator.get_backup_shortest_retention(backup_policy_list)
            log.info(f"min retention: {min_retention}")
            max_frequency = ParamsValidator.get_replication_longest_frequency(replication_policy_list)
            log.info(f"max frequency: {max_frequency}")
            # 备份保留时间中的最小值 必须大于 复制周期中的最大值
            if min_retention and max_frequency and (min_retention <= max_frequency):
                raise EmeiStorBizException(error=ProtectionErrorCodes.OPERATION_INTERVAL_CAN_NOT_MORE_THAN_RETENTION)

    @staticmethod
    def get_backup_shortest_retention(backup_policy_list):
        retention_list = []
        for policy in backup_policy_list:
            if policy.type == PolicyTypeEnum.backup.value:
                retention = policy.retention
                retention_unit = retention.duration_unit
                if retention.retention_type == RetentionTypeEnum.temporary:
                    retention_of_seconds = time_interval_switcher[retention_unit.value](retention.retention_duration)
                    retention_list.append(retention_of_seconds)
        min_retention = min(retention_list) if len(retention_list) > 0 else None
        return min_retention

    @staticmethod
    def get_replication_longest_frequency(replication_policy_list):
        frequency_list = []
        for policy in replication_policy_list:
            if policy.type == PolicyTypeEnum.replication.value:
                schedule = policy.schedule
                interval = schedule.interval
                interval_unit = schedule.interval_unit
                frequency_of_seconds = time_interval_switcher[interval_unit.value](interval)
                frequency_list.append(frequency_of_seconds)
        max_frequency = max(frequency_list) if len(frequency_list) > 0 else None
        return max_frequency


manager = ValidatorManager()
