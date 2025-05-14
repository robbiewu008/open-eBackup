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
import uuid

from typing import List, Optional
from pydantic import BaseModel, Field, IPvAnyAddress, constr

from app.common.event_messages.Common.consts import MAX_NAME_LEN
from .enumerations import ProtectedObjectType

MAX_NET_NAME_LEN = 32


class DiskInfo(BaseModel):
    device_id: str = Field(..., description="Device Identifier, e.g. IDE(0,1) or SCSI(0,0)")
    target_size: int = Field(..., ge=0, description="Target size for disk, in MB")


class NetInfo(BaseModel):
    name: str = Field(..., max_length=MAX_NET_NAME_LEN)
    net: str


class VMInfo(BaseModel):
    disk_info: List[DiskInfo]
    cores: int = Field(..., ge=1, description="# cores assigned to new VM")
    slots: int = Field(..., ge=1, description="# virtual sockets assigned to new VM")
    net_info: List[NetInfo]
    memory: str


class DestinationInfo(BaseModel):
    dest_path: str
    platform_ip: IPvAnyAddress
    vm_info: VMInfo


class MountInfo(BaseModel):
    restore_use_suffix: str = Field(default="True")
    restore_suffix: str
    restore_autostart: bool = False
    host_name: str
    restore_location: str


class PerformanceTargetInfo(BaseModel):
    slot: int = Field(..., ge=1, description="Number of disks")
    max_bw: int = Field(..., ge=1, description="Number of max iops in MBps")
    max_iops: int = Field(..., ge=1, description="")
    max_latency: int = Field(..., ge=1, description="Number of max latency usec")


class LivemountVMInfo(BaseModel):
    dest_info: DestinationInfo
    mount_info: MountInfo
    target_performance_list: List[PerformanceTargetInfo]


class AddLivemountVMData(BaseModel):
    user_id: str = Field(..., max_length=MAX_NAME_LEN)
    snap_id: str = Field(..., max_length=MAX_NAME_LEN)
    livemount_vm_info: LivemountVMInfo
    object_type: constr(regex=f"^{ProtectedObjectType.vm}$")
    rbac_root_id: Optional[uuid.UUID]


class RemoveLivemountVMData(BaseModel):
    mount_id: str = Field(..., max_length=MAX_NAME_LEN)
    user_id: str = Field(..., max_length=MAX_NAME_LEN)
    unmount: bool = Field(
        default=False,
        description="If true - do unmount, else remove livemount data (default value is false)"
    )
    object_type: constr(regex=f"^{ProtectedObjectType.vm}$")
    remove_mount: bool = True
    rbac_root_id: Optional[uuid.UUID]
