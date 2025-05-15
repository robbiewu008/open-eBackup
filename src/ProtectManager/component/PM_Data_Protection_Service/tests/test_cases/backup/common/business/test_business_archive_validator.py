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
from app.backup.common.validators.business.business_archive_validator import BusinessArchiveValidator
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, TriggerEnum, BackupTimeUnit, RetentionTypeEnum, \
    RetentionTimeUnit


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None):
        self.schedule = schedule
        self.action = action
        self.type = PolicyTypeEnum.archiving.value
        self.retention = retention
        self.ext_parameters = ext_parameters


class ScheduleMock:
    def __init__(self, trigger, interval_unit=None, interval=None,
                 window_start=None, window_end=None):
        self.trigger = trigger
        self.interval_unit = interval_unit
        self.interval = interval
        self.window_start = window_start
        self.window_end = window_end


class RetentionMock:
    def __init__(self, retention_type, duration_unit=None, retention_duration=None):
        self.retention_type = retention_type
        self.duration_unit = duration_unit
        self.retention_duration = retention_duration


ExtParametersMock = dict(storage_id="test_storage_id")
EmptyExtParametersMock = dict(storage_id=None)


class TestBusinessArchiveValidator(unittest.TestCase):
    def test_do_validate_success(self):
        ext_parameters = ExtParametersMock
        schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                interval_unit=BackupTimeUnit.weeks,
                                interval=2)
        retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                  duration_unit=RetentionTimeUnit.weeks,
                                  retention_duration=3)
        policy = PolicyMock(action=PolicyActionEnum.archiving, schedule=schedule,
                            retention=retention, ext_parameters=ext_parameters)
        with mock.patch("app.backup.common.validators.business.business_archive_validator"
                        ".BusinessArchiveValidator._check_retention_by_type") \
                as mock_last_function:
            BusinessArchiveValidator._validate_policy(policy)
            mock_last_function.assert_called_with(policy.retention, policy.schedule)
            self.assertIsNotNone(BusinessArchiveValidator)

    def test_should_raise_IllegalParamException_if_policy_schedule_trigger_limit_illegal_when_create_sla(self):
        schedule = ScheduleMock(trigger="test_illegal_trigger")
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_schedule_trigger_limit(schedule)

    def test_should_raise_IllegalParamException_if_policy_ext_params_storage_id_is_empty_when_create_sla(self):
        ext_parameters = EmptyExtParametersMock
        policy = PolicyMock(ext_parameters=ext_parameters)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_storage_id_exists_and_not_empty(policy.ext_parameters)

    def test_should_raise_IllegalParamException_if_policy_permanent_retention_not_empty_when_create_sla(self):
        retention = RetentionMock(retention_type=RetentionTypeEnum.permanent,
                                  duration_unit=RetentionTimeUnit.weeks,
                                  retention_duration=3)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_permanent_retention(retention)

    def test_should_raise_IllegalParamException_if_policy_temporary_retention_empty_when_create_sla(self):
        empty_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary)
        schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                interval_unit=BackupTimeUnit.days,
                                interval=2)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_temporary_retention(empty_retention, schedule)

    def test_should_raise_IllegalParamException_if_policy_temporary_retention_duration_le_0_when_create_sla(self):
        illegal_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                          duration_unit=RetentionTimeUnit.days,
                                          retention_duration=-1)
        schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                interval_unit=BackupTimeUnit.days,
                                interval=2)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_temporary_retention(illegal_retention, schedule)

    def test_should_raise_EmeiStorBizException_if_policy_temporary_interval_gt_duration_when_create_sla(self):
        schedule = ScheduleMock(trigger=TriggerEnum.interval,
                                interval_unit=BackupTimeUnit.days,
                                interval=2)
        legal_retention = RetentionMock(retention_type=RetentionTypeEnum.temporary,
                                        duration_unit=RetentionTimeUnit.days,
                                        retention_duration=1)
        with self.assertRaises(EmeiStorBizException):
            BusinessArchiveValidator._check_temporary_retention(legal_retention, schedule)

    def test_should_raise_IllegalParamException_if_policy_after_backup_complete_trigger_out_of_range_when_create_sla(
            self):
        schedule = ScheduleMock(trigger=TriggerEnum.after_backup_complete,
                                interval_unit=BackupTimeUnit.days,
                                interval=366)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_after_backup_trigger(schedule.interval)

    def test_should_raise_IllegalParamException_if_policy_interval_trigger_out_of_range_when_create_sla(self):
        schedule = ScheduleMock(trigger=TriggerEnum.after_backup_complete,
                                interval_unit=BackupTimeUnit.weeks,
                                interval=5)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_interval_trigger(schedule)

    def test_should_raise_IllegalParamException_if_tape_archiving_policy_contains_temporary_retention(self):
        """
        *用例场景：校验磁带归档如果存在临时保留，则抛出异常
        *前置条件：无
        *检查点: 磁带归档不能包含临时保留
        """
        ext_params = {"protocol": 7}
        retention = RetentionMock(RetentionTypeEnum.temporary, "d", 1)
        with self.assertRaises(IllegalParamException):
            BusinessArchiveValidator._check_tape_archive_has_no_temporary_retention(ext_params, retention)


if __name__ == '__main__':
    unittest.main(verbosity=2)
