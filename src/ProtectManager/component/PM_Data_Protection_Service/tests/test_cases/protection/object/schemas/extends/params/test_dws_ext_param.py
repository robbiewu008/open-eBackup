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
from app.protection.object.common.protection_enums import GaussDBTableTypeEnum


class CloudBackupExtParamExtValuesMock:
    share_type = "NFS"


class TestCloudBackupExtParam(TestCase):
    def setUp(self) -> None:
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.protection.object.schemas.extends.params.dws_ext_param import GaussDBDwsProtectionExtParam
        self.GaussDBDwsProtectionExtParam = GaussDBDwsProtectionExtParam

    def test_type(self):
        """
        *用例场景：元数据备份路径参数是否正常
        *前置条件：接口正常
        *检查点:入参限定
        """
        test_dws_list = [ResourceSubTypeEnum.DWSCluster, ResourceSubTypeEnum.DWSDateBase,
                         ResourceSubTypeEnum.DWSSchema, ResourceSubTypeEnum.DWSTable]
        res = self.GaussDBDwsProtectionExtParam.support_values()
        self.assertEqual(res, test_dws_list)

    def test_check_param(self):
        """
        *用例场景：表集备份方式参数是否正常
        *前置条件：接口正常
        *检查点:入参限定
        """
        GaussDBDws_T = {
            "backup_metadata_path": "/xxx",
            "backup_tool_type": GaussDBTableTypeEnum.ROACH
        }
        ext_nfs = self.GaussDBDwsProtectionExtParam(**GaussDBDws_T)
        self.assertIsNotNone(ext_nfs)


if __name__ == '__main__':
    unittest.main(verbosity=2)
