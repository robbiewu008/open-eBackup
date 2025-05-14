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
# import unittest
# from unittest import mock
# from tests.test_cases.tools import http, env, timezone
#
# mock.patch("requests.get", http.get_request).start()
# mock.patch("requests.post", http.post_request).start()
# mock.patch("os.getenv", env.get_env).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
#            timezone.dmc.query_time_zone).start()
#
# from app.restore.client.restore_client import RestoreClient
# from app.restore.schema.restore import RestoreRequestSchema
#
#
# class CopyClientTest(unittest.TestCase):
#     def setUp(self):
#         super(CopyClientTest, self).setUp()
#         self.restoreClient = RestoreClient
#
#     def test_create_task_success(self):
#         source = {"source_location": "win", "source_name": "test"}
#         target = {"details": [], "restore_target": "C:/", "env_id": "1d6", "env_type": "ms.HostSystem"}
#         restore_req = RestoreRequestSchema(copy_id="adf", object_type="ms.VirtualMachine", source=source, filters=[],
#                                            restore_type="CR", target=target, restore_objects=[], ext_parameters="")
#         response = self.restoreClient.create_task(restore_req)
#         self.assertIsNotNone(response)
#
#     def test_get_target_database_success(self):
#         target_uuid = self.restoreClient.get_target_database("123", "copy")
#         self.assertEqual(target_uuid, "123")
#
#
