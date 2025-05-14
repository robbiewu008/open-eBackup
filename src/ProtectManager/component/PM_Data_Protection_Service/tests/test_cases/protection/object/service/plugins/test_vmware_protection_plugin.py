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
from unittest.mock import Mock

from app.protection.object.common.protection_enums import ResourceFilter, SlaApplyType
from tests.test_cases.common.events import mock_producer

from app.protection.object.schemas.extends.params.vmware_ext_param import VirtualResourceExtParam, VmExtParam


sys.modules['app.common.logger'] = mock.Mock()


class TestVMwareProtectionPlugin(unittest.TestCase):
    def setUp(self):
        super(TestVMwareProtectionPlugin, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        from app.protection.object.service.plugins import vmware_protection_plugin
        self.plugin_service = vmware_protection_plugin
        self.plugin = vmware_protection_plugin.VMwareProtectionPlugin()

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_vm_disk")
    def test_do_convert_extend_parameter(self, mock_query_vm_disk):
        mock_query_vm_disk.return_value = None
        values1 = ["test_value"]
        disk_filters = [
            ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        vm_resource = {"uuid": "fc16731d-a701-42a8-86b0-ef20123c5975"}
        ext_parameters = VirtualResourceExtParam(pre_script="test_pre_script",
                                                 post_script="test_post_script",
                                                 overwrite=True,
                                                 binding_policy=[
                                                     SlaApplyType.APPLY_TO_ALL],
                                                 resource_filters=disk_filters)
        result = self.plugin.do_convert_extend_parameter(disk_filters, vm_resource,
                                                         ext_parameters)
        self.assertIsNone(result)
        mock_query_vm_disk.return_value = [
            {"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d"}]
        result = self.plugin.do_convert_extend_parameter(disk_filters, vm_resource,
                                                         ext_parameters)
        vmextparam = VmExtParam(
            pre_script="test_pre_script",
            post_script="test_post_script",
            all_disk=True,
            disk_info=["a46acc99-21f5-465f-9adb-cabcc2737a3d"]
        )
        self.assertEqual(result, vmextparam)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_resource_list")
    def test_query_sub_resources(self, mock_query_resource_list):
        mock_query_resource_list.return_value = []
        from app.resource.models.resource_models import ResourceTable
        resource_info = ResourceTable(uuid="fc16731d-a701-42a8-86b0-ef20123c5975",
                                      path="1.1.1.1")
        res = self.plugin.query_sub_resources(resource_info)
        self.assertEqual(res, [])

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_resource_list")
    def test_query_sub_resources_by_obj(self, mock_query_resource_list):
        mock_query_resource_list.return_value = []
        from app.protection.object.models.projected_object import ProtectedObject
        obj = ProtectedObject(uuid="fc16731d-a701-42a8-86b0-ef20123c5975",
                              path="1.1.1.1")
        res = self.plugin.query_sub_resources_by_obj(obj)
        self.assertEqual(res, [])

    def test_build_ext_parameters(self):
        ext_parameters_str = "{\"pre_script\":null,\"post_script\":null,\"resource_filters\":[],\"overwrite\":true," \
                             "\"binding_policy\":[\"APPLY_TO_ALL\",\"APPLY_TO_NEW\"]} "
        virtual_resource_ext_param = VirtualResourceExtParam(
            pre_script=None,
            post_script=None,
            resource_filters=[],
            overwrite=True,
            binding_policy=["APPLY_TO_ALL", "APPLY_TO_NEW"]
        )
        res = self.plugin.build_ext_parameters(ext_parameters_str)
        self.assertEqual(res, virtual_resource_ext_param)


if __name__ == '__main__':
    unittest.main(verbosity=2)
