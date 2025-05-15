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
import json
import unittest
from app.common.exception import exception_handler


class TestReplacePlaceHolder(unittest.TestCase):
    def test_retryable_exception(self):
        res = exception_handler.request_exception_handler(None, Exception)
        res = json.loads(res.body)
        self.assertEqual(res.get('retryable'), True)


if __name__ == '__main__':
    unittest.main(verbosity=2)
