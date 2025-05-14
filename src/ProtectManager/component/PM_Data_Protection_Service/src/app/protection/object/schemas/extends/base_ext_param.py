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
import abc
from typing import List, Optional

from pydantic import BaseModel, Field


class BaseExtParam(BaseModel):
    first_backup_esn: Optional[str] = Field(None, description="首次备份esn")
    last_backup_esn: Optional[str] = Field(None, description="上次备份esn")
    priority_backup_esn: Optional[str] = Field(None, description="优先备份esn")
    first_backup_target: Optional[str] = Field(None, description="首次备份单元")
    last_backup_target: Optional[str] = Field(None, description="上次备份单元")
    priority_backup_target: Optional[str] = Field(None, description="优先备份单元")
    failed_node_esn: Optional[str] = Field(None, description="上次失败esn")
    enable_security_archive: Optional[bool] = Field(False, description="安全归档")
    worm_switch: Optional[bool] = Field(False, description="SLA是否开启worm设置")

    @staticmethod
    @abc.abstractmethod
    def support_values() -> List[object]:
        pass
