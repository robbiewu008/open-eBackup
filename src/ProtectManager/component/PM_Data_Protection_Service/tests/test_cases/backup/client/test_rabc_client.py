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
import unittest
from unittest import mock
from unittest.mock import patch, Mock

import requests
from urllib3 import HTTPResponse

from app.backup.client.rbac_client import RBACClient
from app.common.clients.client_util import SystemBaseHttpsClient
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.tools import http, env
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo

mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()


class ProtectedObjMock:
    def __init__(self):
        self.resource_id = "resource_id"
        self.sla_id = "sla_id123"


@patch.object(requests, "post", Mock(status_code=1))
class TestSchedulerClient(unittest.TestCase):
    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    def test_add_resource_set_relation_success(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=200)
        resource_set_relation_info = ResourceSetRelationInfo(resource_object_id="test1", resource_set_type="QOS",
                                                             scope_module="QOS", domain_id_list=["test"])
        RBACClient.add_resource_set_relation(resource_set_relation_info)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    def test_delete_resource_set_relation_success(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=200)
        RBACClient.delete_resource_set_relation(["test1"], "test1")

if __name__ == '__main__':
    unittest.main(verbosity=2)
