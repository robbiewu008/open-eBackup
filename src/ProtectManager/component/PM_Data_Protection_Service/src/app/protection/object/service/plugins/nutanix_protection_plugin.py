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

from app.common.clients.resource_client import ResourceClient
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.schemas.extends.params.nutanix_ext_param import NutanixVmExtParam, NutanixHostExtParam
from app.protection.object.service.projected_object_service import build_protection_object_without_task
from app.protection.object.service.protection_plugin import ProtectionPlugin

SERVICES = [
    ResourceSubTypeEnum.NUTANIX, ResourceSubTypeEnum.NUTANIX_CLUSTER,
    ResourceSubTypeEnum.NUTANIX_HOST, ResourceSubTypeEnum.NUTANIX_VM
]

SUB_TYPE_DICT = {
    ResourceSubTypeEnum.NUTANIX_CLUSTER: [ResourceSubTypeEnum.NUTANIX_VM, ResourceSubTypeEnum.NUTANIX_HOST],
    ResourceSubTypeEnum.NUTANIX_HOST: [ResourceSubTypeEnum.NUTANIX_VM]
}


class NutanixProtectionPlugin(ProtectionPlugin):

    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        """
        容器保护的高级参数转化为 Nutanix 保护的高级参数
            1. 查询当前虚拟机当前的磁盘
            2. 与过滤条件中的磁盘信息进行对比，匹配到符合条件的磁盘信息
            3. 构造 Nutanix 参数对象
        :param filter_list: 磁盘信息过滤器
        :param resource: Nutanix 资源
        :param extend_parameter: 容器保护的高级参数
        :return:
        """

        if resource.get("sub_type") in [ResourceSubTypeEnum.NUTANIX_CLUSTER.value,
                                        ResourceSubTypeEnum.NUTANIX_HOST.value]:
            return NutanixHostExtParam(**{
                "agents": extend_parameter.agents,
                "binding_policy": extend_parameter.binding_policy,
                "overwrite": extend_parameter.overwrite,
                "resource_filters": extend_parameter.resource_filters,
                "archive_res_auto_index": extend_parameter.archive_res_auto_index,
                "backup_res_auto_index": extend_parameter.backup_res_auto_index,
                "disk_filters": extend_parameter.disk_filters
            })

        return NutanixVmExtParam(**{
            "agents": extend_parameter.agents,
            "all_disk": "true",
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "disk_info": []
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object_without_task(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        return self.query_sub_resources_by_obj(resource_info)

    def query_sub_resources_by_obj(self, obj):
        return ResourceClient.query_resource_list({"path": obj.path + os.sep, "sub_type": SUB_TYPE_DICT[obj.sub_type]})

    def query_sub_resources_only_vm(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.NUTANIX_VM})

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return []

    def build_ext_parameters(self, ext_parameters):
        return NutanixHostExtParam(**json.loads(ext_parameters))


def create():
    return NutanixProtectionPlugin()
