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
from datetime import date, time, datetime
from unittest import mock

from app.backup.schemas.sla import SlaPageRequest
from app.common.enums.resource_enum import ResourceSubTypeEnum
from tests.test_cases import common_mocker # noqa
from app.common.enums.sla_enum import TriggerEnum, TriggerActionEnum, PolicyActionEnum, SlaType
from tests.test_cases.tools import http, env
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("requests.put", http.put_request).start()
mock.patch("os.getenv", env.get_env).start()

mock.patch("app.common.database.Database.initialize", mock.Mock).start()
from app.backup.curd import crud_sla


class PolicyMock:
    def __init__(self):
        self.uuid = None
        self.type = "type123"
        self.name = "name123"
        self.action = "action123"
        self.ext_parameters = JsonMock()
        self.retention = RetentionMock()
        self.schedule = ScheduleMock()

class RetentionMock:
    def __init__(self):
        self.retention_type = "retention_type"
        self.retention_duration = "retention_duration"
        self.duration_unit = "duration_unit"


class ScheduleMock:
    def __init__(self):
        self.trigger = "trigger"
        self.interval = "interval"
        self.interval_unit = "interval_unit"
        self.start_time = "start_time"
        self.window_start = "window_start"
        self.window_end = "window_end"
        self.trigger_action = None
        self.days_of_week = None
        self.days_of_month = None
        self.days_of_year = None


class JsonMock:
    def json(self):
        return None

    def get(self):
        self.type = 1


class TestCrudSla(unittest.TestCase):
    def test_build_policy_model(self):
        sla_id = "sla_id123"
        policy = PolicyMock()
        sla_model = "sla_model123"
        build_policy_model = crud_sla.build_policy_model(sla_id, policy, sla_model)
        self.assertIsNotNone(build_policy_model)

    def test_search_rank(self):
        items = []
        name = "name123"
        search_rank = crud_sla.search_rank(items, name)
        self.assertIsNone(search_rank)

    def test_modify_schedule_start_time_when_interval(self):
        class ScheduleMockIsInterval:
            def __init__(self):
                self.trigger = TriggerEnum.interval.value
                self.start_time = date(2022, 1, 16)
                self.window_start = time(3, 10, 2)

        policy = PolicyMock()
        policy.schedule = ScheduleMockIsInterval()
        start_time = crud_sla.modify_schedule_start_time(policy)
        self.assertEqual(start_time, datetime(2022, 1, 16, 3, 10, 2).isoformat())

    def test_modify_schedule_start_time_when_customize_interval(self):
        class ScheduleMockIsCustomizeInterval:
            def __init__(self):
                self.trigger = TriggerEnum.customize_interval.value
                self.trigger_action = TriggerActionEnum.year
                self.start_time = None
                self.days_of_year = date(2022, 1, 16)
                self.window_start = time(3, 10, 2)

        policy = PolicyMock()
        policy.schedule = ScheduleMockIsCustomizeInterval()
        start_time = crud_sla.modify_schedule_start_time(policy)
        self.assertEqual(start_time, datetime(2022, 1, 16, 3, 10, 2).isoformat())

    def test_curd_sla_page_query(self):
        from app.protection.object.service.projected_object_service import ProtectedObjectService
        with mock.patch.object(ProtectedObjectService, "count_by_sla_id", mock.MagicMock()):
            mock_db = mock.MagicMock()
            mock_db.query = mock.Mock(return_value=mock_db)
            mock_db.order_by = mock.Mock(return_value=mock_db)
            mock_db.limit = mock.Mock(return_value=mock_db)
            mock_db.filter = mock.Mock(return_value=mock_db)
            mock_db.join = mock.Mock(return_value=mock_db)
            mock_db.offset = mock.Mock(return_value=mock_db)
            mock_db.count = mock.Mock(return_value=1)
            mock_db_res = mock.MagicMock()
            mock_db.all = mock.Mock(return_value=[mock_db_res])
            request = SlaPageRequest(page_no=1, page_size=2, user_id='123', str='123', actions=[PolicyActionEnum.full],
                                     types=[SlaType.backup],
                                     applications=[ResourceSubTypeEnum.Oracle], name='123')
            res = crud_sla.sla.page_query(db=mock_db, page_req=request)
            self.assertEqual(res['total'], 1)


if __name__ == '__main__':
    unittest.main(verbosity=2)
