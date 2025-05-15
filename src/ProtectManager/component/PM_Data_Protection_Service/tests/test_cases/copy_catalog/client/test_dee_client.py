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
from unittest.mock import Mock
from tests.test_cases import common_mocker # noqa
from app.common.clients import client_util
from tests.test_cases.copy_catalog.util.mock_util import common_mock

from app.copy_catalog.schemas import ModifyCopyAntiRansomwareStatusBody

common_mock()

from urllib3 import HTTPResponse

from app.common.security.kmc_util import Kmc
from app.common.security import kmc_util


class DeeClientTest(unittest.TestCase):
    def setUp(self):
        super(DeeClientTest, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        from app.copy_catalog.client import dee_client
        self.dee_client = dee_client

    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    @mock.patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_alarm_handler(self):
        """
        测试场景：下发误报处理
        前置条件：PM和DEE允许正确
        检查点：正常下发误报处理
        :return:
        """
        self.dee_client.DataEnableEngineHttpsClient = Mock(return_value=HTTPResponse(status=200))
        self.dee_client.alarm_handler(["123"], ModifyCopyAntiRansomwareStatusBody(**{"is_security_snap": True}))
