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

from typing import List
from pydantic import BaseModel, Field

from common.const import RoleType


class ActionResult(BaseModel):
    code: int = Field(description="执行结果")
    body_err: int = Field(None, description="错误码", alias="bodyErr")
    message: str = Field(None, description="错误信息")
    body_err_params: list = Field(None, description="错误码具体参数", alias="bodyErrParams")


class LogDetail(BaseModel):
    log_info: str = Field(None, description="", alias="logInfo")
    log_info_param: list = Field(None, description="", alias="logInfoParam")
    log_timestamp: int = Field(None, description="", alias="logTimestamp")
    log_detail: int = Field(None, description="", alias="logDetail")
    log_detail_param: list = Field(None, description="", alias="logDetailParam")
    log_detail_info: list = Field(None, description="", alias="logDetailInfo")
    log_level: int = Field(None, description="", alias="logLevel")


class JobPermissionInfo(BaseModel):
    """
    查询应用权限信息QueryJobPermission返回Model
    """
    user: str = Field(None, description="操作系统用户")
    group: str = Field(None, description="操作系统用户组")
    file_mode: str = Field(None, description="文件权限", alias="fileMode")
    is_mount: bool = Field(True, description="是否挂载文件系统", alias="isMount")
    extend_info: dict = Field(None, description="扩展信息", alias="extendInfo")


class SubJobDetails(BaseModel):
    # 备份进度详情
    task_id: str = Field(description="主任务ID", alias="taskId")
    sub_task_id: str = Field(None, description="子任务ID", alias="subTaskId")
    task_status: int = Field(description="任务状态", alias="taskStatus")
    log_detail: List[LogDetail] = Field(None, description="", alias="logDetail")
    progress: int = Field(description="进度")
    data_size: int = Field(None, description="", alias="dataSize")
    speed: int = Field(None, description="")
    extend_info: dict = Field(None, description="", alias="extendInfo")


class CopyExtendInfo(BaseModel):
    backup_index_id: str = Field(None, alias="backupIndexId")
    backup_time: int = Field(None, description="agent backup time", alias='backupTime')
    begin_time: int = Field(None, description="begin time", alias='beginTime')
    end_time: int = Field(None, description="end time", alias='endTime')
    data_path: str = Field(None, description="", alias='dataPath')
    meta_path: str = Field(None, description="", alias='metaPath')


class Copy(BaseModel):
    # 上报备份副本信息
    id: str = Field(None, description="backup copy id")
    type: str = Field(None, alias="type")
    format: int = Field(None, alias="format")
    name: str = Field(None, alias="name")
    timestamp: int = Field(None, alias="timestamp")
    transaction_no: int = Field(None, alias="transactionNo")
    protect_env: dict = Field(None, alias="protectEnv")
    protect_object: dict = Field(None, alias="protectObject")
    protect_sub_objects: list = Field(None, alias="protectSubObjects")
    repositories: list = Field(None, alias="repositories")
    snapshots: list = Field(None, alias="snapshots")
    extend_info: dict = Field(None, alias="extendInfo")


class CopyInfoRepModel(BaseModel):
    """
    对应上传副本 Copy-repositories字段元素
    """
    id: str = Field(None, alias="id")
    repository_type: int = Field(alias="repositoryType")
    is_local: bool = Field(alias="isLocal")
    path: str = Field(None, alias="path")
    protocol: str = Field(None, description="Nfs or Cifs", alias="protocol")
    endpoint: dict = Field(None, alias="endpoint")
    extend_info: dict = Field(None, alias="extendInfo")
    remote_path: str = Field(alias="remotePath")
    remote_host: list = Field(None, alias="remoteHost")
    trans_protocol: str = Field(None, alias="transProtocol")


class ClusterNodeExtendInfo(BaseModel):
    """
    集群节点主备信息Extend info
    """
    role: RoleType = Field(default=1, description='node role')
    status: str = Field(None, description='cluster deploy type')


class ClusterNodeInfo(BaseModel):
    """
    集群节点信息
    """
    uuid: str = Field(default='', description='uuid')
    name: str = Field(default='', description='cluster name')
    endpoint: str = Field(default='', description='endpoint')
    type: str = Field(default='Database', description='resource type')
    sub_type: str = Field(None, description='resource sub type', alias='subType')
    extend_info: ClusterNodeExtendInfo = Field(None, description='extend info', alias='extendInfo')


class QueryClusterResponseExtendInfo(BaseModel):
    """
    集群节点主备信息Extend info
    """
    deployType: str = Field(default='AP', description='cluster deploy type')
    clusterVersion: str = Field(None, description='cluster database version')


class QueryClusterResponse(BaseModel):
    """
    集群节点主备信息 --> QueryCluster 接口
    """
    uuid: str = Field(None, description='uuid')
    name: str = Field(default=None, description='name')
    endpoint: str = Field(default='', description='endpoint')
    type: str = Field(default='Database', description='resource type')
    sub_type: str = Field(default=None, description='resource sub type', alias='subType')
    role: RoleType = Field(default=3, description='env role')
    nodes: List[ClusterNodeInfo] = Field(default=[], description='node info')
    extend_info: QueryClusterResponseExtendInfo = Field(None, description='extend info', alias='extendInfo')


class SubJobModel(BaseModel):
    """
    生成子任务结构体
    """
    job_id: str = Field(None, description="job id", alias="jobId")
    sub_job_id: str = Field("", description="sub job id", alias="subJobId")
    job_type: int = Field(None, description="job type", alias="jobType")
    job_name: str = Field(None, description="job name", alias="jobName")
    job_priority: int = Field(None, description="sub job priority", alias="jobPriority")
    policy: int = Field(None, description="sub job policy")
    ignore_failed: bool = Field(False, description="ignore result or not", alias="ignoreFailed")
    exec_node_id: str = Field(None, description="node uuid", alias="execNodeId")
    job_info: str = Field(None, description="job info", alias="jobInfo")


class QueryPermissionModel(BaseModel):
    user: str = Field(default='root', description='user')
    group: str = Field(default='root', description='group')
    file_mode: str = Field(default='0700', description='mod', alias='fileMode')


class ReportCopyInfoModel(BaseModel):
    """
    用于插件主动上报副本信息的结构
    """
    copy_info: Copy = Field(None, alias="copy")
    job_id: str = Field(None, alias="jobId")


class RepositoryPath(BaseModel):
    repository_type: int = Field(None, alias='repositoryType')
    scan_path: str = Field(None, alias='scanPath')


class ScanRepositories(BaseModel):
    scan_repo_list: List[RepositoryPath] = Field(None, alias='scanRepoList')
    save_path: str = Field(None, alias='savePath')


class ProgressStatus(BaseModel):
    task_status: int = Field(description="任务状态", alias="taskStatus")
    progress: int = Field(description="进度")
    log_detail: int = Field(None, description="", alias="logDetail")
    log_detail_param: list = Field(None, description="", alias="logDetailParam")

