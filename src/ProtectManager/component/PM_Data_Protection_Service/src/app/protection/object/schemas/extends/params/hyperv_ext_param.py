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


class HyperVHostExtParam(BaseExtParam):
    """
       HyperV主机保护高级参数
    """
    agents: str = Field(None, description="agents的id,多个用;隔开")
    overwrite: bool = Field(description="主机/Cluster下虚拟机已经绑定SLA时，SLA的覆盖策略")
    binding_policy: List[SlaApplyType] = Field(description="主机/Cluster下虚拟机未绑定SLA时，SLA应用策略")
    resource_filters: Optional[List[ResourceFilter]] = Field(description="批量保护过滤规则列表")
    disk_filters: Optional[List[ResourceFilter]] = Field(None, description="磁盘过滤规则列表")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.HYPER_V_HOST]


class HyperVVMExtParam(BaseExtParam):
    """
        HyperV虚拟机保护高级参数
    """
    agents: str = Field(None, description="agents的id,多个用;隔开")
    disk_info: Optional[List[str]] = Field(description="磁盘id列表")
    all_disk: Optional[str] = Field(description="是否保护全部磁盘")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.HYPER_V_VM]