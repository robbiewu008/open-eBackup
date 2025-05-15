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
import pathlib
import uuid
from enum import Enum
from typing import Union, List, Optional
from pydantic import BaseModel, IPvAnyAddress, constr

from app.common.event_messages.Common.enumerations import ProtectedObjectType, EnvType

MAX_MAX_OBJECT_ID_LENGTH = 128


class SchedulerContext(BaseModel):
    object_type: ProtectedObjectType
    env_type: Optional[EnvType]


class CommonProtectionFields(SchedulerContext):
    es_task_id: uuid.UUID
    es_user_id: str
    env_ip: Optional[IPvAnyAddress]


class VMProtectionContext(CommonProtectionFields):
    object_type: constr(regex=f"^{ProtectedObjectType.vm}$")
    env_type: constr(regex=f"^{EnvType.vmware}$")
    object_id: Optional[Union[uuid.UUID, constr(regex="^$")]]
    client_ip: Optional[Union[IPvAnyAddress, None]]
    paths: List[pathlib.Path]
    rbac_root_id: Optional[uuid.UUID]


class FilesetProtectionContext(CommonProtectionFields):
    object_type: constr(regex=f"^{ProtectedObjectType.fileset}$")
    env_type: Optional[Union[None, constr(regex=f"^$")]]
    object_id: Optional[Union[uuid.UUID, constr(regex="^$")]]
    include_paths: Optional[Union[str, List[str]]]
    exclude_paths: Optional[Union[str, List[str]]]
    paths: List[pathlib.Path]


class BackupConfig(BaseModel):
    channels: int
    pre_process_script: str
    post_process_script: str
    failure_process_script: str


class OracleProtectionContext(CommonProtectionFields):
    object_type: constr(regex=f"^{ProtectedObjectType.db}$")
    env_type: constr(regex=f"^{EnvType.oracle}$")
    object_id: Optional[Union[uuid.UUID, constr(regex="^$")]]
    databases: List[str]
    backup_config: BackupConfig


class ImmediateMode(str, Enum):
    full = "full"
    incremental = "incremental"


class ApplySlaData(BaseModel):
    sla_name: Optional[str]
    object_id: uuid.UUID
    object_type: ProtectedObjectType
    immediate_backup_mode: Optional[
        ImmediateMode] = None  # Ignored unless sla_name is None or empty, than it is considered for immediate backup
    context: Union[
        VMProtectionContext, FilesetProtectionContext, OracleProtectionContext,
    ] = None


# v1/vm/backup - Request input format
class VmSnapRequestIn(BaseModel):
    # Fields common to SchedulerCreateScheduleIn
    sla_name: str
    object_id: str
    object_type: str
    # SchedulerCreateScheduleIn params Union must include this type as well:
    context: VMProtectionContext


class ApplyResponse(BaseModel):
    status: str
    immediate_task_id: Optional[uuid.UUID] = None  # Optional, for immediate backup


class ApplyErrResponse(BaseModel):
    description: str


class RemoveSla(BaseModel):
    object_id: uuid.UUID
    rbac_root_id: Optional[uuid.UUID]


class ChainIdRequest(BaseModel):
    object_id: str
    sla: Optional[str] = None


class ChainIdResponse(BaseModel):
    chain_id: str
    status: str


class ChainIdErrResponse(BaseModel):
    status: str


class ProtectedObjectStatus(BaseModel):
    sla_id: str
    last_backup_status: str
    backup_success_ratio: str


class ProtectedObjectStatusResponse(BaseModel):
    status: str
    object_id: str
    data: ProtectedObjectStatus


class ProtectedObjectsStatus(BaseModel):
    status: str
    objects_status: List[ProtectedObjectStatusResponse]


class ComplianceStatus(str, Enum):
    compliant = "compliant"
    non_compliant = "non_compliant"
    not_applicable = "not_applicable"


class LinkStatus(str, Enum):
    online = "online"
    offline = "offline"


class ComplianceStatusResponse(BaseModel):
    sla_name: str
    compliance_status: ComplianceStatus


class SlaAssociatedResourceRequest(BaseModel):
    sla_name: str
    pageSize: int
    startIndex: int
    sort_params: str


class SlaAssociatedResourceData(BaseModel):
    name: str
    type: ProtectedObjectType
    location: str
    link_status: LinkStatus
    compliance_status: ComplianceStatus


class SlaAssociatedResourceResponse(BaseModel):
    total: int
    currentCount: int
    pageSize: int
    startIndex: int
    associated_resource: List[SlaAssociatedResourceData]
