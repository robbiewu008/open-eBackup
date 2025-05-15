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
patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.business.business_policies_validator import BusinessPoliciesValidator
from app.common.exception.unified_exception import EmeiStorBizException

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, TriggerEnum, BackupTimeUnit, RetentionTypeEnum, \
    RetentionTimeUnit


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


class RetentionMock:
    def __init__(self, retention_type, duration_unit=None, retention_duration=None):
        self.retention_type = retention_type
        self.duration_unit = duration_unit
        self.retention_duration = retention_duration


class ExtParametersMock:
    def __init__(self, external_system_id=None):
        self.external_system_id = external_system_id


class TestBusinessPoliciesValidator(unittest.TestCase):
    def test_do_validate_success(self):
        full_action_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                              duration_unit=RetentionTimeUnit.days,
                                              retention_duration=3)
        full_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                            interval=1,
                                            interval_unit=BackupTimeUnit.days,
                                            start_time="2021-08-19", window_start="02:34:29")
        full_action_policy = PolicyMock(action=PolicyActionEnum.full,
                                        schedule=full_action_schedule,
                                        retention=full_action_retention)

        cumulative_action_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                                    duration_unit=RetentionTimeUnit.days,
                                                    retention_duration=4)
        cumulative_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                                  interval=1,
                                                  interval_unit=BackupTimeUnit.days,
                                                  start_time="2021-08-19", window_start="07:34:39")
        cumulative_action_policy = PolicyMock(action=PolicyActionEnum.cumulative_increment,
                                              schedule=cumulative_action_schedule,
                                              retention=cumulative_action_retention)

        replication_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                                   interval_unit=BackupTimeUnit.days,
                                                   interval=2)
        replication_action_policy = PolicyMock(action=PolicyActionEnum.replication,
                                               type=PolicyTypeEnum.replication,
                                               schedule=replication_action_schedule)
        policies = [full_action_policy, cumulative_action_policy, replication_action_policy]
        with mock.patch(
                "app.backup.common.validators.business.business_policies_validator.BusinessPoliciesValidator"
                "._check_backup_shortest_retention_greater_than_replication_longest_frequency") \
                as mock_last_function:
            BusinessPoliciesValidator.do_validate(policies)
            mock_last_function.assert_called_once()
            self.assertIsNotNone(BusinessPoliciesValidator)

    def test_should_raise_EmeiStorBizException_if_increment_start_time_before_full_when_create_sla(self):
        cumulative_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                                  start_time="2021-08-18", window_start="01:34:39")
        full_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                            start_time="2021-08-18", window_start="02:34:29")
        cumulative_action_policy = PolicyMock(action=PolicyActionEnum.cumulative_increment,
                                              schedule=cumulative_action_schedule)
        full_action_policy = PolicyMock(action=PolicyActionEnum.full,
                                        schedule=full_action_schedule)
        backup_policies = [full_action_policy, cumulative_action_policy]
        with self.assertRaises(EmeiStorBizException):
            BusinessPoliciesValidator._check_backup_full_start_time_before_increment(backup_policies)

    def test_should_raise_EmeiStorBizException_if_backup_policy_retention_duration_le_replication_when_create_sla(self):
        full_action_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                              duration_unit=RetentionTimeUnit.days,
                                              retention_duration=1)
        cumulative_action_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                                    duration_unit=RetentionTimeUnit.days,
                                                    retention_duration=3)
        replication_action_schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                                   interval_unit=BackupTimeUnit.days,
                                                   interval=2)
        full_action_policy = PolicyMock(action=PolicyActionEnum.full,
                                        retention=full_action_retention)
        cumulative_action_policy = PolicyMock(action=PolicyActionEnum.cumulative_increment,
                                              retention=cumulative_action_retention)
        replication_action_policy = PolicyMock(action=PolicyActionEnum.replication,
                                               type=PolicyTypeEnum.replication,
                                               schedule=replication_action_schedule)
        backup_policies = [full_action_policy, cumulative_action_policy]
        replication_policies = [replication_action_policy]
        with self.assertRaises(EmeiStorBizException):
            BusinessPoliciesValidator._check_backup_shortest_retention_greater_than_replication_longest_frequency(
                backup_policies, replication_policies)


if __name__ == '__main__':
    unittest.main(verbosity=2)
