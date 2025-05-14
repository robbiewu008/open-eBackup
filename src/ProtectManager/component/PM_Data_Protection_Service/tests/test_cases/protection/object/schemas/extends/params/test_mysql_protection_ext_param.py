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
from unittest import TestCase

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.params.mysql_protection_ext_param import MysqlProtectionExtParam


class MysqlExtendParamTest(TestCase):
    def test_get_mysql_protection_support_sub_types(self):
        """
        判断获取mysql保护所支持的资源sub type
        :return:
        """
        sub_types = MysqlProtectionExtParam.support_values()
        self.assertEqual(sub_types, [ResourceSubTypeEnum.MysqlInstance, ResourceSubTypeEnum.MysqlDatabase,
                             ResourceSubTypeEnum.MysqlClusterInstance])


if __name__ == '__main__':
    unittest.main(verbosity=2)
