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
from uuid import UUID
from typing import List

from pydantic import BaseModel, Field
from pydantic.main import Optional

from app.common.enums.resource_enum import ResourceSubTypeEnum, AuthType


def optional_to_string(name):
    if name is None:
        return ''
    return name


class DatabaseAuthRequest(BaseModel):
    auth_type: int = Field(description="认证方式，0：数据库认证，1：OS认证", le=1, ge=0)
    db_username: str = Field(None, description="数据库用户名", max_length=60)
    db_password: str = Field(None, description="数据库密码", max_length=60)


class DatabaseResponse(BaseModel):
    uuid: UUID


class DatabaseCluster(BaseModel):
    cluster_name: str = Field(None, description="集群名称", min_length=1, max_length=64,
                              regex="^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    nodes: List[str] = Field(..., description="节点列表")


class DatabaseTableSpace(BaseModel):
    conn_name: str
    file_name: str
    file_no: int
    ts_name: str


class DatabaseRes(BaseModel):
    DBNAME: str = ""
    INSTANCENAME: str = ""


class TimeRange(BaseModel):
    STARTTIME: str = Field(description="开始时间")
    ENDTIME: str = Field(description="结束时间")
    TIMEZONE: str = Field(description="时区")


class DatabaseTimeStamp(BaseModel):
    TIMERANGE: List[TimeRange] = Field(description="时间区间信息列表")


class DatabaseError(BaseModel):
    Code: int = Field(description="错误码")
    Description: str = Field(description="错误描述")


class DatabaseTimeStampResponse(BaseModel):
    Data: DatabaseTimeStamp = Field(description="时间戳信息")
    Error: DatabaseError = Field(description="错误信息")


class Database(BaseModel):
    inst_name: str = Field(alias="instName", description="实例名")
    db_name: str = Field(alias="dbName", description="数据库名称")
    version: str = Field(description="数据库版本号")
    state: int = Field(description="状态")
    is_asm_inst: int = Field(alias="isAsmInst", description="是否ASM实例")
    auth_type: int = Field(alias="authType", description="认证方式")
    db_role: int = Field(alias="dbRole", alidescription="角色")
    oracle_home: str = Field(alias="oracleHome", description="Oracle数据库目录")


class InstanceVerifyRequest(BaseModel):
    host_id: str = Field(description="主机ID", min_length=1, max_length=64)
    host_name: str = Field(None, description="主机名称", min_length=1, max_length=64)
    os_username: str = Field(None, description="实例所属操作系统用户", min_length=0, max_length=64)
    os_type: str = Field(None, description="操作系统类型", min_length=0, max_length=64)
    instance_name: str = Field(description="实例名", min_length=1, max_length=64)
    istest: bool = Field(description="是否测试连接")
    db_type: Optional[ResourceSubTypeEnum] = Field(None, description="资源子类型")
    mysql_sock: str = Field(None, description="端口文件路径", min_length=0, max_length=512)
    port: int = Field(3306, description="端口号", ge=1000, le=65535)
    username: str = Field(description="用户名", min_length=1, max_length=64)
    password: str = Field(description="密码", min_length=1, max_length=64)
    customer: Optional[AuthType] = Field(None, description="认证方式")
    is_manual: bool = Field(False, description="添加实例: True, 认证实例: False")


class RegisterDatabaseSchema(BaseModel):
    host_id: str = Field(description="主机ID", min_length=1, max_length=64)
    os_username: str = Field(None, description="实例所属操作系统用户", min_length=0, max_length=64)
    instance_name: str = Field(description="实例名称", min_length=1, max_length=64)


class AddDataBasesRequest(BaseModel):
    is_cluster: bool = Field(None, description="是否集群")
    host_id: str = Field(description="集群/主机ID", min_length=1, max_length=64)
    instance_infos: List[RegisterDatabaseSchema] = Field(description="数据库实例信息列表")
    database_names: List[str] = Field(description="数据库名称列表")


class RegisterClusterSqlServerDatabaseInfo(BaseModel):
    host_id: str = Field(description="主机ID", min_length=1, max_length=64)
    group_name: str = Field(description="实例所属可用性组", min_length=1, max_length=64)
    instance_name: str = Field(description="SQL Server数据库实例名称", min_length=1, max_length=64)
    database_name: str = Field(description="SQL Server数据库名称", min_length=1, max_length=64)


class AddClusterSqlServerDataBasesRequest(BaseModel):
    cluster_id: str = Field(description="集群ID", min_length=1, max_length=64)
    instance_infos: List[RegisterClusterSqlServerDatabaseInfo] = Field(description="SQL Server数据库实例信息列表")
    database_names: List[str] = Field(description="数据库名称列表")


class DatabaseInfo(BaseModel):
    host_id: str = Field(None, description="主机ID")
    host_name: str = Field(None, description="主机名称")
    host_endpoint: str = Field(None, description="主机地址")
    db_id: str = Field(None, description="数据库标识ID")
    db_type: str = Field(None, description="数据库类型")
    instance_name: str = Field(None, description="实例名")
    database_name: str = Field(description="数据库名称")
    is_register: bool = Field(None, description="是否注册")


class InstanceInfo(BaseModel):
    host_id: str = Field(None, description="主机ID")
    host_name: str = Field(None, description="主机名称")
    host_endpoint: str = Field(None, description="主机地址")
    os_username: str = Field(None, description="实例所属操作系统用户")
    instance_name: str = Field(None, description="实例名")
    is_authorized: bool = Field(None, description="是否认证")
    status: str = Field(None, description="实例状态")
    file_size: str = Field(None, description="数据文件大小")
    real_size: str = Field(None, description="实际数据量")
    log_mode: str = Field(None, description="日志模式")
    db_infos: List[DatabaseInfo] = Field(None, description="数据库信息列表")
    auth_type: str = Field(None, description="认证方式")
    username: str = Field(None, description="认证的用户名")


class AvailabilityGroup(BaseModel):
    group_name: str = Field(None, description="可用性组名称")
    uuid: str = Field(None, description="可用性组的UUID")
    node_type: str = Field(None, description="节点类型")
    is_authorized: bool = Field(None, description="是否认证")


class DbHostInfo(BaseModel):
    host_id: str = Field(None, description="主机ID")
    host_name: str = Field(None, description="主机名称")
    host_endpoint: str = Field(None, description="主机地址")
    instance_infos: List[InstanceInfo] = Field(None, description="实例信息列表")


class ClusterDBInfo(BaseModel):
    cluster_id: str = Field(None, description="集群ID")
    cluster_name: str = Field(None, description="集群名称")
    cluster_endpoint: str = Field(None, description="集群地址")
    cluster_type: str = Field(None, description="集群类型")
    db_host_infos: List[DbHostInfo] = Field(None, description="主机节点信息")


class AddDbClusterRequest(BaseModel):
    cluster_id: str = Field(description="集群ID", min_length=1, max_length=64)
    db_ids_list: List[List[str]] = Field(description="数据库标识ID")


class DbServerInfo(BaseModel):
    host_id: str = Field(None, description="主机/集群ID")
    host_name: str = Field(None, description="主机/集群名称")
    host_endpoint: str = Field(None, description="主机/集群IP地址")
    is_cluster: bool = Field(None, description="是否是集群")
    link_status: int = Field(None, description="状态")
    os_type: str = Field(None, description="操作系统类型")
    is_authorized: bool = Field(None, description="是否认证")
    db_count: int = Field(None, description="注册数据库数量")
    user_id: str = Field(None, description="用户ID")
    authorized_user: str = Field(None, description="授权用户")


class TimeSlot(BaseModel):
    full_time: str = Field(None, description="全量备份时间")
    log_time: str = Field(None, description="日志备份时间")
