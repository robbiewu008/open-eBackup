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
from unittest.mock import Mock

from app.resource.service.vmware.vmware_tag_service import VMTagReader
from tests.test_cases import common_mocker  # noqa


class TagServiceTest(unittest.TestCase):

    def test_get_vm_tags_success(self):
        all_attached_objects_on_tags = [
            {
                "object_ids": [
                    {
                        "id": "vm-1234",
                        "type": "VirtualMachine"
                    }
                ],
                "tag_id": "urn:vmomi:InventoryServiceTag:7b1de48f-72f4-46b2-861a-b88150a4e4df:GLOBAL"
            },
            {
                "object_ids": [
                    {
                        "id": "vm-1234",
                        "type": "VirtualMachine"
                    },
                    {
                        "id": "vm-2345",
                        "type": "VirtualMachine"
                    }
                ],
                "tag_id": "urn:vmomi:InventoryServiceTag:ef150460-445a-4a9b-99f5-31b5d2fc7771:GLOBAL"
            }
        ]
        tag_details = [
            {
                "category_id": "urn:vmomi:InventoryServiceCategory:7e64f053-bb80-4525-854c-a08a463b9d96:GLOBAL",
                "name": "cs_test1",
                "description": "test1",
                "id": "urn:vmomi:InventoryServiceTag:7b1de48f-72f4-46b2-861a-b88150a4e4df:GLOBAL",
                "used_by": []
            },
            {
                "category_id": "urn:vmomi:InventoryServiceCategory:7e64f053-bb80-4525-854c-a08a463b9d96:GLOBAL",
                "name": "cs_test2",
                "description": "test2",
                "id": "urn:vmomi:InventoryServiceTag:ef150460-445a-4a9b-99f5-31b5d2fc7771:GLOBAL",
                "used_by": []
            }
        ]
        VMTagReader.connect = Mock(return_value=None)
        VMTagReader.disconnect = Mock(return_value=None)
        VMTagReader.list_all_attached_objects_on_tags = Mock(return_value=all_attached_objects_on_tags)
        VMTagReader.get_tags = Mock(return_value=tag_details)

        with VMTagReader('1.1.1.1', '443', 'username', 'passwd') as tag_reader:
            tags = tag_reader.get_vm_tags('vm-1234')
            self.assertEqual(tags, 'cs_test1,cs_test2')
            self.assertEqual(('username'.encode(), 'passwd'.encode()), tag_reader.session.auth)
        # ipv6地址测试
        with VMTagReader('2016:8:40:96:c11::220', 444, 'username', 'passwd') as tag_reader:
            tags = tag_reader.get_vm_tags('vm-1234')
            self.assertEqual(tags, 'cs_test1,cs_test2')

        with VMTagReader('1.1.1.1', '443', 'username', 'passwd') as tag_reader:
            tags = tag_reader.get_vm_tags('vm-4567')
            self.assertEqual(tags, '')
            self.assertEqual(('username'.encode(), 'passwd'.encode()), tag_reader.session.auth)


if __name__ == '__main__':
    unittest.main()
