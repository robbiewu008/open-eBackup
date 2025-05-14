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
import abc
from typing import Optional
from pydantic import BaseModel, Field


class DataLayout(BaseModel):
    deduplication: bool = Field(description="是否开启重删")
    compression: bool = Field(description="是否开启压缩")


class BaseExtendParam(BaseModel, abc.ABC):

    @staticmethod
    @abc.abstractmethod
    def is_support(application, policy_type, action) -> bool:
        pass

    qos_id: Optional[str] = Field(None, description="限速策略id")


class BackupExtendParam(BaseModel):
    auto_retry: bool = Field(description="是否开启自动重试")
    auto_retry_times: int = Field(
        None, description="自动重试次数,默认3次", ge=1, le=5)
    auto_retry_wait_minutes: int = Field(
        None, description="自动重试等待时间(分钟),默认5分钟", ge=1, le=30)
