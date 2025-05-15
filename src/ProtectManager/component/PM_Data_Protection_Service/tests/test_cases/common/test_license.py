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
from unittest.mock import Mock, patch
from urllib3 import HTTPResponse
from tests.test_cases import common_mocker # noqa
from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.security.kmc_util import Kmc
from tests.test_cases.tools import functiontools

from tests.test_cases.tools import http
from tests.test_cases.tools.timezone import dmc

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.put", http.get_request).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()
from app.common.enums.license_enum import FunctionEnum
from app.common import license


class TestLicense(unittest.TestCase):
    @patch.object(SystemBaseHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_validate_license_by_resource_type(self, _mock_request):
        _mock_request.return_value = HTTPResponse(status=200)
        function = FunctionEnum.ARCHIVE
        job_id = "job_id123"
        request_id = "8546bb41-abe6-4821-870d-a0252f04df"
        validate_license_by_resource_type = license.validate_license_by_resource_type(function, 'sub_type123',
                                                                                      job_id=job_id,
                                                                                      request_id=request_id,
                                                                                      strict=False)
        self.assertIsNotNone(validate_license_by_resource_type)


if __name__ == '__main__':
    unittest.main(verbosity=2)
