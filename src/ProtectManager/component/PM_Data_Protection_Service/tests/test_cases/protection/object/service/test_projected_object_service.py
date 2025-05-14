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
import uuid
from unittest import mock
from unittest.mock import MagicMock, Mock, patch

from app.common.deploy_type import DeployType
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.log.kernel import convert_storage_type, trans_array_params
from app.common.deploy_type import DeployType
from tests.test_cases.tools import timezone, functiontools
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.common.events import mock_producer  # noqa
from tests.test_cases.common.mock_settings import fake_settings  # noqa
from tests.test_cases.tools import timezone, functiontools


sys.modules['app.common.config'] = Mock()
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.protection.object.common.db_config'] = Mock()
sys.modules['app.backup.common.config.db_config'] = Mock()
sys.modules['app.common.database'] = Mock()
DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)

from app.backup.service import backup_workflow
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.common.security.kmc_util import Kmc
from app.protection.object.service.projected_object_service import create_job, ProtectedObjectService, \
    get_log_data_cyber
from app.base.db_base import database

sla_object = {'uuid': '34926913-b6a6-4c3b-812e-4fba893cbe5e',
              'application': 'CloudBackupFilesystem', 'name': 'Golden', 'policy_list': [
        {
            "action": "difference_increment",
            "type": "backup",
            "schedule": {"window_start": ""}
        },
        {
            "action": "difference_increment",
            "type": "backup",
            "schedule": {"window_start": ""}
        },
    ]
              }


@mock.patch("sqlalchemy.orm.session.Session", MagicMock)
class TestProjectedObjectService(unittest.TestCase):
    def setUp(self):
        super(TestProjectedObjectService, self).setUp()
        from app.protection.object.models.projected_object import ProtectedObject
        self.protected_object = ProtectedObject

    @mock.patch("app.backup.service.backup_service.check_smart_mobility_can_be_backup", Mock(return_value=True))
    @mock.patch("app.backup.service.backup_service.check_can_be_backup_by_deploy_type", Mock(return_value=True))
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch.object(DeployType, "is_cyber_engine_deploy_type", Mock(return_value=True))
    def test_execute_manual_backup_success(self, mock_query_sla):
        from app.protection.object.schemas.protected_object import CurrentManualBackupRequest
        from app.common.enums.sla_enum import PolicyActionEnum
        mock_query_sla.return_value = sla_object
        projected_object = self.protected_object(resource_id=str(uuid.uuid4()),
                                                 sub_type=ResourceSubTypeEnum.CloudBackupFileSystem)
        backup_req = CurrentManualBackupRequest(sla_id=str(uuid.uuid4()),
                                                action=PolicyActionEnum.full.value,
                                                copy_name="test-copy")
        job_id = "job1"
        backup_workflow.backup_start = Mock(return_value=job_id)
        from app.protection.object.service.projected_object_service import execute_manual_backup
        backup = execute_manual_backup(projected_object, backup_req)
        self.assertEqual(backup, job_id)

    @mock.patch("app.backup.service.backup_service.check_smart_mobility_can_be_backup", Mock(return_value=True))
    @mock.patch("app.backup.service.backup_service.check_can_be_backup_by_deploy_type", Mock(return_value=True))
    @mock.patch("app.common.clients.protection_client.ProtectionClient.query_sla")
    @mock.patch.object(Kmc, "decrypt", Mock(return_value=None))
    def test_execute_manual_backup_cyber_engine_success(self, mock_query_sla):
        from app.protection.object.schemas.protected_object import CurrentManualBackupRequest
        from app.common.enums.sla_enum import PolicyActionEnum
        from app.common.deploy_type import DeployType
        DeployType.is_cyber_engine_deploy_type = Mock(return_value=True)
        mock_query_sla.return_value = sla_object
        projected_object = self.protected_object(resource_id=str(uuid.uuid4()),
                                                 sub_type=ResourceSubTypeEnum.CloudBackupFileSystem)
        backup_req = CurrentManualBackupRequest(sla_id=str(uuid.uuid4()),
                                                action=PolicyActionEnum.full.value,
                                                copy_name="test-copy")
        job_id = "job1"
        backup_workflow.backup_start = Mock(return_value=job_id)
        from app.protection.object.service.projected_object_service import execute_manual_backup
        backup = execute_manual_backup(projected_object, backup_req)
        self.assertEqual(backup, job_id)

    def test_filter_sla_policy_should_return_interval_policies_when_given_sla_has_interval_policy(self):
        """
        验证场景：验证创建周期调度时，是否能够筛选正确的策略
        前置条件：无
        验证点：1.周期类型的策略返回True  2.非周期类型的策略返回False
        """
        # Import
        from app.protection.object.service.projected_object_service import filter_sla_policy
        from tests.test_cases.tools import sla_mock
        # Given
        policy_list = sla_mock.nas_filesystem_sla_with_interval_policies.get("policy_list")
        # When and Then
        self.assertEqual(True, filter_sla_policy(None, policy_list[0]), "assert policy is need create schedule")
        self.assertEqual(True, filter_sla_policy(None, policy_list[1]), "assert policy is need create schedule")
        self.assertEqual(False, filter_sla_policy(None, policy_list[2]), "assert policy not need create schedule")
        self.assertEqual(True, filter_sla_policy(None, policy_list[3]), "assert policy is need create schedule")

    @mock.patch("app.common.clients.scheduler_client.SchedulerClient.create_interval_schedule_new")
    def test_build_protection_task_success(self, mock_create_interval_schedule_new):
        """
        验证场景：能够创建正确的保护对象
        前置条件：无
        验证点：1.不传入task_id，则使用根据保护对象id和策略id计算得到的uuid  2.传入task_id，则使用传入的task_id
        """
        from app.protection.object.service.projected_object_service import build_protection_task

        obj_id = "d72c8dcb-104f-49ac-aae9-06dca85bdd3f"
        policy = {"schedule": {"interval": "interval",
                               "interval_unit": "10",
                               "start_time": "2021.07.05"
                               },
                  "uuid": "966b5cad-ac48-4774-aff1-3411c75a11b5"}
        topic = ""
        schedule_params = {}
        mock_create_interval_schedule_new.return_value = "966b5cad-ac48-4774-aff1-3411c75a11b5"
        result = build_protection_task(obj_id, policy, topic, schedule_params)
        task_id = str(uuid.uuid5(uuid.NAMESPACE_OID, obj_id + policy.get("uuid")))

        result1 = build_protection_task(obj_id, policy, topic, schedule_params, "966b5cad-ac48-4774-aff1-3411c75a11b5")

        from app.protection.object.models.projected_object import ProtectedTask

        task = ProtectedTask(uuid="966b5cad-ac48-4774-aff1-3411c75a11b5",
                             policy_id=policy.get("uuid"),
                             protected_object_id=obj_id,
                             schedule_id="966b5cad-ac48-4774-aff1-3411c75a11b5")
        self.assertNotEqual(result.__dict__["schedule_id"], task.__dict__["schedule_id"])
        self.assertEqual(task_id, result.__dict__["schedule_id"])
        self.assertEqual(result1.__dict__["schedule_id"], task.__dict__["schedule_id"])

    def test_whether_has_children_resource_to_backup(self):
        from app.protection.object.service.projected_object_service import has_children_need_backup
        resource_type = ResourceTypeEnum.Namespace
        resource_sub_type = ResourceSubTypeEnum.KubernetesNamespace
        self.assertTrue(has_children_need_backup(resource_type, resource_sub_type))
        resource_type = ResourceTypeEnum.Cluster
        resource_sub_type = ResourceSubTypeEnum.FusionCompute
        self.assertTrue(has_children_need_backup(resource_type, resource_sub_type))
        resource_type = ResourceTypeEnum.Host
        self.assertTrue(has_children_need_backup(resource_type, resource_sub_type))
        resource_type = ResourceTypeEnum.VM
        self.assertFalse(has_children_need_backup(resource_type, resource_sub_type))
        resource_sub_type = ResourceSubTypeEnum.OPENSTACK_PROJECT
        self.assertTrue(has_children_need_backup(resource_type, resource_sub_type))

    def test_check_is_need_build_task_success(self):
        from app.protection.object.service.projected_object_service import check_is_need_build_task
        self.assertFalse(check_is_need_build_task(ResourceTypeEnum.Cluster, ResourceSubTypeEnum.KubernetesNamespace))
        self.assertTrue(check_is_need_build_task(ResourceTypeEnum.Cluster, ResourceSubTypeEnum.KubernetesStatefulSet))
        self.assertFalse(check_is_need_build_task(ResourceTypeEnum.OPENSTACK, ResourceSubTypeEnum.OPENSTACK_PROJECT))

    @patch("app.common.toolkit.create_job_center_task", MagicMock(return_value="1"))
    def test_build_job_center_task_success(self):
        """
        验证场景：创建修改保护任务
        前置条件：无
        验证点：返回任务id正确
        """
        sla = {"name": "test1"}
        user_id = str(uuid.uuid4())
        resource_object = {
            "uuid": str(uuid.uuid4()),
            "name": "test1",
            "type": ResourceTypeEnum.Cluster,
            "sub_type": ResourceSubTypeEnum.KubernetesNamespace

        }
        job_id = create_job(sla, user_id, resource_object)
        self.assertEqual(job_id, "1")

    @patch("app.common.toolkit.create_job_center_task", MagicMock(return_value=None))
    def test_build_job_center_task_raises_exception(self):
        """
        验证场景：创建修改保护任务
        前置条件：无
        验证点：返回任务id失败
        """
        sla = {"name": "test1"}
        user_id = str(uuid.uuid4())
        resource_object = {
            "uuid": str(uuid.uuid4()),
            "name": "test1",
            "type": ResourceTypeEnum.Cluster,
            "sub_type": ResourceSubTypeEnum.KubernetesNamespace

        }
        self.assertRaises(EmeiStorBizException, create_job,
                          sla, user_id, resource_object)

    @mock.patch("app.base.db_base.database.session")
    def test_query_sla_compliance_success(self, _mock_session):
        """
        验证场景：查询sla遵循度
        前置条件：无
        验证点：sla遵循度查询成功
        """
        user_info = {"role-list": ["Role_DP_Admin"], "user-id": str(uuid.uuid4()), "domain-id": "123"}
        _mock_session().__enter__().query().join().filter().filter().count.side_effect = [1, 2]
        with database.session() as session:
            result = ProtectedObjectService.query_sla_compliance(session, user_info)
            self.assertEqual(result.in_compliance, 1)
            self.assertEqual(result.out_of_compliance, 2)

    @mock.patch("app.protection.object.service.projected_object_service.load_resource")
    def test_get_log_data_cyber(self, mock_load_resource):
        """
        验证场景：组装安全一体机告警所需资源设备信息
        前置条件：无
        验证点：返回信息成功
        """
        resource = {}
        resource.update({"path": "CyberEngine OceanProtect/op28/System_vStore/testj"})
        resource.update({"root_uuid": "2102353GTH10L8000008"})
        resource.update({"parent_name": "System_env"})
        resource.update({"parent_uuid": "0"})
        resource.update({"parent_uuid": "0"})
        resource.update({"uuid": "34b147bc-038a-399d-b3b8-72650de2cb3c"})
        resource.update({"name": "test_resource"})
        mock_load_resource.return_value = [resource]
        self.assertEqual("op28", get_log_data_cyber("test")[0])

    def test_convert_storage_type(self):
        """
        验证场景：验证规范设备类型函数功能是否正确
        前置条件：无
        验证点：类型转换正确
        """
        self.assertEqual("OceanStor Pacific", convert_storage_type("Pacific"))
        self.assertEqual("OceanStor Dorado", convert_storage_type("Dorado"))
        self.assertEqual("OceanProtect", convert_storage_type("OceanProtect CyberEngine"))

    def test_trans_array_params(self):
        """
        验证场景：对于数组类型末尾内容为cyber-array-true，则拆分为单独元素组成的二维数组
        前置条件：无
        验证点：转换正确，末尾元素清理掉标记位
        """
        log_params = [['name', 'uuid', 'type', 'cyber-array-true']]
        self.assertEqual(3, len(trans_array_params(log_params)))
        log_params = [['name', 'uuid', 'type']]
        self.assertEqual(1, len(trans_array_params(log_params)))


if __name__ == '__main__':
    unittest.main(verbosity=2)
