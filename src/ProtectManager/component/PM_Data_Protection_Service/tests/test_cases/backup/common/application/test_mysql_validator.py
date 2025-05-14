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

from unittest.mock import patch
from tests.test_cases import common_mocker # noqa
from app.common.enums.resource_enum import ResourceSubTypeEnum
from tests.test_cases.backup.common.context import mock_context  # noqa
patch("app.backup.common.validators.sla_validator.manager").start()
from app.backup.common.validators.application.mysql_validator import MysqlValidator


class TestMysqlValidator(unittest.TestCase):
    """
    判断mysql 检验类是否支持资源sub type
    :return:
    """
    def test_is_support_success(self):
        self.assertTrue(MysqlValidator.is_support(ResourceSubTypeEnum.MysqlDatabase))
        self.assertTrue(MysqlValidator.is_support(ResourceSubTypeEnum.MysqlInstance))
        self.assertTrue(MysqlValidator.is_support(ResourceSubTypeEnum.MysqlClusterInstance))


if __name__ == '__main__':
    unittest.main(verbosity=2)
