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
from unittest import TestCase
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from tests.test_cases.backup.common.context import mock_context # noqa
from tests.test_cases.tools import functiontools, timezone

patch("pydantic.validator", functiontools.mock_decorator).start()
patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.business.trigger.customize_trigger_validator import CustomizeTriggerValidator
from app.common.enums.sla_enum import TriggerActionEnum
from app.common.exception.unified_exception import IllegalParamException

SCHEDULE1 = {"trigger_action": TriggerActionEnum.year, "days_of_month": ["22"]}
SCHEDULE2 = {"trigger_action": TriggerActionEnum.month, "days_of_week": "tue"}
SCHEDULE3 = {"trigger_action": TriggerActionEnum.week, "days_of_year": "2022-2-1"}
SCHEDULE4 = {"trigger_action": TriggerActionEnum.year, "days_of_year": "2021-1-1"}


class TestCustomizeTriggerValidator(TestCase):
    def setUp(self) -> None:
        self.customize_trigger_validator = CustomizeTriggerValidator

    def test_check_schedule_by_trigger_action(self):
        schedule1 = SCHEDULE1
        with self.assertRaises(IllegalParamException):
            self.customize_trigger_validator.check_schedule_by_trigger_action(schedule1)
        schedule2 = SCHEDULE2
        with self.assertRaises(IllegalParamException):
            self.customize_trigger_validator.check_schedule_by_trigger_action(schedule2)
        schedule3 = SCHEDULE3
        with self.assertRaises(IllegalParamException):
            self.customize_trigger_validator.check_schedule_by_trigger_action(schedule3)
        schedule4 = SCHEDULE4
        self.customize_trigger_validator.check_schedule_by_trigger_action(schedule4)
