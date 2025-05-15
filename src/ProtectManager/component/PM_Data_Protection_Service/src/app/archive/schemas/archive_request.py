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
import enum
from typing import Optional, List

from pydantic import BaseModel, Field, StrictInt

from app.common.enums.sla_enum import RetentionTimeUnit, RetentionTypeEnum


class ArchiveMsg(BaseModel):
    request_id: str = Field(max_length=256, description="request id", default=None)
    params: dict = Field(description="请求参数", default=None)


class StorageProtocolEnum(int, enum.Enum):
    S3 = 2
    TAPE = 7


class ArchiveStorageInfo(BaseModel):
    storage_id: str = Field(description="归档存储库ID", max_length=256, min_length=1)
    esn: Optional[str] = Field(description="磁带库esn", max_length=256, min_length=1)
    protocol: StorageProtocolEnum = Field(description="归档存储库类型")


class ArchiveRequest(BaseModel):
    copy_id: str = Field(description="副本ID", max_length=64, min_length=1)
    duration_unit: Optional[RetentionTimeUnit] = Field(
        None, description="保留周期单位[d/w/MO/y]")
    retention_duration: Optional[StrictInt] = Field(None, description="保留周期")
    retention_type: Optional[RetentionTypeEnum] = Field(None, description="保留类型")
    auto_index: Optional[bool] = Field(description="是否自动索引", default=False)
    network_access: Optional[bool] = Field(description="是否开启网络加速", default=False)
    storage_list: List[ArchiveStorageInfo] = Field(description="归档存储库ID列表", max_items=4, min_items=1)
    qos_id: Optional[str] = Field(description="qos", max_length=256, min_length=1)
    driver_count: Optional[int] = Field(None, description="驱动数量")
