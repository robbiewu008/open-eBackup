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

from pydantic import BaseModel, Field

from app.common.enums.db_desesitization_enum import TaskTypeEnum, TaskStatusEnum


class DbDesestitationTaskSchema(BaseModel):
    resource_id: str = Field(None, description="资源ID")
    task_type: Optional[TaskTypeEnum] = Field(None, description="任务类型")
    task_status: Optional[TaskStatusEnum] = Field(None, description="任务状态")
    task_log: str = Field(None, description="任务执行日志")
    task_log_params: List[str] = Field(None, description="任务执行日志参数")
    task_log_detail: str = Field(None, description="任务执行日志详情")
    task_log_detail_params: List[str] = Field(None, description="任务执行日志详情参数")
    task_log_level: str = Field(None, description="任务执行日志级别")
    task_progress: int = Field(None, description="任务进度")
    policy_id: str = Field(None, description="策略ID")
    policy_name: str = Field(None, description="策略名称")
    db_obj: str = Field(None, description="数据库对象信息")
    log_time: int = Field(None, description="任务详情开始时间")
