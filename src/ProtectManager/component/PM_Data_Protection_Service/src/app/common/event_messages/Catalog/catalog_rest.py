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
from enum import Enum

from pydantic import BaseModel, Field

from app.common.event_messages.Common import livemount_vm
from app.common.event_messages.Common.livemount_vm import LivemountVMInfo
from app.common.event_messages.Common.consts import MAX_NAME_LEN


class LivemountTaskStatus(str, Enum):
    success = 'success'
    fail = 'fail'


class CatalogSetLivemountInfoIn(BaseModel):
    livemount_vm_info: LivemountVMInfo
    mount_id: str = Field(..., max_length=MAX_NAME_LEN)
    snap_id: str = Field(..., max_length=MAX_NAME_LEN)
    mount_path: str
    lu_info: str
    lock_id: str


class CatalogLivemountTaskResponse(BaseModel):
    status: LivemountTaskStatus


class CatalogGetLivemountInfoIn(BaseModel):
    mount_id: str = Field(..., max_length=MAX_NAME_LEN)


class CatalogGetLivemountInfoOut(BaseModel):
    livemount_vm_info: livemount_vm.LivemountVMInfo
    snap_id: str = Field(..., max_length=MAX_NAME_LEN)
    mount_path: str
    lu_info: str
    lock_id: str
    policy_obj: dict


class CatalogGetLivemountVMInfo(BaseModel):
    livemount_vm_info: LivemountVMInfo
    mount_id: str = Field(..., max_length=MAX_NAME_LEN)
    snap_id: str = Field(..., max_length=MAX_NAME_LEN)
    mount_path: str
    policy_obj: dict
    # New Fields:
    target_name: str = None
    sla_compliance_percent: int = Field(default=None, ge=1)
    sla_compliance: str = None
    livemount_status: str = None
    updating_policy: str = None


class LivemountVMListOut(BaseModel):
    livemount_vm_list: List[CatalogGetLivemountVMInfo]
    total: int = Field(
        ...,
        ge=0,
        description="Indicates the total number of results that currently exist",
    )
    currentCount: int = Field(
        default=None, ge=0, description="Indicates the number of returned results",
    )
    startIndex: int = Field(
        default=None,
        ge=0,
        description="Indicates the position of the returned data in the complete list",
    )
    isLastPage: bool = Field(
        ..., description="Indicates whether this response includes the last result"
    )


class LivemountVMListIn(BaseModel):
    startIndex: int = Field(
        ..., description="Return entries from the entire list starting at this index"
    )
    pageSize: int = Field(
        ..., ge=1, description="Return this many items or less in the result"
    )
    should_filter_by_rbac: bool
    root_id_set: List[uuid.UUID] = None


class GetSnapInfoSync(BaseModel):
    chain_id: str
    timestamp: str
    es_user_id: str
    policy_obj: dict
    protected_obj: dict


class GetResourceId(BaseModel):
    resource_id: str


class CatalogSnapListRequestIn(BaseModel):
    pattern: Optional[str]  # depracated
    pageSize: int
    startIndex: int
    # New fields
    should_filter_by_rbac: bool
    root_id_set: List[uuid.UUID] = None
    is_admin: bool = None
    rbac_enabled: bool = None


class CatalogSnapListItem(BaseModel):
    snap_id: uuid.UUID
    timestamp: str
    metadata: str


class CatalogSnapListResponse(BaseModel):
    snap_list: List[CatalogSnapListItem]
    is_last_page: bool
    total: int = Field(
        ...,
        ge=0,
        description="Indicates the total number of results that currently exist",
    )
    currentCount: int = Field(
        default=None, ge=0, description="Indicates the number of returned results",
    )
    startIndex: int = Field(
        default=None,
        ge=0,
        description="Indicates the position of the returned data in the complete list",
    )
