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

from fastapi import Query
from pydantic import BaseModel, Field


class NoArchiveReq(BaseModel):
    resource_id: str = Field(Query(..., description="资源id"))
    generated_by: Optional[str] = Field(Query(None, description="副本类型"))
    storage_id: str = Field(Query(..., description="归档存储id"))
    copy_id: Optional[str] = Field(Query(None, description="副本id"))
    user_id: Optional[str] = Field(Query(None, description="用户id"))
    is_query_log_copy: bool = Field(Query(False, description="是否查询日志副本"))
