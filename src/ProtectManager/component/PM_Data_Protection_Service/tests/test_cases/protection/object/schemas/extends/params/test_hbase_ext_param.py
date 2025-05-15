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

from pydantic import ValidationError

from app.protection.object.schemas.extends.params.hbase_ext_param import HbaseBackupSetFilesetExtParam


class HDFSFilesetExtParamTest(TestCase):
    def test_should_raise_IllegalParamException_if_agents_is_miss(self):
        """
        自前后置脚本格式不符合校验抛出异常
        :return:
        """
        with self.assertRaises(ValidationError):
            values = HbaseBackupSetFilesetExtParam(
                pre_script="/opt/sssh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)


if __name__ == '__main__':
    unittest.main(verbosity=2)
