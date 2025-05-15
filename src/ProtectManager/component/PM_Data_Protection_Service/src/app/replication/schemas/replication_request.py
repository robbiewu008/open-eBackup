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
from typing import Optional

from pydantic import BaseModel, Field, StrictInt

from app.common.enums.sla_enum import RetentionTypeEnum, RetentionTimeUnit


class ReplicationRequest(BaseModel):
    copy_id: str = Field(description="副本ID", max_length=64, min_length=1)
    external_system_id: str = Field(description="外部集群id", max_length=32, min_length=1)
    retention_type: RetentionTypeEnum = Field(description="保留类型：1-永久保留 2-临时保留")
    duration_unit: Optional[RetentionTimeUnit] = Field(
        None, description="保留周期单位[d/w/MO/y]")
    retention_duration: Optional[StrictInt] = Field(None, description="保留周期")
    link_deduplication: bool = Field(description="是否开启复制链路重删", default=True)
    link_compression: bool = Field(description="是否开启复制链路压缩", default=False)
    alarm_after_failure: bool = Field(description="是否开启任务失败告警", default=False)
    storage_type: str = Field(description="指定存储类型", max_length=32, min_length=1)
    storage_id: Optional[str] = Field(description="存储单元（组）ID", max_length=256, min_length=1)
    user_id: Optional[str] = Field(description="指定用户id", max_length=256, min_length=1)
    username: Optional[str] = Field(description="用户名", max_length=256, min_length=1)
    password: Optional[str] = Field(description="密码", max_length=2048, min_length=1)
    userType: Optional[str] = Field(description="用户类型", max_length=2048, min_length=1)
