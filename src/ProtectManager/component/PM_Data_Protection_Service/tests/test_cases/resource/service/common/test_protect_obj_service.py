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
import unittest
import uuid
from unittest import mock
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from tests.test_cases.tools import functiontools, timezone

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
from app.common.exception.unified_exception import EmeiStorBizException

datetime_str = "2021-07-01T14:00:00"
SLA = {
    "policy_list": [
        {"type": "replication"},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 30, "interval_unit": "m", "start_time": datetime_str}},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 12, "interval_unit": "h", "start_time": datetime_str}},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 2, "interval_unit": "d", "start_time": datetime_str}},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 1, "interval_unit": "w", "start_time": datetime_str}},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 1, "interval_unit": "MO", "start_time": datetime_str}},
        {"type": "backup", "action": "log",
         "schedule": {"interval": 1, "interval_unit": "y", "start_time": datetime_str}},
        {"type": "backup", "action": "full",
         "schedule": {"window_start": "14:00:00", "window_end": "12:00:00", "interval": 60, "interval_unit": "m",
                      "start_time": datetime_str}},
    ]
}

POLICY = {"uuid": "7ae89ed3-0828-469c-8a9e-b768710ec9f1", "name": "permanent_increment",
          "action": "permanent_increment",
          "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                             "auto_index": True},
          "retention": {"retention_type": 2, "duration_unit": "d", "retention_duration": 2},
          "schedule": {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                       "window_start": "18:00:00", "window_end": "23:00:00", "days_of_month": None,
                       "days_of_year": None, "trigger_action": None, "days_of_week": None}, "type": "backup"}

POLICY2 = {"uuid": "7ae89ed3-0828-469c-8a9e-b768710ec9f1", "name": "permanent_increment",
           "action": "permanent_increment",
           "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                              "auto_index": True},
           "retention": {"retention_type": 2, "duration_unit": "d", "retention_duration": 2},
           "schedule": {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                        "window_start": "6:00:00", "window_end": "12:00:00", "days_of_month": None,
                        "days_of_year": None, "trigger_action": None, "days_of_week": None}, "type": "backup"}

POLICY3 = {"uuid": "7ae89ed3-0828-469c-8a9e-b768710ec9f1", "name": "permanent_increment",
           "action": "permanent_increment",
           "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                              "auto_index": True},
           "retention": {"retention_type": 2, "duration_unit": "d", "retention_duration": 2},
           "schedule": {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                        "window_start": "00:00:00", "window_end": "00:00:00", "days_of_month": None,
                        "days_of_year": None, "trigger_action": None, "days_of_week": None}, "type": "backup"}

POLICY4 = {"uuid": "7ae89ed3-0828-469c-8a9e-b768710ec9f1", "name": "permanent_increment",
           "action": "permanent_increment",
           "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                              "auto_index": True},
           "retention": {"retention_type": 2, "duration_unit": "d", "retention_duration": 2},
           "schedule": {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                        "window_start": "00:00:00", "window_end": "00:00:00", "days_of_month": None,
                        "days_of_year": None, "trigger_action": None, "days_of_week": None}, "type": "archive"}

POLICY5 = {"uuid": "3ad87a13-dbcd-4690-98b1-fb6814b94367", "name": "permanent_increment",
           "action": "permanent_increment",
           "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                              "auto_index": True},
           "retention": {"retention_type": 2, "duration_unit": "MO", "retention_duration": 1},
           "schedule": {"trigger": 4, "interval": None, "interval_unit": None, "start_time": None,
                        "window_start": "04:00:00",
                        "window_end": "14:00:00", "days_of_month": "24,28,30", "days_of_year": None,
                        "trigger_action": "month",
                        "days_of_week": None}, "type": "backup"}

POLICY6 = {"uuid": "3ad87a13-dbcd-4690-98b1-fb6814b94367", "name": "log",
           "action": "log", "type": "backup"}

POLICY7 = {"uuid": "7ae89ed3-0828-469c-8a9e-b768710ec9f1", "name": "permanent_increment",
           "action": "permanent_increment",
           "ext_parameters": {"auto_retry": True, "auto_retry_times": 3, "auto_retry_wait_minutes": 5, "qos_id": "",
                              "auto_index": True},
           "retention": {"retention_type": 2, "duration_unit": "d", "retention_duration": 2},
           "schedule": {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                        "window_start": "20:00:00", "window_end": "06:00:00", "days_of_month": None,
                        "days_of_year": None, "trigger_action": None, "days_of_week": None}, "type": "backup"}


class ProtectObjServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        super(ProtectObjServiceTest, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        from app.resource.service.common import protect_obj_service
        self.protect_obj_service = protect_obj_service

    @patch("app.base.db_base.database.session")
    def test_sync_time(self, _mock_session):
        from app.base.db_base import database
        _mock_session().__enter__().query().filter().first.return_value = None
        resource_id = str(uuid.uuid4())
        with database.session() as session:
            self.assertRaises(EmeiStorBizException, self.protect_obj_service.sync_time, session, resource_id)

        from app.protection.object.models.projected_object import ProtectedObject
        protected_obj = ProtectedObject(
            uuid=str(uuid.uuid4()), resource_id=resource_id)
        _mock_session().__enter__().query().filter().first.return_value = protected_obj
        _mock_session().__enter__().query().filter().update.return_value = None
        with database.session() as session:
            result = self.protect_obj_service.sync_time(session, resource_id)
            self.assertIsNone(result)

        protected_obj.earliest_time = datetime.datetime.now()
        with database.session() as session:
            result = self.protect_obj_service.sync_time(session, resource_id)
            self.assertIsNone(result)

    @mock.patch(
        "app.common.clients.scheduler_client.SchedulerClient.get_schedule_next_time")
    @mock.patch("app.resource.service.common.protect_obj_service.get_protect_obj")
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_policy")
    def test_get_next_time_success(self, _mock_get_policy, _mock_get_protect_obj, _mock_get_next_time):
        from app.protection.object.models.projected_object import ProtectedObject, ProtectedTask
        resource_id = str(uuid.uuid4())
        task1 = ProtectedTask(uuid=str(uuid.uuid4()))
        _mock_get_next_time.return_value = "2021-09-06 02:00:00"
        protected_obj = ProtectedObject(
            uuid=str(uuid.uuid4()), resource_id=resource_id, sla_id=str(uuid.uuid4()), task_list=[task1])
        _mock_get_protect_obj.return_value = protected_obj
        session = ""
        _mock_get_policy.return_value = POLICY
        result = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result)
        _mock_get_policy.return_value = POLICY2
        result2 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result2)
        _mock_get_policy.return_value = POLICY3
        result3 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result3)
        _mock_get_policy.return_value = POLICY4
        result4 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result4)
        _mock_get_policy.return_value = POLICY5
        result5 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result5)
        _mock_get_policy.return_value = POLICY6
        result6 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result6)
        _mock_get_policy.return_value = POLICY7
        result7 = self.protect_obj_service.get_next_time(session, resource_id)
        self.assertIsNotNone(result7)

    @mock.patch(
        "app.common.clients.scheduler_client.SchedulerClient.get_schedule_next_time")
    @mock.patch(
        "app.resource.service.common.protect_obj_service.get_protect_obj")
    def test_get_next_time_failed(self, _mock_get_protect_obj,
                                  _mock_get_next_time):
        from app.protection.object.models.projected_object import \
            ProtectedObject, ProtectedTask
        resource_id = str(uuid.uuid4())
        task1 = ProtectedTask(uuid=str(uuid.uuid4()))
        _mock_get_next_time.return_value = "2021-09-06 02:00:00"
        protected_obj = ProtectedObject(
            uuid=str(uuid.uuid4()), resource_id=resource_id,
            sla_id=str(uuid.uuid4()), task_list=[task1])
        _mock_get_protect_obj.return_value = None
        session = ""
        self.assertRaises(AttributeError, self.protect_obj_service.get_next_time, session, resource_id)

    def test_get_next_time_whether_time_is_in_windows_time_failed(self):
        schedules = {"trigger": 1, "interval": 1, "interval_unit": "h", "start_time": "2022-03-28T18:00:00",
                     "window_start": "00:00:00", "window_end": "00:00:00", "days_of_month": None,
                     "days_of_year": None, "trigger_action": None, "days_of_week": None}
        result = self.protect_obj_service.calculate_time_by_schedule(schedules)
        self.assertIsNotNone(result)


if __name__ == '__main__':
    unittest.main(verbosity=2)
