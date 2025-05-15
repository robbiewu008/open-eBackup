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
from datetime import datetime
from enum import Enum
from typing import List
from uuid import UUID

from pydantic import BaseModel, constr, validator, Field
from pydantic.main import Optional

from app.resource.common.common_enum import MigrateType, HostMigrationIpType, HostMigrationOsType, \
    HostMigrationAgentType
LINUX_DIRECTORY_REX = "^(/[^/|;&$><`\\!+\\[\\]]{1,2048})+$|^/$"


class ObjectTypes(str, Enum):
    host = 'host'
    fileset = 'fileset'
    db = 'db'
    cluster = 'cluster'


def optional_to_string(name):
    if name is None:
        return ''
    return name


class FileSet(BaseModel):
    fileset_id: UUID = None
    name: Optional[str] = ''
    paths: List[constr(min_length=1)]
    include: List[constr(min_length=1)]
    exclude: List[constr(min_length=1)]

    _normalize = validator('name', allow_reuse=True)(optional_to_string)


class FileSets(BaseModel):
    fileset_id: UUID
    fileset_name: Optional[str] = ''
    ip: Optional[str] = ''
    host_id: Optional[str] = ''
    host_name: Optional[str] = ''
    os_type: Optional[str] = ''

    _normalize = validator('fileset_name', allow_reuse=True)(optional_to_string)


class FileSetName(BaseModel):
    fileset_id: UUID
    fileset_name: Optional[str] = ''
    ip: Optional[str] = ''
    host: Optional[str] = ''
    os_type: Optional[str] = ''

    _normalize = validator('fileset_name', allow_reuse=True)(optional_to_string)


class FileSetList(BaseModel):
    filesets: List[FileSets]


class FileSetNameList(BaseModel):
    fileset_names: List[FileSetName]


class HostCreate(BaseModel):
    host_id: str = Field(None, max_length=64, description="主机ID")
    name: constr(min_length=1) = Field(description="主机名称")
    ip: str = Field(None, max_length=128, description="主机IP地址")
    port: str = Field(None, max_length=16, description="主机端口号")
    link_status: str = Field('Unknown', max_length=32, description="主机状态")
    os_type: str = Field('Unknown', max_length=32, description="主机操作系统类型")
    proxy_type: int = Field(description="代理类型")
    is_cluster: bool = Field(False, description="是否集群")
    cluster_info: list = Field(None, description="集群信息")
    path: str = Field(None, max_length=1024, description="资源位置")
    userid: str = Field(None, max_length=255, description="用户ID")


class HostCreateInfo(HostCreate):
    agent_version: str = Field(None, description="agent 版本号")
    agent_timestamp: str = Field(None, description="agent包的时间戳")
    src_deduption: bool = Field(False, description="agent是否支持源端重删")
    install_path: str = Field(None, description="agent安装目录")
    is_auto_synchronize_host_name: bool = Field(False, description="是否同步agent主机名称")


class Host(BaseModel):
    host_id: str = Field(None, description="主机ID")
    name: constr(min_length=1) = Field(description="主机名称")
    endpoint: str = Field(None, description="主机IP地址")
    port: str = Field(None, description="主机端口号")
    link_status: str = Field('Unknown', description="主机状态")
    os_type: str = Field(None, description="主机操作系统类型")
    type: str = Field(None, description="资源类型")
    sub_type: str = Field(None, description="资源子类型")
    is_cluster: bool = Field(False, description="是否集群")
    cluster_info: list = Field(None, description="集群信息")
    authorized_user: str = Field(None, description="授权用户")
    user_id: str = Field(None, description="用户ID")
    error_duplicate_ip: str = Field('Unknown', description="注册主机时已存在的重复IP地址")


class HostDetail(Host):
    # 0 不需鉴权 1 未鉴权 2 已鉴权 3 未知状态
    asm_info: dict = Field({}, description="ASM信息")
    app_type: str = Field(None, description="资源类型")
    extend_db: dict = Field({}, description="数据库信息")


class ClusterHost(BaseModel):
    host_id: str = Field(None, description="主机ID")
    name: str = Field(None, description="主机名称")
    endpoint: str = Field(None, description="主机IP地址")
    os_type: str = Field("Unknown", description="主机操作系统类型")


class HostList(BaseModel):
    hosts: List[Host]


class AsmInfo(BaseModel):
    auth_type: str = Field(description="认证方式")
    inst_name: str = Field(description="实例名称")
    is_cluster: str = Field(description="是否集群")


class AsmAuthRequest(BaseModel):
    auth_type: int = Field(description="认证方式")
    asm_insts: list = Field(None, description="ASM实例列表")
    username: str = Field("", description="用户名", max_length=255)
    password: str = Field("", description="密码", max_length=2048)


class AsmAuthResponse(BaseModel):
    auth_type: int = Field(None, description="认证方式")
    asm_insts: list = Field(None, description="ASM实例列表")
    username: str = Field(None, description="用户名")
    password: str = Field(None, description="密码")


class FileData(BaseModel):
    customer: str
    dispPath: str
    code: str
    nodeType: int
    uuid: str
    expandedFlag: bool
    checked: bool
    checkable: bool
    fullPath: str


class FileTree(BaseModel):
    data_tree: List[FileData]


class FileListResp(BaseModel):
    status: str
    finished: str
    partialSign: int
    totalNum: int
    data: FileTree


class HostMigrationReq(BaseModel):
    # 前端传入
    host_id: str = Field(..., alias="hostId", description="目标集群id", max_length=64)
    host_user_name: str = Field(..., alias="hostUserName", description="目标集群id", min_length=1, max_length=255)
    host_password: str = Field(..., alias="hostPassword", description="目标集群id", min_length=1, max_length=255)
    ssh_macs: str = Field(None, alias="sshMacs", description="消息签名算法", max_length=255)
    business_ip_flag: str = Field(None, alias="businessIpFlag", description="是否业务IP", max_length=64)
    port: int = Field(None, alias="port", description="agent端口", ge=1, le=65536)


class MigrationReq(BaseModel):
    target_cluster_id: int = Field(alias="targetClusterId", description="目标集群id")
    host_migrate_req: List[HostMigrationReq] = Field(alias="hostMigrateReq", description="迁移主机信息", min_items=1,
                                                     max_items=50)
    install_path: str = Field(None, alias="installPath", description="安装目录", regex=LINUX_DIRECTORY_REX, max_length=4096)
    should_check_install_path_permit_high: bool = Field(True, alias="shouldCheckInstallPathPermitHigh",
                                                        description="是否需要校验目录权限过高")


class HostMigrationSchedule(BaseModel):
    # 后端获取
    host_id: str = Field(..., description="目标集群id")
    host_user_name: str = Field(description="主机用户名")
    host_password: str = Field(description="主机密码")
    ssh_macs: str = Field(None, description="消息签名算法")
    business_ip_flag: str = Field(None, description="是否业务IP")
    proxy_host_type: str = Field(..., description="应用类型")
    os_type: str = Field(..., description="操作系统类型")
    ip_type: str = Field(..., description="IP类型：IPV4、IPV6")
    ip_address: str = Field(description="主机ip")
    port: int = Field(None, description="端口")
    target_cluster_ip: str = Field(..., description="目标集群ip")
    target_cluster_port: int = Field(description="目标集群端口")
    job_id: str = Field(..., description="任务ID")
    target_cluster_job_id: str = Field(default=None, description="目标集群任务ID")
    install_path: str = Field(None, description="安装目录")


class HostMigrationExecuteReq(BaseModel):
    ip_address: str = Field(..., alias="ips", description="主机IP")
    proxy_host_type: HostMigrationAgentType = Field(..., alias="type", description="主机应用类型")
    os_type: HostMigrationOsType = Field(..., alias="osType", description="主机系统类型： Linux，Unix")
    ip_type: HostMigrationIpType = Field(..., alias="ipType", description="IP类型：IPV4，IPV6")
    host_username: str = Field(..., alias="username", description="主机用户名")
    host_password: str = Field(..., alias="password", description="主机密码")
    migrate: MigrateType = Field(..., description="迁移类型：0-代表主机")


class HostMigrationUnexpectedExecuteReq(BaseModel):
    created_time: str = Field(default=datetime.now().strftime("%Y-%m-%d %H:%M:%S"), ndescription="开始调度时间")
    job_id: str = Field(..., description="任务ID")
    host_id: str = Field(..., description="目标集群id")

