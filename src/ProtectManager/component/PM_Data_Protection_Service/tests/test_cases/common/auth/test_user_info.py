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
from functools import wraps

from app.common.auth.user_info import user_info_from_token


def mock_decorator(*args, **kwargs):
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            return "123"
        return decorated_function
    return decorator


mock.patch("pydantic.validator", mock_decorator).start()


class TestUserInfo(unittest.TestCase):

    def test_user_info_from_token(self):
        ret = user_info_from_token("token")
        self.assertEqual({'es-admin-role': 'false',
                          'es-auditor-role': 'false',
                          'es-valid-token': 'false',
                          'user-id': '1111',
                          'user-name': 'souschef'}, ret)
