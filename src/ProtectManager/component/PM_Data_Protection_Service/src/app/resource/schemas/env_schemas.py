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
from typing import Optional

from pydantic import BaseModel, Field

from app.common.enums.resource_enum import ResourceTypeEnum, ResourceSubTypeEnum
from app.common.schemas.common_schemas import PageRequest
# The parameters for starting the scan of a new environment.
from app.resource.schemas.resource import EnvironmentResourceSchema


class DeleteEnvSchema(BaseModel):
    env_id: str
    type: Optional[ResourceTypeEnum] = Field(None, description="资源类型")
    sub_type: Optional[ResourceSubTypeEnum] = Field(None, description="资源子类型")


class CreateEnvSchema(BaseModel):
    uuid: str = Field(None, description="环境ID", min_length=1, max_length=64)
    name: Optional[str] = Field(
        None, description="受保护环境名称", min_length=1, max_length=64)
    type: Optional[ResourceTypeEnum] = Field(None, description="资源类型")
    user_name: Optional[str] = Field(None, description="受保护环境用户名")
    password: Optional[str] = Field(None, description="受保护环境密码")
    endpoint: str = Field(None, description="受保护环境地址")
    sub_type: Optional[ResourceSubTypeEnum] = Field(None, description="资源子类型")
    port: int = Field(None, description="受保护环境端口号", ge=0, le=65535)
    rescan_interval_in_sec: Optional[int] = Field(
        default=5, description="资源扫描间隔，单位秒")
    verify_cert: Optional[int] = Field(None, description="是否校验证书，0：不校验，1：校验")
    extend_context: Optional[dict] = Field(dict(), description="资源拓展内容")


class ScanEnvSchema(CreateEnvSchema):
    user_id: str = Field(None, description="用户ID",
                         min_length=1, max_length=255)
    domain_id: str = Field(None, description="域id列表")
    job_id: str = Field(None, description="环境注册的请求id/任务id", min_length=1, max_length=64)


class EnvironmentSchema(EnvironmentResourceSchema):
    uuid: str = Field(None, description="环境ID")
    user_name: Optional[str] = Field(None, description="受保护环境用户名")
    endpoint: str = Field(..., description="受保护环境地址")
    port: int = Field(None, description="受保护环境端口号")
    os_type: Optional[str] = Field(..., description="操作系统类型")
    os_name: Optional[str] = Field(None, description="操作系统名称")
    is_cluster: bool = Field(description="是否为集群")
    scan_interval: int = Field(None, description="扫描间隔")
    sla_id: str = Field(None, description="SLA ID")
    sla_name: str = Field(None, description="SLA名称")
    sla_status: bool = Field(None, description="保护激活状态")
    sla_compliance: bool = Field(None, description="SLA遵从度")
    link_status: int = Field(None, description="连接状态")
    cert_name: str = Field(None, description="证书路径")
    agent_version: str = Field(None, description="Agent版本号")
    agent_timestamp: str = Field(None, description="Agent时间戳")


class InternalEnvironmentSchema(EnvironmentSchema):
    password: Optional[str] = Field(None, description="受保护环境密码")


class UpdateEnvSchema(BaseModel):
    uuid: str = Field(None, description="环境ID", min_length=1, max_length=64)
    name: Optional[str] = Field(
        None, description="受保护环境名称", min_length=1, max_length=64)
    user_name: Optional[str] = Field(None, description="受保护环境用户名")
    password: Optional[str] = Field(None, description="受保护环境密码")
    endpoint: str = Field(..., description="受保护环境IP地址")
    port: int = Field(..., description="受保护环境端口号", ge=0, le=65535)
    verify_cert: Optional[int] = Field(None, description="是否校验证书，0：不校验，1：校验")
    extend_context: Optional[dict] = Field(dict(), description="资源拓展内容")
    rescan_interval_in_sec: Optional[int] = Field(description="资源扫描间隔，单位秒")


class UpdateClientQueryRequest(PageRequest):
    version: str = Field(None, description="agent版本号")


class StorageInfo(BaseModel):
    ip: str = Field(..., description="存储IP地址")
    port: int = Field(..., description="存储端口号", ge=0, le=65535)
    username: Optional[str] = Field(None, description="存储用户名")
    password: Optional[str] = Field(None, description="存储密码")
    type: int = Field(..., description="存储设备类型")


class StorageIp(BaseModel):
    ip: Optional[str] = Field(..., description="存储IP地址")
