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
from datetime import datetime
from typing import List

from app.common.schemas.common_schemas import BaseOrmModel
from pydantic import BaseModel, Field, Json, validator


class NamedResource(BaseOrmModel):
    name: str = Field(description="资源名称")


class ResourceSummary(NamedResource):
    path: str = Field(None, description="资源位置")
    root_uuid: str = Field(None, description="环境ID")


class ResourceDetail(ResourceSummary):
    parent_name: str = Field(None, description="父资源名称")
    parent_uuid: str = Field(None, description="父资源ID")
    children_uuids: list = Field(None, description="子资源ID列表")


class TypedResourceDetail(ResourceDetail):
    type: str = Field(None, description="资源类型")
    sub_type: str = Field(None, description="资源子类型")


class ResourceSchema(TypedResourceDetail):
    uuid: str = Field(description="资源ID")
    created_time: datetime = Field(None, description="资源创建时间")
    ext_parameters: Json = Field(None, description="扩展属性")
    authorized_user: str = Field(None, description="被授权用户")
    user_id: str = Field(None, description="用户ID")
    version: str = Field(None, description="资源版本号")
    sla_id: str = Field(None, description="SLA ID")
    sla_name: str = Field(None, description="SLA名称")
    sla_status: bool = Field(None, description="保护激活状态")
    sla_compliance: bool = Field(None, description="SLA遵从度")
    protection_status: int = Field(None, description="保护状态")

    @validator('ext_parameters', pre=True)
    def json_decode(cls, v):
        if isinstance(v, dict):
            return json.dumps(v)
        return v

    @validator('ext_parameters', pre=True)
    def para_decode(cls, v):
        if isinstance(v, dict):
            return json.dumps(v)
        return v


class EnvironmentResourceSchema(ResourceSchema):
    environment_uuid: str = Field(None, description="环境ID")
    environment_name: str = Field(None, description="环境名称")
    environment_endpoint: str = Field(None, description="环境IP")
    environment_os_type: str = Field(None, description="环境系统类型")
    environment_type: str = Field(None, description="环境资源大类型")
    environment_sub_type: str = Field(None, description="环境资源子类型")
    environment_is_cluster: str = Field(None, description="集群环境标识")
    environment_os_name: str = Field(None, description="环境系统名称")
    labelList: list = Field(None, description="标签列表")


class ResourceProtectionCount(BaseModel):
    resource_sub_type: str = Field(None, description="资源子类型")
    resource_type: str = Field(None, description="资源类型")
    protected_count: int = Field(0, description="已保护资源数目")
    unprotected_count: int = Field(0, description="未保护资源数目")


class ResourceProtectionSummary(BaseModel):
    summary: List[ResourceProtectionCount] = Field([], description="子类型资源保护统计信息列表")
