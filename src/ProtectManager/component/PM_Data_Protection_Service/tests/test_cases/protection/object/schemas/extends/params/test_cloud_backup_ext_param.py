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

from app.common.deploy_type import DeployType
from app.common.exception.unified_exception import EmeiStorBizException


class CloudBackupExtParamExtValuesMock:
    share_type = "NFS"


class TestCloudBackupExtParam(TestCase):
    def setUp(self) -> None:
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        from app.protection.object.schemas.extends.params.cloud_backup_ext_param import CloudBackupExtParam, \
            SystemBaseClient
        self.CloudBackupExtParam = CloudBackupExtParam
        self.SystemBaseClient = SystemBaseClient

    def test_check_share_type(self):
        """
        *用例场景：校验NFS和CFIS参数是否正常
        *前置条件：接口正常
        *检查点:入参限定
        """
        self.SystemBaseClient.query_filesystem = Mock(return_value=None)
        CloudBackup_NFS = {
            "share_type": "NFS",
            "file_system_ids": ["123"]
        }
        ext_nfs = self.CloudBackupExtParam(**CloudBackup_NFS)

        CloudBackup_CIFS = {
            "share_type": "CIFS",
            "file_system_ids": ["123"]
        }
        ext_cifs = self.CloudBackupExtParam(**CloudBackup_CIFS)
        self.assertEqual(ext_nfs.share_type, CloudBackup_NFS['share_type'])
        self.assertEqual(ext_nfs.file_system_ids, CloudBackup_NFS['file_system_ids'])
        self.assertEqual(ext_cifs.share_type, CloudBackup_CIFS['share_type'])
        self.assertEqual(ext_cifs.file_system_ids, CloudBackup_CIFS['file_system_ids'])

    def test_check_synchronize_replication_secondary_when_filesystem_none(self):
        """
        *用例场景：文件系统为空
        *前置条件：接口正常
        *检查点:入参限定
        """
        self.SystemBaseClient.query_filesystem = Mock(return_value=None)
        CloudBackup_NFS = {
            "share_type": "NFS",
            "file_system_ids": ["123"]
        }
        ext_nfs = self.CloudBackupExtParam(**CloudBackup_NFS)
        self.assertEqual(ext_nfs.share_type, CloudBackup_NFS['share_type'])
        self.assertEqual(ext_nfs.file_system_ids, CloudBackup_NFS['file_system_ids'])

    def test_check_synchronize_replication_secondary_when_replication_pair_none(self):
        """
        *用例场景：异步同步查询为空
        *前置条件：接口正常
        *检查点:入参限定
        """
        file_system_info = {
            "id": "0",
            "name": "123",
            "remoteReplicationIds": ["123", "123"]
        }

        self.SystemBaseClient.query_filesystem = Mock(return_value=file_system_info)
        self.SystemBaseClient.query_replication_pair = Mock(return_value=None)
        CloudBackup_NFS = {
            "share_type": "NFS",
            "file_system_ids": ["123"]
        }
        ext_nfs = self.CloudBackupExtParam(**CloudBackup_NFS)
        self.assertEqual(ext_nfs.share_type, CloudBackup_NFS['share_type'])
        self.assertEqual(ext_nfs.file_system_ids, CloudBackup_NFS['file_system_ids'])

    def test_check_synchronize_replication_secondary(self):
        """
        *用例场景：同步复制从端不允许保护
        *前置条件：接口正常
        *检查点:同步复制从端不允许保护
        """
        file_system_info = {
            "id": "0",
            "name": "123",
            "remoteReplicationIds": ["123", "123"]
        }
        replication_pair_info = {
            "id": "123",
            "replicationModel": 1,
            "primary": False
        }
        self.SystemBaseClient.query_filesystem = Mock(return_value=file_system_info)
        self.SystemBaseClient.query_replication_pair = Mock(return_value=replication_pair_info)
        CloudBackup_NFS = {
            "share_type": "NFS",
            "file_system_ids": ["123"]
        }
        ext_nfs = self.CloudBackupExtParam(**CloudBackup_NFS)
        self.assertEqual(ext_nfs.share_type, CloudBackup_NFS['share_type'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
