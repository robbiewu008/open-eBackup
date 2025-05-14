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
from pydantic import Field

from app.resource.schemas.resource import EnvironmentResourceSchema
from pydantic import BaseModel


class RegisterDatabaseSchema(EnvironmentResourceSchema):
    uuid: str = Field(description="资源ID")
    link_status: int = Field(None, description="连接状态")
    instance_uuids: str = Field(description="所有实例ID")
    instance_names: str = Field(description="所有实例名称")
    os_username: str = Field(description="所属操作系统用户名", default=None)
    valid: bool = Field(None, description="是否展示")
    is_cluster: bool = Field(None, description="是否集群")
    verify_status: bool = Field(None, description="是否认证")
    name: str = Field(description="数据库名称")
    root_uuid: str = Field(description="根资源ID")
    parent_uuid: str = Field(description="父资源ID")
    type: str = Field(description="资源类型")
    sub_type: str = Field(description="资源子类型")
    db_username: str = Field(description="数据库用户名", default=None)
    db_password: str = Field(description="数据库密码", default=None)
    db_role: int = Field(description="数据库角色", default=None)
    auth_type: int = Field(description="认证方式", default=None)
    is_asminst: int = Field(description="是否使用ASM存储", default=None)
    version: str = Field(description="数据库版本", default=None)
    asm: str = Field(description="ASM信息", default=None)
    asm_auth: str = Field(description="ASM认证信息", default=None)
    sla_id: str = Field(None, description="SLA ID")
    sla_name: str = Field(None, description="SLA名称")
    sla_status: bool = Field(None, description="保护激活状态")
    sla_compliance: bool = Field(None, description="SLA遵从度")
    identification_status: str = Field(None, description="识别状态")
    desesitization_status: str = Field(None, description="脱敏状态")
    source_type: str = Field(None, description="资源的来源")
    desesitization_policy_id: str = Field(None, description="脱敏策略ID")
    desesitization_policy_name: str = Field(None, description="脱敏策略名称")


class DBRestoreCreateResourceSchema(BaseModel):
    host_id: str = Field(None, description="主机ID")
    instance_name: str = Field(None, description="实例名称")
    database_name: str = Field(None, description="数据库名称")
    sub_type: str = Field(None, description="资源类型")
