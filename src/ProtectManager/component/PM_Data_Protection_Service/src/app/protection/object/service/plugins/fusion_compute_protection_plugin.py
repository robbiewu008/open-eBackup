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
from app.protection.object.schemas.extends.params.fusion_compute_param import FusionComputeExtParam
from app.protection.object.service.projected_object_service import build_protection_object_without_task
from app.protection.object.service.protection_plugin import ProtectionPlugin

SERVICES = [ResourceSubTypeEnum.FusionCompute]


class FusionComputeProtectionPlugin(ProtectionPlugin):

    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        """
        容器保护的高级参数转化为pv保护的高级参数  后续扩展添加
            1. 查询当前虚拟机当前的磁盘
            2. 与过滤条件中的磁盘信息进行对比，匹配到符合条件的磁盘信息
            3. 构造vm参数对象
        :param extend_parameter: 扩展参数
        :param filter_list: 磁盘信息过滤器
        :param resource: vm虚拟机资源
        :return:
        """
        return FusionComputeExtParam(**{
            "agents": extend_parameter.agents,
            "pre_script": extend_parameter.pre_script,
            "post_script": extend_parameter.post_script,
            "failed_script": extend_parameter.failed_script,
            "overwrite": extend_parameter.overwrite,
            "binding_policy": extend_parameter.binding_policy,
            "resource_filters": extend_parameter.resource_filters,
            "disk_filters": extend_parameter.disk_filters,
            "disk_info": extend_parameter.disk_info,
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "snap_delete_speed": extend_parameter.snap_delete_speed
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object_without_task(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        resources = ResourceClient.query_resource_list(
            {"path": resource_info.path + os.sep, "sub_type": ResourceSubTypeEnum.FusionCompute})
        sub_resources = [resource for resource in resources if resource.get("uuid") != resource_info.uuid]
        return sub_resources

    def query_sub_resources_by_obj(self, obj):
        resources = ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.FusionCompute})
        sub_resources = [resource for resource in resources if resource.get("uuid") != obj.uuid]
        return sub_resources

    def query_sub_resources_only_vm(self, obj):
        resources = ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.FusionCompute})
        sub_resources = [resource for resource in resources if resource.get("uuid") != obj.uuid]
        return sub_resources

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return []

    def build_ext_parameters(self, ext_parameters):
        return FusionComputeExtParam(**json.loads(ext_parameters))


def create():
    return FusionComputeProtectionPlugin()
