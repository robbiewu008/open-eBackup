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
from datetime import datetime
from typing import List
from uuid import UUID

from pydantic import BaseModel, Field, validator, root_validator

from app.backup.common.validators.sla_validator import manager, ParamsValidator
from app.backup.schemas.extend_params_manager import ExtendParamsManager
from app.backup.schemas.policy import Policy, PolicyQuery
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum, SlaType, PolicyActionEnum
from app.common.schemas.common_schemas import PageRequest


class SlaId(BaseModel):
    uuid: UUID = Field(description="SLA唯一编码")


class SlaBase(BaseModel):
    name: str = Field(description="SLA名称", min_length=1, max_length=64,
                      regex="^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    type: SlaType = Field(description="SLA类型，1-备份 2-容灾，当前只支持备份")
    application: ResourceSubTypeEnum = Field(
        description="应用类型，请见枚举定义")
    created_time: datetime = Field(None, description="SLA创建时间")

    class Config:
        orm_mode = True
        schema_extra = {
            "name": "test",
            "policy_list": [
                {
                    "name": "full",
                    "type": "backup",
                    "action": "full",
                    "ext_parameters": {
                        "deduplication": "false",
                        "compression": "false"
                    },
                    "retention": {
                        "retention_type": 1,
                        "retention_duration": 7,
                        "duration_unit": "d"
                    },
                    "schedule": {
                        "trigger": 2,
                        "interval": 12,
                        "interval_unit": "h",
                        "start_time": "2020-07-09T11:32:20",
                        "window_start": "12:00:00",
                        "window_end": "12:00:00"
                    }
                },
                {
                    "name": "incremental",
                    "type": "backup",
                    "action": "incremental",
                    "ext_parameters": {
                        "deduplication": "false",
                        "compression": "false"
                    },
                    "retention": {
                        "retention_type": 1,
                        "retention_duration": 14,
                        "duration_unit": "d"
                    },
                    "schedule": {
                        "trigger": 1,
                        "interval": 4,
                        "interval_unit": "h",
                        "start_time": "2020-07-09T11:32:17",
                        "window_start": "00:30:00",
                        "window_end": "00:30:00",
                    }
                },
                {
                    "name": "archiving",
                    "type": "archiving",
                    "ext_parameters": {
                        "storage_type": "tape",
                        "storage_name": "name1",
                        "storage_id": "1122323455",
                        "deduplication": "true",
                        "compression": "true",
                        "encryption": "true"
                    },
                    "retention": {
                        "retention_type": 1,
                        "retention_duration": 30,
                        "duration_unit": "d"
                    },
                    "schedule": {
                        "trigger": 2,
                        "interval": 4,
                        "interval_unit": "h"
                    }
                },
                {
                    "name": "replication",
                    "type": "replication",
                    "ext_parameters": {
                        "external_system_id": "10101010101",
                        "encryption": "true"
                    },
                    "retention": {
                        "retention_type": 1,
                        "retention_duration": 60,
                        "duration_unit": "d"
                    },
                    "schedule": {
                        "trigger": 2,
                        "interval": 4,
                        "interval_unit": "h"
                    }
                }
            ],
            "user_id": "itflsdsaiodoa"
        }


extends_manager = ExtendParamsManager()


class SlaCreate(SlaBase):
    user_id: str = Field(None, description="创建SLA的用户", min_length=1, max_length=64)
    policy_list: List[Policy] = Field(description="SLA对应的策略集合", min_items=1, max_items=16)

    @root_validator
    def check_policy(cls, sla_object):
        application_type = sla_object.get('application', None)
        policy_list = list(sla_object.get('policy_list', []))
        ParamsValidator.check_name_has_no_pre_and_tail_space(sla_object.get('name'), "sla_name")
        manager.do_validate(application_type, policy_list)
        return sla_object

    @validator('policy_list', pre=True, each_item=True)
    def init_policy_extend_params(cls, policy_object, values, **kwargs):
        policy_type = PolicyTypeEnum(policy_object.get("type"))
        application = values.get("application")
        ext_parameters = policy_object.pop('ext_parameters')
        action = policy_object.get("action", "")
        ext_schema = extends_manager.get_extend_param(
            application, policy_type, ext_parameters, action)
        return Policy(ext_parameters=ext_schema, **policy_object)

    @validator('policy_list', each_item=True)
    def check_policy_info(cls, policy_object):
        ParamsValidator.check_name_has_no_pre_and_tail_space(policy_object.name, "policy_name")
        return policy_object


class SlaUpdate(SlaCreate):
    uuid: UUID = Field(description="唯一编码")
    user_id: str = Field(None, description="创建SLA的用户", min_length=1, max_length=64)
    policy_list: List[Policy] = Field(description="SLA对应的策略集合", min_items=1, max_items=16)


class SlaQuery(SlaBase):
    uuid: UUID = Field(description="唯一编码")
    is_global: bool = Field(description="是否是预置策略，true-预置策略  false-非预置策略")
    policy_list: List[PolicyQuery] = Field(description="SLA对应的策略集合")
    resource_count: int = Field(None, description="SLA关联资源的数量")
    archival_count: int = Field(None, description="SLA关联的归档目标数量")
    replication_count: int = Field(None, description="SLA关联的复制目标数量")

    class Config:
        orm_mode = True


class SlaPageRequest(PageRequest):
    user_id: str = None
    name: str = None
    actions: List[PolicyActionEnum] = None
    types: List[SlaType] = None
    applications: List[ResourceSubTypeEnum] = None
