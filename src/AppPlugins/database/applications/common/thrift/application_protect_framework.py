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
from typing import Optional

from pydantic import BaseModel, Field

from common.thrift.application_protect_base_data_type import StorageRepository, JobPermission, Copy


class ResourceScope(int, Enum):
    """
    resource scope
    """
    # entier system valid
    ENTIRE_SYSTEM = 0
    # single node valid
    SINGLE_NODE = 1
    # single job valid
    SINGLE_JOB = 2
    PRODUCTION_RESOURCE = 3


class Resource(BaseModel):
    """
    job resource description, creating for multiple node using share resource
    """
    # resource scopelist
    scope: ResourceScope
    # resource scope key for identifing different scope 
    # scopeKey is 'system' when scope is equal to 'ENTIRE_SYSTEM'
    # scopeKey is empty string when scope is equal to 'SINGLE_NODE'
    # scopeKey is job id or sub job id when scope is equal to 'SINGLE_JOB'
    scope_key: str = Field(alias="scopeKey")
    # resource keylist
    resource_key: str = Field(alias="resourceKey")
    # resource valuelist
    resource_value: str = Field(alias="resourceValue")
    # if the resource is shared, how many nodes can be uselist
    shared_num: Optional[int] = Field(default=None, alias="sharedNum")
    lock_type: str = Field(alias="")


class ResourceStatus(BaseModel):
    """
    resource status
    """
    # resource informationlist
    resource: Resource
    # if the resource is locked, how many nodes have beed lock itlist
    lock_num: Optional[int] = Field(default=None, alias="lockNum")


class SubJobStatus(int, Enum):
    """
    sub job status
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
    """
    job log level
    """
    # information log
    TASK_LOG_INFO = 1
    # warning log
    TASK_LOG_WARNING = 2
    # error log
    TASK_LOG_ERROR = 3
    # serious warning log
    TASK_LOG_SERIOUS_WARN = 4


class LogDetail(BaseModel):
    """
    job log detail
    """
    # jog levellist
    level: JobLogLevel = Field(alias="logLevel")
    # log description label, which must be defined in systemlist
    description: str = Field(alias="logInfo")
    # log description parameterslist
    params: Optional[list[str]] = Field(default=None, alias="logInfoParam")
    # log time stamplist
    timestamp: int = Field(alias="logTimestamp")
    # log errorcode, which must be defined in systemlist
    error_code: Optional[int] = Field(default=None, alias="logDetail")
    # log errorcode parameterslist
    error_params: Optional[list[str]] = Field(default=None, alias="logDetailParam")
    # log additional information for detail descriptionlist
    additional_desc: Optional[list[str]] = Field(default=None, alias="logDetailInfo")



class CheckPoint(BaseModel):
    """
    job checkPoint, if the job is need to redo, the checkpoint is required,
    but plugin need record the checkpoint when execuing job
    """
    # checkpoint valuelist
    check_point: str = Field(alias="checkPoint")


class SubJobDetails(BaseModel):
    """
    report job detail information
    """
    # main job idlist
    job_id: str = Field(alias="taskId")
    # sub job id, for subjob report detaillist
    sub_job_id: Optional[str] = Field(default=None, alias="subTaskId")
    # job statuslist
    job_status: Optional[SubJobStatus] = Field(default=None, alias="taskStatus")
    # job additional status, define keyword status as follow:
    # application_availabe : during instant restoration, when an application is available,
    # the availability status needs to be reported to proxy framework
    additional_status: Optional[str] = Field(default=None, alias="additionalStatus")
    # job log detaillist
    log_detail: Optional[list[LogDetail]] = Field(default=None, alias="logDetail")
    # job progresslist
    progress: Optional[int] = Field(default=None)
    # job have moved data size, unit KBlist
    data_size: Optional[int] = Field(default=None, alias="dataSize")
    # job move data speed, unit KB/slist
    speed: Optional[int] = Field(default=None)
    # job extend informationlist
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class PrepareRepositoryByPlugin(BaseModel):
    """
    prepare repository by Plugin
    """
    # repository informationlist
    repository: list[StorageRepository]
    # repository permission*/
    permission: JobPermission
    # extend informationlist
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class AlarmDetails(BaseModel):
    """
    alarm detail information
    """
    # alarm IDlist
    alarm_id: str = Field(alias="alarmId")
    # sequence idlist
    sequence: int
    # alarm type: invalid(0) communication(1) environment(2) device(3) business(4) operation(5) security(6)list
    alarm_type: int = Field(alias="alarmType")
    # source type: user,alarm,event,notify,resource,proctection,recovery,backup_cluster,network_entitylist
    source_type: str = Field(alias="sourceType")
    # alarm severity: 0:event 1:warning 2:minor 3:important 4:criticallist
    severity: int
    # alarm parameterlist
    parameter: str
    # resource IDlist
    resource_id: str = Field(alias="resourceId")
