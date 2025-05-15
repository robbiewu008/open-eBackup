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
from app.protection.object.common.protection_enums import SlaApplyType, ResourceFilter
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.protection.object.schemas.extends.params.hbase_ext_param import PROTECTION_REGEX


class FusionComputeExtParam(BaseExtParam):

    """
    校验FusionCompute保护的参数
    """
    pre_script: Optional[str] = Field(None, description="备份前运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(None, description="备份成功运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    agents: Optional[str] = Field(None, description="agents的id,多个用,隔开")
    overwrite: bool = Field(None, description="主机/Cluster下虚拟机已经绑定SLA时，SLA的覆盖策略")
    binding_policy: List[SlaApplyType] = Field(None, description="主机/Cluster下虚拟机未绑定SLA时，SLA应用策略")
    resource_filters: Optional[List[ResourceFilter]] = Field(None, description="批量保护过滤规则列表")
    disk_filters: Optional[List[ResourceFilter]] = Field(None, description="过滤规则列表")
    disk_info: Optional[List[str]] = Field(None, description="磁盘列表")
    snap_delete_speed: int = Field(None, description="FC快照删除速率，单位MB/s", min=0, max=1000)
    is_consistent: bool = Field(False, description="是否开启一致性备份")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.FUSION_ONE_COMPUTE]
