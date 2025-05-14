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
# import unittest
# from unittest.mock import Mock
#
# from tests.test_cases.backup.common.context import mock_context  # noqa
# from app.backup.schemas.sla import SlaCreate
# from unittest import mock
#
# from tests.test_cases.tools.sla_mock import sla_json
# from tests.test_cases.tools.timezone import dmc
#
# mock.patch("app.common.database.Database.initialize", mock.Mock).start()
# mock.patch(
#     "app.common.clients.device_manager_client.device_manager_client.init_time_zone",
#     dmc.query_time_zone).start()
# from functools import wraps
# from app.common.events import producer
#
#
# def mock_decorator(*args, **kwargs):
#     def decorator(f):
#         @wraps(f)
#         def decorated_function(*args, **kwargs):
#             return f(*args, **kwargs)
#
#         return decorated_function
#
#     return decorator
#
#
# mock.patch('app.common.security.right_control.right_control',
#            mock_decorator).start()
#
#
# ExternalSystemObjectsMock = [{'clusterId': '2', 'esn': '2102354DEY10M3000002', 'ips': {...}, 'mgrIpList': [...],
#                              'netplaneinfo': [...], 'password': 'AAAAAgAAAAAAAAAAAAA...B4KPPRdZY', 'port': 25081,
#                              'status': 27, 'username': 'repuser227'}]
#
#
# @mock.patch.object(producer, "produce", Mock(return_value=None))
# class TestSlaCreateCheckPolicy(unittest.TestCase):
#     @mock.patch("app.backup.client.replication_client.ReplicationClient.query_external_system")
#     def test_sla_create_success(self, mock_query_external_system):
#         mock_query_external_system.return_value = ExternalSystemObjectsMock
#         sla = SlaCreate(**sla_json)
#         self.assertEqual("<class 'app.backup.schemas.sla.SlaCreate'>", str(sla.__class__))
#
#
# if __name__ == '__main__':
#     unittest.main(verbosity=2)
