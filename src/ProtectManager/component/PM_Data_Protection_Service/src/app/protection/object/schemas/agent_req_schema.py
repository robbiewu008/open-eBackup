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


class Authentication(BaseModel):
    auth_type: Optional[int] = Field(2, description="鉴权类型", alias="authType")
    auth_key: Optional[str] = Field("", description="鉴权用户名称", alias="authKey")
    auth_pwd: Optional[str] = Field("", description="鉴权用户密码", alias="authPwd")
    extend_info: Optional[dict] = Field({}, description="鉴权扩展信息", alias="extendInfo")


class AppEnv(BaseModel):
    uuid: Optional[str] = Field("", description="环境UUID")
    name: Optional[str] = Field("", description="环境名称")
    type: Optional[str] = Field("", description="环境类型")
    sub_type: Optional[str] = Field("", description="环境子类型", alias="subType")
    endpoint: Optional[str] = Field("", description="环境访问地址")
    port: int = Field(description="环境访问地址端口")
    role: Optional[str] = Field("", description="节点角色类型，0-none，1-active，2-standby，3-shard")
    auth: Optional[Authentication] = Field(None, description="环境鉴权信息")
    extend_info: Optional[dict] = Field({}, description="环境扩展信息", alias="extendInfo")


class Application(BaseModel):
    uuid: Optional[str] = Field("", description="资源UUID")
    name: Optional[str] = Field("", description="资源名称")
    parent_uuid: Optional[str] = Field("", description="父资源UUID", alias="parentUuid")
    parent_name: Optional[str] = Field("", description="父资源名称", alias="parentName")
    type: Optional[str] = Field("", description="资源类型")
    sub_type: Optional[str] = Field("", description="资源子类型", alias="subType")
    auth: Optional[Authentication] = Field(None, description="资源鉴权信息")
    extend_info: Optional[dict] = Field({}, description="资源扩展信息", alias="extendInfo")


class CheckAppReq(BaseModel):
    app_env: AppEnv = Field(None, description="应用环境信息", alias="appEnv")
    application: Application = Field(None, description="应用信息")
