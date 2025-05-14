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

from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum


class CheckAuthModel(BaseModel):
    resource_set_type: ResourceSetTypeEnum = Field(None, description="资源集类型")
    operation: OperationTypeEnum = Field(None, description="操作类型")
    auth_operation_list: List[AuthOperationEnum] = Field(None, description="操作权限列表")
    target: str = Field(None, description="资源id")
