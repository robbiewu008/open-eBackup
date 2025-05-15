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

from app.common.clients.resource_client import ResourceClient
from app.protection.object.models.projected_object import ProtectedObject


class TestResourceClient(unittest.TestCase):
    def test_umount_agent_and_lan_free(self):
        """
        *用例场景：检查解除agent挂载是否成功
        *前置条件：参数为空
        *检查点: 检查解除agent挂载是否成功
        """
        projected_object = ProtectedObject()
        ret = ResourceClient.umount_agent_and_lan_free(projected_object)
        self.assertEqual(ret, {})
