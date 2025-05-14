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

from app.resource.schemas.resource import EnvironmentResourceSchema, ResourceSchema


class DatabaseSchema(EnvironmentResourceSchema):
    link_status: str = Field(description="连接状态")
    verify_status: bool = Field(description="是否认证")
    os_username: str = Field(description="所属操作系统用户名", default=None)
    db_username: str = Field(description="数据库用户名", default=None)
    db_role: int = Field(description="数据库角色", default=None)
    auth_type: int = Field(description="认证类型", default=None)
    inst_name: str = Field(description="实例名称")
    is_asminst: int = Field(description="是否使用ASM存储", default=None)
    version: str = Field(description="数据库版本", default=None)
    asm: str = Field(description="ASM信息", default=None)
    asm_auth: str = Field(description="ASM认证信息", default=None)
    valid: bool = Field(description="是否展示")
    sla_id: str = Field(None, description="SLA ID")
    sla_name: str = Field(None, description="SLA名称")
    sla_status: bool = Field(None, description="保护激活状态")
    sla_compliance: bool = Field(None, description="SLA遵从度")
    identification_status: str = Field(None, description="识别状态")
    desesitization_status: str = Field(None, description="脱敏状态")
    source_type: str = Field(None, description="识别状态")
    desesitization_policy_id: str = Field(None, description="脱敏策略ID")
    desesitization_policy_name: str = Field(None, description="脱敏策略名称")


class InternalDatabaseSchema(DatabaseSchema):
    db_password: str = Field(description="db密码", default=None)


class AsmInfoSchema(ResourceSchema):
    username: str = Field(description="数据库用户名", default=None)
    password: str = Field(description="数据库密码", default=None)
    verify_status: bool = Field(description="是否认证", default=False)
    auth_type: int = Field(description="认证类型", default=None)
    asm: str = Field(description="ASM实例信息", default=None)
    asm_instances: str = Field(description="ASM实例名字")
