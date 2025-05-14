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
import sys
import unittest
from unittest import mock
from unittest.mock import Mock

from pydantic import ValidationError

from tests.test_cases.common.mock_settings import fake_settings  # noqa
from app.common.clients.client_util import SystemBaseHttpsClient

user_info = {'exp': 1623816780,
             'expires_at': 1623816780000,
             'iat': 1623216780,
             'iss': 'emeistor.huawei.com',
             'issued_at': 1623216780000,
             'user': {'id': '88a94c476f12a21e016f12a246e50009',
                      'name': 'sysadmin',
                      'roles': [{'id': '1',
                                 'name': 'Role_SYS_Admin'}]},
             'user_roles': ["mmdp"]}

mock_token = "eyJhbGciOiJSUzI1NiJ9.eyJleHBpcmVzX2F0IjoxNjIzODE2NzgwMDAwLCJpc3MiOiJlbW" \
             "Vpc3Rvci5odWF3ZWkuY29tIiwidXNlcl9yb2xlcyI6W10sImV4cCI6MTYyMzgxNjc4MCwidXNl" \
             "ciI6eyJpZCI6Ijg4YTk0YzQ3NmYxMmEyMWUwMTZmMTJhMjQ2ZTUwMDA5IiwibmFtZSI6InN5c2Fkb" \
             "WluIiwicm9sZXMiOlt7Im5hbWUiOiJSb2xlX1NZU19BZG1pbiIsImlkIjoiMSJ9XX0sImlhdCI6MTY" \
             "yMzIxNjc4MCwiaXNzdWVkX2F0IjoxNjIzMjE2NzgwMDAwfQ.mvoRJeiz7f894VBgqLF2vy4dehwHNG7" \
             "WRDcCGULDBGCJW0wMJks3UIPO6Oykd40TEGypBtAD4Q2RrbhjT8Uez41s20VA0SnBM7g2NKvtjsR-tO" \
             "O5AhbniT-JAXpK1d3yT-Ha4QmMe4lDDv7ofK5siXP5Sh9YbENfSWwwoqsuhZoC4JJfPthmbXy6USvFn4" \
             "C2K7yp2VgDPo46YQSdcuJ7dx6R9yD4IthefzdZHdYyE3p5XBRpV0IIdmJOfouiz7RtW9BpQAAjqe9AhRmo" \
             "tEOeQt3d6Nw6xmodSvzfXDHiTj5mZm1nxOWpjEE3SzAvy-bqpbXL-M_d8hcCMKNnhcQSNw"


class TestReplicationApi(unittest.TestCase):
    def setUp(self):
        super(TestReplicationApi, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.common.events.producer'] = Mock()
        sys.modules['app.resource_lock.kafka.messaging_utils'] = Mock()
        sys.modules['app.resource_lock.db.db_utils'] = Mock()
        sys.modules['app.common.kafka'] = Mock()
        sys.modules['app.protection.object.schemas.extends.extends_params_manager'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()

    def tearDown(self) -> None:
        del sys.modules['app.common.events.producer']
        del sys.modules['app.resource_lock.kafka.messaging_utils']
        del sys.modules['app.resource_lock.db.db_utils']
        del sys.modules['app.common.kafka']
        del sys.modules['app.protection.object.schemas.extends.extends_params_manager']
        del sys.modules['app.backup.common.config.db_config']

    @mock.patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @mock.patch("app.common.security.jwt_utils.get_user_info_from_token", Mock(return_value=user_info))
    def test_should_raise_ValidationError_when_request_replicate_if_missing_required_field(self, mock_request):
        from app.replication.api.replication_api import replicate
        from app.replication.schemas.replication_request import ReplicationRequest
        with self.assertRaises(ValidationError) as ex:
            replicate(user_info, ReplicationRequest(**{"copy_id": "123456", "retention_type": 1}))

    @mock.patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @mock.patch("app.common.security.jwt_utils.get_user_info_from_token", Mock(return_value=user_info))
    def test_should_success_when_request_replicate(self, mock_request):
        from app.replication.api.replication_api import replicate
        from app.replication.schemas.replication_request import ReplicationRequest
        from app.replication.api.replication_api import _resolve_user_info
        u = _resolve_user_info(mock_token)
        replicate(user_info,
                  ReplicationRequest(
                      **{"copy_id": "123456", "external_system_id": "2", "retention_type": 1, "storage_type": "123456",
                         "storage_id": "2", "user_id": 1, "username": "123456", "password": "2"}))
