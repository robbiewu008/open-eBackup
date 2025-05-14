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
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.schemas.extends.params.huawei_cloud_stack_ext_param import CloudHostExtParam, ProjectExtParam
from app.protection.object.service.projected_object_service import build_protection_object_without_task
from app.protection.object.service.protection_plugin import ProtectionPlugin
from app.common import logger

log = logger.get_logger(__name__)

SERVICES = [ResourceSubTypeEnum.HCSProject, ResourceSubTypeEnum.HCSCloudHost]


class HuaweiCloudStackProtectionPlugin(ProtectionPlugin):
    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        res = ResourceClient.query_v2_resource(resource.get("uuid"))
        disk_info_obj = json.loads(res.get("extendInfo").get("host")).get("diskInfo")
        if not disk_info_obj:
            raise EmeiStorBizException(error=ResourceErrorCodes.CLOUD_HOST_DISK_INFO_IS_EMPTY,
                                       message='Failed to obtain the cloud host disk information.')
        return CloudHostExtParam(**{
            "agents": extend_parameter.agents,
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "disk_info": []
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object_without_task(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        return ResourceClient.query_resource_list(
            {"path": resource_info.path + os.sep, "type": ResourceTypeEnum.CloudHost})

    def query_sub_resources_by_obj(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "type": ResourceTypeEnum.CloudHost})

    def query_sub_resources_only_vm(self, obj):
        return ResourceClient.query_resource_list(
            {"path": obj.path + os.sep, "type": ResourceTypeEnum.CloudHost})

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return []

    def build_ext_parameters(self, ext_parameters):
        return ProjectExtParam(**json.loads(ext_parameters))


def create():
    return HuaweiCloudStackProtectionPlugin()
