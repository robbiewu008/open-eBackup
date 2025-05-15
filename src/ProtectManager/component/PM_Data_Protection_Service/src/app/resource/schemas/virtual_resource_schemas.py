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

from pydantic import Field, BaseModel

from app.resource.schemas.resource import ResourceSchema, EnvironmentResourceSchema


class HostNasVolumn(BaseModel):
    name: str = Field(None, description="nas卷名称")
    type: str = Field(None, description="nas卷类型")
    remote_host: str = Field(None, description="nas卷remoteHost")
    remote_host_names: List[str] = Field(None, description="nas卷remoteHostNames")
    remote_path: str = Field(None, description="nas卷remotePath")


class DatastoreSchema(BaseModel):
    mo_id: str = Field(None, description="数据存储的moId")
    uuid: str = Field(None, description="数据存储的UUID")
    url: str = Field(None, description="数据存储的URL")
    name: str = Field(None, description="数据存储的名称")
    type: str = Field(None, description="数据存储的类型")
    partitions: List[str] = Field(None, description="数据存储的磁盘分区列表")


class VirtualResourceSchema(EnvironmentResourceSchema):
    # Only for an ESX/ESXi that is managed by a VCenter:
    vm_ip: str = Field(None, description="虚拟机的IP地址")
    env_ip: str = Field(None, description="对应的保护环境IP")
    link_status: int = Field(None, description="资源状态，0-在线 1-离线")
    mo_id: str = Field(None, description="虚拟机的moId")
    instance_id: str = Field(None, description="虚拟机的生产环境实例id")
    alias_type: str = Field(None, description="类型别名")
    alias_value: str = Field(None, description="值别名")
    firmware: str = Field(None, description="虚拟机引导选项中的固件信息")

    # Only for a DataStore:
    capacity: int = Field(0, description="容量，仅用于数据存储")
    free_space: int = Field(0, description="空闲容量，仅用于数据存储")
    uncommitted: int = Field(0, description="未提交容量，仅用于数据存储")
    partitions: List[str] = Field(None, description="数据存储的磁盘分区列表")

    children: int = Field(None, description="子资源的数量")
    is_template: bool = Field(False, description="是否未模板虚拟机")
    sla_id: str = Field(None, description="SLA ID")
    sla_name: str = Field(None, description="SLA名称")
    sla_status: bool = Field(None, description="保护激活状态")
    sla_compliance: bool = Field(None, description="SLA遵从度")
    os_type: str = Field(None, description="操作系统类型")
    tags: str = Field(None, description="虚拟机标记")
    in_group: str = Field(None, description="是否加入虚拟机组")
    resource_group_id: str = Field(None, description="资源组id")
    resource_group_name: str = Field(None, description="资源组名称")


class VirtualDiskDetailSchema(ResourceSchema):
    uuid: str = Field(None, description="硬盘ID")
    slot: str = Field(description="槽位号")
    # Only for an ESX/ESXi that is managed by a VCenter:
    capacity: int = Field(0, description="硬盘容量，单位KB")
    datastore: DatastoreSchema = Field(None, description="数据存储信息")
    disk_type: str = Field(None, description="数据存储的硬盘类型")
    disk_lun: str = Field(None, description="数据存储的硬盘LUN")
    is_nas: bool = Field(False, description="是否为nas类型")
    nas_info: HostNasVolumn = Field(None, description="nas卷信息")


class InitiatorsSchema(BaseModel):
    type: int = Field(None, description="启动器的类型")
    name: str = Field(None, description="启动器的名称")


class RdmProductInitiatorDetailSchema(BaseModel):
    esxi_name: str = Field(None, description="Esxi的名称")
    esxi_moref: str = Field(None, description="硬盘数据存储的MoRef(Mo_id)")
    initiators: InitiatorsSchema = Field(description="数据存储Initiator信息")


class FolderSchema(BaseModel):
    mo_id: str = Field(None, description="虚拟文件夹的moId")
    name: str = Field(None, description="虚拟文件夹的名称")


class HardwareSchema(BaseModel):
    num_cpu: int = Field(description="虚拟机CPU数量")
    num_cores_per_socket: int = Field(description="每个Socket的CPU数量")
    memory: int = Field(description="虚拟机内存大小，单位MB")
    controller: List = Field([], description="虚拟机控制器种类")


class HostSystem(BaseModel):
    mo_id: str = Field(None, description="ESX主机的moId")
    name: str = Field(None, description="ESX主机的名称")
    uuid: str = Field(None, description="ESX主机的UUID")
    version: str = Field(None, description="版本")


class VirtualMachineRuntime(BaseModel):
    host: HostSystem = Field(None, description="ESX主机")


class VMwareSchema(BaseModel):
    uuid: str = Field(description="虚拟机ID")
    hardware: HardwareSchema = Field(description="虚拟机的硬件信息")
    runtime: VirtualMachineRuntime = Field(None, description="虚拟机运行环境")
    vmx_datastore: DatastoreSchema = Field(description="虚拟机的数据存储信息")
    firmware: str = Field(None, description="虚拟机引导选项中的固件信息")
    vm_parent_location: str = Field(None, description="mob中虚拟机的parent字段的的mo_id信息")


class ClusterConfigSchema(BaseModel):
    drs_enabled: bool = Field(None, description="集群是否开启DRS服务")
