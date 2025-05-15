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


class QosReq(BaseModel):
    name: str = Field(description="qos 名称", min_length=1, max_length=64,
                      regex="^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    speed_limit: int = Field(description="限制速率,单位M", ge=10, le=5120)
    description: str = Field(description="描述", max_length=255)


class QosRes(BaseModel):
    uuid: str = Field(description="id")
    name: str = Field(description="qos 名称")
    speed_limit: int = Field(description="限制速率,单位M")
    description: str = Field(description="描述")

    class Config:
        orm_mode = True
