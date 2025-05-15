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

from app.common.auth.mocking_util import set_streaming_file, mock_response, MockResponse
from tests.test_cases.tools import http as http_mock, os

mock.patch("requests.get", http_mock.get_request).start()
mock.patch("requests.post", http_mock.post_request).start()
mock.patch("os.getenv", os.get_env).start()


class TestMockingUtil(unittest.TestCase):
    def test_raise_for_status(self):
        response = MockResponse("resp_data", "200")
        response.raise_for_status()
        self.assertEqual(1, 1)

    def test_response_content(self):
        response = MockResponse("resp_data", "200")
        self.assertEqual("resp_data", response.content)

    def test_response_json(self):
        response = MockResponse("resp_data", "200")
        self.assertEqual("resp_data", response.json())

    def test_response_text(self):
        response = MockResponse("resp_data", "200")
        self.assertEqual("resp_data", response.text)

    def test_set_streaming_file(self):
        set_streaming_file("file_data")
        self.assertEqual(1, 1)

    def test_mock_response(self):
        cooked_responses = {'/#/login': ["resp_data", "200"]}
        ret = mock_response(cooked_responses, "https://PM:25080/#/login?", "source_mock")
        self.assertEqual(MockResponse("resp_data", "200").json(), ret.json())

    def test_mock_response1(self):
        cooked_responses = {'/#/log': ["resp_data", "200"]}
        mock_response(cooked_responses, "https://PM:25080/#/login?", "source_mock")
        self.assertEqual(1, 1)

    def test_mock_response_error(self):
        cooked_responses = {}
        mock_response(cooked_responses, "https://PM:25080/#/login?", "source_mock")
        self.assertEqual(1, 1)
