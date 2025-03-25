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


class ExecuteResult(int, Enum):
    # plugin or proxy framework executes successfully
    SUCCESS = 0
    # plugin or proxy framework executes successfully, client need to perform this operation continue
    CONTINUE = 100
    # plugin or proxy framework is busy, client should perform this operation after a period of time
    BUSY = 101
    # an internal error occurred in the plugin or proxy framework
    INTERNAL_ERROR = 200


class ActionResult(BaseModel):
    """
    plugin return action result
    """
    # execute result
    code: ExecuteResult
    # business error code, this error code is need to applied in OMRP
    body_err: Optional[int] = Field(default=None, alias="bodyErr")
    # return result message, message maybe contain failed result
    message: Optional[str] = Field(default=None)
    # business error parameter
    body_err_params: Optional[list[str]] = Field(default=None, alias="bodyErrParams")


class ApplicationPlugin(BaseModel):
    """
    application plugin struct
    """
    # plugin name
    name: str
    # plugin rpc service ip address
    end_point: str = Field(alias="endPoint")
    # plugin rpc service ip port
    port: int
    # plugin running main process
    process_id: str = Field(alias="processId")


class AuthType(int, Enum):
    """
    authentication type
    """
    # no authentication
    NO_AUTHENTICATION = 0
    # authentication by local os account
    OS_PASSWORD = 1
    # authentication by application account
    APP_PASSWORD = 2
    # authentication by ladp account
    LADP = 3
    # authentication by ak sk
    AKSK = 4
    # authentication by kerberos
    KERBEROS = 5
    # authentication by token
    TOKEN = 6
    # authentication by certification
    OAUTH2 = 7
    # authentication by other method
    OTHER = 8


class Authentication(BaseModel):
    """
    authentication struct
    authType is in {OS_PASSWORD|APP_PASSWORD|LADP}
        authkey is user name, authPwd is password
    authType is AKSK
        authkey is user name, authPwd is password
    authType is in {KERBEROS|TOKEN|OAUTH2|OTHER}
    the authentication information is stored in extendInfo
    """
    # authentication type
    auth_type: AuthType = Field(alias="authType")
    # authentication user name
    auth_key: Optional[str] = Field(default=None, alias="authkey")
    # authentication user password
    auth_pwd: Optional[str] = Field(default=None, alias="authPwd")
    # authentication extend information, when authentication by certification, certification's thumbprint is optional
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class ApplicationResource(BaseModel):
    """
    application resource struct, .eg database datafile or vmware virtual machine disk
    """
    # whether the resource is leaf resource, leaf resource have no sub resource
    type: str
    # application type
    sub_type: Optional[str] = Field(default=None, alias="subType")
    # resource id, length:0~256, resource inner id
    id: str
    # resource name
    name: Optional[str] = Field(default=None)
    # parent resource id
    parent_id: Optional[str] = Field(default=None, alias="parentId")
    # parent resource name
    parent_name: Optional[str] = Field(default=None, alias="parentName")
    # resource extend information
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class Application(BaseModel):
    """
    application struct, .eg database or VMware virtual machine
    """
    # application type
    type: str
    # application sub type
    sub_type: Optional[str] = Field(default=None, alias="subType")
    # application id, if application have inner id, it's the inner id
    id: Optional[str] = Field(default=None)
    # application name
    name: str
    # application parent id
    parent_id: Optional[str] = Field(default=None, alias="parentId")
    # application parent name
    parent_name: Optional[str] = Field(default=None, alias="parentName")
    # application authentication
    auth: Optional[Authentication] = Field(default=None)
    # application extend information
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class ApplicationEnvironment(BaseModel):
    """
    environment struct, .eg database host or VMware vCenter
    """
    # environment id
    id: str
    # environment name
    name: str
    # environment type
    type: str
    # sub environment type
    sub_type: Optional[str] = Field(default=None, alias="subType")
    # environment service ip address, only platform environment have this field
    endpoint: Optional[str] = Field(default=None)
    # environment service ip port
    port: Optional[int] = Field(default=None)
    # environment service authentication
    auth: Optional[Authentication] = Field(default=None)
    # application environment node list, when the application is local at cluster, nodes will have all nodes
    nodes: Optional[list["ApplicationEnvironment"]] = Field(default=None)
    # environment extend information
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class SubJobType(int, Enum):
    """
    subjob type
    """
    # prerequisite job
    PRE_SUB_JOB = 0
    # generate job
    GENERATE_SUB_JOB = 1
    # business job
    BUSINESS_SUB_JOB = 2
    # post job
    POST_SUB_JOB = 3


class ExecutePolicy(int, Enum):
    """
    sub job execute policy
    """
    # sub job will dispatched in any node
    ANY_NODE = 0
    # whether executing the job in creating job node
    LOCAL_NODE = 1
    # sub job will dispatched in every node one time
    EVERY_NODE_ONE_TIME = 2
    # retry at other node when one node failed
    RETRY_OTHER_NODE_WHEN_FAILED = 3
    # sub job will executed at fixed node
    FIXED_NODE = 4


class SubJob(BaseModel):
    """
    sub job struct, plugin will generate new job with this struct
    """
    # main job id, new sub job belong to main job
    job_id: str = Field(alias="jobId")
    # sub job id, the id is null when adding new sub job, the id is not null when executing sub job
    sub_job_id: Optional[str] = Field(default=None, alias="subJobId")
    # sub job type
    job_type: SubJobType = Field(alias="jobType")
    # sub job name, one sub job have unique name in main job scope
    job_name: str = Field(alias="jobName")
    # sub job priority, begin with 1, and sub job will execute by priority, the value is null when executing sub job
    job_priority: Optional[int] = Field(default=None, alias="jobPriority")
    # sub job execute policy
    policy: Optional[ExecutePolicy] = Field(default=None)
    # whether main job will ignore this job executing result, the value is null when executing sub job
    ignore_failed: Optional[bool] = Field(default=None, alias="ignoreFailed")
    # sub job executing node uuid, backup agent will check when acquiring job
    exec_node_id: Optional[str] = Field(default=None, alias="execNodeId")
    # extend job information*/
    job_info: Optional[str] = Field(default=None, alias="jobInfo")


class RepositoryDataType(int, Enum):
    """
    storage repository data type
    """
    # repository will save the meta data
    META_REPOSITORY = 0
    # repository will save the data
    DATA_REPOSITORY = 1
    # repository will save the cache data
    CACHE_REPOSITORY = 2
    # repository will save the log
    LOG_REPOSITORY = 3
    # repository will save the index data
    INDEX_REPOSITORY = 4
    # repository will save log meta data
    LOG_META_REPOSITORY = 5


class RepositoryProtocolType(int, Enum):
    """
    storage repository protoco type
    """
    # repository using CIFS protocol
    CIFS = 0
    # repository using NFS protocol
    NFS = 1
    # repository using S3 protocol
    S3 = 2
    # repository using blcok protocol
    BLOCK = 3
    # repository using local directory
    LOCAL_DIR = 4
    # repository using tape
    TAPE = 5


class CopyFormatType(int, Enum):
    """
    backup copy type
    """
    # copy using backup storage snapshot
    INNER_SNAPSHOT = 0
    # copy using backup storage directory
    INNER_DIRECTORY = 1
    # copy using extenal data
    EXTERNAL = 2


class CopyDataType(int, Enum):
    """
    copy data type
    """
    # full backup copy
    FULL_COPY = 1
    # incremental backup copy*/
    INCREMENT_COPY = 2
    # different backup copy*/
    DIFF_COPY = 3
    # log backup copy
    LOG_COPY = 4
    # external storage snapshot copy
    SNAPSHOT_COPY = 5
    # permanent incremental backup copy
    PERMANENT_INCREMENTAL_COPY = 6
    # replication copy
    REPLICATION_COPY = 7
    # cloud storage copy
    CLOUD_STORAGE_COPY = 8
    # tape storage copy
    TAPE_STORAGE_COPY = 9
    # clone copy
    CLONE_COPY = 10


class HostAddress(BaseModel):
    """
    Host Address
    """
    # host address ip .eg NFS server ip address in NFS protocol
    ip: str
    # host address port
    port: int
    # protocol supported by logical ports. eg 1:NFS,2:CIFS,3:NFS+CIFS,1024:Dataturbo*/
    support_protocol: Optional[int] = Field(default=None, alias="supportProtocol")


class RepositoryRole(int, Enum):
    """
    repository role
    """
    # repository is master
    REPO_MASTER = 0
    # repository is slave
    REPO_SLAVE = 1


class StorageRepository(BaseModel):
    """
    backup storage repository
    """
    # backup repository id
    id: str
    # backup repository type
    repository_type: RepositoryDataType = Field(alias="repositoryType")
    # repository role if task have several repositories
    role: Optional[RepositoryRole] = Field(default=None)
    # backup repository path is local A8000 path*/
    is_local: bool = Field(alias="isLocal")
    # backup repository local mount path
    path: Optional[list[str]] = Field(default=None)
    # backup respository protocol type
    protocol: RepositoryProtocolType
    # backup repository authentication information
    auth: Optional[Authentication] = Field(default=None)
    # backup respository manage endpoint
    endpoint: Optional[HostAddress] = Field(default=None)
    # backup respository data access path
    remote_path: Optional[str] = Field(default=None, alias="remotePath")
    # backup respository data access name, .eg cifs protocol share name
    remote_name: Optional[str] = Field(default=None, alias="remoteName")
    # backup respository data access endpoint
    remote_host: Optional[list[HostAddress]] = Field(default=None, alias="remoteHost")
    # extend authentication information of repository,
    # .eg backup to NFS, the extendAuth is the authentication informatio of nfs server
    extend_Auth: Optional[Authentication] = Field(default=None, alias="extendAuth")
    # backup repository cifs authentication information
    cifs_auth: Optional[Authentication] = Field(default=None, alias="cifsAuth")
    # respository extend information
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class Snapshot(BaseModel):
    """
    storage snapshot information
    """
    # storage snapshot id
    id: str
    # storage snapshot parent name, nas file system name or logical unit name
    parent_name: str = Field(alias="parentName")


class Copy(BaseModel):
    """
    backup copy information
    """
    # backup copy data format type
    format_type: Optional[CopyFormatType] = Field(default=None, alias="format")
    # backup copy data type
    data_type: Optional[CopyDataType] = Field(default=None, alias="type")
    # backup copy id
    id: str
    # backup copy name
    name: Optional[str] = Field(default=None)
    # backup copy protect application timestamp, it's requried  when application need support log backup
    timestamp: Optional[int] = Field(default=None)
    # backup copy application transaction No.,
    # it's requried when application need support log backup and application have inner transaction No.
    transaction_no: Optional[int] = Field(default=None, alias="transactionNo")
    # protect environment, .eg protect vCenter or protect host
    protect_env: Optional[ApplicationEnvironment] = Field(default=None, alias="protectEnv")
    # protect application, .eg protect virtual machine or database
    protect_object: Optional[Application] = Field(default=None, alias="protectObject")
    # protect resource about application, .eg protect vm disk or database datafile
    protect_sub_objects: Optional[list[ApplicationResource]] = Field(default=None, alias="protectSubObjects")
    # when image is inner type, path is respository type in backup storage,
    # if the application use multiple filesystem, this field need list
    repositories: Optional[list[StorageRepository]] = Field(default=None)
    # additional storage snapshot list information
    snapshots: Optional[list[Snapshot]] = Field(default=None)
    # copy extend information
    extend_info: Optional[str] = Field(default=None, alias="extendInfo")


class BackupJobType(int, Enum):
    """
    backup job type
    """
    # full backup
    FULL_BACKUP = 1
    # incremental backup, backup different data base on lastest backup
    INCREMENT_BACKUP = 2
    # different backup, backup different data base on lastest full backup
    DIFF_BACKUP = 3
    # log backup
    LOG_BAKCUP = 4
    # external storage backup
    SNAPSHOT = 5
    # permanent incremental backup, after incremental backup, plugin need synthetic full backup copy
    PERMANENT_INCREMENTAL_BACKUP = 6


class QueryByPage(BaseModel):
    """
    query data by page
    """
    # page number
    page_no: int = Field(alias="pageNo")
    # one page size
    page_size: int = Field(alias="pageSize")
    # data order field
    orders: Optional[list[str]] = Field(default=None)
    # filter condition
    conditions: Optional[str] = Field(default=None)


class JobPermission(BaseModel):
    """
    job permission
    """
    # user id
    user: str
    # group id
    group: str
    # file mode
    file_mode: Optional[str] = Field(default=None, alias="fileMode")
    # whether mount file system before executing jobs
    is_mount: Optional[bool] = Field(default=None, alias="isMount")
    # job permission extend information
    extend_info: Optional[dict] = Field(default=None, alias="extendInfo")
