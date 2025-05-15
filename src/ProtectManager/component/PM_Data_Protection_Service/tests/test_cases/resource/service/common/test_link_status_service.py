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
import copy
import sys
import time
import uuid
from unittest import mock
from uuid import UUID

import unittest
from unittest.mock import Mock
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from app.resource.schemas.link_status import LinkStateUpdate


class LinkStatusServiceTest(unittest.TestCase):
    def setUp(self) -> None:
        super(LinkStatusServiceTest, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        sys.modules['app.common.events.producer'] = Mock()
        from app.resource.service.common import link_status_service
        self.link_status_service = link_status_service

    def tearDown(self) -> None:
        del sys.modules['app.common.events.producer']

    @patch("app.base.db_base.database.session")
    def test_get_link_state_for_all_none_param(self, _mock_session):
        source_role, source_addr, destination_role, destination_addr = "", "", "", ""
        from app.resource.models.resource_models import LinkStatusTable
        _mock_session().__enter__().query(LinkStatusTable).all.return_value = []
        result = self.link_status_service.get_link_state(source_role, source_addr, destination_role, destination_addr)
        self.assertIsInstance(result, list)
        self.assertIs(0, len(result))

    @patch("app.base.db_base.database.session")
    def test_get_link_state_for_no_none_param(self, _mock_session):
        source_role, source_addr, destination_role, destination_addr = "src_role", "192.168.0.1", "dest_role",\
                                                                       "192.168.1.1"
        from app.resource.models.resource_models import LinkStatusTable
        fake_items = [
            LinkStatusTable(
                uuid=str(uuid.uuid4()), source_role="src_role", source_addr="192.168.0.1", dest_role="dest_role",
                dest_addr="192.168.1.1", state=0)
        ]
        _mock_session().__enter__().query(LinkStatusTable)\
            .filter(LinkStatusTable.source_role == source_role)\
            .filter(LinkStatusTable.source_addr == source_addr)\
            .filter(LinkStatusTable.dest_role == destination_role)\
            .filter(LinkStatusTable.dest_addr == destination_addr)\
            .filter(LinkStatusTable.update_time >= int(time.time()) - self.link_status_service.QUERY_LINK_STATUS_LIMIT_SECONDS)\
            .all.return_value = fake_items
        result = self.link_status_service.get_link_state(source_role, source_addr, destination_role, destination_addr)
        self.assertIsInstance(result, list)
        self.assertIs(1, len(result))
        self.assertIsInstance(result[0], dict)

    @patch("app.base.db_base.database.session")
    def test_modify_link_state_for_empty_item(self, _mock_session):
        update_req = LinkStateUpdate(sourceRole="src_role2", sourceAddr="192.168.0.1", destRole="dest_role2",
                                     destAddr="192.168.1.1", state=1, updateTime=time.time_ns())
        from app.resource.models.resource_models import LinkStatusTable
        _mock_session().__enter__().query(LinkStatusTable) \
            .filter(LinkStatusTable.source_role == update_req.source_role) \
            .filter(LinkStatusTable.source_addr == update_req.source_addr) \
            .filter(LinkStatusTable.dest_role == update_req.dest_role) \
            .filter(LinkStatusTable.dest_addr == update_req.dest_addr) \
            .all.return_value = []
        _mock_session().__enter__().add.return_value = None
        result = self.link_status_service.modify_link_state(update_req)
        self.assertTrue(UUID(result))

    @patch("app.base.db_base.database.session")
    def test_modify_link_state_for_not_empty_item(self, _mock_session):
        update_req = LinkStateUpdate(sourceRole="src_role3", sourceAddr="192.168.0.1", destRole="dest_role3",
                                     destAddr="192.168.1.1", state=1, updateTime=time.time_ns())
        from app.resource.models.resource_models import LinkStatusTable
        fake_uuid4 = str(uuid.uuid4())
        link_status_obj = LinkStatusTable(uuid=fake_uuid4, source_role="src_role3", source_addr="192.168.0.1",
                                          dest_role="dest_role3", dest_addr="192.168.1.1", state=0)
        fake_items = [link_status_obj]
        _mock_session().__enter__().query(LinkStatusTable) \
            .filter(LinkStatusTable.source_role == update_req.source_role) \
            .filter(LinkStatusTable.source_addr == update_req.source_addr) \
            .filter(LinkStatusTable.dest_role == update_req.dest_role) \
            .filter(LinkStatusTable.dest_addr == update_req.dest_addr) \
            .all.return_value = fake_items
        _mock_session().__enter__().query(LinkStatusTable).filter(
                LinkStatusTable.uuid == fake_items[0].uuid).update.return_value = None
        updated_link_status_obj = copy.deepcopy(link_status_obj)
        updated_link_status_obj.state = update_req.state
        _mock_session().__enter__().query(LinkStatusTable).filter(
                LinkStatusTable.uuid == fake_items[0].uuid).first.return_value = updated_link_status_obj
        result = self.link_status_service.modify_link_state(update_req)
        self.assertEqual(fake_uuid4, result)


if __name__ == '__main__':
    unittest.main(verbosity=2)
