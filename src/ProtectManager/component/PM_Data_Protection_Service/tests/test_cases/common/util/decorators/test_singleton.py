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

from app.common.util.decorators.singleton import singleton


class A:
    pass


@singleton
class B:
    pass


class TestSingletonDecorator(unittest.TestCase):
    def test_non_singleton_class_success(self):
        a1 = A()
        a2 = A()
        self.assertNotEqual(id(a1), id(a2))

    def test_singleton_class_success(self):
        b1 = B()
        b2 = B()
        self.assertEqual(id(b1), id(b2))
