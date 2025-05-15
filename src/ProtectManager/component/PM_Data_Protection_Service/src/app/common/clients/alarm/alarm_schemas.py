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
from typing import List

from pydantic import BaseModel, Field


class SendAlarmReq(BaseModel):
    sequence: int = Field(None, description="序列号", alias="sequence")
    user_id: str = Field(None, description="用户id", alias="userId")
    alarm_source: str = Field(None, description="告警源", alias="alarmSource")
    source_type: str = Field(None, description="源类型", alias="sourceType")
    alarm_id: str = Field(None, description="告警id", alias="alarmId")
    name: str = Field(None, description="名称", alias="name")
    desc: str = Field(None, description="描述", alias="desc")
    params: str = Field(None, description="告警参数", alias="params")
    alarm_type: str = Field(None, description="告警类型", alias="alarmType")
    advice: str = Field(None, description="告警建议", alias="advice")
    effect: str = Field(None, description="告警影响", alias="effect")
    location: str = Field(None, description="告警定位", alias="location")
    severity: int = Field(None, description="告警等级", alias="severity")
    create_time: int = Field(None, description="告警时间", alias="createTime")
    type: str = Field(None, description="告警类型", alias="type")
    resource_id: str = Field(None, description="资源id", alias="resourceId")


class ClearAlarmReq(BaseModel):
    entity_id_set: List[str] = Field(description="告警id", alias="entityIdSet")
