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
from enum import Enum
from typing import List

from pydantic import BaseModel, Field


class LockType(str, Enum):
    read_lock = "r"
    write_lock = "w"


class LockResource(BaseModel):
    id: str = Field(description="资源id")
    lock_type: LockType = Field(description="资源锁类型", alias="lockType")


class LockRequest(BaseModel):
    request_id: str = Field(description="请求id", alias="requestId")
    lock_id: str = Field(description="资源锁id", alias="lockId")
    resources: List[LockResource] = Field(description="锁定资源列表", alias="resources")
    priority: int = Field(description="加锁优先级", default=3)


class LockResponse(BaseModel):
    is_success: bool = Field(description="资源加锁结果", alias="isSuccess")
    failed_resource: str = Field(None, description="加锁失败的资源", alias="failedResource")

