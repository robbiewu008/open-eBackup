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

from pydantic import ValidationError

from app.common.exception.unified_exception import EmeiStorBizException
from app.protection.object.schemas.extends.params.volume_protection_ext_param import VolumeProtectionExtParam


from tests.test_cases import common_mocker  # noqa

class VolumeExtParamTest(TestCase):

    def test_should_raise_IllegalParamException_if_script_Illegal(self):
        """
        自前后置脚本格式不符合校验抛出异常
        :return:
        """
        with self.assertRaises(ValidationError):
            values = VolumeProtectionExtParam(
                pre_script="/opt/sssh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_if_script_regix(self):
        values = VolumeProtectionExtParam(
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.bat")
        self.assertTrue(True, "无异常抛出")


    def test_script_path_with_os_type(self):
        sys.modules['app.protection.object.db'] = Mock()
        sys.modules['app.resource.service.common.resource_service'] = Mock()
        from app.protection.object.schemas.protected_object import check_os_type_path
        with self.assertRaises(EmeiStorBizException):
            script_path = ['a.sh', 'a.sh', None]
            check_os_type_path("windows", script_path)

if __name__ == '__main__':
    unittest.main(verbosity=2)
