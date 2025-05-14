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
from app.protection.object.schemas.extends.params.fileset_protection_ext_param import FilesetProtectionExtParam
from tests.test_cases.copy_catalog.util.mock_util import common_mock


from tests.test_cases import common_mocker  # noqa

class FilesetExtParamTest(TestCase):

    def test_should_raise_IllegalParamException_if_script_Illegal(self):
        """
        自前后置脚本格式不符合校验抛出异常
        :return:
        """
        with self.assertRaises(ValidationError):
            values = FilesetProtectionExtParam(
                pre_script="/opt/sssh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_should_raise_IllegalParamException_if_param_Illegal(self):
        """
        打开“小文件聚合”开关时，无“聚合文件大小”和“待聚合文件最大大小”参数报错
        :return:
        """
        with self.assertRaises(IllegalParamException):
            values = FilesetProtectionExtParam(
                cross_file_system=True,
                consistent_backup=True,
                backup_nfs=True,
                channels=2,
                backup_sms=True,
                sparse_file_detection=True,
                backup_continue_with_files_backup_failed=True,
                small_file_aggregation=True,
                pre_script="/opt/ss.sh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_fileset_ext_params_aggregation_file_size_illegal(self):
        """"
        聚合文件大小输入有误，聚合文件大小只允许输入（128、256、1204、2048、4096）
        :return:
        """
        with self.assertRaises(IllegalParamException):
            values = FilesetProtectionExtParam(
                cross_file_system=True,
                consistent_backup=True,
                backup_nfs=True,
                backup_sms=True,
                channels=2,
                sparse_file_detection=True,
                backup_continue_with_files_backup_failed=True,
                small_file_aggregation=True,
                aggregation_file_size=100,
                aggregation_file_max_size=1024,
                pre_script="/opt/ss.sh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_fileset_ext_params_aggregation_file_max_size_illegal(self):
        """"
        待聚合文件大小最大值输入有误，待聚合文件大小最大值只允许输入（128、256、1204、20486、4096）
        :return:
        """
        with self.assertRaises(IllegalParamException):
            values = FilesetProtectionExtParam(
                cross_file_system=True,
                consistent_backup=True,
                backup_nfs=True,
                channels=2,
                backup_sms=True,
                sparse_file_detection=True,
                backup_continue_with_files_backup_failed=True,
                small_file_aggregation=True,
                aggregation_file_size=128,
                aggregation_file_max_size=100,
                pre_script="/opt/ss.sh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_fileset_sla_ext_params_aggregation_file_size_big_than_max_size(self):
        """"
        保护扩展参数“待聚合文件最大大小”要小于等于“聚合文件大小”
        :return:
        """
        with self.assertRaises(IllegalParamException):
            values = FilesetProtectionExtParam(
                cross_file_system=True,
                consistent_backup=True,
                backup_nfs=True,
                channels=2,
                backup_sms=True,
                sparse_file_detection=True,
                backup_continue_with_files_backup_failed=True,
                small_file_aggregation=True,
                aggregation_file_size=128,
                aggregation_file_max_size=1024,
                pre_script="/opt/ss.sh",
                post_script="/opt/ss.sh",
                failed_script="/opt/ss.sh")
            self.assertRaises(ValidationError, values)

    def test_fileset_sla_ext_params_correct(self):
        """"
        小文件聚合参数正确，NFS开启且跨文件备份系统开启
        :return:
        """

        values = FilesetProtectionExtParam(
            cross_file_system=True,
            consistent_backup=True,
            backup_nfs=True,
            channels=2,
            backup_sms=True,
            sparse_file_detection=True,
            backup_continue_with_files_backup_failed=True,
            small_file_aggregation=True,
            aggregation_file_size=4096,
            aggregation_file_max_size=1024,
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh")
        self.assertTrue(True, "无异常抛出")


    def test_if_script_regix(self):
        values = FilesetProtectionExtParam(
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
