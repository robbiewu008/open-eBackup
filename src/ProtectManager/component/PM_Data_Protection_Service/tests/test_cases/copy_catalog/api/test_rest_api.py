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
from unittest.mock import Mock

from app.common.exception.unified_exception import IllegalParamException
from app.copy_catalog.models.tables_and_sessions import CopyTable
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()

from app.common.schemas.common_schemas import BasePage


class TestRestApi(unittest.TestCase):
    def setUp(self):
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        from app.copy_catalog.service import anti_ransomware_service, copy_delete_workflow
        from app.copy_catalog.api import anti_api, rest_api
        self.anti_ransomware_service = anti_ransomware_service
        self.anti_api = anti_api
        self.rest_api = rest_api
        self.copy_delete_workflow = copy_delete_workflow

    def test_query_anti_ransomware_copies_resource_success(self):
        resource_sub_type = "vim.VirtualMachine"
        page_no: int = 0
        page_size: int = 10
        self.anti_ransomware_service.query_anti_ransomware_copies_resource = Mock(return_value=
                                                                                  BasePage(items=[], total=0, pages=0,
                                                                                           page_no=page_no,
                                                                                           page_size=0))
        copies = self.anti_api.query_anti_ransomware_copies_resource(resource_sub_type, page_no,
                                                            page_size, None, None)
        self.assertIsNotNone(copies)

    def test_query_anti_ransomware_copies_summary_success(self):
        resource_sub_type = ["vim.VirtualMachine"]
        self.anti_ransomware_service.query_anti_ransomware_copies_summary = Mock(return_value=[])
        copies = self.anti_api.query_anti_ransomware_copies_summary(resource_sub_type)
        self.assertIsNotNone(copies)

    def test_delete_log_copy(self):
        self.copy_delete_workflow.get_deleting_copy = Mock(return_value=CopyTable(backup_type=4))
        copy_id = 'copyId'
        with self.assertRaises(IllegalParamException):
            self.rest_api.check_copy_can_be_delete(copy_id)


if __name__ == '__main__':
    unittest.main(verbosity=2)