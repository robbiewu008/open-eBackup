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
from app.backup.common.validators.application.common_validator import CommonValidator
from app.backup.common.validators.config.sla_config import SlaSpecificationConfig
from app.backup.common.validators.unify_validator import UnifyValidator
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum, TriggerEnum, RetentionTypeEnum, \
    RetentionTimeUnit, IntervalUnit
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException


class PolicyMock:
    def __init__(self, action=None, schedule=None, retention=None, ext_parameters=None,
                 name=None, type=None):
        self.schedule = schedule
        self.action = action
        self.type = type
        self.retention = retention
        self.ext_parameters = ext_parameters
        self.name = name
        self.schedule = schedule


class ExtParametersMock:
    def __init__(self, qos_id):
        self.qos_id = qos_id


class ScheduleMock:
    def __init__(self, start_time="00:01:00"):
        self.trigger = TriggerEnum.interval
        self.interval_unit = IntervalUnit.hours
        self.interval = TriggerEnum.interval
        self.start_time = "2021-12-05"
        self.window_start = start_time
        self.window_end = "03:01:06"


class RetentionMock:
    def __init__(self):
        self.retention_type = RetentionTypeEnum.temporary
        self.retention_duration = 3
        self.duration_unit = RetentionTimeUnit.days


class TestUnifyValidator(unittest.TestCase):
    def setUp(self) -> None:
        self.sla_config_builder = SlaSpecificationConfig.Builder() \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.full, 1) \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment, 1)\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.log, 1) \
            .config(PolicyTypeEnum.replication, PolicyActionEnum.replication, 4) \
            .check_log_time(True)
        self.full = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.full)
        self.cumulative = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.cumulative_increment)
        self.difference = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.difference_increment)
        self.log = PolicyMock(type=PolicyTypeEnum.backup, action=PolicyActionEnum.log)
        self.archive = PolicyMock(type=PolicyTypeEnum.archiving, action=PolicyActionEnum.archiving)
        self.replication = PolicyMock(type=PolicyTypeEnum.replication, action=PolicyActionEnum.replication)

    def test_do_validate_success(self):
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            policies = [self.full, self.difference]
            ret = CommonValidator.do_validate(policies)
            self.assertIsNone(ret)

    def test_should_raise_EmeiStorBizException_if_sla_config_or_external_policies_is_none_when_create_sla(self):
        with self.assertRaises(EmeiStorBizException):
            UnifyValidator(None, None).validate()

    def test_should_return_none_if_unified_validate_is_false_when_create_sla(self):
        sla_config = self.sla_config_builder.unify_validate(False).build()
        policies = [self.full, self.cumulative]
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            ret = UnifyValidator(sla_config, policies).validate()
            self.assertIsNone(ret)

    def test_should_raise_EmeiStorBizException_if_policy_action_not_match_support_when_create_sla(
            self):
        policies = [self.full, self.cumulative]
        with self.assertRaises(EmeiStorBizException):
            UnifyValidator(self.sla_config_builder.build(), policies).validate()

    def test_should_raise_EmeiStorBizException_if_policy_type_not_match_support_when_create_sla(self):
        policies = [self.full, self.archive]
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_policy_action_match_support") as mock_business:
            with self.assertRaises(EmeiStorBizException):
                UnifyValidator(self.sla_config_builder.build(), policies).validate()

    def test_should_return_none_if_one_full_backup_is_false_when_create_sla(self):
        sla_config = self.sla_config_builder.require_one_full_or_increment_backup(False).build()
        policies = [self.full, self.difference]
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            ret = UnifyValidator(sla_config, policies).validate()
            self.assertIsNone(ret)

    def test_should_raise_IllegalParamException_if_sla_have_no_full_and_increment_backup_when_create_sla(self):
        sla_config = self.sla_config_builder.build()
        policies = [self.log]
        with self.assertRaises(IllegalParamException):
            UnifyValidator(sla_config, policies).validate()

    def test_should_return_none_if_difference_cumulative_permanent_co_exist_is_true_when_create_sla(self):
        sla_config = self.sla_config_builder.require_difference_cumulative_permanent_co_exist(True).build()
        policies = [self.full, self.difference]
        with mock.patch.object(UnifyValidator, "_UnifyValidator__check_specification") as mock_business, \
                mock.patch.object(UnifyValidator, "_UnifyValidator__check_business") as mock_specification:
            ret = UnifyValidator(sla_config, policies).validate()
            self.assertIsNone(ret)

    def test_should_raise_EmeiStorBizException_if_sla_have_both_difference_and_cumulative_when_create_sla(
            self):
        policies = [self.full, self.difference, self.cumulative]
        sla_config = self.sla_config_builder \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.cumulative_increment, 1) \
            .require_difference_cumulative_permanent_co_exist(False).build()
        with self.assertRaises(EmeiStorBizException):
            UnifyValidator(sla_config, policies).validate()

    def test_should_raise_EmeiStorBizException_if_action_count_greater_than_support_when_create_sla(
            self):
        sla_config = self.sla_config_builder.build()
        policies = [self.full, self.full]
        with self.assertRaises(EmeiStorBizException):
            UnifyValidator(sla_config, policies).validate()

    def test_should_raise_EmeiStorBizException_if_check_backup_policies_ext_parameters_not_equal_when_create_sla(
            self):
        """
        用例场景：SLA备份策略业务参数正常
        前置条件：pm业务正常运行
        检查点： 创建sla参数错误，增量和全量备份高级参数不一致。
        """
        sla_config = self.sla_config_builder\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.full, 1)\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment, 1)\
            .check_backup_policy_extend_param_equal(False).build()

        self.full.ext_parameters = ExtParametersMock(qos_id='123')
        self.full.schedule = ScheduleMock()
        self.full.retention = RetentionMock()

        self.difference.ext_parameters = ExtParametersMock(qos_id='124')
        self.difference.schedule = ScheduleMock(start_time="00:02:00")
        self.difference.retention = RetentionMock()

        policies = [self.full, self.difference]
        res = UnifyValidator(sla_config, policies).validate()
        self.assertIsNone(res)

    def test_should_raise_EmeiStorBizException_if_check_backup_policies_ext_parameters_equal_when_create_sla(
            self):
        """
        用例场景：SLA备份策略业务参数正常
        前置条件：pm业务正常运行
        检查点： 扩展参数一致。
        """
        sla_config = self.sla_config_builder\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.log, 1)\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment, 1)\
            .check_backup_policy_extend_param_equal(True).build()

        self.log.ext_parameters = ExtParametersMock(qos_id='1')
        self.log.schedule = ScheduleMock()
        self.log.retention = RetentionMock()

        self.difference.ext_parameters = ExtParametersMock(qos_id='2')
        self.difference.schedule = ScheduleMock(start_time="00:02:00")
        self.difference.retention = RetentionMock()

        policies = [self.log, self.difference]
        with self.assertRaises(IllegalParamException):
            UnifyValidator(sla_config, policies).validate()


if __name__ == '__main__':
    unittest.main(verbosity=2)
