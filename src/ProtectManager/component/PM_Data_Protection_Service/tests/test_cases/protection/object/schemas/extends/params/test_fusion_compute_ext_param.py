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

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.params.fusion_compute_param import FusionComputeExtParam


class FusionComputeExtParamTest(TestCase):
    def test_should_raise_IllegalParamException_if_script_is_error(self):
        """
        自验前置脚本格式不符合校验抛出异常
        :return:
        """
        with self.assertRaises(ValidationError):
            values = FusionComputeExtParam(
                pre_script="/opt/sssh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)


    def test_get_fusion_compute_support_sub_type(self):
        """
        判断获取FusionCompute保护所支持的资源sub type
        :return:
        """
        sub_types = FusionComputeExtParam.support_values()
        self.assertEqual(sub_types, [ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.FUSION_ONE_COMPUTE])


if __name__ == '__main__':
    unittest.main(verbosity=2)
