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
from fastapi import HTTPException
from unittest import mock
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
import requests
from urllib3 import HTTPResponse

from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.security.kmc_util import Kmc
from tests.test_cases.tools import http
from tests.test_cases.tools import functiontools


mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.http.get", http.get_request).start()

from app.common.rpc.system_base_rpc import encrypt, decrypt

def pre_post_request_error():
    response = requests.Response()
    response.status_code = 500
    return response


class TestSystemBaseRpc(unittest.TestCase):
    def setUp(self):
        super(TestSystemBaseRpc, self).setUp()

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", mock.Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", mock.Mock(return_value="encrypt_text"))
    def test_encrypt(self, mock_request_post):
        mock_request_post.return_value = HTTPResponse(status=200, body=b'{"ciphertext": "ciphertext"}')
        ciphertext = encrypt('plaintext')
        self.assertEqual("ciphertext", ciphertext)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", mock.Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", mock.Mock(return_value="encrypt_text"))
    def test_encrypt_error(self, mock_request_post):
        mock_request_post.return_value = HTTPResponse(status=500)
        try:
            encrypt('plaintext')
        except Exception as ex:
            self.assertIsInstance(ex, HTTPException)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", mock.Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", mock.Mock(return_value="encrypt_text"))
    def test_decrypt(self, mock_request_post):
        mock_request_post.return_value = HTTPResponse(status=200, body=b'{"plaintext": "plaintext"}')
        ciphertext = decrypt('ciphertext')
        self.assertEqual("plaintext", ciphertext)

    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", mock.Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", mock.Mock(return_value="encrypt_text"))
    def test_decrypt_error(self, mock_request_post):
        mock_request_post.return_value = HTTPResponse(status=500)
        try:
            decrypt('ciphertext')
        except Exception as ex:
            self.assertIsInstance(ex, HTTPException)


if __name__ == '__main__':
    unittest.main(verbosity=2)
