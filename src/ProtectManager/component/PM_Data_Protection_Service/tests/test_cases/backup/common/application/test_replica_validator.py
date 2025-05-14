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
from tests.test_cases.backup.common.context import mock_context  # noqa
from app.common.exception.unified_exception import EmeiStorBizException

patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.application.replica_validator import ReplicaCopyValidator
from app.backup.common.validators.unify_validator import UnifyValidator
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None,
                 name=None, type=None):
        self.schedule = schedule
        self.action = action
        self.type = type
        self.retention = retention
        self.ext_parameters = ext_parameters
        self.name = name


class TestReplicaCopyValidator(unittest.TestCase):
    def setUp(self) -> None:
        self.full = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.full)
        self.difference = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.difference_increment)
        self.cumulative = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.cumulative_increment)
        self.log = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.log)
        self.replication = PolicyMock(type=PolicyTypeEnum.replication, action=PolicyActionEnum.replication)
        self.archive = PolicyMock(type=PolicyTypeEnum.archiving, action=PolicyActionEnum.archiving)

    def test_do_validate_success(self):
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_backup_policies_ext_parameters_must_equal") \
                        as mock_ext_parameters_equal:
            ret = ReplicaCopyValidator.do_validate([self.archive])
            self.assertIsNone(ret)

    def test_should_raise_EmeiStorBizException_if_replica_can_not_create_full_when_create_sla(self):
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            with self.assertRaises(EmeiStorBizException):
                ReplicaCopyValidator.do_validate([self.full, self.archive])

    def test_should_raise_EmeiStorBizException_if_replica_action_count_greater_than_support_when_create_sla(
            self):
        archive_policies = [self.archive for i in range(5)]
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            with self.assertRaises(EmeiStorBizException):
                ReplicaCopyValidator.do_validate(archive_policies)


if __name__ == '__main__':
    unittest.main(verbosity=2)
