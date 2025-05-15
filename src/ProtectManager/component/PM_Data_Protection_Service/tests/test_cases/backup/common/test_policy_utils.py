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
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("requests.put", http.put_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()

from app.backup.common import policy_utils


class TestPolicyUtils(unittest.TestCase):
    def test_get_interval_minutes(self):
        interval_s = "23m"
        get_interval_minutes = policy_utils.get_interval_minutes(interval_s)
        self.assertEqual(get_interval_minutes, 23)
        interval_s = "23h"
        get_interval_hours = policy_utils.get_interval_minutes(interval_s)
        self.assertEqual(get_interval_hours, 1380)
        interval_s = "23d"
        get_interval_days = policy_utils.get_interval_minutes(interval_s)
        self.assertEqual(get_interval_days, 33120)
        interval_s = "23w"
        get_interval_weeks = policy_utils.get_interval_minutes(interval_s)
        self.assertEqual(get_interval_weeks, 231840)
        interval_s = "23a"
        get_interval_weeks = policy_utils.get_interval_minutes(interval_s)
        self.assertEqual(get_interval_weeks, 1)

    def test_get_backup_interval(self):
        policy = {"interval":"23m", "backup_mode":"full"}
        get_backup_interval = policy_utils.get_backup_interval(policy)
        self.assertEqual(get_backup_interval, "23m")


if __name__ == '__main__':
    unittest.main(verbosity=2)
