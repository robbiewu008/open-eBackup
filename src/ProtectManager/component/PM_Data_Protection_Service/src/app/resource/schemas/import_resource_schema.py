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
from pydantic import Field, BaseModel

from app.resource.schemas.resource import EnvironmentResourceSchema


class ImportResourceSchema(EnvironmentResourceSchema):
    location: str = Field(None, description="导入资源存储位置", max_length=128)


class ImportResourceRequest(BaseModel):
    name: str = Field(None, description="导入资源名称", max_length=128)


class UpdateRetentionRequest(BaseModel):
    retention_type: int = Field(description="副本保留类型：1（永久保留）2（指定时间保留）")
    retention_duration: int = Field(None, description="副本保留时间")
    duration_unit: str = Field(None, description="副本保留时间单位（天、周、月、年）")
