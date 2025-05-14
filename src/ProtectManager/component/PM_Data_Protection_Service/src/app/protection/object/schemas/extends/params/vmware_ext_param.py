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
from typing import Optional, List

from pydantic import Field
from pydantic.main import BaseModel

from app.protection.object.common.protection_enums import DiskType, SlaApplyType, ResourceFilter, VmwareAgentHostInfo
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.common.enums.resource_enum import ResourceSubTypeEnum


class DiskInfo(BaseModel):
    type: DiskType = Field(..., description="磁盘类型")
    slot: str = Field(..., description="磁盘槽位号")
    name: str = Field(..., description="磁盘名称")
    uuid: str = Field(..., description="磁盘id")
    datastore_mo_id: str = Field(..., description="磁盘所在datastore的moid")
    datastore_name: str = Field(..., description="磁盘所在datastore的名称")
    datastore_wwn: List[str] = Field([], description="华为存储所对应LUN的WWN")


class ProtectResource(BaseModel):
    resource_id: str = Field(..., description="资源ID", max_length=64)
    filters: Optional[List[ResourceFilter]] = Field(description="过滤规则列表")


class VmExtParam(BaseExtParam):
    """
    虚拟机保护高级参数
    """

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.VirtualMachine]

    pre_script: Optional[str] = Field(description="前置执行脚本")
    post_script: Optional[str] = Field(description="后置执行脚本")
    all_disk: Optional[bool] = Field(description="是否保护全部磁盘")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
    disk_info: Optional[List[str]] = Field(description="保护VM的磁盘id信息")
    concurrent_requests: str = Field(None, description="并发数")
    concurrent_requests_uuid: str = Field(None, description="并发数uuid")
    host_list: Optional[List[VmwareAgentHostInfo]] = Field(description="VMware备份恢复任务指定的代理主机的信息列表")


class VirtualResourceExtParam(BaseExtParam):
    """
    虚拟化容器（主机集群）保护高级参数
    """

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.HostSystem, ResourceSubTypeEnum.ClusterComputeResource]

    pre_script: str = Field(None, description="前置执行脚本")
    post_script: str = Field(None, description="后置执行脚本")
    overwrite: bool = Field(description="主机/Cluster下虚拟机已经绑定SLA时，SLA的覆盖策略")
    binding_policy: List[SlaApplyType] = Field(description="主机/Cluster下虚拟机未绑定SLA时，SLA应用策略")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
    resource_filters: Optional[List[ResourceFilter]] = Field(description="批量保护过滤规则列表")
    resource_tag_filters: Optional[List[ResourceFilter]] = Field(description="批量保护时根据标签过滤规则列表")
    disk_filters: Optional[List[ResourceFilter]] = Field(None, description="磁盘过滤规则列表")
    concurrent_requests: str = Field(None, description="并发数")
    concurrent_requests_uuid: str = Field(None, description="并发数uuid")
    host_list: Optional[List[VmwareAgentHostInfo]] = Field(description="VMware备份恢复任务指定的代理主机的信息列表")
