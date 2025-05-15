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
from datetime import datetime
from unittest import mock

from app.common.exception.unified_exception import IllegalParamException
from app.copy_catalog.schemas import CopyInfoWithArchiveFieldsSchema
from app.restore.schema.restore import DownloadRequestSchema
from app.restore.service.download import Download


class DownloadTest(unittest.TestCase):
    @mock.patch("app.restore.service.download.query_copy_info_by_copy_id")
    def test_download_throw_exception(self, mock_query_copy_info_by_copy_id):
        download_req = DownloadRequestSchema(paths=["list"], recordId="record_id", copyId="copyId", userId="userId")
        download = Download(download_req)
        copy = CopyInfoWithArchiveFieldsSchema(storage_id="storage_id", timestamp="timestamp",
                                               display_timestamp=datetime.now(), deletable=True, status="status",
                                               generated_by="generated_by", indexed="indexed", generation=1,
                                               retention_type=1, resource_id="resource_id",
                                               resource_name="resource_name",
                                               resource_type="resource_type", resource_location="resource_location",
                                               resource_status="resource_status",
                                               resource_properties="resource_properties",
                                               browse_mounted="Umount")
        copy.storage_id = "storage_id"
        mock_query_copy_info_by_copy_id.return_value = copy
        mock_result = mock.Mock(return_value="True")
        download._check_copy_is_using = mock_result
        self.assertRaises(IllegalParamException, download.download)


if __name__ == '__main__':
    unittest.main(verbosity=2)
