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
import logging
import unittest
import urllib3
from unittest import mock

from app.common.auth import propagate_http_error
from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases.tools import functiontools

mock.patch("pydantic.validator", functiontools.mock_decorator).start()


class TestRestOps(unittest.TestCase):

    def test_propagate_http(self):
        response = urllib3.response.HTTPResponse()
        response.status = 200
        propagate_http_error(response, 200, None)
        self.assertEqual(1, 1)

    def test_propagate_http_error(self):
        response = urllib3.response.HTTPResponse()
        response.status_code = 500
        response._content = {"aa": "bb"}
        self.assertRaises(EmeiStorBizException, propagate_http_error, response, 200, None, logging)

    def test_propagate_http_error2(self):
        response = urllib3.response.HTTPResponse()
        response.status_code = 500
        self.assertRaises(EmeiStorBizException, propagate_http_error, response, 200, None, logging)
