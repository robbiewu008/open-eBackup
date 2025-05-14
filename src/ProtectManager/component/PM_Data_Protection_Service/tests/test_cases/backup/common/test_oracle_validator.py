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

from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum
from app.backup.common.validators.extends import oracle_validator

class PolicyMock:
    def __init__(self):
        self.schedule = ScheduleMock()
        self.action = "action"
        self.type = PolicyTypeEnum.backup.value


class PolicyMocks:
    def __init__(self):
        self.schedule = ScheduleMocks()
        self.action = PolicyActionEnum.log
        self.type = PolicyTypeEnum.backup.value


class ScheduleMock:
    def __init__(self):
        self.interval_unit = RetentionMock()
        self.interval = 20
        self.start_time = "20:12:10"


class ScheduleMocks:
    def __init__(self):
        self.interval_unit = RetentionMock()
        self.interval = 18
        self.start_time = "22:12:10"


class RetentionMock:
    def __init__(self):
        self.value = "m"


class TestOracleValidator(unittest.TestCase):
    def test_do_validate(self):
        """
        用例名称：测试日志备份时间和日志备份频率是否合理
        前置条件：输入policies参数合法
        check点：日志备份首次开始时间必须晚于数据备份首次开始时间, 日志备份频率必须小于数据备份频率
        """
        policies = [PolicyMock(), PolicyMocks()]
        do_validate = oracle_validator.OracleValidator.do_validate(policies)
        self.assertIsNone(do_validate)


if __name__ == '__main__':
    unittest.main(verbosity=2)
