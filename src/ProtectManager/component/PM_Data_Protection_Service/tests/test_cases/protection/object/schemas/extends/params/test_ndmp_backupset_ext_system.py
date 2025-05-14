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

from app.protection.object.common.protection_enums import NasProtocolType
from app.protection.object.schemas.extends.params.ndmp_backupset_ext_system import NdmpExtParam
from app.common.exception.unified_exception import IllegalParamException


class NdmpExtValuesMock:
    def __init__(self, protocol, filters):
        self.protocol = protocol
        self.filters = filters


class TestNdmpExtParam(TestCase):
    def test_check_ndmp_ext_param_success(self):
        """
        验证场景：测试NDMP共享保护扩展参数
        前置条件：参数正常
        验证点：无异常抛出
        """
        values = NdmpExtValuesMock(NasProtocolType.NDMP.value, ['/opt'])
        result = NdmpExtParam.check_ext_params(values.__dict__)
        self.assertEqual(result, values.__dict__)

    def test_check_ndmp_ext_param_protocol_error(self):
        """
        验证场景：测试NDMP共享保护扩展参数，协议错误
        前置条件：无
        验证点：异常抛出
        """
        values = NdmpExtValuesMock(NasProtocolType.NFS.value, ['/opt'])
        with self.assertRaises(IllegalParamException):
            NdmpExtParam.check_ext_params(values.__dict__)

    def test_check_ndmp_ext_param_single_filter_error(self):
        """
        验证场景：测试NDMP共享保护扩展参数，单个过滤条件过长
        前置条件：无
        验证点：异常抛出
        """
        values = NdmpExtValuesMock(NasProtocolType.NDMP.value, ['/opopopopopopopopopopopopopopopopopopopopopopopo'
                                                                'popopopopopopopopopopopopopopopopopopopopopopopopopopo'
                                                                'popopopopopopopopopopopopopopopopopopopopopopopopopopo'
                                                                'popopopopopopopopopopopopopopopopopopopopopopopopopopo'
                                                                'popopopopopopopopopopopopopopoooiiiikkkkkkkkkk'])
        with self.assertRaises(IllegalParamException):
            NdmpExtParam.check_ext_params(values.__dict__)


if __name__ == '__main__':
    unittest.main(verbosity=2)
