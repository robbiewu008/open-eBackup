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
from pydantic import Field


from app.backup.schemas.base_ext_param import BackupExtendParam, BaseExtendParam
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum


class NasFileSystemExtendParam(BaseExtendParam, BackupExtendParam):
    """
    NasFileSystem扩展参数
    """

    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.backup and application is ResourceSubTypeEnum.NasFileSystem

    auto_index: bool = Field(description="是否开启自动索引")
    deduplication: bool = Field(True, description="是否开启重复数据删除")
