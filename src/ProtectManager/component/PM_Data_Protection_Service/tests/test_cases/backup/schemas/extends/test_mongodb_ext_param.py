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
import unittest
from unittest import TestCase
from unittest.mock import Mock

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.params.mongodb_ext_param import MongoDBProtectionExtParam


class TestMongoDBProtectionExtParam(TestCase):
    def setUp(self) -> None:
        super(TestMongoDBProtectionExtParam, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        self.MongoDBProtectionExtParam = MongoDBProtectionExtParam

    def test_support_values(self):
        res = [ResourceSubTypeEnum.MONGODB_CLUSTER, ResourceSubTypeEnum.MONGODB_SINGLE]
        support_values = MongoDBProtectionExtParam.support_values()
        self.assertEqual(res, support_values)


if __name__ == '__main__':
    unittest.main(verbosity=2)