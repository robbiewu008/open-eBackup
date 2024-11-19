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

from common.schemas.thrift_base_data_type import ApplicationEnvironment, \
    Application, ApplicationResource, Copy, SubJob, \
    BackupJobType, StorageRepository, QueryByPage


class Qos(BaseModel):
    """
    job qos limit
    """
    bandwidth: int = Field(default=None, description='qos limit speed, unit megabyte')
    protect_iops: int = Field(default=None, description='qos limit iops with protect environment', alias="protectIops")
    backup_iops: int = Field(default=None, description='qos limit iops with backup storage', alias="backupIops")


class ResourceFilter(BaseModel):
    """
    资源过滤器
    """
    filter_by: str = Field(default=None,
                           description='filter id, filter by name, ID, Formate, ModifyTime, or CreateTime',
                           alias="filterBy")
    type: str = Field(default=None, description='filter type')
    rule: str = Field(default=None, description='filter rule, fuzzy match or exact match and so on')
    mode: str = Field(default=None, description='filter mode, INCLUDE or EXCLUDE')
    values: List[str] = Field(default=None, description='filter values')


class DataLayout(BaseModel):
    encryption: bool = Field(default=None, description='whether encryption')
    deduption: bool = Field(default=None, description='whether deduption')
    compression: bool = Field(default=None, description='whether data compression')
    native_data: bool = Field(default=None, description='whether backup data is native backup mode', alias="nativeData")
    extend_info: dict = Field(default=None,
                              description='data layout extend information, '
                                          'json string format, .eg encryption algorithm',
                              alias="extendInfo")


class JobScripts(BaseModel):
    pre_script: str = Field(default=None,
                            description='pre-processing script, which will be called before executing job',
                            alias="preScript")
    post_script: str = Field(default=None, description='post-processing script, '
                                                       'which will be called after executing job successfully',
                             alias="postScript")
    fail_post_script: str = Field(default=None, description='post-processing script, '
                                                            'which will be called after executing job failed',
                                  alias="failPostScript")


class BackupJobParam(BaseModel):
    backup_type: BackupJobType = Field(default=None, description='backup job type', alias="backupType")
    filters: List[ResourceFilter] = Field(default=None, description='backup resource filter')
    data_layout: DataLayout = Field(default=None, description='backup advance param', alias="dataLayout")
    qos: Qos = Field(default=None, description='backup qos limit')
    scripts: JobScripts = Field(default=None, description='backup script configuration')
    advance_params: dict = Field(default=None, description='backup advance parameters, json string format',
                                 alias="advanceParams")


class BackupJob(BaseModel):
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    job_param: BackupJobParam = Field(default=None, description='backup job parameter', alias='jobParam')
    protect_env: ApplicationEnvironment = Field(default=None, description='backup protect environment',
                                                alias='protectEnv')
    protect_object: Application = Field(default=None, description='backup protect application', alias='protectObject')
    protect_sub_object: List[ApplicationResource] = Field(default=None,
                                                          description='backup protect application resource',
                                                          alias='protectSubObject')
    repositories: List[StorageRepository] = Field(default=None, description='backup storage respository information')
    copies: List[Copy] = Field(default=None, description='the copy information to be created', alias='copy')
    extend_info: dict = Field(default=None, description='Job extend information', alias='extendInfo')


class RestoreJobType(int, Enum):
    """
    恢复任务类型
    """
    # restore byte-for-byte 普通恢复
    NORMAL_RESTORE = 1
    # instant restore 即时恢复
    INSTANT_RESTORE = 2
    # fine grained restore 细粒度恢复
    FINE_GRAINED_RESTORE = 3


class RestoreJobParam(BaseModel):
    """
    恢复任务参数
    """
    restore_type: RestoreJobType = Field(default=None, description='恢复类型', alias='restoreType')
    restore_mode: str = Field(default=None, description='恢复模式 LocalRestore: 本地恢复，使用挂载的文件系统恢复，'
                                                        'RemoteRestore远程恢复，例如使用归档的数据直接从远程恢复',
                              alias='restoreMode')
    filters: List[ResourceFilter] = Field(default=None, description='filters')
    qos: Qos = Field(default=None, description='qos限速')
    scripts: JobScripts = Field(default=None, description='scripts')
    advance_params: dict = Field(default=None, description='advanceParams, 例如使用时间戳恢复', alias='advanceParams')


class RestoreJob(BaseModel):
    """
    恢复任务schema
    """
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    job_param: RestoreJobParam = Field(default=None, description='恢复任务参数', alias='jobParam')
    target_env: ApplicationEnvironment = Field(default=None, description='恢复目标环境', alias='targetEnv')
    target_object: Application = Field(default=None, description='恢复目标对象', alias='targetObject')
    restore_sub_objects: List[ApplicationResource] = Field(default=None, description='恢复目标子对象，用于细粒度恢复',
                                                           alias='restoreSubObjects')
    copies: List[Copy] = Field(default=None, description='用于恢复的副本列表')
    extend_info: dict = Field(default=None, description='恢复任务扩展信息', alias='extendInfo')


class LivemountJobParam(BaseModel):
    """
    livemount job parameters
    """
    scripts: JobScripts = Field(default=None, description='scripts')
    advance_params: dict = Field(default=None, description='advanceParams', alias='advanceParams')


class CancelLivemountJobParam(BaseModel):
    """
    cancel livemount job parameters
    """
    scripts: JobScripts = Field(default=None, description='scripts')
    advance_params: dict = Field(default=None, description='advanceParams', alias='advanceParams')


class LivemountJob(BaseModel):
    """
    livemount job description
    """
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    job_param: LivemountJobParam = Field(default=None, description='任务参数', alias='jobParam')
    target_env: ApplicationEnvironment = Field(default=None, description='目标环境', alias='targetEnv')
    target_object: Application = Field(default=None, description='目标对象', alias='targetObject')
    target_sub_objects: List[ApplicationResource] = Field(default=None, description='目标子对象',
                                                          alias='targetSubObjects')
    copy_info: Copy = Field(default=None, description='用于挂载的副本', alias='copy')
    extend_info: dict = Field(default=None, description='任务扩展信息', alias='extendInfo')


class CancelLivemountJob(BaseModel):
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    job_param: CancelLivemountJobParam = Field(default=None, description='任务参数', alias='jobParam')
    target_env: ApplicationEnvironment = Field(default=None, description='目标环境', alias='targetEnv')
    target_object: Application = Field(default=None, description='目标对象', alias='targetObject')
    target_sub_objects: List[ApplicationResource] = Field(default=None, description='目标子对象',
                                                          alias='targetSubObjects')
    copy_info: Copy = Field(default=None, description='副本', alias='copy')
    extend_info: dict = Field(default=None, description='任务扩展信息', alias='extendInfo')


class BuildIndexJobParam(BaseModel):
    pre_copy_id: str = Field(default=None, description='previous copy id', alias='preCopyId')
    index_path: str = Field(default=None, description='build index metadata path', alias='indexPath')


class BuildIndexJob(BaseModel):
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    job_param: BuildIndexJobParam = Field(default=None, description='job_param', alias='jobParam')
    index_env: ApplicationEnvironment = Field(default=None, description='index_env', alias='indexEnv')
    index_protect_object: Application = Field(default=None, description='index_protect_object',
                                              alias='indexProtectObject')
    index_protect_sub_object: List[ApplicationResource] = Field(default=None, description='index_protect_sub_object',
                                                                alias='indexProtectSubObject')
    repositories: List[StorageRepository] = Field(default=None,
                                                  description='build index respository list, '
                                                              'including meta data, data, cache')
    copies: List[Copy] = Field(default=None, description='副本')
    extend_info: dict = Field(default=None, description='任务扩展信息', alias='extendInfo')


class DelCopyJob(BaseModel):
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    protect_env: ApplicationEnvironment = Field(default=None, description='protect environment',
                                                alias='protectEnv')
    protect_object: Application = Field(default=None, description='protect application', alias='protectObject')
    repositories: List[StorageRepository] = Field(default=None, description='storage respository information')
    copies: List[Copy] = Field(default=None, description='the copy information to be created')
    extend_info: dict = Field(default=None, description='Job extend information', alias='extendInfo')


class CheckCopyJob(BaseModel):
    request_id: str = Field(default=None, description='requestId，用于记录日志等目的', alias='requestId')
    job_id: str = Field(default=None, description='主任务id', alias='jobId')
    repositories: List[StorageRepository] = Field(default=None, description='storage respository information')
    copies: List[Copy] = Field(default=None, description='the copy information')
    extend_info: dict = Field(default=None, description='Job extend information', alias='extendInfo')


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
    items: List[ApplicationResource] = Field(default=None, description='resource elements list')
    page_no: int = Field(default=0, description='页号', alias='pageNo')
    page_size: int = Field(default=200, description='页大小', alias='pageSize')
    pages: int = Field(default=1, description='总页数')
    total: int = Field(default=0, description='总大小')


class ListResourceRequest(BaseModel):
    app_env: ApplicationEnvironment = Field(default=None, description='', alias='appEnv')
    applications: List[Application] = Field(default=0, description='')
    condition: QueryByPage = Field(default=200, description='')


class RestoreSubJob(BaseModel):
    """
    执行恢复子任务参数
    """
    job: RestoreJob = Field(default=None, description='恢复任务信息')
    sub_job: SubJob = Field(default=None, description='子任务信息，子任务拆分的信息在此处', alias='subJob')


class AllowRestoreInLocalSchema(BaseModel):
    """判断是否允许任务执行参数"""
    job: RestoreJob = Field(default=None, description='恢复任务信息')
    sub_job: SubJob = Field(default=None, description='子任务信息', alias='subJob')


class RestorePrerequisite(BaseModel):
    """恢复前置任务参数"""
    job: RestoreJob = Field(default=None, description='恢复任务信息')

