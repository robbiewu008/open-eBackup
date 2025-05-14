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
import importlib
import unittest
from tests.test_cases.tools import http, env, redis_mock
from unittest import mock
from tests.test_cases import common_mocker # noqa
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("redis.Redis", redis_mock.RedisMock).start()
from tests.test_cases.tools.timezone import dmc
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
from sqlalchemy.orm.query import Query


class MyTestCase(unittest.TestCase):
    def test_qos_data_condition_filter(self):
        condition = {"name":""}
        qos_service = importlib.import_module("app.backup.service.qos_service")
        query = Query
        qos_data_condition_filter = qos_service.qos_data_condition_filter(condition)
        self.assertIsNotNone(qos_data_condition_filter(query))


if __name__ == '__main__':
    unittest.main(verbosity=2)
