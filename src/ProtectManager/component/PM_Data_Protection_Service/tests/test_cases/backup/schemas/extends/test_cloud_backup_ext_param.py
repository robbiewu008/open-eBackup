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
from unittest.mock import Mock, patch

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException

ext_parameters_1 = {
                "qos_id": "77542262-f070-4862-8890-2662eb263266",
                "auto_index": True,
                "auto_retry": True,
                "auto_retry_times": 3,
                "auto_retry_wait_minutes": 5,
                "storage_id": "1213",
                "open_aggregation": True,
                "network_acceleration": False,
                "is_synthetic_full_copy_period": True,
                "synthetic_full_copy_period": 1
            }

ext_parameters_2 = {
                "qos_id": "77542262-f070-4862-8890-2662eb263266",
                "auto_index": True,
                "auto_retry": True,
                "auto_retry_times": 3,
                "auto_retry_wait_minutes": 5,
                "storage_id": "1213",
                "open_aggregation": True,
                "network_acceleration": True,
                "is_synthetic_full_copy_period": True,
                "synthetic_full_copy_period": 1
            }

ext_parameters_3 = {
                "qos_id": "77542262-f070-4862-8890-2662eb263266",
                "auto_index": True,
                "auto_retry": True,
                "auto_retry_times": 3,
                "auto_retry_wait_minutes": 5,
                "storage_id": "1213",
                "open_aggregation": True,
                "network_acceleration": False,
                "is_synthetic_full_copy_period": True,
                "synthetic_full_copy_period": 1
            }

error = IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["network_acceleration and qos is mutually exclusive"])


class TestCloudBackupExtParam(TestCase):
    def setUp(self) -> None:

        super(TestCloudBackupExtParam, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.backup.schemas.extends.cloud_backup_ext_param import CloudBackupExtendParam, ArchiveClient

        self.CloudBackupExtParam = CloudBackupExtendParam
        self.ArchiveClient = ArchiveClient

    def test_validate_storage_id_when_exit(self):
        storage_id = "123"
        storage_info = {"storage_id": "1"}
        self.ArchiveClient.query_storage_info = Mock(return_value=storage_info)
        ext = self.CloudBackupExtParam(**ext_parameters_3).validate_storage_id('123')
        self.assertEqual(ext, storage_id)

    def test_validate_storage_id_when_not_exit(self):
        storage_info = None
        self.ArchiveClient.query_storage_info = Mock(return_value=storage_info)
        with self.assertRaises(IllegalParamException):
            self.CloudBackupExtParam(**ext_parameters_2).validate_storage_id('123')

    def test_check_acceleration_qos_when_simultaneously_enabled(self):
        with self.assertRaises(IllegalParamException):
            self.CloudBackupExtParam(**ext_parameters_2).check_acceleration_qos()

    def test_check_acceleration_qos_when_not_simultaneously_enabled(self):
        storage_info = {"storage_id": "1"}
        self.ArchiveClient.query_storage_info = Mock(return_value=storage_info)
        self.CloudBackupExtParam.validate_storage_id = Mock(return_value='123')

        res = self.CloudBackupExtParam(**ext_parameters_1).check_acceleration_qos(ext_parameters_1)
        self.assertEqual(res, ext_parameters_1)

    def test_is_synthetic_full_copy_period_when_min(self):
        ext_parameters_1['synthetic_full_copy_period'] = 0
        storage_info = {"storage_id": "1"}
        self.ArchiveClient.query_storage_info = Mock(return_value=storage_info)
        self.CloudBackupExtParam.validate_storage_id = Mock(return_value='123')
        with self.assertRaises(IllegalParamException):
            self.CloudBackupExtParam(**ext_parameters_1).check_acceleration_qos(ext_parameters_1)

    def test_is_synthetic_full_copy_period_when_max(self):
        ext_parameters_1['synthetic_full_copy_period'] = 101
        storage_info = {"storage_id": "1"}
        self.ArchiveClient.query_storage_info = Mock(return_value=storage_info)
        self.CloudBackupExtParam.validate_storage_id = Mock()
        with self.assertRaises(IllegalParamException):
            self.CloudBackupExtParam(**ext_parameters_1).check_acceleration_qos(ext_parameters_1)


class TestCloudBackupActionExtendParam(TestCase):
    def setUp(self) -> None:
        super(TestCloudBackupActionExtendParam, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.backup.schemas.extends.cloud_backup_ext_param import CloudBackupSnapshotExtendParam

        self.CloudBackupSnapshotExtendParam = CloudBackupSnapshotExtendParam

    def test_is_cloud_backup_by_action(self):
        res = self.CloudBackupSnapshotExtendParam(**{"is_security_snap": True,
                                                     "auto_retry": True,
                                                     "auto_retry_times": 3,
                                                     "auto_retry_wait_minutes": 5})
        self.assertIsNotNone(res)


if __name__ == '__main__':
    unittest.main(verbosity=2)
