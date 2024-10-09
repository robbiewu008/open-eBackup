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

from pydantic import BaseModel, Field

from goldendb.handle.common.const import SubJobType, ExecutePolicy


class ActionResponse(BaseModel):
    code: int = Field(default=0, description="执行结果")
    body_err: int = Field(None, description="错误码", alias='bodyErr')
    message: str = Field(default='', description="错误信息")
    body_err_params: list = Field(None, description="错误码具体参数", alias="bodyErrParams")


class SubJob(BaseModel):
    job_id: str = Field(default='', description='任务ID', alias='jobId')
    sub_job_id: str = Field(default='', description='子任务ID', alias='subJobId')
    job_type: str = Field(default=SubJobType.BUSINESS_SUB_JOB, description='任务类型', alias='jobType')
    job_name: str = Field(default='', description='任务名', alias='jobName')
    job_priority: int = Field(default=1, description='任务优先级', alias='jobPriority')
    policy: int = Field(default=ExecutePolicy.FIXED_NODE, description='策略')
    ignore_failed: bool = Field(default=False, description='是否忽略失败结果', alias='ignoreFailed')
    exec_node_id: str = Field(default='', description='执行任务节点ID', alias='execNodeId')
    job_info: str = Field(default='', description='任务信息', alias='jobInfo')


class PermissionInfo(BaseModel):
    user: str = Field(default='root', description='user')
    group: str = Field(default='root', description='group')
    file_mode: str = Field(default='0700', description='mod', alias='fileMode')


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