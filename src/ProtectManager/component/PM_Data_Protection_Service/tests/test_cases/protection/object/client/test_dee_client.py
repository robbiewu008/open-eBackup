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

from urllib3 import HTTPResponse

from app.common.clients import client_util
from app.common.security import kmc_util
from app.common.security.kmc_util import Kmc
from tests.test_cases import common_mocker  # noqa


class DeeClientTest(unittest.TestCase):
    def setUp(self):
        super(DeeClientTest, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        from app.protection.object.client import dee_client
        self.dee_client = dee_client
        mock.patch('app.protection.object.client.dee_client.update_self_learning_config', mock.Mock).start()

    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    @mock.patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_open_self_learning(self):
        """
        测试场景：开启自学习
        前置条件：PM和DEE允许正确
        检查点：开启自学习无异常
        :return:
        """
        self.dee_client.DataEnableEngineHttpsClient.request = Mock(return_value=HTTPResponse(status=200))
        self.dee_client.update_self_learning_config("123", "device", True, 1, 30)
