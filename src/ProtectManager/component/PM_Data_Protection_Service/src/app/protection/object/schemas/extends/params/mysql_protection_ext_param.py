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
from app.protection.object.schemas.extends.params.hbase_ext_param import PROTECTION_REGEX


class MysqlProtectionExtParam(BaseExtParam):

    """
    校验MySQL保护的参数
    """
    pre_script: Optional[str] = Field(None, description="备份前运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(None, description="备份成功运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    force_optimize: Optional[str] = Field(None, description="是否运行优化表命令")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.MysqlInstance, ResourceSubTypeEnum.MysqlDatabase,
                ResourceSubTypeEnum.MysqlClusterInstance]
