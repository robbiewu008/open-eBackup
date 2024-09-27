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

from enum import Enum
from typing import List

from pydantic import BaseModel, Field

from common.schemas.thrift_base_data_type import StorageRepository, JobPermission


class ResourceScope(int, Enum):
    # entier system valid
    ENTIRE_SYSTEM = 0
    # single node valid
    SINGLE_NODE = 1
    # single job valid
    SINGLE_JOB = 2
    PRODUCTION_RESOURCE = 3


class Resource(BaseModel):
    scope: ResourceScope = Field(default=None, description='ResourceScope')
    scope_key: str = Field(default=None, description='scopeKey', alias='scopeKey')
    resource_key: str = Field(default=None, description='resourceKey', alias='resourceKey')
    resource_value: str = Field(default=None, description='resourceValue', alias='resourceValue')
    shared_num: int = Field(default=None, description='sharedNum', alias='sharedNum')
    lock_type: str = Field(default=None, description='lockType', alias='lockType')


class ResourceStatus(BaseModel):
    resource: Resource = Field(default=None, description='resource')
    lockNum: int = Field(default=None, description='lockNum', alias='lockNum')


class SubJobStatus(int, Enum):
    """
    子任务状态
    """
    # initilizing
    INITIALIZING = 0
    # running
    RUNNING = 1
    # aborting
    ABORTING = 2
    # complete successfully
    COMPLETED = 3
    # aborted
    ABORTED = 4
    # abort failed.
    ABORTED_FAILED = 5
    # failed
    FAILED = 6
    # job failed, and it can not been retry
    FAILED_NOTRY = 7
    # partial complete successfully
    PARTIAL_COMPLETED = 13


class JobLogLevel(int, Enum):
    # information log
    TASK_LOG_INFO = 1
    # warning log
    TASK_LOG_WARNING = 2
    # error log
    TASK_LOG_ERROR = 3
    # serious warning log
    TASK_LOG_SERIOUS_WARN = 4


class LogDetail(BaseModel):
    level: JobLogLevel = Field(default=None, description='JobLogLevel')
    description: str = Field(default=None, description='description')
    params: List[str] = Field(default=None, description='params')
    timestamp: int = Field(default=None, description='timestamp')
    error_code: int = Field(default=None, description='errorCode', alias='errorCode')
    error_params: List[str] = Field(default=None, description='errorParams', alias='errorParams')
    additional_desc: List[str] = Field(default=None, description='additionalDesc', alias='additionalDesc')


class CheckPoint(BaseModel):
    checkPoint: str = Field(default=None, description='checkPoint')


class SubJobDetails(BaseModel):
    job_id: str = Field(default=None, description='jobId', alias='jobId')
    sub_job_id: str = Field(default=None, description='subJobId', alias='subJobId')
    job_status: SubJobStatus = Field(default=None, description='jobStatus', alias='jobStatus')
    additional_status: str = Field(default=None, description='additionalStatus', alias='additionalStatus')
    log_detail: List[LogDetail] = Field(default=None, description='logDetail', alias='logDetail')
    progress: int = Field(default=None, description='progress')
    data_size: int = Field(default=None, description='dataSize', alias='dataSize')
    speed: int = Field(default=None, description='speed:')
    extend_info: dict = Field(default=None, description='extendInfo', alias='extendInfo')


class PrepareRepositoryByPlugin(BaseModel):
    repository: List[StorageRepository] = Field(default=None, description='repository')
    permission: JobPermission = Field(default=None, description='permission')
    extend_info: int = Field(default=None, description='extendInfo', alias='extendInfo')
