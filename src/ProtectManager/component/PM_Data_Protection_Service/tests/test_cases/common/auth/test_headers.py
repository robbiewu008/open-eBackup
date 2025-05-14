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

from tests.test_cases.tools import http as http_mock, os
from app.common.auth import header

mock.patch("requests.get", http_mock.get_request).start()
mock.patch("requests.post", http_mock.post_request).start()
mock.patch("os.getenv", os.get_env).start()


class TestHeaders(unittest.TestCase):

    def test_header(self):
        ret = header("token", "req_id", None)
        self.assertEqual({'X-Auth-Token': 'token',
                          'X-Request-ID': 'req_id',
                          'es-admin-role': 'false',
                          'es-auditor-role': 'false',
                          'es-valid-token': 'false',
                          'user-id': '1111',
                          'user-name': 'souschef'}, ret)
