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
from enum import Enum
from typing import List, Optional

from pydantic import BaseModel, Field


# Types of entities that env_discovery knows how to 'discover'.
class EntityType(str, Enum):
    # In search used as a "don't care" type
    NotDefined = "undefined"
    # Used to schedule rescans
    RescanSchedule = "schedule"
    # Used to prevent two instances scanning the same env at the same time
    RescanToken = "rescan_token"
    # In search this means ALL root objects (vcenter, esx, esxi, server)
    VMWareVCenter = "vmware_vcenter"
    VMWareDataCenter = "vmware_datacenter"
    VMWareCluster = "vmware_cluster"
    VMWareFolder = "vmware_folder"
    VMWareResourcePool = "vmware_resource_pool"
    VMWareVirtualApp = "vmware_v_app"
    # In search this includes esxi as well
    VMWareEsx = "vmware_esx"
    # In search this includes esx as well
    VMWareEsxi = "vmware_esxi"
    VMWareServer = "vmware_server"
    # In search this means vm where is_template == false
    VMWareVm = "vmware_vm"
    # In search this means vm where is_template == true
    VMWareVmTemplate = "vmware_vm_template"
    VMWareDataStore = "vmware_datastore"
    VMWareNetwork = "vmware_network"
    # In search this means all objects where the root_folder == 'host'
    VMWareHost = "vmware_host"
    # Not used at this time
    FusionCompute = "fusion_compute"


class EnvironmentType(Enum):
    EBackupHost = 0
    AnyBackupHost = 1
    Database = 2
    Application = 3
    VM = 4
    BigData = 5
    PrivateCloud = 6

    # Note: This is only used to get the env_type when adding, modifying
    # or deleting a protected env via the eisoo_adapter.
    def to_adapter_type(self):
        switcher = {
            EntityType.VMWareServer: 'VMWARE',
            EntityType.VMWareEsx: 'VMWARE',
            EntityType.VMWareEsxi: 'VMWARE',
            EntityType.VMWareVCenter: 'VMWARE',
            EntityType.FusionCompute: 'FUSIONCOMPUTE',
        }
        return switcher.get(self, -1)

    def from_product_line_id(pl_id):
        if pl_id == 'gsx':
            return EntityType.VMWareServer
        if pl_id == 'esx':
            return EntityType.VMWareEsx
        if pl_id == 'embeddedEsx':
            return EntityType.VMWareEsxi
        if pl_id == 'vpx':
            return EntityType.VMWareVCenter
        return EntityType.NotDefined


class ResourceType(Enum):
    Host = 'host'
    Fileset = 'Fileset'
    Oracle = 'oracle'
    Asm = 'asm'
    SqlServer = 'sqlserver'
    Gaussdb = 'gaussdb'
    Saphana = 'saphana'
    Gbase = 'gbase'
    DaMeng = 'dameng'
    HyperV = 'hyper-v'
    Fusionsphere = 'fusionsphere'
    Exchange = 'exchange'


# 鉴权类型 0-OS鉴权未开启 1-OS鉴权开启
class AuthType(Enum):
    OS_OFF = 0
    OS_ON = 1


# 资源状态 0-在线 1-离线
class ResourceStatus(Enum):
    ON_LINE = 0
    OFF_LINE = 1


# 主机状态 0-离线 1-在线 2-迁移中
class HostOnlineStatus(Enum):
    ON_LINE = 1
    OFF_LINE = 0
    MIGRATE = 2


# The parameters for starting the scan of a new environment.
class EnvParams(BaseModel):
    name: Optional[str] = ...
    env_type: Optional[EnvironmentType] = EnvironmentType.AnyBackupHost
    user_name: Optional[str] = ...
    password: Optional[str] = ...
    ip: Optional[str] = ...
    port: Optional[str] = ...
    rescan_interval_in_sec: int = 5
    extend_context: Optional[dict] = None


# Same as EnvParams with additional object type required by the scheduler
class SchedulerEnvParams:
    env_name: str
    env_type: EntityType
    user_name: str
    password: str
    ip: str
    ports: str
    rescan_interval_in_sec: int
    object_type: str

    def __init__(self, env_params: EnvParams):
        self.env_name = env_params.name
        self.env_type = env_params.env_type
        self.user_name = env_params.user_name
        self.password = env_params.password
        self.ip = env_params.ip
        self.ports = env_params.port
        self.rescan_interval_in_sec = env_params.rescan_interval_in_sec
        self.object_type = 'vm'


# The parameters for deleting an environment.
class DeleteEnvParams(BaseModel):
    env_name: str
    env_type: EnvironmentType
    ip: str
    ports: str


# The parameters for scanning an environment or part of it.
class ScanParams(BaseModel):
    env_name: str = ""
    env_type: EntityType = EntityType.VMWareVCenter
    user_name: str
    password: str
    ip: str
    ports: str = ""
    scan_time: int = 0


# The parameters collected for objects in scanned environments and saved to ES for later searches.
class MachineParameters(BaseModel):
    name: str = ""
    path: str = "/"
    entity_type: EntityType = EntityType.VMWareVm
    parent_name: str = ""
    parent_uuid: str = ""
    root_folder: str = ""
    env_ip: str = ""
    env_name: str = ""
    scan_time: int = 500
    root_uuid: str = ""
    uuid: str = ""
    # Only for an ESX/ESXi that is managed by a VCenter:
    vcenter_ip: str = ""
    # Only for a DataStore:
    capacity: int = 0
    free_space: int = 0
    uncommitted: int = 0
    # Only for a VM - the first network in use (for a Network, matches the name):
    network_name: str = ""
    # Only for a VM
    is_template: bool = False


# A sub set of MachineParameters saved to the ES for use by global search (names required by global search).
class MachineParametersGlobalSearch(BaseModel):
    resname: str = ""
    resparname: str = ""
    resparid: str = ""
    resid: str = ""
    restype: str = ""
    reshostip: str = ""
    reshostname: str = ""


class SearchResponse(BaseModel):
    results: List[MachineParameters] = None


# The parameters for building an ES query.
class SearchParameters(BaseModel):
    name: str = Field(
        default="", description="Search by name (closest matches).",
    )
    path: str = Field(
        default="", description="Search by path (closest matches).",
    )
    entity_type: EntityType = Field(
        default=EntityType.VMWareVm,
        description="Search by type (exact match). Note: vcenter finds all root objects, esx/esxi finds all esx*.",
    )
    root_type: EnvironmentType = Field(
        default=EnvironmentType.AnyBackupHost,
        description="Search by root_folder type (exact match).",
    )
    parent_name: str = Field(
        default="",
        description="Search by parent name (exact match). Note: Multiple objects can have the same name!",
    )
    parent_uuid: str = Field(
        default="", description="Search by parent UUID (exact match).",
    )
    env: str = Field(
        default="",
        description="Search by IP or name of the registered protectable env.",
    )
    uuid: str = Field(default="", description="Search by UUID (exact match).")
    startIndex: int = Field(
        default=0,
        ge=0,
        description="Return entries from the entire list starting at this index.",
    )
    pageSize: int = Field(
        default=100, ge=1, description="Return this many items or less in the result."
    )
