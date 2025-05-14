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
import json
import os
from typing import List

from app.common.enums.schedule_enum import ExecuteType
from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.service.projected_object_service import build_protection_object, build_protection_task, \
    build_schedule_params, filter_sla_policy
from app.protection.object.service.protection_plugin import ProtectionPlugin
from app.protection.object.schemas.extends.params.vmware_ext_param import VmExtParam, VirtualResourceExtParam
from app.protection.object.service.batch_protection_service import filter_disks_with_uuid
from app.common.clients.resource_client import ResourceClient
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.resource.kafka.topics import SCAN_VM_UNDER_COMPUTE_RES

SERVICES = [ResourceSubTypeEnum.VirtualMachine, ResourceSubTypeEnum.ClusterComputeResource,
            ResourceSubTypeEnum.HostSystem]


class VMwareProtectionPlugin(ProtectionPlugin):

    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        """
        容器保护的高级参数转化为虚拟机保护的高级参数
            1. 查询当前虚拟机当前的磁盘
            2. 与过滤条件中的磁盘信息进行对比，匹配到符合条件的磁盘信息
            3. 构造vm参数对象
        :param filter_list: 磁盘信息过滤器
        :param resource: vm虚拟机资源
        :param extend_parameter: 容器保护的高级参数
        :return:
        """
        disks = ResourceClient.query_vm_disk(resource.get("uuid"))
        if disks is None or len(disks) <= 0:
            return None
        disk_info = list(disk.get("uuid") for disk in disks if filter_disks_with_uuid(disk, filter_list))
        if len(disk_info) <= 0:
            return None
        return VmExtParam(**{
            "pre_script": extend_parameter.pre_script,
            "post_script": extend_parameter.post_script,
            "all_disk": len(disks) == len(disk_info),
            "concurrent_requests": extend_parameter.concurrent_requests,
            "concurrent_requests_uuid": extend_parameter.concurrent_requests_uuid,
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "host_list": extend_parameter.host_list,
            "disk_info": disk_info
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object(resource_info, sla, ext_params, SCAN_VM_UNDER_COMPUTE_RES)

    def query_sub_resources(self, resource_info):
        # 集群下的虚拟机的父资源是集群而不是主机，对集群添加保护不会保护主机，因此子资源只需要查询虚拟机
        return ResourceClient.query_resource_list(
                    {"path": resource_info.path + os.sep, "type": ResourceTypeEnum.VM})

    def query_sub_resources_by_obj(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "type": ResourceTypeEnum.VM})

    def query_sub_resources_only_vm(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "type": ResourceTypeEnum.VM})

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return list(build_protection_task(
            projected_object.uuid,
            policy,
            SCAN_VM_UNDER_COMPUTE_RES,
            build_schedule_params(SCAN_VM_UNDER_COMPUTE_RES, resource_id, sla.get("uuid"),
                                  projected_object.chain_id, policy,
                                  ExecuteType.AUTOMATIC.value)
        ) for policy in sla["policy_list"]
          if filter_sla_policy(sla, policy))

    def build_ext_parameters(self, ext_parameters):
        return VirtualResourceExtParam(**json.loads(ext_parameters))


def create():
    return VMwareProtectionPlugin()
