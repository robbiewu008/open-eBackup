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

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum


class CloudBackupExtParamExtValuesMock:
    share_type = "NFS"


class TestCloudBackupExtParam(TestCase):
    def setUp(self) -> None:
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.backup.schemas.extends.dws_backup_ext_param import GaussDBTExtendParam
        self.GaussDBTExtendParam = GaussDBTExtendParam

    def test_type(self):
        """
        *用例场景：类型是否正常
        *前置条件：接口正常
        *检查点:入参限定
        """
        res = self.GaussDBTExtendParam.is_support(ResourceSubTypeEnum.DWSCluster, PolicyTypeEnum.backup)
        self.assertEqual(res, True)

    def test_check_param(self):
        """
        *用例场景：参数是否正常
        *前置条件：接口正常
        *检查点:入参限定
        """
        ext_parameters_2 = {
            "qos_id": "77542262-f070-4862-8890-2662eb263266",
            "auto_index": True,
            "auto_retry": True,
            "auto_retry_times": 3,
            "auto_retry_wait_minutes": 5,
            "storage_id": "1213",
        }
        ext_nfs = self.GaussDBTExtendParam(**ext_parameters_2)
        self.assertIsNotNone(ext_nfs)


if __name__ == '__main__':
    unittest.main(verbosity=2)
