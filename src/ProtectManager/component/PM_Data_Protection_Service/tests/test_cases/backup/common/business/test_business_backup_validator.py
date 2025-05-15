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
import unittest
from unittest import mock
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context # noqa
from tests.test_cases.tools import functiontools, timezone

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.business.business_backup_validator import BusinessBackupValidator
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, TriggerEnum, BackupTimeUnit, RetentionTypeEnum, \
    RetentionTimeUnit, TriggerActionEnum


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None):
        self.schedule = schedule
        self.action = action
        self.type = PolicyTypeEnum.backup.value
        self.retention = retention


class ScheduleMock:
    def __init__(self, trigger, interval_unit=None, interval=None, start_time=None,
                 window_start=None, window_end=None, trigger_action=None,
                 days_of_year=None, days_of_month=None, days_of_week=None):
        self.trigger = trigger
        self.interval_unit = interval_unit
        self.interval = interval
        self.start_time = start_time
        self.window_start = window_start
        self.window_end = window_end
        self.days_of_year = days_of_year
        self.days_of_month = days_of_month
        self.days_of_week = days_of_week
        self.trigger_action = trigger_action


class RetentionMock:
    def __init__(self, retention_type, duration_unit=None, retention_duration=None):
        self.retention_type = retention_type
        self.duration_unit = duration_unit
        self.retention_duration = retention_duration


class TestBusinessBackupValidator(unittest.TestCase):
    def setUp(self) -> None:
        self.retention = RetentionMock(RetentionTypeEnum.temporary,
                                       RetentionTimeUnit.days,
                                       3)
        self.schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                     interval_unit=BackupTimeUnit.days,
                                     interval=2, start_time="2021-08-28 09:41:01", window_start="00:00:01",
                                     window_end="23:00:59")
        self.policy = PolicyMock(PolicyActionEnum.full, schedule=self.schedule, retention=self.retention)

    """
    用例场景：SLA备份策略业务参数正常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略参数正常，则能够校验通过  
    """
    def test_do_validate_success(self):
        with mock.patch("app.backup.common.validators.business.business_backup_validator"
                        ".BusinessBackupValidator._check_start_time_and_window_start_end_not_empty") \
                as mock_last_function:
            BusinessBackupValidator._validate_policy(self.policy)
            mock_last_function.assert_called_with(self.policy.action, self.policy.schedule)
            self.assertIsNotNone(BusinessBackupValidator)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略action为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_policy_action_is_empty_when_create_sla(self):
        policy = PolicyMock()
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略调度如果不是周期性或指定时间，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_schedule_trigger_not_interval_or_customize_when_create_sla(self):
        self.policy.schedule.trigger = TriggerEnum.backup_complete
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果全量备份调度单位为分钟，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_full_policy_schedule_unit_is_minutes_when_create_sla(self):
        self.policy.schedule.interval_unit = BackupTimeUnit.minutes
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果全量备份调度单位超过了范围，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_policy_schedule_unit_out_of_range_when_create_sla(self):
        self.policy.schedule.interval = 366
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果永久保留的retention不为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_policy_permanent_retention_not_empty_when_create_sla(self):
        self.policy.retention.retention_type = RetentionTypeEnum.permanent
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果临时保留的retention为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_policy_temporary_retention_empty_when_create_sla(self):
        empty_retention = RetentionMock(RetentionTypeEnum.temporary)
        self.policy.retention = empty_retention
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果临时保留的单位超出范围，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_policy_temporary_retention_unit_out_of_range_when_create_sla(self):
        illegal_retention = RetentionMock(RetentionTypeEnum.temporary,
                                          RetentionTimeUnit.days,
                                          366)
        self.policy.retention = illegal_retention
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果临时保留时，备份频率大于保留时间，不能创建SLA 
    """
    def test_should_raise_EmeiStorBizException_if_policy_temporary_interval_gt_duration_when_create_sla(self):
        schedule = ScheduleMock(TriggerEnum.interval,
                                BackupTimeUnit.days,
                                2)
        legal_retention = RetentionMock(RetentionTypeEnum.temporary,
                                        RetentionTimeUnit.days,
                                        1)
        self.policy.schedule = schedule
        self.policy.retention = legal_retention
        with self.assertRaises(EmeiStorBizException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定时间的trigger action为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_customize_interval_trigger_action_is_empty_when_create_sla(self):
        retention = RetentionMock(RetentionTypeEnum.temporary,
                                  RetentionTimeUnit.days,
                                  3)
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval,
                                days_of_month="15", window_start="00:00:01",
                                window_end="23:00:59")
        self.policy.schedule = schedule
        self.policy.retention = retention
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定每周备份但是days_of_week字段为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_week_customize_interval_days_of_week_is_empty_when_create_sla(self):
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.week)
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定每月备份但是days_of_month字段为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_month_customize_interval_days_of_month_is_empty_when_create_sla(
            self):
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.month)
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定每年备份但是days_of_year字段为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_year_customize_interval_days_of_month_is_empty_when_create_sla(self):
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.year)
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定每周备份但是days_of_week字段存在重复元素，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_week_customize_interval_days_of_week_duplicate_when_create_sla(self):
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.week,
                                days_of_week="1,1")
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果指定每月备份但是days_of_month字段存在重复元素，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_month_customize_interval_days_of_month_duplicate_when_create_sla(
            self):
        schedule = ScheduleMock(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.month,
                                days_of_month="1,1,3")
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果window_time或者window_end为空，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_window_time_empty_when_create_sla(self):
        schedule = ScheduleMock(TriggerEnum.interval,
                                BackupTimeUnit.minutes,
                                6, start_time="2021-08-28T09:41:01")
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略调度周期为年，同时包含interval，不能创建SLA
    """
    def test_should_raise_IllegalParamException_if_interval_not_empty_when_create_sla_trigger_action_year(self):
        schedule = ScheduleMock(TriggerEnum.customize_interval,
                                interval=1,
                                trigger_action=TriggerActionEnum.year)
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略调度周期为年，同时包含interval_unit不能创建SLA
    """
    def test_should_raise_IllegalParamException_if_interval_unit_not_empty_when_create_sla_trigger_action_year(self):
        schedule = ScheduleMock(TriggerEnum.customize_interval,
                                interval_unit=BackupTimeUnit.minutes.value,
                                trigger_action=TriggerActionEnum.year)
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)

    """
    用例场景：SLA备份策略参数异常，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略调度周期为年，同时包含start_time，能创建SLA
    """
    def test_should_raise_IllegalParamException_if_start_time_not_empty_when_create_sla_trigger_action_year(self):
        schedule = ScheduleMock(TriggerEnum.customize_interval,
                                start_time="2021-08-28T09:41:01",
                                window_start="09:41:01",
                                window_end="09:50:01",
                                days_of_year="2021-08-28",
                                trigger_action=TriggerActionEnum.year)
        self.policy.schedule = schedule
        BusinessBackupValidator._validate_policy(self.policy)
        self.assertIsNotNone(schedule)

    """
    用例场景：SLA周期备份单位为周，则抛出异常
    前置条件：pm业务正常运行
    检查点：  SLA备份策略如果interval_unit为week，不能创建SLA 
    """
    def test_should_raise_IllegalParamException_if_interval_unit_is_week_when_create_sla(self):
        schedule = ScheduleMock(TriggerEnum.interval,
                                BackupTimeUnit.weeks,
                                6, start_time="2021-08-28T09:41:01")
        self.policy.schedule = schedule
        with self.assertRaises(IllegalParamException):
            BusinessBackupValidator._validate_policy(self.policy)


if __name__ == '__main__':
    unittest.main(verbosity=2)
