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
from urllib3.response import HTTPResponse

from unittest.mock import patch, Mock

from app.common.clients.client_util import InfrastructureHttpsClient
from app.common.security.kmc_util import Kmc


def get_request_x8000():
    response = HTTPResponse()
    response._body = b'{"data": [{"deploy_type": "d0"}], "error": null}'
    response.status = 200
    return response


def get_request_cloudbackup():
    response = HTTPResponse()
    response._body = b'{"data": [{"deploy_type": "d3"}], "error": null}'
    response.status = 200
    return response


def get_request_error():
    response = HTTPResponse()
    response._body = b'{"data": "","error": {"errId": 1644385028,"errMsg": ' \
                     b'"The config parameter (deploy_type) does not exist."}}'
    response.status = 200
    return response


class TestDeployType(unittest.TestCase):

    def setUp(self) -> None:
        from tests.test_cases.common.mock_settings import fake_settings
        from app.common.deploy_type import DeployType
        self.DeployType = DeployType()

    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_get_deploy_type_when_X8000(self, _request_get):
        system_type = "d0"
        _request_get.return_value = get_request_x8000()
        res = self.DeployType.get_deploy_type()
        self.assertEqual(res, system_type)

    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_get_deploy_type_when_cloudbackup(self, _request_get):
        system_type = "d3"
        _request_get.return_value = get_request_cloudbackup()
        res = self.DeployType.get_deploy_type()
        self.assertEqual(res, system_type)

    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    def test_get_deploy_type_when_error(self, _request_get):
        system_type = ""
        _request_get.return_value = get_request_error()
        res = self.DeployType.get_deploy_type()
        self.assertEqual(res, system_type)
