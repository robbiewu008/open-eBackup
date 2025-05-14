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
from app.protection.object.common.protection_enums import ResourceFilter, SlaApplyType


class ApsaraStackExtParam(BaseExtParam):
    """
        高级参数
    """
    agents: str = Field(None, description="agents的id,多个用;隔开")
    disk_info: Optional[List[str]] = Field(description="磁盘id列表")
    overwrite: bool = Field(None, description="资源集/可用区下云服务器已经绑定SLA时，SLA的覆盖策略")
    all_disk: Optional[str] = Field(description="是否保护全部磁盘")
    binding_policy: List[SlaApplyType] = Field(None, description="可用区/资源集下云服务器未绑定SLA时，SLA应用策略")
    resource_filters: Optional[List[ResourceFilter]] = Field(None, description="批量保护过滤规则列表")
    open_consistent_snapshots: Optional[str] = Field(description="是否开启一致性快照")
    disk_filters: Optional[List[ResourceFilter]] = Field(None, description="磁盘过滤规则列表")
    archive_res_auto_index: Optional[bool] = Field(description="归档是否启用自动索引")
    backup_res_auto_index: Optional[bool] = Field(description="备份是否启用自动索引")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.APSARA_STACK_INSTANCE, ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
                ResourceSubTypeEnum.APSARA_STACK_ZONE]