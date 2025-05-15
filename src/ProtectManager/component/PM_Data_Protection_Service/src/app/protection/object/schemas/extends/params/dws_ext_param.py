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
from app.protection.object.common.protection_enums import GaussDBTableTypeEnum
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.protection.object.schemas.extends.params.hbase_ext_param import PROTECTION_REGEX


class GaussDBDwsProtectionExtParam(BaseExtParam):

    """
    校验GaussDBT保护的参数
    """
    backup_metadata_path: Optional[str] = Field(..., description="元数据备份路径", max_length=256)
    backup_tool_type: Optional[GaussDBTableTypeEnum] = Field(None, description="表集备份方式")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.DWSCluster, ResourceSubTypeEnum.DWSDateBase, ResourceSubTypeEnum.DWSSchema,
                ResourceSubTypeEnum.DWSTable]
