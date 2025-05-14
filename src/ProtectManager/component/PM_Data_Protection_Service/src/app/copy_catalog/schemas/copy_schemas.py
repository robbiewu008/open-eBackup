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

from app.copy_catalog.common.copy_status import CopyWormStatus


class UpdateCopyPropertiesRequest(BaseModel):
    key: str = Field(description="需要更新的key")
    value: str = Field(description="需要更新的value")


class CopyWormStatusUpdate(BaseModel):
    worm_status: CopyWormStatus = Field(description="副本worm状态")


class CopyAntiRansomwareQuery(BaseModel):
    resource_id: str = Field(description="资源id")
    generated_by_list: List[str] = Field(None, description="副本类型集合")
    copy_start_time: str = Field(None, description="该时间之后完成的副本(微秒)")
    page_no: int = Field(description="分页页面编码")
    page_size: int = Field(description="分页数据条数")
    status: int = Field(None, description="状态")


class DeleteExcessCopiesRequest(BaseModel):
    retention_quantity: int = Field(alias="retentionQuantity", description="副本最大保留数量")
    generated_by: str = Field(alias="generatedBy", description="副本生成类型")
    user_id: str = Field(None, alias="userId", description="用户id")
