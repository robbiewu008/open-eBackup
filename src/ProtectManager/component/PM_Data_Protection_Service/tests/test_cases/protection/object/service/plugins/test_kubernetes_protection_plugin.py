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
from tests.test_cases.common.events import mock_producer

from app.protection.object.common.protection_enums import ResourceFilter

from app.protection.object.schemas.extends.params.kubernetes_ext_param import StatefulSetExtParam, NamespaceExtParam
from tests.test_cases.tools import functiontools

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
sys.modules['app.common.logger'] = mock.Mock()


class TestKubernetesProtectionPlugin(unittest.TestCase):
    def setUp(self):
        super(TestKubernetesProtectionPlugin, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.protection.object.service.plugins import kubernetes_protection_plugin
        self.plugin_service = kubernetes_protection_plugin
        self.plugin = kubernetes_protection_plugin.KubernetesProtectionPlugin()

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_do_convert_extend_parameter(self, mock_query_v2_resource):
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"invmdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-invmdb-1-1-m-0-backup\",\"name\":\"pv-invmdb-1-1-m-0-backup\",\"size\":\"50Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},{\"lunName\":\"1-invmdb-1-1-m-0-data\",\"name\":\"pv-invmdb-1-1-m-0-data\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},{\"lunName\":\"1-invmdb-1-1-m-0-lredo\",\"name\":\"pv-invmdb-1-1-m-0-lredo\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},{\"lunName\":\"1-invmdb-1-1-m-0-redo\",\"name\":\"pv-invmdb-1-1-m-0-redo\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"}]}],\"storageClassNames\":[\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\",\"storage-invmdb-backup-1-1\"]}\n"
            }}
        filter_list = [
            ResourceFilter(values=["*"], filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        resource = {"uuid": "fc16731d-a701-42a8-86b0-ef20123c5975"}
        ext_parameters = StatefulSetExtParam(pre_script="test_pre_script.sh",
                                             post_script="test_post_script.sh",
                                             failed_script="test_fail_script.sh",
                                             agents="123"
                                             )
        result = self.plugin.do_convert_extend_parameter(filter_list, resource,
                                                         ext_parameters)
        stateful_ext_param = StatefulSetExtParam(
            pre_script="test_pre_script.sh",
            post_script="test_post_script.sh",
            failed_script="test_fail_script.sh",
            pod_info=[
                {
                    "name": "invmdb-1-1-m-0",
                    "pvs": [
                        "pv-invmdb-1-1-m-0-backup",
                        "pv-invmdb-1-1-m-0-data",
                        "pv-invmdb-1-1-m-0-lredo",
                        "pv-invmdb-1-1-m-0-redo"
                    ]
                }
            ],
            agents="123"
        )
        self.assertEqual(result, stateful_ext_param)

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
        namespace_ext_param = NamespaceExtParam(
            pre_script="test_pre_script.sh",
            post_script="test_post_script.sh",
            failed_script="test_fail_script.sh",
            resource_filters=[],
            overwrite=True,
            binding_policy=["APPLY_TO_ALL", "APPLY_TO_NEW"]
        )
        res = self.plugin.build_ext_parameters(ext_parameters_str)
        self.assertEqual(res, namespace_ext_param)


if __name__ == '__main__':
    unittest.main(verbosity=2)
