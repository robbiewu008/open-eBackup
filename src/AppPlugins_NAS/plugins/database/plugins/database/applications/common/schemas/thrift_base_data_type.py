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

from common.const import CopyDataTypeEnum


class ActionResult(BaseModel):
    """
    return action result
    """
    code: int = Field(default=None, description="执行结果")
    body_err: int = Field(default=None, description="错误码", alias="bodyErr")
    message: str = Field(default=None, description="错误信息")
    body_err_params: list = Field(default=None, description="错误码具体参数", alias="bodyErrParams")


class ApplicationPlugin(BaseModel):
    """
    application plugin struct
    """
    name: str = Field(default=None, description="plugin name")
    end_point: str = Field(default=None, description="plugin rpc service ip address", alias="endPoint")
    port: int = Field(default=None, description="plugin rpc service ip port")
    process_id: str = Field(default=None, description="plugin running main process", alias="processId")


class AuthType(int, Enum):
    """
    认证类型
    """
    # 无认证
    NO_AUTHENTICATION = 0
    # os认证
    OS_PASSWORD = 1
    # 用户名密码认证
    APP_PASSWORD = 2
    # LADP认证
    LADP = 3
    # ak sk
    AKSK = 4
    # kerberos
    KERBEROS = 5
    # token
    TOKEN = 6
    # OAUTH2
    OAUTH2 = 7
    # other
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
    auth_type: AuthType = Field(default=None, description='authType', alias="authType")
    auth_key: str = Field(default=None, description='user name', alias="authkey")
    auth_pwd: str = Field(default=None, description='user password', alias="authPwd")
    extend_info: dict = Field(default=None, description='authentication extend information', alias="extendInfo")


class ApplicationResource(BaseModel):
    """
    资源信息, 例如： database datafile or vmware virtual machine disk
    """
    type: str = Field(default=None, description='whether the resource is leaf resource')
    sub_type: str = Field(default=None, description='application type', alias="subType")
    id: str = Field(default=None, description='resource id, length:0~256, resource inner id *')
    name: str = Field(default=None, description='名称')
    parent_id: str = Field(default=None, description='parent resource id', alias="parentId")
    parent_name: str = Field(default=None, description='parent resource name', alias="parentName")
    extend_info: dict = Field(default=None, description='resource extend information', alias="extendInfo")


class Application(BaseModel):
    """
    应用信息, 例如： database or VMware virtual machine
    """
    type: str = Field(default=None, description='保护对象类型，区分虚拟化、数据库等大类型')
    sub_type: str = Field(default=None, description='保护对象子类型，明确是那种数据库', alias="subType")
    id: str = Field(default=None, description='保护对象id')
    name: str = Field(default=None, description='保护对象名称')
    parent_id: str = Field(default=None, description='保护对象id', alias="parentId")
    parent_name: str = Field(default=None, description='保护对象parent id，例如host的id', alias="parentName")
    auth: Authentication = Field(default=None, description='保护对象鉴权信息')
    extend_info: dict = Field(default=None, description='保护对象扩展信息', alias="extendInfo")


class ApplicationEnvironment(BaseModel):
    """
    环境信息, 例如： database host or VMware vCenter
    """
    id: str = Field(default=None, description='环境id')
    name: str = Field(default=None, description='环境名称')
    type: str = Field(default=None, description='类型，例如Host表明是主机')
    sub_type: str = Field(default=None, description='子类型', alias="subType")
    endpoint: str = Field(default=None, description='保护环境的ip')
    port: int = Field(default=None, description='端口')
    auth: Authentication = Field(default=None, description='保护环境的认证信息')
    nodes: list = Field(default=None, description='节点列表')
    extend_info: dict = Field(default=None, description='保护环境扩展信息', alias="extendInfo")


class SubJobType(int, Enum):
    """
    子任务类型
    """
    # 前置任务
    PRE_SUB_JOB = 0
    # 生成子任务
    GENERATE_SUB_JOB = 1
    # 执行子任务
    BUSINESS_SUB_JOB = 2
    # 后置子任务
    POST_SUB_JOB = 3


class ExecutePolicy(int, Enum):
    """
    执行策略
    """
    # 子任务分发至任意节点
    ANY_NODE = 0
    # 在创建任务的节点上执行子任务
    LOCAL_NODE = 1
    # 子任务分发至每个节点
    EVERY_NODE_ONE_TIME = 2
    # 子任务执行失败时重新调度至另一节点
    RETRY_OTHER_NODE_WHEN_FAILED = 3
    # 子任务在指定节点上执行
    FIXED_NODE = 4


class SubJob(BaseModel):
    """
    子任务
    """
    job_id: str = Field(default='', description='主任务id，子任务归属于某个主任务', alias="jobId")
    sub_job_id: str = Field(default='', description='子任务id', alias="subJobId")
    job_type: SubJobType = Field(default=None, description='sub job type', alias="jobType")
    job_name: str = Field(default=None, description='子任务名称，在主任务内唯一', alias="jobName")
    job_priority: int = Field(default=None, description='子任务优先级，1最高，依次降低', alias="jobPriority")
    policy: ExecutePolicy = Field(default=None, description='子任务执行策略')
    ignore_failed: bool = Field(default=None, description='whether main job will ignore this job executing result, '
                                                          'the value is null when executing sub job',
                                alias="ignoreFailed")
    exec_node_id: str = Field(default=None, description='sub job executing node uuid, '
                                                        'backup agent will check when acquiring job',
                              alias="execNodeId")
    job_info: str = Field(default=None, description='extend job information', alias="jobInfo")


class RepositoryDataType(int, Enum):
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
    副本类型
    """
    # 全量备份副本
    FULL_COPY = 1
    # 增量备份副本
    INCREMENT_COPY = 2
    # 差异备份副本
    DIFF_COPY = 3
    # 日志备份福诶
    LOG_COPY = 4
    # external storage snapshot copy
    SNAPSHOT_COPY = 5
    # 永久增量备份副本
    PERMANENT_INCREMENTAL_COPY = 6
    # 复制副本
    REPLICATION_COPY = 7
    # cloud storage copy
    CLOUD_STORAGE_COPY = 8
    # tape storage copy
    TAPE_STORAGE_COPY = 9
    # clone copy
    CLONE_COPY = 10


class HostAddress(BaseModel):
    ip: str = Field(default=None, description='host address ip .eg NFS server ip address in NFS protocol，'
                                              '多个以逗号分开，eg:ip1,ip2')
    port: int = Field(default=None, description='端口')
    support_protocol: int = Field(default=None, description='支持协议类型 1:NFS,2:CIFS,3:NFS+CIFS,1024:Dataturbo',
                                  alias="supportProtocol")


class RepositoryRole(int, Enum):
    # repository is master
    REPO_MASTER = 0
    # repository is slave
    REPO_SLAVE = 1


class StorageRepository(BaseModel):
    """
    backup storage repository
    """
    id: str = Field(default=None, description='仓库id')
    repository_type: RepositoryDataType = Field(default=None, description='repositoryType', alias="repositoryType")
    role: RepositoryRole = Field(default=None, description='repository role if task have several repositories')
    is_local: bool = Field(default=None, description='是否本地路径', alias="isLocal")
    path: List[str] = Field(default=None, description='仓库本地挂载路径')
    protocol: RepositoryProtocolType = Field(default=None, description='支持协议类型')
    auth: Authentication = Field(default=None, description='仓库鉴权信息')
    endpoint: HostAddress = Field(default=None, description='仓库ip等信息')
    remote_path: str = Field(default=None, description='backup respository data access path', alias="remotePath")
    remote_name: str = Field(default=None, description='backup respository data access name, '
                                                       '.eg cifs protocol share name', alias="remoteName")
    remote_host: List[HostAddress] = Field(default=None, description='backup respository data access endpoint',
                                           alias="remoteHost")
    extend_Auth: Authentication = Field(default=None, description='仓库扩展鉴权信息', alias="extendAuth")
    extend_info: dict = Field(default=None, description='仓库扩展信息', alias="extendInfo")


class Snapshot(BaseModel):
    id: str = Field(default='', description='storage snapshot id')
    parent_name: str = Field(default='', description='storage snapshot parent name, '
                                                     'nas file system name or logical unit name', alias="parentName")


class Copy(BaseModel):
    """
    副本信息
    """
    format: CopyFormatType = Field(default=None, description='副本格式formatType')
    # 副本类型，agent thrift接口中定义的type是int，ubc下发的是str，插件看到的就是str，
    # 枚举值来自于common中定义的CopyDataTypeEnum
    type: CopyDataTypeEnum = Field(default=None, description='副本类型，例如full')
    id: str = Field(default='', description='副本uuid')
    name: str = Field(default=None, description='副本名称')
    timestamp: int = Field(default=None, description='backup copy protect application timestamp')
    transaction_no: int = Field(default=None, description='backup copy application transaction No.',
                                alias="transactionNo")
    protect_env: ApplicationEnvironment = Field(default=None, description='protect environment, '
                                                                          '.eg protect vCenter or protect host',
                                                alias="protectEnv")
    protect_object: Application = Field(default=None, description='protect application, '
                                                                  '.eg protect virtual machine or database',
                                        alias="protectObject")
    protect_sub_objects: List[ApplicationResource] = Field(
        default=None,
        description='protect resource about application, .eg protect vm disk or database datafile',
        alias="protectSubObjects")
    repositories: List[StorageRepository] = Field(default=None, description='仓库信息')
    snapshots: List[Snapshot] = Field(default=None, description='快照信息')
    extend_info: dict = Field(default=None, description='副本对应的扩展信息', alias="extendInfo")


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
    page_no: int = Field(default=None, description='page number', alias="pageNo")
    page_size: int = Field(default=None, description='one page size', alias="pageSize")
    orders: List[str] = Field(default=None, description='data order field')
    conditions: str = Field(default=None, description='filter condition')


class JobPermission(BaseModel):
    """
    job permission
    """
    user: str = Field(default=None, description='user id')
    group: str = Field(default=None, description='group id')
    file_mode: str = Field(default=None, description='file mode', alias="fileMode")
    is_mount: bool = Field(default=None, description='whether mount file system before executing jobs', alias="isMount")
    extend_info: str = Field(default=None, description='job permission extend information', alias="extendInfo")
