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

common_mock()

from urllib3 import HTTPResponse

from app.common.clients.client_util import ProtectionServiceHttpsClient
from app.common.security.kmc_util import Kmc
from app.common.security import kmc_util


class CopyClientTest(unittest.TestCase):
    def setUp(self):
        super(CopyClientTest, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()

    @mock.patch.object(ProtectionServiceHttpsClient, "request", autospec=True)
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    @mock.patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_should_raise_EmeiStorBizException_if_call_resource_manager_rest_fail_when_create_import_resource(
            self, _mock_request):
        """
        测试场景：创建导入资源
        前置条件：资源管理服务异常
        检查点：资源管理rest接口异常，抛异常
        :return:
        """
        _mock_request.return_value = HTTPResponse(status=500)
        from app.copy_catalog.client.copy_client import create_import_resource
        from app.common.exception.unified_exception import EmeiStorBizException
        with self.assertRaises(EmeiStorBizException):
            create_import_resource('dwsClusterFilesystem')
