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
from unittest import TestCase, mock
from unittest.mock import Mock, MagicMock, patch

from app.common.clients import client_util
from app.common.clients.system_base_client import SystemBaseClient
from app.common.security import kmc_util
from app.common.security.kmc_util import Kmc

sys.modules['app.resource.service.vmware.service_instance_manager'] = MagicMock()

from pydantic.main import BaseModel

from app.common.deploy_type import DeployType
from app.protection.object.schemas.extends.params.oracle_ext_param import OracleExtParam
from app.common.enums.resource_enum import ResourceSubTypeEnum
from tests.test_cases import common_mocker  # noqa
from app.protection.object.schemas.extends.params.fileset_protection_ext_param import FilesetProtectionExtParam
from app.protection.object.schemas.extends.params.huawei_cloud_stack_ext_param import CloudHostExtParam
from app.protection.object.schemas.extends.params.kubernetes_ext_param import StatefulSetExtParam, NamespaceExtParam
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.tools import timezone, functiontools, http, env

mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
from tests.test_cases.tools.timezone import dmc

mock.patch(
    "app.common.clients.device_manager_client.device_manager_client.init_time_zone",
    dmc.query_time_zone).start()
sys.modules['app.common.events.producer'] = mock.Mock()
sys.modules['app.common.events.topics'] = mock.Mock()
sys.modules['app.common.context.db_session'] = mock.Mock()
sys.modules['app.backup.common.config.db_config'] = mock.Mock()
DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
DeployType.is_hyper_detect_deploy_type = Mock(return_value=False)
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.protection_client.ProtectionClient.check_exist_copies_location_before_protect",
           mock.Mock).start()
mock.patch(
    "app.common.clients.device_manager_client.device_manager_client.init_time_zone",
    timezone.dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.protection.object.common.db_config'] = Mock()
mock.patch("app.protection.object.client.dee_client.update_self_learning_config", mock.Mock(
    return_value={'is_open': True, 'type': 1, 'duration': 30, 'progress': 0})).start()

from app.common.clients.resource_client import ResourceClient
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.schemas.common_schemas import BatchOperationResult
from app.common.enums.protected_object_enum import Status
from app.protection.object.models.projected_object import ProtectedObject
from app.protection.object.common.protection_enums import ResourceFilter, \
    SlaApplyType
from app.protection.object.schemas.extends.params.vmware_ext_param import \
    ProtectResource, VirtualResourceExtParam, \
    VmExtParam


class CommonResourceExtParam(BaseModel):
    pass


class ModifyProtectionExecuteReq(BaseModel):
    sla_id = "ce8d0c9e983211eb80bcfa163ef214c6"
    resource_name = "zsq_1234"
    resource_id = "ce8d0c9e983211eb80bcfa163ef21477"
    ext_parameters = CommonResourceExtParam()
    job_id = "ce8d0c9e983211eb80bcfa163ef21488"
    request_id = "ce8d0c9e983211eb80bcfa163ef21499"
    resource_type = "Fileset"
    resource_sub_type = "Fileset"
    origin_sla_id = "ce8d0c9e983211eb80bcfa163ef21400"
    is_sla_modify = False
    status = Status.Active.value


class BatchProtectionExecuteReq(BaseModel):
    sla_id = "ce8d0c9e983211eb80bcfa163ef214c6"
    resources = [ProtectResource(
        resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
        filters=[
            {
                "filter_by": "SLOT",
                "type": "DISK",
                "rule": "ALL",
                "mode": "INCLUDE",
                "values": [
                    "*"
                ]
            }
        ])]
    ext_parameters = {
        "pre_script": "/a/a.sh",
        "post_script": "/a/a.sh",
        "resource_filters": [],
        "overwrite": True,
        "binding_policy": [
            "APPLY_TO_ALL",
            "APPLY_TO_NEW"
        ]
    }
    job_id = "ce8d0c9e983211eb80bcfa163ef214c6"
    request_id = "ce8d0c9e983211eb80bcfa163ef214c6"
    user_id = "ce8d0c9e983211eb80bcfa163ef214c6"


class TestBatchProtectionService(TestCase):
    def setUp(self):
        super(TestBatchProtectionService, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.resource_lock.db.db_utils'] = Mock()
        sys.modules['app.backup.common.config.db_config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()

    def tearDown(self) -> None:
        super(TestBatchProtectionService, self).tearDown()
        del sys.modules['app.resource_lock.db.db_utils']

    def test_check_validity_volume_path(self):
        values1 = []
        values2 = []
        fliters1 = [ResourceFilter(values=values1, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        fliters2 = [ResourceFilter(values=values2, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        resource = ProtectResource(
            resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
        )
        from app.protection.object.service.batch_protection_service import check_validity_volume_path
        result = check_validity_volume_path([resource])
        self.assertIsNone(result)

        resource1 = ProtectResource(
            resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
            filters=fliters1)
        resource2 = ProtectResource(
            resource_id="4d77d21896a911ebb1b7fa163ef214c6",
            filters=fliters2)
        resource_list = [resource1, resource2]
        self.assertRaises(EmeiStorBizException, check_validity_volume_path,
                          resource_list)
        values3 = [
            "/",
            "/boot",
            "/var/lib/docker",
            "/var/lib/kubelet/stem-base/0"
        ]
        values4 = [
            "/",
            "/boot"
        ]
        fliters3 = [ResourceFilter(values=values3, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        fliters4 = [ResourceFilter(values=values4, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        resource3 = ProtectResource(
            resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
            filters=fliters3)
        resource4 = ProtectResource(
            resource_id="4d77d21896a911ebb1b7fa163ef214c6",
            filters=fliters4)
        resource_list1 = [resource3, resource4]
        result1 = check_validity_volume_path(resource_list1)
        self.assertIsNone(result1)

    def test_get_validity_volume_path_success(self):
        values = []
        values1 = [
            "/",
            "/boot",
            "/var/lib/docker",
            "/var/lib/kubelet/stem-base/0"
        ]
        values2 = [
            "/",
            "/boot"
        ]
        values.append(values1)
        values.append(values2)
        fliters1 = [ResourceFilter(values=values1, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        fliters2 = [ResourceFilter(values=values2, filter_by="ID", rule="ALL",
                                   type="VOLUME", mode="INCLUDE")]
        resource1 = ProtectResource(
            resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
            filters=fliters1)
        resource2 = ProtectResource(
            resource_id="4d77d21896a911ebb1b7fa163ef214c6",
            filters=fliters2)
        volumes = tuple(values)
        resource_list = [resource1, resource2]
        from app.protection.object.service.batch_protection_service import get_validity_volume_path
        result, types = get_validity_volume_path(resource_list)
        self.assertEqual(result, volumes)

    def test_protect_pre_check(self):
        sla_obj = {"application": "ABBackupClient", "policy_list":[]}
        resource_obj = {"sub_type": "ABBackupClient", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_nas_protect_pre_check(self):
        sla_obj = {"application": "NasFileSystem", "policy_list":[]}
        resource_obj = {"sub_type": "NasFileSystem", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_postgresql_protect_pre_check(self):
        sla_obj = {"application": "PostgreSQL", "policy_list":[]}
        resource_obj = {"sub_type": "PostgreInstance", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_kingbase_protect_pre_check(self):
        sla_obj = {"application": "KingBase", "policy_list":[]}
        resource_obj = {"sub_type": "KingBaseInstance", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_db2_protect_pre_check(self):
        sla_obj = {"application": "DB2", "policy_list":[]}
        resource_obj = {"sub_type": "DB2-database", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_dameng_protect_pre_check(self):
        sla_obj = {"application": "Dameng", "policy_list":[]}
        resource_obj = {"sub_type": "Dameng-singleNode", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    @mock.patch.object(ResourceClient, "query_resource")
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    @mock.patch("app.protection.object.service.batch_protection_service.update_protected_object_fields")
    @mock.patch("sqlalchemy.orm.Query.first")
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch(
        "app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step")
    @unittest.skip
    def test_common_resource_protection_modify(self, _mock_query_resource, _mock_query_sla,
                                               _mock_protected_object_fields, first, record_job):
        _mock_query_resource.return_value = ModifyProtectionExecuteReq()
        _mock_query_sla.return_value = ModifyProtectionExecuteReq()
        _mock_protected_object_fields = {}
        from app.protection.object.service.batch_protection_service import common_resource_protection_modify
        res = common_resource_protection_modify(ModifyProtectionExecuteReq())
        test_res = BatchOperationResult(all_ids=[], success_ids=["ce8d0c9e983211eb80bcfa163ef21477"], failed_ids=[])
        var = {"all_ids": [],
               "success_ids": ["ce8d0c9e983211eb80bcfa163ef21477"],
               "failed_ids": []}
        self.assertEqual(var, res)
        sla_changed = ModifyProtectionExecuteReq()
        sla_changed.is_sla_modify = True
        res2 = common_resource_protection_modify(sla_changed)
        self.assertEqual(test_res, res2)

    def test_filter_by_rule(self):
        filters = None
        value = "test_value"
        from app.protection.object.service.batch_protection_service import filter_by_rule
        result = filter_by_rule(value, filters)
        self.assertTrue(result)
        values1 = ["test_value"]
        filters = [ResourceFilter(values=values1, filter_by="ID", rule="ALL",
                                  type="VOLUME", mode="INCLUDE")]
        result = filter_by_rule(value, filters)
        self.assertTrue(result)
        values1 = ["test_value"]
        filters = [
            ResourceFilter(values=values1, filter_by="ID", rule="START_WITH",
                           type="VOLUME", mode="INCLUDE")]
        result = filter_by_rule(value, filters)
        self.assertTrue(result)
        values1 = ["test_value"]
        filters = [
            ResourceFilter(values=values1, filter_by="ID", rule="END_WITH",
                           type="VOLUME", mode="INCLUDE")]
        result = filter_by_rule(value, filters)
        self.assertTrue(result)
        values1 = ["test_value"]
        filters = [ResourceFilter(values=values1, filter_by="ID", rule="FUZZY",
                                  type="VOLUME", mode="INCLUDE")]
        result = filter_by_rule(value, filters)
        self.assertTrue(result)
        # 测试多个规则同时过滤
        values1 = ["value"]
        filters = [ResourceFilter(values=values1, filter_by="NAME", rule="START_WITH", type="VM", mode="INCLUDE"),
                   ResourceFilter(values=values1, filter_by="NAME", rule="END_WITH", type="VM", mode="INCLUDE")]
        result = filter_by_rule(value, filters)
        self.assertTrue(result)

    def test_filter_resources(self):
        resource_list = [{"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d", "sub_type": "CNWARE_VM", "type": "VM"}]
        values1 = [
            "/",
            "/boot",
            "/var/lib/docker",
            "/var/lib/kubelet/stem-base/0"
        ]
        filters = [ResourceFilter(values=values1, filter_by="ID", rule="ALL",
                                  type="VOLUME", mode="INCLUDE")]
        from app.protection.object.service.batch_protection_service import filter_resources
        result = filter_resources(resource_list, filters)
        self.assertEqual(result, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        filters1 = [ResourceFilter(values=values1, filter_by="NAME", rule="ALL",
                                   type="VM", mode="INCLUDE")]
        result1 = filter_resources(resource_list, filters1)
        self.assertEqual(result1, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])

    def test_always_not_filter_resources_success(self):
        from app.protection.object.service.batch_protection_service import always_not_filter_resources
        resource_list = [{"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d", "sub_type": "FusionCompute", "type": "Host"},
                         {"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d", "sub_type": "FusionCompute", "type": "VM"}]
        not_filter_resources = always_not_filter_resources(resource_list)
        self.assertEqual(len(not_filter_resources), 1)

    def test_filter_disks(self):
        disk = "test_disk"
        filters = None
        from app.protection.object.service.batch_protection_service import filter_disks
        result = filter_disks(disk, filters)
        self.assertEqual(result, disk)
        filters = []
        result = filter_disks(disk, filters)
        self.assertEqual(result, disk)
        values = ["test_value", "*"]
        values1 = ["test_value"]
        filters1 = [ResourceFilter(values=values, filter_by="SLOT", rule="ALL",
                                   type="DISK", mode="INCLUDE")]
        disk1 = {"slot": "test_value"}
        result = filter_disks(disk1, filters1)
        self.assertTrue(result)
        filters2 = [ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                                   type="DISK", mode="INCLUDE")]
        result = filter_disks(disk1, filters2)
        self.assertTrue(result)
        disk2 = {"slot": "test_value1"}
        filters3 = [ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                                   type="DISK", mode="EXCLUDE")]
        result = filter_disks(disk2, filters3)
        self.assertTrue(result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_vm_disk")
    def test_covert_to_vm_ext_parameters(self, mock_query_vm_disk):
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
        from app.protection.object.service.batch_protection_service import covert_to_vm_ext_parameters
        result = covert_to_vm_ext_parameters(disk_filters, vm_resource,
                                             ext_parameters)
        self.assertIsNone(result)
        mock_query_vm_disk.return_value = [
            {"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d"}]
        from app.protection.object.service.batch_protection_service import covert_to_vm_ext_parameters
        result = covert_to_vm_ext_parameters(disk_filters, vm_resource,
                                             ext_parameters)
        vmextparam = VmExtParam(
            pre_script="test_pre_script",
            post_script="test_post_script",
            all_disk=True,
            disk_info=["a46acc99-21f5-465f-9adb-cabcc2737a3d"]
        )
        self.assertEqual(result, vmextparam)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.build_protection_object")
    def test_protect_common_resource(self, mock_build_protection_object):
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "application": "vim.HostSystem"}
        values1 = ["test_value"]
        mock_build_protection_object.return_value = ProtectedObject(**{
            "uuid": "cc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1/aaa",
            "sub_type": "KubernetesStatefulSet"
        })
        disk_filters = [
            ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        ext_parameters = VirtualResourceExtParam(pre_script="test_pre_script",
                                                 post_script="test_post_script",
                                                 overwrite=False,
                                                 binding_policy=[
                                                     SlaApplyType.APPLY_TO_ALL],
                                                 resource_filters=disk_filters,
                                                 disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters
        resource_info = {"uuid": "de8d0c9e983211eb80bcfa163ef21477", "name": "test_1234"}
        resource_list = [resource_info]
        from app.protection.object.service.batch_protection_service import protect_common_resource
        request_id = "a46acc99-21f5-465f-9adb-cabcc2737a3d"
        result = protect_common_resource(request_id, resource_list, sla_obj,
                                         execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("de8d0c9e983211eb80bcfa163ef21477")
        self.assertEqual(batch_result, result)

    @patch.object(ResourceClient, "query_resource",
                  Mock(return_value=dict(uuid="34926913-b6a6-4c3b-812e-4fba893cbe5e", name="fake_resource",
                                         sub_type="KubernetesStatefulSet")))
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    def test_protect_current_resource(self):
        sla_obj = dict(uuid='34926913-b6a6-4c3b-812e-4fba893cbe5e', application='KubernetesStatefulSet', name='Golden',
                       policy_list=[
                           {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, ])
        values1 = ["test_value"]
        disk_filters = [{
            "values": values1,
            "filter_by": "SLOT",
            "rule": "ALL",
            "type": "DISK",
            "mode": "EXCLUDE"
        }]
        ext_parameters = NamespaceExtParam(pre_script="test_pre_script.sh",
                                           post_script="test_post_script.sh",
                                           overwrite=False,
                                           binding_policy=[
                                               SlaApplyType.APPLY_TO_ALL],
                                           resource_filters=disk_filters,
                                           disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters
        from app.protection.object.service.batch_protection_service import protect_current_resource
        request_id = "a46acc99-21f5-465f-9adb-cabcc2737a3d"
        res = ProtectResource(resource_id="de8d0c9e983211eb80bcfa163ef21477", filters=[])
        from app.base.db_base import database
        with database.session() as session:
            result = protect_current_resource(session, request_id, res, sla_obj, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("de8d0c9e983211eb80bcfa163ef21477")
        self.assertEqual(batch_result, result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.build_extend_parameter",
                Mock(return_value=Mock()))
    def test_protect_sub_resources(self, mock_query_v2_resource):
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\","
                       "\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"invmdb-1-1-m-0\","
                       "\"pvs\":[{\"lunName\":\"1-invmdb-1-1-m-0-backup\",\"name\":\"pv-invmdb-1-1-m-0-backup\","
                       "\"size\":\"50Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-data\",\"name\":\"pv-invmdb-1-1-m-0-data\",\"size\":\"80Gi\","
                       "\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-lredo\",\"name\":\"pv-invmdb-1-1-m-0-lredo\","
                       "\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-redo\",\"name\":\"pv-invmdb-1-1-m-0-redo\",\"size\":\"85Gi\","
                       "\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"}]}],\"storageClassNames\":["
                       "\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\","
                       "\"storage-invmdb-backup-1-1\"]}\n "
            }}
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
                   'application': 'KubernetesStatefulSet', 'name': 'Golden', 'policy_list': [
                {
                    "action": "difference_increment",
                    "type": "backup",
                    "uuid": "test-uuid1",
                    "schedule": {"interval": "interval",
                                 "interval_unit": "10",
                                 "start_time": "2021.07.05"
                                 }
                }, {
                    "action": "difference_increment",
                    "type": "backup",
                    "uuid": "test-uuid1",
                    "schedule": {"interval": "interval",
                                 "interval_unit": "10",
                                 "start_time": "2021.07.05"
                                 }
                },
            ]
                   }
        values1 = ["test_value"]
        disk_filters = [{
            "values": values1,
            "filter_by": "SLOT",
            "rule": "ALL",
            "type": "DISK",
            "mode": "EXCLUDE"
        }]
        ext_parameters = NamespaceExtParam(pre_script="test_pre_script.sh",
                                           post_script="test_post_script.sh",
                                           overwrite=False,
                                           binding_policy=[
                                               SlaApplyType.APPLY_TO_ALL],
                                           resource_filters=disk_filters,
                                           disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        sub_resource_list = [{
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "sub_type": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/"
        }]
        execute_req.ext_parameters = ext_parameters
        from app.protection.object.service.batch_protection_service import protect_sub_resources
        request_id = "a46acc99-21f5-465f-9adb-cabcc2737a3d"
        res = ProtectResource(resource_id="de8d0c9e983211eb80bcfa163ef21477", filters=[])
        from app.base.db_base import database
        with database.session() as session:
            result = protect_sub_resources(session, request_id, res, sub_resource_list, sla_obj, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("311fed99-44e7-9bed-497a-fa7b4ee35d86")
        self.assertEqual(batch_result, result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    def test_handle_protect_failed(self):
        from app.protection.object.service.batch_protection_service import handle_protect_failed
        from app.base.db_base import database
        with database.session() as session:
            handle_protect_failed("job_id", "a46acc99-21f5-465f-9adb-cabcc2737a3d", "resource_name", "sla_name",
                                  session,
                                  "311fed99-44e7-9bed-497a-fa7b4ee35d86")
        self.assertTrue(True, "handle_protect_failed success")

    @mock.patch("app.protection.object.db.projected_object.query_multi_by_params")
    @mock.patch("app.protection.object.service.protection_plugin_manager.ProtectionPluginManager"
                ".query_sub_resources_by_obj")
    def test_get_resource_obj_list(self, mock_query_resource_list, mock_query_multi_by_params):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="de8d0c9e983211eb80bcfa163ef21477",
                                       sub_type="KubernetesStatefulSet")
        mock_query_resource_list.return_value = [{"uuid": "de8d0c9e983211eb80bcfa163ef21477", "name": "test_1234"}]
        mock_query_multi_by_params.return_value = [("15b1d5de-7e8e-4386-b0a5-ebc3eaa5ebf7",)]
        object_list = [protect_obj1]
        from app.protection.object.service.batch_protection_service import get_resource_obj_list
        from app.base.db_base import database
        database.session().__enter__ = MagicMock()
        database.session().__exit__ = MagicMock()
        with database.session() as session:
            all_object_list = get_resource_obj_list(session, object_list)
        self.assertTrue(len(all_object_list) == 2, "test_get_resource_obj_list success")

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.batch_delete_schedules", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.delete_by_condition", MagicMock)
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_remove_protection(self):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="de8d0c9e983211eb80bcfa163ef21477",
                                       sub_type="KubernetesStatefulSet")
        object_list = [protect_obj1]
        from app.protection.object.service.batch_protection_service import remove_protection
        from app.base.db_base import database
        database.session().__enter__ = MagicMock()
        database.session().__exit__ = MagicMock()
        with database.session() as session:
            remove_protection(session, object_list)
        self.assertTrue(True, "No Expection")

    @mock.patch("app.protection.object.db.projected_object.query_one_by_resource_id")
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.batch_delete_schedules", MagicMock)
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    def test_container_protection_modify(self, mock_query_projected_object):
        mock_query_projected_object.return_value = ProtectedObject(**{
            "uuid": "fc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1",
            "ext_parameters": "{\"pre_script\":\"test_pre_script.sh\","
                              "\"post_script\":\"test_post_script.sh\","
                              "\"failed_script\":\"test_fail_script.sh\",\"resource_filters\":[],"
                              "\"overwrite\":true,\"binding_policy\":[\"APPLY_TO_ALL\","
                              "\"APPLY_TO_NEW\"]}"
        })
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
                   'application': 'KubernetesStatefulSet'}
        ext_parameters = NamespaceExtParam(pre_script="test_pre_script.sh",
                                           post_script="test_post_script.sh",
                                           overwrite=False,
                                           binding_policy=[SlaApplyType.APPLY_TO_ALL])
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters
        from app.protection.object.service.batch_protection_service import container_protection_modify
        result = container_protection_modify(sla_obj, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("ce8d0c9e983211eb80bcfa163ef21477")
        self.assertEqual(batch_result, result)

    @mock.patch("app.resource.service.common.resource_service.query_resource_by_id")
    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.batch_delete_schedules", MagicMock)
    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.service.protection_plugin_manager.ProtectionPluginManager"
                ".query_sub_resources")
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    @mock.patch("app.protection.object.service.batch_protection_service.build_protection_object")
    @mock.patch(
        "app.protection.object.service.protection_plugin_manager.ProtectionPluginManager.convert_extend_parameter",
        Mock(return_value={}))
    def test_sub_resources_protection_modify_success(self, mock_build_protection_object, mock_query_v2_resource,
                                                     mock_query_sub_resources, mock_query_projected_object):
        mock_build_protection_object.return_value = ProtectedObject(**{
            "uuid": "cc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1/aaa",
            "sub_type": "KubernetesStatefulSet"
        })
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\","
                       "\"nameSpace\":\"ns000000000000000000001\",\"volumeNames\":[\"volumeNames1\","
                       "\"volumeNames2\"],\"storageClassNames\":[\"storage-invmdb-redo-1-1\","
                       "\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\",\"storage-invmdb-backup-1-1\"]} "
            }
        }
        mock_query_projected_object.return_value = ProtectedObject(**{
            "uuid": "fc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1",
            "ext_parameters": "{\"pre_script\":\"test_pre_script.sh\","
                              "\"post_script\":\"test_post_script.sh\","
                              "\"failed_script\":\"test_fail_script.sh\",\"resource_filters\":[],"
                              "\"overwrite\":true,\"binding_policy\":[\"APPLY_TO_ALL\","
                              "\"APPLY_TO_NEW\"]}"
        })
        mock_query_sub_resources.return_value = [
            {"uuid": "de8d0c9e983211eb80bcfa163ef21477", "name": "test_1234", "sub_type": "KubernetesStatefulSet"}]
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
                   'application': 'KubernetesStatefulSet'}
        ext_parameters = NamespaceExtParam(pre_script="test_pre_script.sh",
                                           post_script="test_post_script.sh",
                                           overwrite=False,
                                           binding_policy=[SlaApplyType.APPLY_TO_ALL])
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters
        execute_req.resource_sub_type = "KubernetesStatefulSet"
        from app.protection.object.service.batch_protection_service import sub_resources_protection_modify
        result = sub_resources_protection_modify(sla_obj, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("de8d0c9e983211eb80bcfa163ef21477")
        self.assertEqual(batch_result, result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    def test_handle_protect_success(self):
        from app.protection.object.service.batch_protection_service import handle_protect_success
        from app.base.db_base import database
        with database.session() as session:
            handle_protect_success("job_id", "a46acc99-21f5-465f-9adb-cabcc2737a3d", "resource_name", "sla_name",
                                   session,
                                   "311fed99-44e7-9bed-497a-fa7b4ee35d86")
        self.assertTrue(True, "handle_protect_success success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_vm_disk")
    @mock.patch("app.protection.object.service.batch_protection_service.build_protection_object")
    def test_handle_need_create(self, mock_build_protection_object, mock_query_vm_disk):
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "application": "ABBackupClient"}
        values1 = ["test_value"]
        mock_build_protection_object.return_value = ProtectedObject(**{
            "uuid": "cc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1/aaa",
            "sub_type": "KubernetesStatefulSet"
        })
        disk_filters = [
            ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        ext_parameters = VirtualResourceExtParam(pre_script="test_pre_script",
                                                 post_script="test_post_script",
                                                 overwrite=False,
                                                 binding_policy=[
                                                     SlaApplyType.APPLY_TO_ALL],
                                                 resource_filters=disk_filters,
                                                 disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters

        resource_map = {"ce8d0c9e983211eb80bcfa163ef21477": {
            "uuid": "ce8d0c9e983211eb80bcfa163ef21477",
            "sub_type": "vim.VirtualMachine"}}
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="de8d0c9e983211eb80bcfa163ef21477")
        existing_object_list = [protect_obj1]
        mock_query_vm_disk.return_value = [
            {"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d"}]
        protected_resource_ids = ["de8d0c9e983211eb80bcfa163ef21477", "ce8d0c9e983211eb80bcfa163ef21477"]
        from app.protection.object.service.batch_protection_service import handle_need_create
        result = handle_need_create(sla_obj, existing_object_list, protected_resource_ids,
                                    resource_map, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("ce8d0c9e983211eb80bcfa163ef21477")
        self.assertEqual(batch_result, result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_multi_by_params")
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_do_remove_protection(self, mock_query_multi_by_params):
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "application": "ABBackupClient"}
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="de8d0c9e983211eb80bcfa163ef21477",
                                       sub_type="KubernetesStatefulSet")
        mock_query_multi_by_params.return_value = [protect_obj1]
        from app.protection.object.service.batch_protection_service import do_remove_protection
        result = do_remove_protection("request_id", "job_id", "a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                      sla_obj)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("a46acc99-21f5-465f-9adb-cabcc2737a3d")
        self.assertEqual(batch_result, result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_vm_disk")
    def test_handle_need_modify(self, mock_query_vm_disk):
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "application": "ABBackupClient"}
        values1 = ["test_value"]
        disk_filters = [
            ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        ext_parameters = VirtualResourceExtParam(pre_script="test_pre_script",
                                                 post_script="test_post_script",
                                                 overwrite=False,
                                                 binding_policy=[
                                                     SlaApplyType.APPLY_TO_ALL],
                                                 resource_filters=disk_filters,
                                                 disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters

        resource_map = {"ce8d0c9e983211eb80bcfa163ef21477": {
            "uuid": "ce8d0c9e983211eb80bcfa163ef21477",
            "sub_type": "vim.VirtualMachine"}}
        protect_obj1 = ProtectedObject(name="zsq_1234",
                                       resource_id="ce8d0c9e983211eb80bcfa163ef21477")
        existing_object_list = [protect_obj1]
        mock_query_vm_disk.return_value = [
            {"uuid": "a46acc99-21f5-465f-9adb-cabcc2737a3d"}]
        from app.protection.object.service.batch_protection_service import handle_need_modify
        result = handle_need_modify(MagicMock, sla_obj, existing_object_list,
                                    resource_map, execute_req)
        self.assertEqual(BatchOperationResult(), result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_multi_by_params")
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_handle_need_delete(self, mock_query_multi_by_params):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="de8d0c9e983211eb80bcfa163ef21477",
                                       sub_type="KubernetesStatefulSet")
        mock_query_multi_by_params.return_value = [protect_obj1]
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "application": "ABBackupClient"}
        values1 = ["test_value"]
        disk_filters = [
            ResourceFilter(values=values1, filter_by="SLOT", rule="ALL",
                           type="DISK", mode="EXCLUDE")]
        ext_parameters = VirtualResourceExtParam(pre_script="test_pre_script",
                                                 post_script="test_post_script",
                                                 overwrite=False,
                                                 binding_policy=[
                                                     SlaApplyType.APPLY_TO_ALL],
                                                 resource_filters=disk_filters,
                                                 disk_filters=disk_filters)
        execute_req = ModifyProtectionExecuteReq()
        execute_req.ext_parameters = ext_parameters
        protected_resource_ids = ["b46acc99-21f5-465f-9adb-cabcc2737a3d"]
        sub_resource_ids = ["a46acc99-21f5-465f-9adb-cabcc2737a3d", "b46acc99-21f5-465f-9adb-cabcc2737a3d"]
        from app.protection.object.service.batch_protection_service import handle_need_delete
        result = handle_need_delete(sla_obj, protected_resource_ids, sub_resource_ids, execute_req)
        batch_result = BatchOperationResult()
        batch_result.append_success_id("a46acc99-21f5-465f-9adb-cabcc2737a3d")
        self.assertEqual(batch_result, result)

    def test_batch_protect_host_pre_check(self):
        sla_obj = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                   "sub_type": "VMBackupAgent", "link_status": 1, "os_type": 1}
        from app.protection.object.service.batch_protection_service import batch_protect_host_pre_check
        batch_protect_host_pre_check(sla_obj, 1, None)
        self.assertTrue(True, "success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.update_job",
                MagicMock)
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_resource")
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService"
                ".check_resource_is_protected")
    @mock.patch("app.protection.object.service.batch_protection_service.protect_by_resource_type")
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService"
                "._execute_protection_post_action")
    def test_execute_batch_protection(self, mock_post_action,
                                      mock_protect_by_resource_type, mock_check_resource_is_protected,
                                      mock_query_resource, mock_query_sla):
        execute_req = BatchProtectionExecuteReq()
        mock_query_resource.return_value = ["ce8d0c9e983211eb80bcfa163ef21477"]
        mock_query_sla.return_value = {"uuid": "ce8d0c9e983211eb80bcfa163ef21477",
                                       "application": "ABBackupClient"}
        batch_result = BatchOperationResult()
        batch_result.append_success_id("a46acc99-21f5-465f-9adb-cabcc2737a3d")
        mock_check_resource_is_protected.return_value = batch_result
        mock_protect_by_resource_type.return_value = batch_result
        mock_post_action.return_value = True
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        BatchProtectionService.execute_batch_protection("request_id", execute_req)
        self.assertTrue(True, "success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.record_job_step",
                MagicMock)
    @mock.patch("app.protection.object.service.batch_protection_service.BatchProtectionService.update_job",
                MagicMock)
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_resource")
    @unittest.skip
    def test_modify_protection_execute(self, mock_query_resource):
        mock_query_resource.return_value = {"name": "test"}
        execute_req = ModifyProtectionExecuteReq()
        batch_result = BatchOperationResult()
        batch_result.append_success_id("a46acc99-21f5-465f-9adb-cabcc2737a3d")
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        BatchProtectionService.modify_protection_execute(execute_req)
        self.assertTrue(True, "success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    @mock.patch("app.protection.object.client.job_client.JobClient.count_job")
    def test_check_resource_status(self, mock_count_job, mock_query_by_resource_ids):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                       sub_type="KubernetesStatefulSet")
        mock_query_by_resource_ids.return_value = [protect_obj1]
        mock_count_job.return_value = 0
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            BatchProtectionService.check_resource_status(session, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        self.assertTrue(True, "success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    @mock.patch("app.protection.object.client.job_client.JobClient.count_job")
    def test_check_resource_status_exception(self, mock_count_job, mock_query_by_resource_ids):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                       sub_type="KubernetesStatefulSet")
        mock_query_by_resource_ids.return_value = [protect_obj1]
        mock_count_job.return_value = 1
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            self.assertRaises(EmeiStorBizException, BatchProtectionService.check_resource_status, session,
                              ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    @mock.patch("app.protection.object.client.job_client.JobClient.count_job")
    @patch.object(ResourceClient, "query_resource",
                  Mock(return_value=dict(uuid="34926913-b6a6-4c3b-812e-4fba893cbe5e", name="fake_resource",
                                         sub_type="KubernetesStatefulSet",
                                         environment_sub_type="CyberEngineOceanProtect")))
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_batch_remove_protection(self, mock_count_job, mock_query_by_resource_ids):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                       sub_type="KubernetesStatefulSet",
                                       ext_parameters="{\"share_type\": \"NFS\", \"file_system_ids\": [\"1\"]}")
        mock_query_by_resource_ids.return_value = [protect_obj1]
        mock_count_job.return_value = 0
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            result = BatchProtectionService.batch_remove_protection(session, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        self.assertEqual(result, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    @mock.patch("app.protection.object.client.job_client.JobClient.count_job")
    @mock.patch("app.common.events.producer.produce", mock.Mock())
    def test_batch_remove_protection_if_no_protected_object(self, mock_count_job, mock_query_by_resource_ids):
        mock_query_by_resource_ids.return_value = []
        mock_count_job.return_value = 0
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            result = BatchProtectionService.batch_remove_protection(session, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        self.assertEqual(result, [])

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    def test_batch_deactivate(self, mock_query_by_resource_ids):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                       sub_type="KubernetesStatefulSet")
        mock_query_by_resource_ids.return_value = [protect_obj1]
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            BatchProtectionService.batch_deactivate(session, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        self.assertTrue(True, "success")

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    def test_batch_activate(self, mock_query_by_resource_ids):
        protect_obj1 = ProtectedObject(name="test_1234",
                                       resource_id="a46acc99-21f5-465f-9adb-cabcc2737a3d",
                                       sub_type="KubernetesStatefulSet")
        mock_query_by_resource_ids.return_value = [protect_obj1]
        from app.protection.object.service.batch_protection_service import BatchProtectionService
        from app.base.db_base import database
        with database.session() as session:
            BatchProtectionService.batch_activate(session, ["a46acc99-21f5-465f-9adb-cabcc2737a3d"])
        self.assertTrue(True, "success")

    def test_check_action_and_small_file_correct(self):
        # check_action_and_small_file方法正常传参
        policy = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65",
            name="full",
            action="full"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        extend_parameters = FilesetProtectionExtParam(
            cross_file_system=True,
            consistent_backup=True,
            backup_nfs=True,
            channels=2,
            sparse_file_detection=True,
            backup_continue_with_files_backup_failed=True,
            small_file_aggregation=True,
            aggregation_file_size=4096,
            aggregation_file_max_size=1024,
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh")
        from app.protection.object.service.projected_object_service import check_action_and_small_file
        check_action_and_small_file(sla_obj.get("policy_list"), extend_parameters)
        self.assertTrue(True, "无异常抛出")

    @patch("app.base.db_base.database.session")
    def test_check_sql_server_protected_resource_correct(self, _mock_session):
        from app.protection.object.service.batch_protection_service import check_sql_server_protected_resource
        # check_sql_server_protected_resource方法传数据库类型资源
        database_obj = dict(
            uuid="123456",
            sub_type="SQLServer-database",
            parent_uuid="654321"
        )
        _mock_session().__enter__().query().filter().count.return_value = 1
        self.assertRaises(EmeiStorBizException, check_sql_server_protected_resource, database_obj)
        # check_sql_server_protected_resource方法传单实例类型资源
        instance_obj = dict(
            uuid="123456",
            sub_type="SQLServer-instance"
        )
        _mock_session().__enter__().query().filter().all.return_value = []
        _mock_session().__enter__().query().filter().count.return_value = 0
        check_sql_server_protected_resource(instance_obj)
        self.assertTrue(True, "无异常抛出")

    def test_check_action_and_small_file(self):
        # 当同时选择永久增量备份和小文件聚合，报错
        policy = dict(
            uuid="9b17382f-7164-4f5b-8d77-2910a0be348c",
            name="permanent_increment",
            action="permanent_increment"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        extend_parameters = FilesetProtectionExtParam(
            cross_file_system=True,
            consistent_backup=True,
            backup_nfs=True,
            channels=2,
            sparse_file_detection=True,
            backup_continue_with_files_backup_failed=True,
            small_file_aggregation=True,
            aggregation_file_size=4096,
            aggregation_file_max_size=1024,
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh")
        from app.protection.object.service.projected_object_service import check_action_and_small_file
        self.assertRaises(EmeiStorBizException, check_action_and_small_file,
                          sla_obj.get("policy_list"), extend_parameters)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_kubernetes_volume_names_empty_exception(self, query_v2_resource):
        query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"volumeNames\":[\"volumeNames1\",\"volumeNames2\"],\"storageClassNames\":[\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\",\"storage-invmdb-backup-1-1\"]}"
            }
        }
        resource_obj = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65"
        )
        ext_params = StatefulSetExtParam(
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh",
            volume_names=[]
        )
        from app.protection.object.service.batch_protection_service import check_kubernetes_volume_names
        self.assertRaises(EmeiStorBizException, check_kubernetes_volume_names,
                          resource_obj, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_kubernetes_volume_names_invalid_exception(self, query_v2_resource):
        query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"volumeNames\":[\"volumeNames1\",\"volumeNames2\"],\"storageClassNames\":[\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\",\"storage-invmdb-backup-1-1\"]}"
            }
        }
        resource_obj = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65"
        )
        ext_params = StatefulSetExtParam(
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh",
            volume_names=[
                "volumeNames_erro"
            ]
        )
        from app.protection.object.service.batch_protection_service import check_kubernetes_volume_names
        self.assertRaises(EmeiStorBizException, check_kubernetes_volume_names,
                          resource_obj, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_kubernetes_volume_names_success(self, query_v2_resource):
        query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"volumeNames\":[\"volumeNames1\",\"volumeNames2\"],\"storageClassNames\":[\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\",\"storage-invmdb-backup-1-1\"]}"
            }
        }
        resource_obj = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65"
        )
        ext_params = StatefulSetExtParam(
            pre_script="/opt/ss.sh",
            post_script="/opt/ss.sh",
            failed_script="/opt/ss.sh",
            volume_names=[
                "volumeNames1"
            ]
        )
        from app.protection.object.service.batch_protection_service import check_kubernetes_volume_names
        check_kubernetes_volume_names(resource_obj, ext_params)
        self.assertTrue(True, "无异常抛出")

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_hcs_disk_info_invalid_exception(self, mock_query_v2_resource):
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "cloudHost测试",
            "type": "CloudHost",
            "subType": "HCSCloudHost",
            "extendInfo": {
                "host": "{\"diskInfo\":[{\"attr\":\"SCSI\",\"id\":\"07a1b210-ff6c-4283-9488-667003a676f2\","
                        "\"lunWWN\":\"658f987100b749bcc5d445080000007e\",\"mode\":\"true\","
                        "\"name\":\"ecs-4d45-0003-volume-0000\",\"size\":\"10\"}],"
                        "\"id\":\"dcc7a215-5c43-431d-91f0-18f4105b6ce9\",\"name\":\"ecs-4d45-0003\","
                        "\"regionId\":\"sc-cd-1\",\"status\":\"SHUTOFF\"} "
            }}
        resource_obj = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65"
        )
        ext_params = CloudHostExtParam(
            disk_info=["error_id"]
        )
        from app.protection.object.service.batch_protection_service import check_hcs_disk_info
        self.assertRaises(EmeiStorBizException, check_hcs_disk_info,
                          resource_obj, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_hcs_disk_info_success(self, mock_query_v2_resource):
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "cloudHost测试",
            "type": "CloudHost",
            "subType": "HCSCloudHost",
            "extendInfo": {
                "host": "{\"diskInfo\":[{\"attr\":\"SCSI\",\"id\":\"07a1b210-ff6c-4283-9488-667003a676f2\","
                        "\"lunWWN\":\"658f987100b749bcc5d445080000007e\",\"mode\":\"true\","
                        "\"name\":\"ecs-4d45-0003-volume-0000\",\"size\":\"10\"}],"
                        "\"id\":\"dcc7a215-5c43-431d-91f0-18f4105b6ce9\",\"name\":\"ecs-4d45-0003\","
                        "\"regionId\":\"sc-cd-1\",\"status\":\"SHUTOFF\"} "
            }}
        resource_obj = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65"
        )
        ext_params = CloudHostExtParam(
            disk_info=["07a1b210-ff6c-4283-9488-667003a676f2"]
        )
        from app.protection.object.service.batch_protection_service import check_hcs_disk_info
        check_hcs_disk_info(resource_obj, ext_params)
        self.assertTrue(True, "无异常抛出")

    def test_protect_pre_check_mysql_success(self):
        """
        用例场景：检验保护MySQL单实例资源时成功
        前置条件：资源是MySQL-instance"
        检查点: 不报错
        """
        sla_obj = {"application": "MySQL", "policy_list":[]}
        resource_obj = {"sub_type": "MySQL-instance", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_protect_pre_check_open_gauss_success(self):
        """
        用例场景：检验保护OpenGauss单实例资源时成功
        前置条件：资源是OpenGauss-instance
        检查点: 不报错
        """
        sla_obj = {"application": "OpenGauss", "policy_list":[]}
        resource_obj = {"sub_type": "OpenGauss-instance", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    def test_protect_pre_check_sql_server_success(self):
        """
        用例场景：检验保护SQL Server单实例资源时成功
        前置条件：资源是SQLServer-instance
        检查点: 不报错
        """
        sla_obj = {"application": "SQLServer", "policy_list":[]}
        resource_obj = {"sub_type": "SQLServer-instance", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_protect_pre_check_kubernetes_namespace_success(self, mock_query_v2_resource):
        sla_obj = {"application": "KubernetesStatefulSet", "policy_list":[]}
        resource_obj = {"sub_type": "KubernetesNamespace", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_kubernetes_namespace_success(self, mock_query_v2_resource):
        sla_obj = {"application": "KubernetesStatefulSet", "policy_list":[]}
        resource_obj = {"sub_type": "KubernetesNamespace", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        result = batch_protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params)
        self.assertIsNone(result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_not_support_common_sla_exception(self, mock_query_v2_resource):
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "subType": "Fileset"
        }
        mock_query_v2_resource.return_value = resource_obj
        sla_obj = {"application": "Common", "policy_list":[]}
        resource_obj = {"sub_type": "Fileset", "os_type": None}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.base.db_base.database.session")
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_db2_success(self, mock_query_v2_resource, database_mock):
        sla_obj = {"application": "DB2", "policy_list":[]}
        resource_obj = {"sub_type": "DB2-database", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        database_mock().__enter__().query(ProtectedObject.uuid).filter().count.return_value = 0
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        result = batch_protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params)
        self.assertIsNone(result)

    @mock.patch("app.base.db_base.database.session")
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_db2_not_support_relate_protect(self, mock_query_v2_resource, database_mock):
        sla_obj = {"application": "DB2", "policy_list":[]}
        resource_obj = {"sub_type": "DB2-database", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        database_mock().__enter__().query(ProtectedObject.uuid).filter().count.return_value = 1
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_batch_protect_pre_check_open_gauss_success(self, mock_policy, mock_query_v2_resource):
        """
        用例场景：openGauss数据库不支持绑定增量sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为OpenGauss-database，不支持增量备份策略。
        """
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
                   'application': 'OpenGauss',
                   'policy_list': [
                       {"name": "full01", "type": "backup", "action": "full",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.05"
                                     }},
                       {"name": "全量01", "type": "backup", "action": "difference_increment",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.06"
                                     }}
                   ]
                   }
        resource_obj = {"sub_type": "OpenGauss-database", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        mock_policy.return_value = {"id": "OpenGauss-database", "schedule": {"setWorm": False}}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_protect_pre_check_hcs_project_success(self, mock_query_v2_resource):
        sla_obj = {"application": "HCSCloudHost", "policy_list":[]}
        resource_obj = {"sub_type": "HCSProject", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import protect_pre_check
        result = protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, False)
        self.assertIsNone(result)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_hcs_project_success(self, mock_query_v2_resource):
        sla_obj = {"application": "HCSCloudHost", "policy_list":[]}
        resource_obj = {"sub_type": "HCSProject", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        result = batch_protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params)
        self.assertIsNone(result)

    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_batch_protect_pre_check_dameng_success(self, mock_policy):
        """
        用例场景：dameng集群不支持绑定日志sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为Dameng-cluster，不支持日志备份策略。
        """
        sla_obj = {'uuid': '56defa38-52c1-4e65-9110-b29612c5973a',
                   'application': 'Dameng',
                   'policy_list': [
                       {"name": "全量01", "type": "backup", "action": "full",
                        "ext_parameters":{"worm_switch":False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.05"
                                     }},
                       {"name": "日志01", "type": "backup", "action": "log",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.06"
                                     }}
                   ]
                   }
        resource_obj = {"sub_type": "Dameng-cluster", "os_type": None}
        mock_policy.return_value = {"id": "Dameng-cluster", "schedule": {"setWorm": False}}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_batch_protect_pre_check_dameng_success_if_policy_contain_archiving(self, mock_policy):
        """
        用例场景：dameng集群不支持绑定归档sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为Dameng-cluster，不支持归档策略。
        """
        sla_obj = {'uuid': '56defa38-52c1-4e65-9110-b29612c5973a',
                   'application': 'Dameng',
                   'policy_list': [
                       {"name": "全量01", "type": "backup", "action": "full",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.05"
                                     }},
                       {"name": "归档", "type": "archiving", "action": "archiving",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2021.07.06"
                                     }}
                   ]
                   }
        resource_obj = {"sub_type": "Dameng-cluster", "os_type": None}
        mock_policy.return_value = {"id": "Dameng-cluster", "schedule": {"setWorm": False}}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_batch_protect_pre_check_db2_tablespace_success(self, mock_policy):
        """
        用例场景：db2表空间不支持绑定日志sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为DB2-tablespace，不支持日志备份策略。
        """
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe78',
                   'application': 'DB2',
                   'policy_list': [
                       {"name": "full01", "type": "backup", "action": "full",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2022.07.05"
                                     }},
                       {"name": "log01", "type": "backup", "action": "log",
                        "ext_parameters": {"worm_switch": False},
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2022.07.06"
                                     }}
                   ]
                   }
        resource_obj = {"sub_type": "DB2-tablespace", "os_type": None}
        mock_policy.return_value = {"id": "DB2-tablespace", "schedule": {"setWorm": False}}
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        self.assertRaises(EmeiStorBizException, batch_protect_pre_check,
                          sla_obj, resource_obj, first_os_type, ext_params)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_batch_protect_pre_check_golden_db_success(self,mock_query_v2_resource):
        """
        用例场景：GoldenDB集群实例修改保护成功
        前置条件：pm业务正常运行
        检查点： 资源类型为GoldenDB_clusterInstance，GoldenDB集群实例修改保护。
        """
        sla_obj = {"application": "GoldenDB","policy_list":[]}
        resource_obj = {"sub_type": "GoldenDB-clusterInstance", "os_type": None}
        mock_query_v2_resource.return_value = resource_obj
        first_os_type = None
        ext_params = None
        from app.protection.object.service.batch_protection_service import batch_protect_pre_check
        result = batch_protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params)
        self.assertIsNone(result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    @mock.patch("app.protection.object.service.batch_protection_service.build_protection_object")
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    @mock.patch("app.protection.object.db.projected_object.query_one_by_resource_id")
    @mock.patch(
        "app.protection.object.service.protection_plugin_manager.ProtectionPluginManager.convert_extend_parameter",
        MagicMock)
    def test_sync_sub_resource_add_kubernetes_stateful_set_success(self, mock_query_projected_object,
                                                                   mock_query_sla, mock_build_protection_object,
                                                                   mock_query_v2_resource):
        resource = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "sub_type": "KubernetesStatefulSet",
            "path": "1.1.1.1/Datacenter/pepeline_culte/",
            "parent_uuid": "211fed99-44e7-9bed-497a-fa7b4ee35d86"
        }
        from app.protection.object.models.projected_object import ProtectedObject
        mock_query_projected_object.return_value = ProtectedObject(**{
            "uuid": "fc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1",
            "ext_parameters": "{\"pre_script\":\"test_pre_script.sh\","
                              "\"post_script\":\"test_post_script.sh\","
                              "\"failed_script\":\"test_fail_script.sh\",\"resource_filters\":[],"
                              "\"overwrite\":true,\"binding_policy\":[\"APPLY_TO_ALL\","
                              "\"APPLY_TO_NEW\"]}"
        })
        mock_query_sla.return_value = {"uuid": "fc16731d-a701-42a8-86b0-ef20123c5975"}
        mock_build_protection_object.return_value = ProtectedObject(**{
            "uuid": "cc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1/aaa",
            "sub_type": "KubernetesStatefulSet"
        })
        mock_query_v2_resource.return_value = {
            "uuid": "311fed99-44e7-9bed-497a-fa7b4ee35d86",
            "name": "statefulSet测试",
            "type": "StatefulSet",
            "subType": "KubernetesStatefulSet",
            "extendInfo": {
                "sts": "{\"id\":\"18d8a9c9-0b59-4ebd-a215-903eb57e8651\",\"name\":\"invmdb-1-1-m\","
                       "\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"invmdb-1-1-m-0\","
                       "\"pvs\":[{\"lunName\":\"1-invmdb-1-1-m-0-backup\",\"name\":\"pv-invmdb-1-1-m-0-backup\","
                       "\"size\":\"50Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-data\",\"name\":\"pv-invmdb-1-1-m-0-data\",\"size\":\"80Gi\","
                       "\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-lredo\",\"name\":\"pv-invmdb-1-1-m-0-lredo\","
                       "\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"},"
                       "{\"lunName\":\"1-invmdb-1-1-m-0-redo\",\"name\":\"pv-invmdb-1-1-m-0-redo\",\"size\":\"85Gi\","
                       "\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\"}]}],\"storageClassNames\":["
                       "\"storage-invmdb-redo-1-1\",\"storage-invmdb-data-1-1\",\"storage-invmdb-lredo-1-1\","
                       "\"storage-invmdb-backup-1-1\"]}\n "
            }}
        from app.protection.object.service.batch_protection_service import sync_sub_resource_add
        from app.base.db_base import database
        with database.session() as session:
            result = sync_sub_resource_add(session, resource)
        self.assertIsNone(result)

    @mock.patch("app.base.db_base.database.session", MagicMock)
    @mock.patch("app.protection.object.db.projected_object.query_by_resource_ids")
    def test_check_is_exist_projected_object_success(self, mock_query_by_resource_ids):
        mock_query_by_resource_ids.return_value = [ProtectedObject(**{
            "uuid": "cc16731d-a701-42a8-86b0-ef20123c5975",
            "path": "1.1.1.1/aaa",
            "sub_type": "KubernetesStatefulSet"
        })]
        resource = ProtectResource(
            resource_id="ce8d0c9e983211eb80bcfa163ef214c6",
        )
        from app.protection.object.service.batch_protection_service import check_is_exist_projected_object
        self.assertRaises(EmeiStorBizException, check_is_exist_projected_object,
                          [resource])

    def test_check_gaussdbt_single_matches_sla_exception(self):
        """
        用例场景：GaussDBT单机不支持绑定通用sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GaussDBT-single，不支持通用备份策略。
        """
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe78',
                   'application': 'Common',
                   'policy_list': [
                       {"name": "full01", "type": "backup", "action": "full",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2022.07.05"
                                     }},
                       {"name": "log01", "type": "backup", "action": "log",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2022.07.06"
                                     }}
                   ]
                   }
        resource_type = "GaussDBT-single"
        from app.protection.object.service.batch_protection_service import check_gaussdbt_single_matches_sla
        self.assertRaises(EmeiStorBizException, check_gaussdbt_single_matches_sla, resource_type, sla_obj)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_general_db_matches_sla_exception_when_difference_increment(self, mock_query_v2_resource):
        """
        用例场景：GBase不支持绑定差异增量和差异sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GeneralDb，数据库类型为GBase,不支持增量和差异备份策略。
        """
        sla_obj = {
            'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
            'application': 'GeneralDb',
            'policy_list': [
                {"name": "full", "type": "backup", "action": "full"},
                {"name": "difference_increment", "type": "backup", "action": "difference_increment"}
            ]
        }
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "GeneralDb",
            "extendInfo": {
                "databaseType": "GBase",
                "scriptConf": "{\"backup\":{\"support\":[{\"backupType\":\"full\"}]}}"
            }
        }
        mock_query_v2_resource.return_value = resource_obj
        from app.protection.object.service.batch_protection_service import check_generaldb_matches_sla
        self.assertRaises(EmeiStorBizException, check_generaldb_matches_sla, resource_obj, sla_obj)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_general_db_matches_sla_success_when_replication_or_archive(self, mock_query_v2_resource):
        """
        用例场景：GBase支持绑定复制和归档sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GeneralDb，数据库类型为GBase,支持复制和归档策略。
        """
        sla_obj = {
            'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
            'application': 'GeneralDb',
            'policy_list': [
                {"name": "replication", "type": "replication", "action": "replication"},
                {"name": "archiving", "type": "archiving", "action": "archiving"}
            ]
        }
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "GeneralDb",
            "extendInfo": {
                "databaseType": "GBase",
                "scriptConf": "{\"backup\":{\"support\":[{\"backupType\":\"full\"}]}}"
            }
        }
        mock_query_v2_resource.return_value = resource_obj
        from app.protection.object.service.batch_protection_service import check_generaldb_matches_sla
        check_generaldb_matches_sla(resource_obj, sla_obj)
        self.assertIsNotNone(resource_obj)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_eapp_mysql_matches_sla(self, mock_query_v2_resource):
        """
        用例场景：GBase不支持绑定差异增量和差异sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GeneralDb，数据库类型为GBase,不支持增量和差异备份策略。
        """
        sla_obj = {
            'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
            'application': 'GeneralDb',
            'policy_list': [
                {"name": "full", "type": "backup", "action": "full"},
                {"name": "log", "type": "backup", "action": "log"}
            ]
        }
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "MySQL-clusterInstance",
            "extendInfo": {
                "clusterType": "EAPP"
            }
        }
        mock_query_v2_resource.return_value = resource_obj
        from app.protection.object.service.batch_protection_service import check_eapp_mysql_matches_sla
        self.assertRaises(EmeiStorBizException, check_eapp_mysql_matches_sla, resource_obj, sla_obj)

    def test_check_oracle_matches_sla(self):
        """
        用例场景：oracle存储快照备份不支持绑定差异sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GeneralDb，数据库类型为Oracle,不支持增量和差异备份策略。
        """
        sla_obj = {
            'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
            'application': 'GeneralDb',
            'policy_list': [
                {"name": "full", "type": "backup", "action": "cumulative_increment"}
            ]
        }
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "Oracle"
        }
        ext_params = OracleExtParam(
            storage_snapshot_flag=True
        )
        from app.protection.object.service.batch_protection_service import check_oracle_matches_sla
        self.assertRaises(EmeiStorBizException, check_oracle_matches_sla, resource_obj, sla_obj, ext_params)

    def test_check_gaussdbt_single_matches_sla_not_support_different_and_cumulative(self):
        """
        用例场景：GaussDBT单机不支持绑定增量和差异sla策略
        前置条件：pm业务正常运行
        检查点： 资源类型为GaussDBT-single，不支持增量和差异备份策略。
        """
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
                   'application': 'GaussDBT',
                   'policy_list': [
                       {"name": "full01", "type": "backup", "action": "full",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2023.07.05"
                                     }},
                       {"name": "log01", "type": "backup", "action": "difference_increment",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2023.07.06"
                                     }}
                   ]
                   }
        resource_type = "GaussDBT-single"
        from app.protection.object.service.batch_protection_service import check_gaussdbt_single_matches_sla
        self.assertRaises(EmeiStorBizException, check_gaussdbt_single_matches_sla, resource_type, sla_obj)

    def test_update_cyber_engine_self_learning_config(self):
        """
        用例场景：安全一体机开启自学习
        前置条件：dee业务正常运行
        检查点： 无报错
        """
        from app.protection.object.service.batch_protection_service import update_cyber_engine_self_learning_config
        from app.protection.object.schemas.extends.params.cloud_backup_ext_param import CloudBackupExtParam
        ext_parameters = CloudBackupExtParam(**{
            'share_type': 'NFS',
            'file_system_ids': ['1'],
            'is_open': True,
            'type': 1,
            'duration': 30,
        })
        update_cyber_engine_self_learning_config(ext_parameters, "d1", "r1")
        self.assertEqual(ext_parameters.progress, 0)

    @mock.patch.object(SystemBaseClient, "query_filesystem")
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch.object(kmc_util, "_build_kmc_handler", Mock(return_value=None))
    @mock.patch.object(client_util, "_build_ssl_context", Mock(return_value=None))
    @mock.patch.object(DeployType, "is_hyper_detect_deploy_type", Mock(return_value=True))
    @mock.patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_check_can_be_protected_for_anti_ransomware(self, _mock_query_fs):
        """
        用例场景：测试配置了smart mobility的文件系统不能保护
        前置条件：NA
        检查点： 抛出异常
        """
        from app.protection.object.service.batch_protection_service import check_can_be_protected_for_anti_ransomware
        from app.protection.object.schemas.extends.params.cloud_backup_ext_param import CloudBackupExtParam
        ext_parameters = CloudBackupExtParam(**{
            'share_type': 'NFS',
            'file_system_ids': ['1'],
            'is_open': True,
            'type': 1,
            'duration': 30,
        })
        filesystem_info = {"hasSmartMobility": "1"}
        _mock_query_fs.return_value = filesystem_info
        self.assertRaises(EmeiStorBizException, check_can_be_protected_for_anti_ransomware, ResourceSubTypeEnum.CloudBackupFileSystem.value, ext_parameters)

    def test_check_exchange_resource_matches_sla(self):
        """
        用例场景：exchange邮箱支持全量备份和增量备份
        前置条件：PM业务正常运行
        检查点： 抛出异常
        """
        sla_obj = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe79',
                   'application': 'Exchange',
                   'policy_list': [
                       {"name": "full01", "type": "backup", "action": "full",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2023.07.05"
                                     }},
                       {"name": "log01", "type": "backup", "action": "log",
                        "schedule": {"interval": "interval",
                                     "interval_unit": "10",
                                     "start_time": "2023.07.06"
                                     }}
                   ]
                   }
        resource_type = "Exchange-mailbox"
        from app.protection.object.service.batch_protection_service import check_exchange_resource_matches_sla
        self.assertRaises(EmeiStorBizException, check_exchange_resource_matches_sla, resource_type, sla_obj)

    def test_check_tidb_back_type(self):
        """
        用例场景：安全一体机开启自学习
        前置条件：dee业务正常运行
        检查点： 无报错
        """
        policy = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65",
            name="full",
            action="full"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        from app.protection.object.service.batch_protection_service import check_tidb_resource_matches_sla
        check_tidb_resource_matches_sla(ResourceSubTypeEnum.TiDB_CLUSTER.value, sla_obj)
        self.assertTrue(True)

    def test_check_tidb_back_type_error(self):
        """
        用例场景：安全一体机开启自学习
        前置条件：dee业务正常运行
        检查点： 无报错
        """
        from app.protection.object.service.batch_protection_service import check_tidb_resource_matches_sla
        sla_obj = dict(uuid='34926913-b6a6-4c3b-812e-4fba893cbe5e', application='KubernetesStatefulSet', name='Golden',
                       policy_list=[
                           {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, ])
        with self.assertRaises(EmeiStorBizException):
            check_tidb_resource_matches_sla(ResourceSubTypeEnum.TiDB_DATABASE.value, sla_obj)

    def test_check_oceanbase_back_type(self):
        policy = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65",
            name="full",
            action="full"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        from app.protection.object.service.batch_protection_service import check_oceanbase_resource_matches_sla
        check_oceanbase_resource_matches_sla(ResourceSubTypeEnum.OCEANBASE_CLUSTER.value, sla_obj)
        self.assertTrue(True)

    def test_check_tidb_back_type_error(self):
        from app.protection.object.service.batch_protection_service import check_oceanbase_resource_matches_sla
        sla_obj = dict(uuid='34926913-b6a6-4c3b-812e-4fba893cbe5e', application='KubernetesStatefulSet', name='Golden',
                       policy_list=[
                           {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, ])
        with self.assertRaises(EmeiStorBizException):
            check_oceanbase_resource_matches_sla(ResourceSubTypeEnum.OCEANBASE_TENANT.value, sla_obj)

    def test_check_tdsql_back_type(self):
        policy = dict(
            uuid="3bd9e448-816d-43b1-9b16-1feca34ece65",
            name="full",
            action="full"
        )
        sla_obj = dict(
            policy_list=[policy]
        )
        from app.protection.object.service.batch_protection_service import check_tdsql_resource_matches_sla
        check_tdsql_resource_matches_sla(ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value, sla_obj)
        self.assertTrue(True)

    def test_check_tdsql_back_type_error(self):
        from app.protection.object.service.batch_protection_service import check_tdsql_resource_matches_sla
        sla_obj = dict(uuid='34926913-b6a6-4c3b-812e-4fba893cbe5e', application='KubernetesStatefulSet', name='Golden',
                       policy_list=[
                           {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, {
                               "action": "difference_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2021.07.05"
                                            }
                           }, ])
        with self.assertRaises(EmeiStorBizException):
            check_tdsql_resource_matches_sla(ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value, sla_obj)

    def test_check_apsara_stack_resource_matches_sla_error(self):
        """
        用例场景：阿里云虚拟机备份，下发备份类型为permanent_increment
        前置条件：PM业务正常运行
        检查点： 抛出异常
        """
        from app.protection.object.service.batch_protection_service import check_apsara_stack_resource_matches_sla
        sla_obj = dict(uuid='fbc5f656-cd13-47ea-8043-cecaed15fbee',
                       application='KubernetesStatefulSet',
                       name='Golden',
                       policy_list=[
                           {
                               "action": "permanent_increment",
                               "type": "backup",
                               "uuid": "test-uuid1",
                               "schedule": {"interval": "interval",
                                            "interval_unit": "10",
                                            "start_time": "2024.02.21"
                                            }
                           },
                       ])
        with self.assertRaises(EmeiStorBizException):
            check_apsara_stack_resource_matches_sla(ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value, sla_obj)

    def test_check_vmware_matches_sla_valid(self):
        """
        用例场景: 验证 check_vmware_matches_sla 函数在提供有效的 SLA 对象和资源类型时不会抛出异常。
        前置条件:
        SLA 对象为 {"application": "Replica"}。
        资源类型为 "VirtualMachine"。
        检查点:
        调用 check_vmware_matches_sla 不会抛出 EmeiStorBizException。
        """
        from app.protection.object.service.batch_protection_service import check_vmware_matches_sla
        valid_sla_obj = {"application": "Replica", "policy_list":[]}
        try:
            check_vmware_matches_sla("vim.VirtualMachine", valid_sla_obj)
        except EmeiStorBizException:
            self.fail("check_vmware_matches_sla raised EmeiStorBizException unexpectedly!")

    def test_check_vmware_matches_sla_invalid(self):
        """
        用例场景: 验证 check_vmware_matches_sla 函数在提供无效的 SLA 对象时会抛出异常。
        前置条件:
        SLA 对象为 {"application": "Common"}。
        资源类型为 "VirtualMachine"、"HostSystem" 和 "ClusterComputeResource"。
        检查点:
        调用 check_vmware_matches_sla 时抛出 EmeiStorBizException。
        """
        from app.protection.object.service.batch_protection_service import check_vmware_matches_sla
        invalid_sla_obj = {"application": "Common", "policy_list":[]}

        with self.assertRaises(EmeiStorBizException):
            check_vmware_matches_sla("vim.VirtualMachine", invalid_sla_obj)

        with self.assertRaises(EmeiStorBizException):
            check_vmware_matches_sla("vim.HostSystem", invalid_sla_obj)

        with self.assertRaises(EmeiStorBizException):
            check_vmware_matches_sla("vim.ClusterComputeResource", invalid_sla_obj)

    def test_check_vmware_sla_modified_no_change(self):
        """
        用例场景: 验证 check_vmware_sla_modified 在 SLA 没有修改时不会抛出异常。
        前置条件:
        SLA 对象为 {"application": "Common"}。
        资源类型为 "VirtualMachine"。
        is_sla_changed 设置为 False。
        检查点:
        调用 check_vmware_sla_modified 不会抛出 EmeiStorBizException。
        """
        from app.protection.object.service.batch_protection_service import check_vmware_sla_modified
        valid_sla_obj = {"application": "Common", "policy_list":[]}
        try:
            check_vmware_sla_modified("vim.VirtualMachine", valid_sla_obj, False)
        except EmeiStorBizException:
            self.fail("check_vmware_sla_modified raised EmeiStorBizException unexpectedly!")

    def test_check_vmware_sla_modified_with_change(self):
        """
        用例场景: 验证 check_vmware_sla_modified 在 SLA 被修改时会抛出异常。
        前置条件:
        SLA 对象为 {"application": "Common"}。
        资源类型为 "vim.VirtualMachine"。
        is_sla_changed 设置为 True。
        检查点:
        调用 check_vmware_sla_modified 时抛出 EmeiStorBizException。
        """
        from app.protection.object.service.batch_protection_service import check_vmware_sla_modified
        invalid_sla_obj = {"application": "Common", "policy_list":[]}

        with self.assertRaises(EmeiStorBizException):
            check_vmware_sla_modified("vim.VirtualMachine", invalid_sla_obj, True)

    @patch("app.resource.service.common.resource_service.query_resource_by_id", MagicMock(return_value=None))
    @patch("app.base.db_base.database.session")
    def test_check_exchange_resource_has_related(self, database_mock):
        from app.protection.object.service.batch_protection_service import check_exchange_resource_has_related
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe78",
            "root_uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "Exchange-group",
            "extendInfo": {
                "databaseType": "GBase",
                "scriptConf": "{\"backup\":{\"support\":[{\"backupType\":\"full\"}]}}"
            }
        }
        check_exchange_resource_has_related(resource_obj)

        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe78",
            "root_uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "Exchange-database",
            "name": "Exchange_name",
            "extendInfo": {
                "databaseType": "GBase",
                "scriptConf": "{\"backup\":{\"support\":[{\"backupType\":\"full\"}]}}"
            }
        }
        check_exchange_resource_has_related(resource_obj)

        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe78",
            "root_uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "sub_type": "Exchange-mailbox",
            "extendInfo": {
                "databaseType": "GBase",
                "scriptConf": "{\"backup\":{\"support\":[{\"backupType\":\"full\"}]}}"
            }
        }
        check_exchange_resource_has_related(resource_obj)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_check_hyper_v_disk_info_success(self, mock_query_v2_resource):
        from app.protection.object.service.batch_protection_service import check_hyper_v_disk_info
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "subType": "HyperV.VM",
            "extendInfo": {
                "disks": "[{\"uuid\":\"960af471-d503-4015-a7db-be9535ceb7ba\",\"name\":\"test1\",\"extendInfo\":{\"Format\":\"VHDX\",\"IsShared\":\"false\"}}]"
            }
        }
        mock_query_v2_resource.return_value = resource_obj
        check_hyper_v_disk_info(resource_obj)

    @mock.patch("app.common.clients.resource_client.ResourceClient.query_v2_resource")
    def test_check_check_hyper_v_disk_info_fail(self, mock_query_v2_resource):
        from app.protection.object.service.batch_protection_service import check_hyper_v_disk_info
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "subType": "HyperV.VM",
            "extendInfo": {
                "disks": "[{\"uuid\":\"960af471-d503-4015-a7db-be9535ceb7ba\",\"name\":\"test1\",\"extendInfo\":{\"Format\":\"VHDX\",\"IsShared\":\"true\"}}]"
            }
        }
        mock_query_v2_resource.return_value = resource_obj
        with self.assertRaises(EmeiStorBizException):
            check_hyper_v_disk_info(resource_obj)

    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_check_worm_switch_and_worm_policy_success(self, mock_query_policy):
        from app.protection.object.service.batch_protection_service import check_worm_switch_and_worm_policy
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "subType": "HyperV.VM",
            "extendInfo": {
                "disks": "[{\"uuid\":\"960af471-d503-4015-a7db-be9535ceb7ba\",\"name\":\"test1\",\"extendInfo\":{\"Format\":\"VHDX\",\"IsShared\":\"false\"}}]"
            }
        }
        sla = {
          "name":"sla_test",
           "policy_list":[{"name":"backup_test",
                           "type":"backup",
                           "ext_parameters":{
             "worm_switch":True,
          }}]
        }
        worm_policy = {}
        mock_query_policy.return_value = worm_policy
        check_worm_switch_and_worm_policy(resource_obj, sla)

    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.query_policy_by_resource_id")
    def test_check_worm_switch_and_worm_policy_invalid(self, mock_query_policy):
        """
        用例场景: 验证 check_worm_switch_and_worm_policy 函数在校验的SLA的worm开关和worm策略中的开关同时开启时会抛出异常。
        前置条件:
        SLA 对象为 {"id":"123"}。
        资源类型为 "backup"
        检查点:
        调用 check_worm_switch_and_worm_policy 时抛出 EmeiStorBizException。
        """
        from app.protection.object.service.batch_protection_service import check_worm_switch_and_worm_policy
        resource_obj = {
            "uuid": "12345678-b6a6-4c3b-812e-4fba893cbe79",
            "subType": "HyperV.VM",
            "extendInfo": {
                "disks": "[{\"uuid\":\"960af471-d503-4015-a7db-be9535ceb7ba\",\"name\":\"test1\",\"extendInfo\":{\"Format\":\"VHDX\",\"IsShared\":\"false\"}}]"
            }
        }
        sla = {
            "name": "sla_test",
            "policy_list": [{"name": "backup_test",
                             "type": "backup",
                             "ext_parameters": {
                                 "worm_switch": True,
                             }}]
        }
        worm_policy = {"id": "123", "name": "worm_policy_test", "schedule": {"setWorm": True}}
        mock_query_policy.return_value = worm_policy
        with self.assertRaises(EmeiStorBizException):
            check_worm_switch_and_worm_policy(resource_obj, sla)

if __name__ == '__main__':
    unittest.main(verbosity=2)
