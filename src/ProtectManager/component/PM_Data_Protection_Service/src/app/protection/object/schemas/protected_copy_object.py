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
from typing import List, Optional
from uuid import UUID

from pydantic import BaseModel, Field


class ProtectedCopyObjectId(BaseModel):
    uuid: str = Field(None, description="保护副本对象的ID")


class ProtectedCopyBaseExtParam(BaseModel):
    enable_security_archive: Optional[bool] = Field(False, description="安全归档")


class ProtectedCopyObjectUpdate(BaseModel):
    sla_id: UUID = Field(description="新SLA的ID")
    resource_id: str = Field(description="保护对象对应资源的ID", min_length=1, max_length=128)
    ext_parameters: ProtectedCopyBaseExtParam = Field(None, description="扩展属性")


class ProtectedCopyBatchOperationReq(BaseModel):
    resource_ids: List[str] = Field(..., description="批量操作资源ID列表")


class ManualReplicationReq(BaseModel):
    sla_id: str = Field(description="sla的Id", max_length=64, min_length=1)
    resource_id: str = Field(description="资源id", max_length=64, min_length=1)
    policy_id: str = Field(description="选择的sla策略id", max_length=64, min_length=1)
    copy_protected_obj: dict = Field(None, description="副本保护对象")
    copy_format_list: list = Field(None, description="副本格式列表")
    payload: dict = Field(None, description="payload")
    resource_obj: dict = Field(None, description="资源对象")
    request_id: str = Field(None, description="请求id")


class CopyProtectedObjectQuery(BaseModel):
    protected_resource_id: str = Field(description="资源id")
    protected_object_uuid: str = Field(description="副本保护对象id")
    protected_sla_id: str = Field(description="sla的ID")
    protected_sla_name: str = Field(None, description="sla名称")
    protected_status: bool = Field(None, description="保护状态")
    type: str = Field(None, description="资源类型：如Host、Fileset、Database、VM等")
    sub_type: str = Field(None, description="资源子类型：如DBBackupAgent、Fileset、Oracle、vim.VirtualMachine等")
    chain_id: str = Field(None, description="副本链id")

    class Config:
        orm_mode = True


class SlaResourceQuantityRelationship(BaseModel):
    sla_id: str = Field(None, description="SLA的ID")
    resource_count: int = Field(None, description="关联副本数")
