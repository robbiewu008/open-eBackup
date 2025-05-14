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
from typing import List, Optional

from pydantic import Field

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
PROTECTION_REGEX = "^(.+)[\.]{1}(bat){1}$"


class ADProtectionExtParam(BaseExtParam):
    """
    Exchange DAG组、数据库保护高级参数
    """
    m_isConsistent: Optional[bool] = Field(None, description="执行VSS副本一致性校验")
    dag_backup: Optional[str] = Field(None, description="可用性组备份")
    pre_script: Optional[str] = Field(None, description="保护前执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(None, description="保护后执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="保护失败执行脚本", regex=PROTECTION_REGEX, max_length=8192)

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.EXCHANGE_GROUP, ResourceSubTypeEnum.EXCHANGE_DATABASE]
