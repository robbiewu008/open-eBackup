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

from sqlalchemy import text

from tests.test_cases.tools import functiontools
from tests.test_cases import common_mocker # noqa
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
from tests.test_cases.tools.timezone import dmc
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()
from app.common import database
from sqlalchemy.orm import query
from typing import Callable, TypeVar
T = TypeVar('T')


class TestDatabase(unittest.TestCase):
    def test_paginate(self):
        entities = [text('fields')]
        orders = ["str1", "str2", "str3", "str4"]
        conditions = {"%condition_key": "value_condition", "condition_key": ["value_condition"],
                      "%condition_keys": "value_condition"}
        query.Query.count = mock.Mock(return_value=20)
        query.Query.all = mock.Mock(return_value=[])
        converter = Callable[[any], T]
        extra_fields = {"name": "extra_fields_name"}

        sel = database.Database("name", None, "POSTGRES_HOST", "6432", "POSTGRES_USERNAME", "POSTGRES_PASSWORD",
                                "POSTGRES_ENCODING", False)
        paginate = database.Database.paginate(sel, *entities, page=1, size=20, orders=orders, conditions=conditions,
                                              converter=converter, extra_fields=extra_fields)
        self.assertIsNotNone(paginate)


if __name__ == '__main__':
    unittest.main(verbosity=2)
