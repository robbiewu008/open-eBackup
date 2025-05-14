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

from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException
from app.protection.object.schemas.extends.params.kubernetes_ext_param import NamespaceExtParam, check_resource_filters, \
    StatefulSetExtParam


class KubernetesExtParamTest(TestCase):
    def test_should_raise_IllegalParamException_if_script_is_error(self):
        """
        自验前置脚本格式不符合校验抛出异常
        :return:
        """
        with self.assertRaises(ValidationError):
            values = NamespaceExtParam(
                pre_script="/opt/sssh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)


    def test_check_resource_filters_sucess(self):
        parameters = [{"filter_by": "NAME", "type":
                          "VM", " rule": "ALL", "mode": "INCLUDE", "values": ["b"]},
                                           {"filter_by": "NAME", "type": "VM", "rule": "ALL",
                                            "mode": "EXCLUDE", "values": ["b"]}]
        filters = check_resource_filters(parameters)
        self.assertEqual(len(filters), len(parameters))

    def test_check_resource_filters_raise_IllegalParamException_if_value_none(self):
        parameters = [{"filter_by": "NAME", "type":
                          "VM", " rule": "ALL", "mode": "INCLUDE", "values": []},
                                           {"filter_by": "NAME", "type": "VM", "rule": "ALL",
                                            "mode": "EXCLUDE", "values": ["b"]}]
        with self.assertRaises(EmeiStorBizException):
            check_resource_filters(parameters)

    def test_namespace_check_ext_params_agents_raise_IllegalParamException_if_value_none(self):
        with self.assertRaises(EmeiStorBizException):
            NamespaceExtParam.check_namespace_ext_params_agents("")

    def test_statefulset_check_ext_params_agents_raise_IllegalParamException_if_value_none(self):
        with self.assertRaises(EmeiStorBizException):
            StatefulSetExtParam.check_stateful_set_ext_params_agents("")

if __name__ == '__main__':
    unittest.main(verbosity=2)
