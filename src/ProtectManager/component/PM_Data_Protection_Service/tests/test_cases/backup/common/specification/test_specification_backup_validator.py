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
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context # noqa
from app.backup.common.validators.specification.specification_backup_validator import SpecificationBackupValidator
from app.common.exception.unified_exception import IllegalParamException

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None):
        self.schedule = schedule
        self.action = action
        self.type = PolicyTypeEnum.backup.value
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


class ExtParameters:
    def __init__(self, channel_number):
        self.channel_number = channel_number


class TestBackupValidator(unittest.TestCase):
    def test_do_validate_success(self):
        first_ext_parameters = ExtParameters(channel_number=4)
        second_ext_parameters = ExtParameters(channel_number=4)
        full_backup_policy = PolicyMock(action=PolicyActionEnum.full,
                                        ext_parameters=first_ext_parameters)
        cumulative_increment_backup_policy = PolicyMock(action=PolicyActionEnum.cumulative_increment,
                                                        ext_parameters=second_ext_parameters)
        backup_polices = [full_backup_policy, cumulative_increment_backup_policy]
        SpecificationBackupValidator.do_validate(backup_polices)
        self.assertIsNotNone(backup_polices)

    def test_should_raise_IllegalParamException_if_has_backup_policy_and_count_le_0_when_create_sla(self):
        backup_polices = []
        with self.assertRaises(IllegalParamException):
            SpecificationBackupValidator._check_backup_policies_count_not_empty(backup_polices)


if __name__ == '__main__':
    unittest.main(verbosity=2)
