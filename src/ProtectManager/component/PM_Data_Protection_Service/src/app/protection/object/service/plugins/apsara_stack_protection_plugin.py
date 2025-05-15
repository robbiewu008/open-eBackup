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

from app.base.db_base import database
from app.common import logger
from app.protection.object.service.protection_plugin import ProtectionPlugin
from app.common.clients.resource_client import ResourceClient
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.common.protection_enums import ResourceFilter
from app.protection.object.schemas.extends.params.apsara_stack_ext_param import ApsaraStackExtParam
from app.protection.object.service.projected_object_service import build_protection_object_without_task
from app.resource.models.resource_models import ResourceTable, ResExtendInfoTable

log = logger.get_logger(__name__)

SERVICES = [
    ResourceSubTypeEnum.APSARA_STACK, ResourceSubTypeEnum.APSARA_STACK_ZONE,
    ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET, ResourceSubTypeEnum.APSARA_STACK_INSTANCE,
    ResourceSubTypeEnum.APSARA_STACK_DISK
]


def query_sub_resource_list_in_resource_set(root_uuid, resource_set_name):
    with database.session() as session:
        members = session.query(ResourceTable) \
            .join(ResExtendInfoTable, ResExtendInfoTable.resource_id == ResourceTable.uuid) \
            .filter(ResourceTable.root_uuid == root_uuid) \
            .filter(ResourceTable.sub_type == ResourceSubTypeEnum.APSARA_STACK_INSTANCE) \
            .filter(ResExtendInfoTable.key == 'resourceSetName') \
            .filter(ResExtendInfoTable.value == resource_set_name).all()
    members_list = []
    for member in members:
        member_dict = {column.key: getattr(member, column.key) for column in member.__table__.columns}
        members_list.append(member_dict)
    return members_list


class ApsaraStackProtectionPlugin(ProtectionPlugin):
    def do_convert_extend_parameter(self, filter_list: List[ResourceFilter], resource, extend_parameter):
        return ApsaraStackExtParam(**{
            "agents": extend_parameter.agents,
            "disk_info": extend_parameter.disk_info,
            "all_disk": extend_parameter.all_disk,
            "disk_filters": extend_parameter.disk_filters,
            "archive_res_auto_index": extend_parameter.archive_res_auto_index,
            "backup_res_auto_index": extend_parameter.backup_res_auto_index,
            "open_consistent_snapshots": extend_parameter.open_consistent_snapshots
        })

    def build_protection_object(self, resource_info, sla, ext_params):
        return build_protection_object_without_task(resource_info, sla, ext_params)

    def query_sub_resources(self, resource_info):
        # 阿里云可用区的子资源只有云服务器，同时资源集的子资源也只有云服务器。资源集不是可用区的子资源，因此查询条件只有云服务器
        if resource_info.sub_type == ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET:
            return query_sub_resource_list_in_resource_set(resource_info.root_uuid, resource_info.name)
        return ResourceClient.query_resource_list(
                {"path": resource_info.path + os.sep, "sub_type": ResourceSubTypeEnum.APSARA_STACK_INSTANCE})

    def query_sub_resources_by_obj(self, obj):
        if obj.sub_type == ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET:
            return query_sub_resource_list_in_resource_set(obj.env_id, obj.name)
        return ResourceClient.query_resource_list(
                {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.APSARA_STACK_INSTANCE})

    def query_sub_resources_only_vm(self, obj):
        if obj.sub_type == ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET:
            return query_sub_resource_list_in_resource_set(obj.env_id, obj.name)
        return ResourceClient.query_resource_list(
                {"path": obj.path + os.sep, "sub_type": ResourceSubTypeEnum.APSARA_STACK_INSTANCE})

    def build_task_list(self, sla, resource_id, projected_object, execute_req):
        return []

    def build_ext_parameters(self, ext_parameters):
        return ApsaraStackExtParam(**json.loads(ext_parameters))


def create():
    return ApsaraStackProtectionPlugin()