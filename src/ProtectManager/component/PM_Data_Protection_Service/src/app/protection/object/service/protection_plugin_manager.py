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
from typing import List

from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.service.protection_plugin_factory import protection_plugin_factory
from app.common.enums.resource_enum import ResourceSubTypeEnum


class ProtectionPluginManager:

    def __init__(self, sub_type: ResourceSubTypeEnum):
        self.impl = protection_plugin_factory.create_plugin(sub_type)

    def convert_extend_parameter(self, filters: List[ResourceFilter], resource, extend_parameter):
        return self.impl.do_convert_extend_parameter(filters, resource, extend_parameter)

    def build_protection_object(self, resource_info, sla, ext_params):
        return self.impl.build_protection_object(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        # 需要对子资源添加保护/修改保护时，查询子资源
        return self.impl.query_sub_resources(resource_info)

    def query_sub_resources_by_obj(self, obj):
        # 移除保护时查询子资源，移除父类需要将父类下所有子类资源全部移除，例如移除集群保护需要移除主机和虚拟机
        return self.impl.query_sub_resources_by_obj(obj)

    def query_sub_resources_only_vm(self, obj):
        # 调度备份任务时查询子资源，只需要调度虚拟机/云服务器级别
        return self.impl.query_sub_resources_only_vm(obj)

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return self.impl.build_task_list(sla, resource_id, projected_object, execute_req)

    def build_ext_parameters(self, ext_parameters):
        return self.impl.build_ext_parameters(ext_parameters)