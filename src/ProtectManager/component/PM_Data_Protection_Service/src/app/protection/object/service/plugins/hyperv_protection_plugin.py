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

from app.common import logger
from app.common.clients.resource_client import ResourceClient
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.schemas.extends.params.hyperv_ext_param import HyperVHostExtParam, HyperVVMExtParam
from app.protection.object.service.batch_protection_service import check_hyper_v_disk_info
from app.protection.object.service.projected_object_service import build_protection_object_without_task
from app.protection.object.service.protection_plugin import ProtectionPlugin

log = logger.get_logger(__name__)

SERVICES = [ResourceSubTypeEnum.HYPER_V_HOST, ResourceSubTypeEnum.HYPER_V_VM]


class HyperVProtectionPlugin(ProtectionPlugin):
    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        check_hyper_v_disk_info(resource)
        res = ResourceClient.query_v2_resource(resource.get("uuid"))
        disk_info_obj = json.loads(res.get("extendInfo").get("disks"))
        disk_info = []
        if len(disk_info_obj) > 0:
            for obj in disk_info_obj:
                disk_info.append(obj.get("uuid"))
        return HyperVVMExtParam(**{
            "agents": extend_parameter.agents,
            "disk_info": disk_info,
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "all_disk": "true"
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object_without_task(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        # HyperV集群只有主机和虚拟机能加保护，只需考虑查询主机下虚拟机即可。SCVMM注册后无法对集群添加保护，同样无需考虑查询集群下子资源的情况。
        return ResourceClient.query_resource_list(
            {"path": resource_info.path + os.sep, "sub_type": ResourceSubTypeEnum.HYPER_V_VM})

    def query_sub_resources_by_obj(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.HYPER_V_VM})

    def query_sub_resources_only_vm(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.HYPER_V_VM})

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return []

    def build_ext_parameters(self, ext_parameters):
        return HyperVHostExtParam(**json.loads(ext_parameters))


def create():
    return HyperVProtectionPlugin()