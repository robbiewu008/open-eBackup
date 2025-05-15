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
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context # noqa
from app.backup.common.validators.specification.specification_archive_validator import SpecificationArchiveValidator
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None,
                 name=None):
        self.schedule = schedule
        self.action = action
        self.type = PolicyTypeEnum.archiving.value
        self.retention = retention
        self.ext_parameters = ext_parameters
        self.name = name


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
    def __init__(self, channel_number=None, storage_id=None):
        self.channel_number = channel_number
        self.storage_id = storage_id


class TestArchiveValidator(unittest.TestCase):
    def test_do_validate_success(self):
        first_ext_parameters = ExtParameters(storage_id="first_storage_id")
        second_ext_parameters = ExtParameters(storage_id="second_storage_id")
        first_policy = PolicyMock(name="first",
                                  ext_parameters=first_ext_parameters,
                                  action=PolicyActionEnum.archiving)
        second_policy = PolicyMock(name="second",
                                   ext_parameters=second_ext_parameters,
                                   action=PolicyActionEnum.archiving)
        archive_polices = [first_policy, second_policy]
        with mock.patch(
                "app.backup.common.validators.specification.specification_archive_validator"
                ".SpecificationArchiveValidator._check_archive_policies_storage_id_not_repeat") \
                as mock_last_function:
            SpecificationArchiveValidator.do_validate(archive_polices)
            mock_last_function.assert_called_once()
            self.assertIsNotNone(SpecificationArchiveValidator)

    def test_should_raise_EmeiStorBizException_if_archive_policy_count_out_of_range_when_create_sla(self):
        # 最多可创建4个归档策略
        archive_polices = [PolicyMock() for i in range(5)]
        with self.assertRaises(EmeiStorBizException):
            SpecificationArchiveValidator._check_archive_policies_count_lt_limit(archive_polices)

    def test_should_raise_IllegalParamException_archive_policy_name_replicate_when_create_sla(self):
        first_name_replicate_policy = PolicyMock(name="first", action=PolicyActionEnum.archiving)
        second_name_replicate_policy = PolicyMock(name="first", action=PolicyActionEnum.archiving)
        archive_polices = [first_name_replicate_policy, second_name_replicate_policy]
        with self.assertRaises(IllegalParamException):
            SpecificationArchiveValidator._check_archive_policies_name_not_repeat(archive_polices)

    def test_should_raise_IllegalParamException_archive_policy_storage_id_replicate_when_create_sla(self):
        first_ext_parameters = ExtParameters(storage_id="test_storage_id")
        second_ext_parameters = ExtParameters(storage_id="test_storage_id")
        first_storage_id_replicate_policy = PolicyMock(name="first",
                                                       ext_parameters=first_ext_parameters)
        second_storage_id_replicate_policy = PolicyMock(name="second",
                                                        ext_parameters=second_ext_parameters)
        archive_polices = [first_storage_id_replicate_policy,
                           second_storage_id_replicate_policy]
        with self.assertRaises(IllegalParamException):
            SpecificationArchiveValidator._check_archive_policies_storage_id_not_repeat(archive_polices)


if __name__ == '__main__':
    unittest.main(verbosity=2)
