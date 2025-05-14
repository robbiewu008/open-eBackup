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
from app.protection.object.schemas.extends.params.fusion_compute_param import FusionComputeExtParam
from tests.test_cases.common.events import mock_producer

from tests.test_cases.tools import functiontools

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
sys.modules['app.common.logger'] = mock.Mock()


class TestFusionComputeProtectionPlugin(unittest.TestCase):
    def setUp(self):
        super(TestFusionComputeProtectionPlugin, self).setUp()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.protection.object.service.plugins import fusion_compute_protection_plugin
        self.plugin_service = fusion_compute_protection_plugin
        self.plugin = fusion_compute_protection_plugin.FusionComputeProtectionPlugin()

    def test_do_convert_extend_parameter(self):
        filter_list = [ResourceFilter(values=["*"], filter_by="SLOT", rule="ALL", type="DISK", mode="EXCLUDE")]
        resource = {"uuid": "fc16731d-a701-42a8-86b0-ef20123c5975"}
        ext_parameters = FusionComputeExtParam(pre_script="test_pre_script.sh",
                                               post_script="test_post_script.sh",
                                               failed_script="test_fail_script.sh",
                                               overwrite=False,
                                               binding_policy=[SlaApplyType.APPLY_TO_ALL, SlaApplyType.APPLY_TO_NEW]
                                               )
        result = self.plugin.do_convert_extend_parameter(filter_list, resource, ext_parameters)
        fc_ext_param = FusionComputeExtParam(pre_script="test_pre_script.sh",
                                             post_script="test_post_script.sh",
                                             failed_script="test_fail_script.sh",
                                             overwrite=False,
                                             binding_policy=[SlaApplyType.APPLY_TO_ALL, SlaApplyType.APPLY_TO_NEW]
                                             )
        self.assertEqual(fc_ext_param, result)

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

    @mock.patch("app.protection.object.service.projected_object_service.build_protection_task")
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new")
    @mock.patch("uuid.uuid4")
    def test_build_task_list_success(self, mock_build_protection_task, mock_create_interval_schedule_new, mock_uuid4):
        from app.protection.object.models.projected_object import ProtectedTask
        mock_build_protection_task.build_protection_task = Mock(return_value=ProtectedTask(uuid="123",
                                                                                           policy_id="123",
                                                                                           protected_object_id="123",
                                                                                           schedule_id="123"))

        mock_create_interval_schedule_new.return_value = "966b5cad-ac48-4774-aff1-3411c75a11b5"
        mock_uuid4.return_value = "15b1d5de-7e8e-4386-b0a5-ebc3eaa5ebf7"
        sla = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e'}
        resource_id = "d72c8dcb-104f-49ac-aae9-06dca85bdd3f"
        from app.protection.object.models.projected_object import ProtectedObject
        obj = ProtectedObject(uuid="fc16731d-a701-42a8-86b0-ef20123c5975",
                              path="1.1.1.1")
        execute_req = {}
        result = self.plugin.build_task_list(sla, resource_id, obj, execute_req)
        self.assertEqual(len(result), 0)

    def test_build_ext_parameters(self):
        ext_parameters_str = "{\"pre_script\":\"test_pre_script.sh\",\"post_script\":\"test_post_script.sh\"," \
                             "\"failed_script\":\"test_fail_script.sh\",\"resource_filters\":[],\"overwrite\":true," \
                             "\"binding_policy\":[\"APPLY_TO_ALL\",\"APPLY_TO_NEW\"]} "
        fc_ext_param = FusionComputeExtParam(
            pre_script="test_pre_script.sh",
            post_script="test_post_script.sh",
            failed_script="test_fail_script.sh",
            resource_filters=[],
            overwrite=True,
            binding_policy=["APPLY_TO_ALL", "APPLY_TO_NEW"]
        )
        res = self.plugin.build_ext_parameters(ext_parameters_str)
        self.assertEqual(fc_ext_param, res)


if __name__ == '__main__':
    unittest.main(verbosity=2)
