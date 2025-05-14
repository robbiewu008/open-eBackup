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
from tests.test_cases.tools import http, env
from tests.test_cases import common_mocker # noqa
from tests.test_cases.tools.timezone import dmc
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("requests.put", http.put_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()

from app.backup.config import config


class TestConfig(unittest.TestCase):
    def test_reload(self):
        sig = ""
        frame = ""
        reload = config.reload(sig, frame)
        self.assertIsNone(reload)


if __name__ == '__main__':
    unittest.main(verbosity=2)
