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
from pydantic.types import OptionalInt


class BackupWorkFlowSchema(BaseModel):
    resource_id: str = Field(description="资源id")
    sla_id: str = Field(description="sla id")
    chain_id: str = Field(description="副本链id")
    policy: dict = Field(description="本次执行的备份策略")
    execute_type: str = Field(description="执行备份策略类型")
    auto_retry: bool = Field(description="是否开启自动重试")
    auto_retry_times: OptionalInt = Field(description="自动重试次数")
    auto_retry_wait_minutes: OptionalInt = Field(description="自动重试等待间隔")
