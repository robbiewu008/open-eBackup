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
import asyncio
import sys
import unittest
from unittest import mock
from unittest.mock import Mock


class TestRestoreApi(unittest.TestCase):
    def setUp(self):
        super(TestRestoreApi, self).setUp()
        sys.modules['app.common.events.producer'] = mock.Mock()
        sys.modules['app.backup.service'] = mock.Mock()
        sys.modules['app.base.db_base'] = mock.Mock()
        sys.modules['app.protection.object.db'] = mock.Mock()
        sys.modules['app.common.events.consumer'] = mock.Mock()
        sys.modules['app.protection.object.models.projected_object'] = mock.Mock()
        sys.modules['app.protection.object.schemas.protected_object'] = mock.Mock()
        sys.modules['app.resource.models.database_models'] = Mock()
        sys.modules['app.common.context.context'] = Mock()
        sys.modules['app.common.redis_session'] = Mock()
        sys.modules['app.common.logger'] = Mock()
        sys.modules['app.copy_catalog.service.import_copy_service'] = Mock()

    @unittest.skip
    @mock.patch("app.restore.api.restore_api.get_user_id")
    @mock.patch("app.restore.api.restore_api.service")
    @mock.patch("app.common.security.right_control.auth_resource_check")
    @mock.patch("app.common.security.right_control.auth_rule_check")
    @mock.patch("app.common.security.jwt_utils.get_user_info_from_token")
    @mock.patch("app.common.log.kernel.record_log")
    @mock.patch("app.common.log.kernel.page_query_data_by_uuid_list")
    @mock.patch("app.copy_catalog.util.copy_auth_verify_util.check_copy_operation_auth", mock.Mock())
    def test_create_restore(self,
                           mock_page_query_data_by_uuid_list,
                           mock_record_log,
                           mock_token,
                           mock_auth_rule_check,
                           mock_auth_resource_check,
                           mock_service,
                           mock_get_user_id):
        """
        用例名称：创建restore_api任务成功。
        前置条件：参数正常。
        check点：不抛出异常，record_log入参符合预期，能正确抓取到返回值。
        """
        from app.restore.api.restore_api import create_restore
        from app.restore.schema.restore import RestoreRequestSchema
        token = "eyJhbGciOiJSUzI1NiJ9.eyJleHBpcmVzX2F0IjoxNjIzODE2NzgwMDAwLCJpc3MiOiJlbW" \
                "Vpc3Rvci5odWF3ZWkuY29tIiwidXNlcl9yb2xlcyI6W10sImV4cCI6MTYyMzgxNjc4MCwidXNl" \
                "ciI6eyJpZCI6Ijg4YTk0YzQ3NmYxMmEyMWUwMTZmMTJhMjQ2ZTUwMDA5IiwibmFtZSI6InN5c2Fkb" \
                "WluIiwicm9sZXMiOlt7Im5hbWUiOiJSb2xlX1NZU19BZG1pbiIsImlkIjoiMSJ9XX0sImlhdCI6MTY" \
                "yMzIxNjc4MCwiaXNzdWVkX2F0IjoxNjIzMjE2NzgwMDAwfQ.mvoRJeiz7f894VBgqLF2vy4dehwHNG7" \
                "WRDcCGULDBGCJW0wMJks3UIPO6Oykd40TEGypBtAD4Q2RrbhjT8Uez41s20VA0SnBM7g2NKvtjsR-tO" \
                "O5AhbniT-JAXpK1d3yT-Ha4QmMe4lDDv7ofK5siXP5Sh9YbENfSWwwoqsuhZoC4JJfPthmbXy6USvFn4" \
                "C2K7yp2VgDPo46YQSdcuJ7dx6R9yD4IthefzdZHdYyE3p5XBRpV0IIdmJOfouiz7RtW9BpQAAjqe9AhRmo" \
                "tEOeQt3d6Nw6xmodSvzfXDHiTj5mZm1nxOWpjEE3SzAvy-bqpbXL-M_d8hcCMKNnhcQSNw"

        restore_req = {
            "request_id": "123-456",
            "copy_id": "dccff80c-4c78-4df4-9047-a86d3b66b951",
            'source': {
                    'source_name': 'str',
                    'source_location': 'str'
                },
                'object_type': 'File',
                'restore_location': 'O',
                'restore_type': 'CR',
                'restore_objects': [
                    'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/grub',
                    'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/efi'
                ],
                'target': {
                    'env_id': '266ea41d-adf5-480b-af50-15b940c2b846',
                    'env_type': 'Host',
                    'restore_target': '/tmp',
                    'details': [{
                        'src_id': ['gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/ef'],
                        'target_id': '/tmp',
                        'target_type': 'File'
                    }
                    ]
                },
                'filters': [{
                    'type': 1,
                    'model': 1,
                    'content': ''

                }],
                'ext_parameters': {
                    'FILE_REPLACE_STRATEGY': 'replace/ignore/replace_old_file',
                    'BEFORE_RESTORE_SCRIPT': 'before_restore_script.sh',
                    'AFTER_RESTORE_SCRIPT': 'after_restore_script.sh',
                    'RESTORE_FAILED_SCRIPT': 'restore_failed_script.sh',
                    'RESTORE_TO_SINGLE_DIRECTORY': 'true',
                    'CHANNELS': 100,  # 并行管道数
                    'IS_SINGLE_RESTORE': 0,  # 文件集是否单目录恢复：0否，1是
                    'PASSOWRD': 'PASSWORD',
                }
        }
        mock_service.submit_restore_job = Mock(return_value=["123-456"])
        mock_page_query_data_by_uuid_list.return_value = [
            {'resource_name': 'testFile',
             'display_timestamp': 123456}]

        asyncio.run(create_restore(
            token=token,
            resource_id="789",
            restore_req=RestoreRequestSchema(**restore_req)))
        args, kwargs = mock_record_log.call_args
        self.assertEqual(args[2][2], ["123-456"])
        self.assertEqual(len(args[2]), 3)

if __name__ == '__main__':
    unittest.main(verbosity=2)