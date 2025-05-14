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
import datetime
import sys
import unittest
import uuid
from unittest import mock

sys.modules['app.common.logger'] = mock.Mock()
# mock为了导入Policy、Schedule
_mock_validator_manager_init = mock.patch("app.backup.common.validators.validator_manager.ValidatorManager.__init__",
                                          mock.Mock(return_value=None))
_mock_validator_manager_init.start()
from app.backup.schemas.policy import Policy, Schedule
from app.common.enums.sla_enum import PolicyTypeEnum, TriggerEnum, PolicyActionEnum, TriggerActionEnum


class BackupPolicyServiceTest(unittest.TestCase):
    @classmethod
    def tearDownClass(cls) -> None:
        _mock_validator_manager_init.stop()

    def setUp(self) -> None:
        from app.backup.service import backup_policy_service
        self.backup_policy_service = backup_policy_service

    @mock.patch("app.backup.service.backup_policy_service.create_customize_next_start_time")
    @mock.patch("app.backup.service.backup_policy_service.create_interval_next_start_time")
    def test_get_start_time_by_action_success(self, _mock_interval_next_start_time, _mock_customize_next_start_time):
        d1 = datetime.datetime(2022, 5, 1, 0, 0, 0)
        d2 = datetime.datetime(2022, 6, 1, 0, 0, 0)
        _mock_interval_next_start_time.return_value = 1651334400
        _mock_customize_next_start_time.return_value = 1654012800
        schedule_one = Schedule(trigger=TriggerEnum.interval, start_time=d1, days_of_week=["wed", "sun"])
        policy_one = Policy(uuid=uuid.uuid4(), name="sla1", action=PolicyActionEnum.full, schedule=schedule_one,
                            type=PolicyTypeEnum.backup)
        schedule_two = Schedule(trigger=TriggerEnum.customize_interval, start_time=d2, days_of_week=["mon", "wed"])
        policy_two = Policy(uuid=uuid.uuid4(), name="sla1", action=PolicyActionEnum.log, schedule=schedule_two,
                            type=PolicyTypeEnum.backup)
        policy_list = [policy_one, policy_two]
        start_time_dict = self.backup_policy_service.get_start_time_by_action(policy_list)
        self.assertDictEqual(start_time_dict, {"full": 1651334400, "log": 1654012800})

    def test_create_interval_next_start_time_success(self):
        d1 = datetime.datetime(2022, 1, 1)
        window_start = datetime.time(0, 0, 0)
        schedule_one = Schedule(trigger=TriggerEnum.interval, start_time=d1, window_start=window_start,
                                days_of_week=["wed", "sun"])
        ret = self.backup_policy_service.create_interval_next_start_time(schedule_one)
        self.assertEqual(ret, 1640966400.0)

    @mock.patch("app.backup.service.backup_policy_service.get_cron_next_time")
    @mock.patch("app.backup.service.backup_policy_service.gen_cron_expression")
    def test_create_customize_next_start_time_success(self, _mock_gen_cron_expression, _mock_get_cron_next_time):
        _mock_gen_cron_expression.return_value = "0 0 0 ? * 1"
        _mock_get_cron_next_time.return_value = ["2022-02-06 00:00:00"]
        d1 = datetime.datetime(2022, 2, 1)
        window_start = datetime.time(0, 0, 0)
        schedule_one = Schedule(trigger=TriggerEnum.customize_interval, trigger_action=TriggerActionEnum.week,
                                start_time=d1, window_start=window_start, days_of_week=["sun"])
        ret = self.backup_policy_service.create_customize_next_start_time(schedule_one)
        self.assertEqual(ret, 1644076800.0)
