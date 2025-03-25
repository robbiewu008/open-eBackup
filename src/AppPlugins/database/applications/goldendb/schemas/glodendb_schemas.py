#
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
#

from common.const import SubJobStatusEnum
from pydantic import BaseModel, Field


class ActionResponse(BaseModel):
    code: int = Field(default=0, description="执行结果")
    body_err: int = Field(None, description="错误码", alias='bodyErr')
    message: str = Field(default='', description="错误信息")
    body_err_params: list = Field(None, description="错误码具体参数", alias="bodyErrParams")


class SqliteTable(BaseModel):
    uuid: str = Field(None, description='UUID')
    name: str = Field(None, description='name')
    type: str = Field(None, description='type')
    parent_path: str = Field(default='/', description='父节点路径')
    parent_uuid: str = Field(None, description='PARENT_UUID')
    size: str = Field(None, description='大小')
    create_time: str = Field(None, description='创建时间')
    modify_time: str = Field(None, description='修改时间')
    extend_info: str = Field(None, description='扩展信息')
    res_type: str = Field(None, description='RES_TYPE')
    res_sub_type: str = Field(None, description='RES_SUB_TYPE')


class TaskInfo(BaseModel):
    """
    生成管理节点执行备份/恢复任务信息结构体
    """
    pid: str = Field(None, description="process id", alias="pid")
    job_id: str = Field(None, description="job id", alias="jobId")
    sub_job_id: str = Field(None, description="sub job id", alias="subJobId")
    task_type: str = Field(None, description="task type", alias="taskType")
    json_param_object: dict = Field(None, description="json param object", alias="jsonParam")
    log_comm: str = Field(None, description="log common message", alias="logComm")


class StatusInfo(BaseModel):
    """
    记录管理节点执行备份/恢复任务状态
    """
    priority: int = Field(None, description="管理节点执行任务的优先级", alias="priority")
    status: int = Field(SubJobStatusEnum.RUNNING.value, description="任务状态", alias="status")
    log_detail: int = Field(None, description="错误码", alias="logDetail")
    log_detail_param: list = Field(None, description="前端显示的参数", alias="logDetailParam")
    log_info: str = Field(None, description="子任务标签", alias="logInfo")
