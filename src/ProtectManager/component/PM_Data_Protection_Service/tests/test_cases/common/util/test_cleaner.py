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

from app.common.util.cleaner import clear


class TestCleaner(unittest.TestCase):
    def test_clear_000(self):
        target = None
        clear(target)
        password = None
        clear(password)
        self.assertEqual(target, None)
        self.assertEqual(password, None)

    def test_clear_001(self):
        target = ""
        clear(target)
        password = ""
        clear(password)
        self.assertNotEqual(password, None)
        self.assertEqual(len(password), 0)
        self.assertEqual(password, target)
        for index in range(len(password)):
            self.assertEqual(password[index], '\x00')

    def test_clear_002(self):
        target = "0000000000"
        clear(target)
        password = "Huawei@123"
        clear(password)
        self.assertEqual(len(password), 10)
        self.assertEqual(password, target)
        for index in range(len(password)):
            self.assertEqual(password[index], '\x00')

    def test_clear_003(self):
        target = "00000000000"
        clear(target)
        password = "Huawei@123"
        clear(password)
        self.assertEqual(len(password), 10)
        self.assertNotEqual(password, target)
        for index in range(len(password)):
            self.assertEqual(password[index], '\x00')
