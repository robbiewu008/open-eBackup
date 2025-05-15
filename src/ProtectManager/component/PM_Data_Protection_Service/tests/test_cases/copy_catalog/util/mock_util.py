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
import sys
from unittest import mock
from unittest.mock import Mock

from tests.test_cases.tools import http, env, functiontools, timezone, jwt_utils_mock


def common_mock():
    sys.modules['app.common.events.producer'] = Mock()
    sys.modules['app.resource_lock.service.lock_service'] = Mock()
    mock.patch("requests.get", http.get_request).start()
    mock.patch("requests.post", http.post_request).start()
    mock.patch("os.getenv", env.get_env).start()
    mock.patch("pydantic.validator", functiontools.mock_decorator).start()
    mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
               timezone.dmc.query_time_zone).start()
    mock.patch("app.common.database.Database.initialize", mock.Mock).start()
    mock.patch('app.common.security.jwt_utils.get_user_info_from_token', jwt_utils_mock.get_user).start()
    mock.patch('app.copy_catalog.client.dee_client.alarm_handler', mock.Mock).start()
    mock.patch('app.copy_catalog.client.dee_client.check_copy_browse_status', mock.Mock).start()
