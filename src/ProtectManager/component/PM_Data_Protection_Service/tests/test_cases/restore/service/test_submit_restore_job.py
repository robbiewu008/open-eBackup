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
from unittest import mock

from app.restore.schema.restore import RestoreRequestSchema, RestoreType
from app.common.enums.resource_enum import ResourceSubTypeEnum


def test_content_get(key="", t="restore_type"):
    if t in "object_type":
        return ResourceSubTypeEnum.HyperV
    if t in "restore_type":
        return RestoreType.CR.value
    return None


def test_prepare_content_get(key="restore_type", t=dict):
    if key in "target":
        return {'env_id': 'env_id', 'env_type': 'env_type'}
    if key in "restore_type":
        return RestoreType.CR.value
    return None


class SubmitRestoreJobTest(unittest.TestCase):
    def setUp(self):
        super(SubmitRestoreJobTest, self).setUp()
        self.mock_context = mock.Mock()
        sys.modules['app.common.context.context'] = mock.Mock()
        sys.modules['app.common.config'] = mock.Mock()


    @mock.patch("app.restore.service.service.SystemBaseHttpsClient")
    @mock.patch("app.restore.service.service.RestoreClient")
    def test_submit_restore_job(self, mock_restore_client, mock_system_base_https_client):
        from app.restore.service import service
        source = {"source_location": "win", "source_name": "test"}
        target = {"details": [], "restore_target": "C:/", "env_id": "1d6", "env_type": "ms.HostSystem"}
        restore_req = RestoreRequestSchema(copy_id="adf", object_type="ms.VirtualMachine", source=source, filters=[],
                                           restore_type="CR", target=target, restore_objects=[], ext_parameters="")
        mock_restore_client.create_task = mock.Mock(return_value={None})
        mock_system_base_https_client_instance = mock_system_base_https_client.return_value
        mock_system_base_https_client_instance.request.return_value.status = 200
        with self.assertRaises(TypeError):
            service.submit_restore_job(restore_req, "abc")

    def test_restore_process(self):
        from app.restore.service import service
        from app.common.events.consumer import EsEvent
        request = EsEvent(request_id="123")
        data = service.restore_process(request)
        self.assertIsNotNone(data)

    @mock.patch("app.restore.service.service.Context")
    def test_restore_prepare(self, mock_Context):
        from app.restore.service import service
        from app.common.events.consumer import EsEvent

        request = EsEvent(request_id="123")
        mock_Context.return_value.get = mock.Mock(side_effect=[
            {'env_id': "123", 'env_type': "FileSet"},
            {'source_name': "my_source"},
            '456',
            "Oracle"
        ])
        data = service.restore_prepare(request)
        self.assertIsNotNone(data)


if __name__ == '__main__':
    unittest.main(verbosity=2)