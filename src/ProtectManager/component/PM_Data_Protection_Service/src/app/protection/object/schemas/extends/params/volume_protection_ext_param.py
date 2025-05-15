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
PROTECTION_REGEX = "^(.+)[\.]{1}(sh|bat){1}$"


class VolumeProtectionExtParam(BaseExtParam):
    """
    校验主机文件集保护的参数
    """
    system_backup_flag: Optional[bool] = Field(False, description="系统状态备份")
    pre_script: Optional[str] = Field(None, description="备份前运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(None, description="备份成功运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")
    snapshot_size_percent: Optional[int] = Field(None, description="备份容量百分比", ge=0, le=100)

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.Volume]
