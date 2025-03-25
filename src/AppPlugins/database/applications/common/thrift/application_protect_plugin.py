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

from common.thrift.application_protect_base_data_type import ApplicationEnvironment, \
    Application, ApplicationResource, Copy, SubJob, \
    BackupJobType, StorageRepository, QueryByPage


class Qos(BaseModel):
    """
    job qos limit
    """
    # qos limit speed, unit megabyte
    bandwidth: Optional[int] = Field(default=None)
    # qos limit iops with protect environment
    protect_iops: Optional[int] = Field(default=None, alias="protectIops")
    # qos limit iops with backup storage
    backup_iops: Optional[int] = Field(default=None, alias="backupIops")


class ResourceFilter(BaseModel):
    # filter id, filter by name, ID, Formate, ModifyTime, or CreateTime
    filter_by: str = Field(alias="filterBy")
    # filter type
    type: str
    # filter rule, fuzzy match or exact match and so on
    rule: str
    # filter mode, INCLUDE or EXCLUDE
    mode: str
    # filter values
    values: list[str]


class DataLayout(BaseModel):
    # whether encryption
    encryption: bool
    # whether deduption
    deduption: bool
    # whether data compression
    compression: bool
    # whether backup data is native backup mode
    native_data: bool = Field(alias="nativeData")
    # data layout extend information, json string format, .eg encryption algorithm
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class JobScripts(BaseModel):
    # pre-processing script, which will be called before executing job
    pre_script: str = Field(alias="preScript")
    # post-processing script, which will be called after executing job successfully
    post_script: str = Field(alias="postScript")
    # post-processing script, which will be called after executing job failed
    fail_post_script: str = Field(alias="failPostScript")


class BackupJobParam(BaseModel):
    """
    backup job parameters
    """
    # backup job type
    backup_type: BackupJobType = Field(alias="backupType")
    # backup resource filter
    filters: Optional[list[ResourceFilter]] = Field(default=None)
    # backup advance param
    data_layout: DataLayout = Field(alias="dataLayout")
    # backup qos limit
    qos: Optional[Qos] = Field(default=None)
    # backup script configuration
    scripts: Optional[JobScripts] = Field(default=None)
    # backup advance parameters, json string format
    advance_params: Optional[str] = Field(default=None, alias="advanceParams")


class BackupJob(BaseModel):
    """
    backup job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # backup job parameter*/
    job_param: BackupJobParam = Field(alias="jobParam")
    # backup protect environment
    protect_env: ApplicationEnvironment = Field(alias="protectEnv")
    # backup protect application
    protect_object: Application = Field(alias="protectObject")
    # backup protect application resource*/
    protect_sub_object: Optional[list[ApplicationResource]] = Field(default=None, alias="protectSubObject")
    # backup storage respository information, including meta data, data, log, cache
    repositories: list[StorageRepository]
    # the copy information to be created
    _copy: Copy = Field(alias="copy")
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class RestoreJobType(int, Enum):
    """
    restore job type
    """
    # restore byte-for-byte
    NORMAL_RESTORE = 1
    # instant restore
    INSTANT_RESTORE = 2
    # fine grained restore
    FINE_GRAINED_RESTORE = 3


class RestoreJobParam(BaseModel):
    """
    restore job parameters
    """
    # restore type
    restore_type: RestoreJobType = Field(alias="restoreType")
    # restore mode,
    # RemoteRestore : proxy restore data from archive microservice archive
    # LocalRestore  : proxy restore data from mounted file system
    restore_mode: str = Field(alias="restoreMode")
    # restore resource filter
    filters: Optional[list[ResourceFilter]] = Field(default=None)
    # restore qos limit
    qos: Optional[Qos] = Field(default=None)
    # restore script configuration
    scripts: Optional[JobScripts] = Field(default=None)
    # restore advance parameters, json string format, .eg restore by timestamp and restore to exact timestamp
    advance_params: Optional[str] = Field(default=None, alias="advanceParams")


class RestoreJob(BaseModel):
    """
    restore job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # restore job parameter
    job_param: RestoreJobParam = Field(alias="jobParam")
    # restore target environment
    target_env: ApplicationEnvironment = Field(alias="targetEnv")
    # restore target application
    target_object: Application = Field(alias="targetObject")
    # restore target application resource, it can be used in fine grained restore
    restore_sub_objects: Optional[list[ApplicationResource]] = Field(default=None, alias="restoreSubObjects")
    # copy is used for restoring
    copies: list[Copy]
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class LivemountJobParam(BaseModel):
    """
    livemount job parameters
    """
    # livemount script configuration
    scripts: Optional[JobScripts] = Field(default=None)
    # mount advance parameters
    advance_params: Optional[str] = Field(default=None, alias="advanceParams")


class CancelLivemountJobParam(BaseModel):
    """
    cancel livemount job parameters
    """
    # livemount script configuration
    scripts: Optional[JobScripts] = Field(default=None)
    # mount advance parameters
    advance_params: Optional[str] = Field(default=None, alias="advanceParams")


class LivemountJob(BaseModel):
    """
    livemount job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # mount job parameter
    job_param: Optional[LivemountJobParam] = Field(default=None, alias="jobParam")
    # mount target environment
    target_env: ApplicationEnvironment = Field(alias="targetEnv")
    # mount target application
    target_object: Application = Field(alias="targetObject")
    # mount target application resource
    target_sub_objects: Optional[list[ApplicationResource]] = Field(default=None, alias="targetSubObjects")
    # copy is used for restoring
    _copy: Copy = Field(alias="copy")
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class CancelLivemountJob(BaseModel):
    """
    cancel livemount job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # cancel livemount job parameter
    job_param: Optional[CancelLivemountJobParam] = Field(default=None, alias="jobParam")
    # cancel livemount target environment
    target_env: ApplicationEnvironment = Field(alias="targetEnv")
    # cancel livemount target application
    target_object: Application = Field(alias="targetObject")
    # cancel livemount target application resource
    target_sub_objects: Optional[list[ApplicationResource]] = Field(default=None, alias="targetSubObjects")
    # copy is used for restoring
    _copy: Copy = Field(alias="copy")
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class BuildIndexJobParam(BaseModel):
    """
    build index job parameters
    """
    # previous copy id
    pre_copy_id: str = Field(alias="preCopyId")
    # build index metadata path
    index_path: str = Field(alias="indexPath")


class BuildIndexJob(BaseModel):
    """
    build index job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # build index job parameter
    job_param: Optional[BuildIndexJobParam] = Field(default=None, alias="jobParam")
    # environment for building index
    index_env: ApplicationEnvironment = Field(alias="indexEnv")
    # application for building index
    index_protect_object: Application = Field(alias="indexProtectObject")
    # application resource for building index
    index_protect_sub_object: Optional[list[ApplicationResource]] = Field(default=None, alias="indexProtectSubObject")
    # build index respository list, including meta data, data, cache
    repositories: list[StorageRepository]
    # copy list is used for restoring
    copies: list[Copy]
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class DelCopyJob(BaseModel):
    """
    elete copy job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # environment for deleting copy
    protect_env: ApplicationEnvironment = Field(alias="protectEnv")
    # application for deleting copy
    protect_object: Application = Field(alias="protectObject")
    # application resource for deleting copy
    repositories: list[StorageRepository]
    # copy list is used for deleting
    copies: list[Copy]
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class CheckCopyJob(BaseModel):
    """
    Check copy job description
    """
    # current requestid in system, using write log for fix issue
    request_id: str = Field(alias="requestId")
    # current main job id
    job_id: str = Field(alias="jobId")
    # environment for checking copy
    protectEnv: ApplicationEnvironment
    # application for checking copy
    protectObject: Application
    # application resource for checking copy
    repositories: list[StorageRepository]
    # copy list is used for checking
    copies: list[Copy]
    # Job extend information, json string format
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class BackupLimit(int, Enum):
    """
    executing backup node limit
    """
    # any cluste node can be execute
    NO_LIMIT = 0
    # only master node can be execute
    ONLY_MASTER = 1
    # only slave node can be execute
    ONLY_SLAVE = 2
    # backup on master node first
    FIRST_MASTER = 3
    # backup on slave node first
    FIRST_SLAVE = 4


class JobResult(int, Enum):
    """
    jobs results
    """
    # all previous jobs are successfully executed
    SUCCESS = 0
    # some previous jobs fail to be executed
    FAILED = 1
    # some previous jobs are aborted
    ABORTED = 2


class ResourceResultByPage(BaseModel):
    """
    list resource result with page
    """
    # resource elements list
    items: list[ApplicationResource] = Field(alias="resourceList")
    # current page no
    page_no: int = Field(alias="pageNo")
    # maximum number of elements in one page
    page_size: int = Field(alias="pageSize")
    # total page number
    pages: int
    # total elements number
    total: int


class ListResourceRequest(BaseModel):
    """
    list resource by page request
    """
    # environment for list resource
    app_env: ApplicationEnvironment = Field(alias="appEnv")
    # parent application resource for list resource
    applications: Optional[list[Application]] = Field(default=None)
    # condition for list resource
    condition: QueryByPage
