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
from unittest import mock
import sys
from tests.test_cases.tools import timezone

sys.modules['app.common.redis_session'] = mock.Mock()
import app.common.redis_session

app.common.redis_session = mock.Mock()

mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
mock.patch("app.common.config.Settings.get_db_password", mock.Mock(return_value="1234")).start()
mock.patch("app.common.config.Settings.get_kafka_password", mock.Mock(return_value="1234")).start()
mock.patch("app.common.config.Settings.get_redis_password", mock.Mock(return_value="1234")).start()
