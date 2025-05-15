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


class TestGaussDBTBackupExtParam(TestCase):
    def setUp(self) -> None:
        super(TestGaussDBTBackupExtParam, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.backup.schemas.extends.gaussdbt_backup_ext_param import GaussDBTExtendParam

        self.GaussDBTExtendParam = GaussDBTExtendParam

    def test_sla_param_when_param_is_true(self):
        """
          *用例场景：参数正确
          *前置条件：接口正常
          *检查点:入参限定
        """
        ext_parameters_1 = {'auto_retry': True,
                            'auto_retry_times': 3,
                            'auto_retry_wait_minutes': 5,
                            'slave_node_first': True,
                            'qos_id': '123'}
        ext = self.GaussDBTExtendParam(**ext_parameters_1)
        self.assertEqual(ext.dict(), ext_parameters_1)

    def test_sla_param_when_param_is_false(self):
        """
          *用例场景：参数不正确
          *前置条件：接口不正确
          *检查点:入参限定
          """
        ext_parameters_2 = {
            "qos_id": "123",
            "auto_retry": True,
            "auto_retry_times": 3,
            "auto_retry_wait_minutes": 5,
            "slave_node_first": None
        }
        with self.assertRaises(ValidationError):
            self.GaussDBTExtendParam(**ext_parameters_2)


if __name__ == '__main__':
    unittest.main(verbosity=2)
