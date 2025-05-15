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

from app.common.enums.resource_enum import ResourceSubTypeWithOrderEnum


class ResourceSubTypeWithOrderEnumTest(unittest.TestCase):
    def test_get_resource_sub_type_order_success(self):
        """
        用例场景：获取resource_sub_type order，order告警国际化会使用
        前置条件：无
        检查点：获取order成功
        """
        resource_sub_type = "SQLServer-clusterInstance"
        self.assertEqual(ResourceSubTypeWithOrderEnum.get_order(resource_sub_type), 15)


    def test_get_sub_type_san_client_success(self):
        """
        用例场景：获取resource_sub_type order，order告警国际化会使用
        前置条件：无
        检查点：获取order成功
        """
        resource_sub_type = "SBackupAgent"
        self.assertEqual(ResourceSubTypeWithOrderEnum.get_order(resource_sub_type), 149)
