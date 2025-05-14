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
from typing import Optional, List

from pydantic import Field

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam

SMALL_FILE_AGGREGATION_SIZE = [128, 256, 1024, 2048, 4096]


class ObjectSetProtectionExtParam(BaseExtParam):
    """
    Object set 保护高级参数
    """
    multiNodeBackupSwitch: Optional[bool] = Field(None, description="多节点并行备份")
    prefixSplitDepth: Optional[int] = Field(None, description="列举最大遍历目录深度", min=1, max=20)
    prefixSplitter: Optional[str] = Field(None, description="列举分隔符")
    continueOnFailedSwitch: Optional[bool] = Field(None, description="单节点故障继续保护")
    isBackupAcl: Optional[bool] = Field(False, description="是否备份ACL")
    useBucketLog: Optional[bool] = Field(None, description="使用桶日志进行增量扫描")
    aggregateSwitch: Optional[bool] = Field(False, description="小文件聚合")
    maxSizeAfterAggregate: Optional[int] = Field(False, description="聚合文件大小")
    maxSizeToAggregate: Optional[int] = Field(False, description="待聚合文件最大大小")
    checkPoint: Optional[bool] = Field(False, description="是否开启断点续备")
    retryNum: Optional[int] = Field(3, description="重传次数", ge=1, le=5)
    archive_res_auto_index: Optional[bool] = Field(description="对象归档是否启用自动索引")
    tape_archive_auto_index: Optional[bool] = Field(description="磁带归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.OBJECT_SET]
