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

from typing import Dict, Optional, List

from pydantic import BaseModel, Field

from common.const import RoleType


class BackupJobPermission(BaseModel):
    user: str = Field(None, description="数据库操作用户")
    group: str = Field(None, description="数据库操作所属组")
    file_mode: str = Field(None, description="权限", alias="fileMode")


class BackupProgressInfo(BaseModel):
    last_objects: int = Field(None, description="上次备份数量", alias="last_objects")
    backup_objects: int = Field(None, description="已备份数量", alias="backup_objects")
    total_objects: int = Field(None, description="数据文件总数量", alias="total_objects")
    total_size: int = Field(None, description="数据文件总大小", alias="total_size")
    s_time: int = Field(None, description="开始时间", alias="s_time")
    c_time: int = Field(None, description="当前时间", alias="c_time")
    status: int = Field(None, description="任务状态", alias="status")
    error_code: int = Field(None, description="错误码", alias="error_code")
    message: str = Field(None, description="详细信息", alias="message")


class RestoreConfigParam(BaseModel):
    system_user: str = Field(description="运行数据库的操作系统用户")
    target_version: str = Field(description="目标实例版本")
    target_install_path: str = Field(description="目标实例安装路径")
    target_data_path: str = Field(description="目标实例数据路径")
    log_copy_path: str = Field(None, description="日志副本挂载路径")
    recovery_target_time: str = Field(None, description="恢复目标时间")
    target_archive_path: str = Field(None, description="目标实例归档路径")
    extend_info: Optional[Dict] = Field(dict(), description="恢复配置扩展参数")


class RestoreSpeed(BaseModel):
    total_size: int = Field(None, description="数据文件总大小")
    current_size: int = Field(None, description="已恢复数据文件大小")
    s_time: int = Field(None, description="开始时间")
    c_time: int = Field(None, description="当前时间")
    speed: int = Field(0, description="恢复速度，单位：KB/s")


class RestoreProgress(BaseModel):
    progress: int = Field(0, description="恢复进度值")
    message: str = Field("", description="恢复信息")
    err_code: int = Field(0, description="错误码")
    err_code_param: list = Field([], description="错误码参数列表")


class ClupCluster(BaseModel):
    cluster_id: str = Field(default=None, description='集群id')
    cluster_data: str = Field(default=None, description='集群详情')
    cluster_state: str = Field(default=None, description='集群状态')


class ClupDb(BaseModel):
    pgdata: str = Field(default=None, description='数据库目录')
    is_primary: str = Field(default=None, description='主备库')
    host: str = Field(default=None, description='所在主机')
    port: str = Field(default=None, description='端口')
    pg_bin_path: str = Field(default=None, description='PG软件目录')
    version: str = Field(default=None, description='版本')
    os_user: str = Field(default=None, description='操作系统用户')


class ClupClusterNodeInfo(BaseModel):
    """
    集群节点信息
    """
    uuid: str = Field(default='', description='uuid')
    name: str = Field(default='', description='cluster name')
    endpoint: str = Field(default='', description='endpoint')
    type: str = Field(default='Database', description='resource type')
    sub_type: str = Field(None, description='resource sub type', alias='subType')
    extend_info: dict = Field(None, description='extend info', alias='extendInfo')


class QueryClupClusterResponse(BaseModel):
    """
    集群节点主备信息 --> QueryCluster 接口
    """
    uuid: str = Field(None, description='uuid')
    name: str = Field(default=None, description='name')
    endpoint: str = Field(default='', description='endpoint')
    type: str = Field(default='Database', description='resource type')
    sub_type: str = Field(default=None, description='resource sub type', alias='subType')
    role: RoleType = Field(default=3, description='env role')
    nodes: List[ClupClusterNodeInfo] = Field(default=[], description='node info')
    extend_info: dict = Field({}, description='extend info', alias='extendInfo')
