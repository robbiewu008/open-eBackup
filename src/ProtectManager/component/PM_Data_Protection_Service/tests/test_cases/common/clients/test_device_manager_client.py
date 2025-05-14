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
import builtins
import unittest
from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from app.common.clients.device_manager_client import device_manager_client, replace_time_zone

DEFAULT_TIMEZONE = "Asia/Shanghai"
RESPONSE_KEY_TIMEZONE = "CMO_SYS_TIME_ZONE_NAME"


def mock_timezone_file(path):
    return open(path, 'w')


class TestDeviceManagerClient(unittest.TestCase):
    def setUp(self):
        super(TestDeviceManagerClient, self).setUp()

    def test_replace_time_zone(self):
        self.assertEqual(DEFAULT_TIMEZONE, replace_time_zone('Asia/Beijing'))
        self.assertEqual('US/Pacific', replace_time_zone('US/Pacific-New'))
        self.assertEqual('Asia/Urumqi', replace_time_zone('Asia/Urumqi'))

    @patch.object(builtins, 'open')
    def test_init_time_zone_default(self, mock_open_file):
        mock_open_file.return_value = mock_timezone_file("test_timezone1")
        result_TIMEZONE = device_manager_client.init_time_zone()
        self.assertEqual(DEFAULT_TIMEZONE, result_TIMEZONE)