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
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context # noqa
patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.business.business_log_backup_validator import BusinessLogBackupValidator
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum, BackupTimeUnit, TriggerEnum
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None,
                 type=PolicyTypeEnum.backup.value):
        self.schedule = schedule
        self.action = action
        self.type = type
        self.retention = retention
        self.ext_parameters = ext_parameters


class ScheduleMock:
    def __init__(self, trigger=None, interval_unit=None, interval=None,
                 start_time=None, window_start=None, window_end=None):
        self.trigger = trigger
        self.interval_unit = interval_unit
        self.interval = interval
        self.start_time = start_time
        self.window_start = window_start
        self.window_end = window_end


class TestBusinessLogBackupValidator(unittest.TestCase):
    def test_should_raise_IllegalParamException_if_log_minutes_lt_5_when_create_sla(self):
        schedule = ScheduleMock(TriggerEnum.interval,
                                BackupTimeUnit.minutes,
                                3)
        policy = PolicyMock(PolicyActionEnum.log, schedule)
        with self.assertRaises(IllegalParamException):
            BusinessLogBackupValidator.check_log_interval_minutes_unit_greater_or_equal_to_five([policy])

    def test_should_raise_IllegalParamException_if_oracle_sla_backup_start_time_after_log_when_create_sla(self):
        backup_schedule = ScheduleMock(trigger=TriggerEnum.interval, interval=18, start_time="2021-03-14",
                                       window_start="21:00:00")
        log_schedule = ScheduleMock(trigger=TriggerEnum.interval, interval=17, start_time="2021-03-14 20:00:00")
        backup_policy = PolicyMock(type=PolicyTypeEnum.backup,
                                   action=PolicyActionEnum.full,
                                   schedule=backup_schedule)
        log_policy = PolicyMock(type=PolicyTypeEnum.backup,
                                action=PolicyActionEnum.log,
                                schedule=log_schedule)
        backup_policies = [backup_policy, log_policy]
        with self.assertRaises(EmeiStorBizException) as ex:
            BusinessLogBackupValidator.check_policy_backup_start_time_before_log(backup_policies)

    def test_should_raise_IllegalParamException_if_oracle_sla_backup_schedule_interval_less_than_log_when_create_sla(self):
        backup_schedule = ScheduleMock(interval=1, interval_unit=BackupTimeUnit.days, start_time="21:00:00")
        log_schedule = ScheduleMock(interval=2, interval_unit=BackupTimeUnit.days, start_time="22:00:00")
        backup_policy = PolicyMock(type=PolicyTypeEnum.backup,
                                   action=PolicyActionEnum.full,
                                   schedule=backup_schedule)
        log_policy = PolicyMock(type=PolicyTypeEnum.backup,
                                action=PolicyActionEnum.log,
                                schedule=log_schedule)
        backup_policies = [backup_policy, log_policy]
        with self.assertRaises(IllegalParamException) as ex:
            BusinessLogBackupValidator.check_policy_backup_schedule_interval_greater_than_log(backup_policies)
        message = "The log backup frequency must be less than other backup frequencies"
        self.assertEqual(message, ex.exception.args[2])


if __name__ == '__main__':
    unittest.main(verbosity=2)
