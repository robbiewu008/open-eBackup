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

from pydantic import Field
from app.common.enums.resource_enum import ResourceSubTypeEnum

from app.backup.schemas.base_ext_param import BackupExtendParam, BaseExtendParam
from app.common.enums.sla_enum import PolicyTypeEnum


class EccOracleExtendParam(BaseExtendParam, BackupExtendParam):
    """
    SAP_ON_ORACLE扩展参数
    """
    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.backup and application is ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE

    channel_number: Optional[int] = Field(description="备份通道数", ge=1, le=254)
