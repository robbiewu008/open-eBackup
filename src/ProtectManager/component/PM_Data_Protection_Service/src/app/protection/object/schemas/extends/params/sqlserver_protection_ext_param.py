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

from app.base import consts
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam

# SQL Server针对Windows环境使用.bat脚本
BAT_SCRIPT_REGEX = "^(.+)[\.](bat)$"


class SqlServerProtectionExtParam(BaseExtParam):
    """
    校验SQL Server保护的参数
    """
    pre_script: Optional[str] = Field(None, description="备份前运行脚本", regex=BAT_SCRIPT_REGEX,
                                      max_length=consts.DEFAULT_WINDOWS_PATH_MAX)
    post_script: Optional[str] = Field(None, description="备份成功运行脚本", regex=BAT_SCRIPT_REGEX,
                                       max_length=consts.DEFAULT_WINDOWS_PATH_MAX)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=BAT_SCRIPT_REGEX,
                                         max_length=consts.DEFAULT_WINDOWS_PATH_MAX)

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.SQLServer, ResourceSubTypeEnum.SQLServerInstance,
                ResourceSubTypeEnum.SQLServerClusterInstance, ResourceSubTypeEnum.SQLServerCluster,
                ResourceSubTypeEnum.SQLServerAlwaysOn, ResourceSubTypeEnum.SQLServerDatabase]
