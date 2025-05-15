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
import ssl
import sys
import unittest
from unittest import TestCase, mock
from unittest.mock import Mock, MagicMock

from urllib3 import HTTPResponse

from app.base.consts import CRL_DIR


class TestHwAgentRpc(TestCase):
    def setUp(self) -> None:
        super(TestHwAgentRpc, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.resource.rpc.hw_agent_rpc import get_ssl_context
        from app.resource.rpc.hw_agent_rpc import url_request_for_agent
        self.get_ssl_context = get_ssl_context
        self.url_request_for_agent = url_request_for_agent

    @mock.patch("app.resource.rpc.hw_agent_rpc.os.path.exists", Mock(return_value=True))
    @mock.patch("app.resource.rpc.hw_agent_rpc.ssl.SSLContext.load_cert_chain", MagicMock)
    @mock.patch("app.resource.rpc.hw_agent_rpc.ssl.SSLContext.load_verify_locations", MagicMock)
    def test_get_ssl_context_success_when_crl_is_exist(self):
        res = self.get_ssl_context(CRL_DIR)
        self.assertTrue(res.verify_flags == ssl.VERIFY_CRL_CHECK_LEAF)

    @mock.patch("app.resource.rpc.hw_agent_rpc.os.path.exists", Mock(return_value=False))
    @mock.patch("app.resource.rpc.hw_agent_rpc.ssl.SSLContext.load_cert_chain", MagicMock)
    @mock.patch("app.resource.rpc.hw_agent_rpc.ssl.SSLContext.load_verify_locations", MagicMock)
    def test_get_ssl_context_success_when_crl_is_not_exist(self):
        res = self.get_ssl_context(CRL_DIR)
        self.assertTrue(res.verify_flags != ssl.VERIFY_CRL_CHECK_LEAF)

    @mock.patch("app.resource.rpc.hw_agent_rpc.get_key_password", MagicMock)
    @mock.patch("app.resource.rpc.hw_agent_rpc.get_ssl_context", MagicMock)
    @mock.patch("app.common.util.cleaner.clear", MagicMock)
    @mock.patch("urllib3.request.RequestMethods.request", Mock(return_value=HTTPResponse(status=200, body='{}')))
    @mock.patch("app.resource.rpc.hw_agent_rpc.get_agent_port", Mock(return_value='8090'))
    @unittest.skip
    def test_url_request_for_agent(self):
        res = self.url_request_for_agent('GET', '1.1.1.1', "/agent/host")
        self.assertTrue(res.status == 200)
