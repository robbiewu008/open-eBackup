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

from app.common.enums.resource_enum import ResourceSubTypeEnum
from pydantic import Field

from app.protection.object.schemas.extends.base_ext_param import BaseExtParam


class OracleExtParam(BaseExtParam):
    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.Oracle, ResourceSubTypeEnum.ORACLE_CLUSTER]

    pre_script: Optional[str] = Field(None, description="保护前执行脚本")
    post_script: Optional[str] = Field(None, description="保护后执行脚本")
    failed_script: Optional[str] = Field(None, description="保护失败执行脚本")
    agents: Optional[str] = Field(None, description="代理主机")
    delete_archived_log: Optional[bool] = Field(None, description="备份完成删除归档日志")
    storage_snapshot_flag: Optional[bool] = Field(False, description="是否存储快照备份")
    snapshot_agents: Optional[str] = Field(None, description="存储快照代理主机")
    max_storage_usage_ratio: Optional[str] = Field(None, description="生产存储使用容量阈值")
    concurrent_requests: str = Field(None, description="并发数")
    delete_before_time: Optional[str] = Field(None, description="删除多长时间之前的归档日志")
    delete_before_time_unit: Optional[str] = Field(None, description="删除多长时间之前的归档日志（单位）")