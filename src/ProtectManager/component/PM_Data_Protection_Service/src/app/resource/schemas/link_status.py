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
from pydantic import BaseModel, Field


class LinkStateId(BaseModel):
    uuid: str = Field(description="链路状态唯一id", min_length=1, max_length=64)


class LinkState(BaseModel):
    source_role: str = Field(None, alias="sourceRole", description="链路源角色")
    source_addr: str = Field(None, alias="sourceAddr", description="链路源地址")
    dest_role: str = Field(None, alias="destRole", description="链路目的角色")
    dest_addr: str = Field(None, alias="destAddr", description="链路目的地址")
    state: int = Field(None, description="链路状态")
    update_time: int = Field(None, alias="updateTime", description="链路状态更新时间")

    class Config:
        orm_mode = True


class LinkStateUpdate(BaseModel):
    source_role: str = Field(alias="sourceRole", description="链路源角色", min_length=1, max_length=64)
    source_addr: str = Field(alias="sourceAddr", description="链路源地址", min_length=1, max_length=64)
    dest_role: str = Field(alias="destRole", description="链路目的角色", min_length=1, max_length=64)
    dest_addr: str = Field(alias="destAddr", description="链路目的地址", min_length=1, max_length=64)
    state: int = Field(description="链路状态", ge=0, le=1)
    update_time: int = Field(alias="updateTime", description="链路状态更新时间")


class LinkStateCreate(BaseModel):
    source_role: str = Field(None, description="链路源角色")
    source_addr: str = Field(None, description="链路源地址")
    dest_role: str = Field(None, description="链路目的角色")
    dest_addr: str = Field(None, description="链路目的地址")
    state: int = Field(None, description="链路状态")
    update_time: int = Field(None, description="链路状态更新时间")

    class Config:
        orm_model = True
