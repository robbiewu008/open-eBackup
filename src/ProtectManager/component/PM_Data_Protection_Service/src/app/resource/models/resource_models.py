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

from fastapi import Query
from pydantic import BaseModel
from sqlalchemy import Column, String, DateTime, ForeignKey, BOOLEAN, Integer, Boolean, Text, BigInteger, DECIMAL
from sqlalchemy.dialects.postgresql import ARRAY
from sqlalchemy.orm import aliased

from app.base.db_base import Base
from app.common.logger import get_logger
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.schemas.env_schemas import EnvironmentSchema, InternalEnvironmentSchema
from app.resource.schemas.resource import EnvironmentResourceSchema

log = get_logger(__name__)


class ResourceTable(Base):
    __tablename__ = 'resources'
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(64), primary_key=True)
    name = Column(String(256))
    type = Column(String(64))
    user_id = Column(String(255))
    authorized_user = Column(String(255))
    # 子类型
    sub_type = Column(String(64))
    # 资源的来源：restore、livemount
    source_type = Column(String(16))
    path = Column(String(1024))
    parent_name = Column(String(256))
    parent_uuid = Column(String(64))
    root_uuid = Column(String(64))
    children_uuids = Column(ARRAY(String))
    discriminator = Column(String(64))
    created_time = Column(DateTime, nullable=False, default=datetime.now)
    version = Column(String(64))
    protection_status = Column(Integer, default=0, comment="资源保护状态, 0-未保护，1-已保护,2-创建中")
    __filter_fields__ = [
        "%name%", "!discriminator",
        "!properties", "%path%", "%authorized_user%"
    ]
    __mapper_args__ = {
        'polymorphic_on': discriminator,
        'polymorphic_identity': __tablename__
    }
    __schema__ = EnvironmentResourceSchema

    # 2.12新增资源组id属性
    @classmethod
    def __relation__(cls):
        environment = aliased(EnvironmentTable)
        return {
            "joins": {
                environment: environment.uuid == cls.root_uuid,
                ProtectedObject: ProtectedObject.resource_id == cls.uuid,
                ResourceDesesitizationTable: ResourceDesesitizationTable.uuid == cls.uuid
            },
            "alias": {
                "environment_uuid": environment.uuid,
                "%environment_name%": environment.name,
                "%environment_endpoint%": environment.endpoint,
                "%authorized_user%": environment.authorized_user,
                "%sla_name%": ProtectedObject.sla_name,
                "sla_status": ProtectedObject.status,
                "sla_compliance": ProtectedObject.sla_compliance,
                "resource_group_id": ProtectedObject.resource_group_id,
                "environment_os_type": environment.os_type,
                "identification_status": ResourceDesesitizationTable.identification_status,
                "desesitization_status": ResourceDesesitizationTable.desesitization_status,
                "desesitization_policy_name": ResourceDesesitizationTable.desesitization_policy_name
            },
            "order_by": ["-created_time"],
            "converter": lambda env, po, deses: [{
                                                     "environment_uuid": env.uuid,
                                                     "environment_name": env.name,
                                                     "environment_endpoint": env.endpoint,
                                                     "environment_os_type": env.os_type,
                                                     "environment_type": env.type,
                                                     "environment_sub_type": env.sub_type,
                                                     "environment_is_cluster": env.is_cluster,
                                                     "environment_os_name": env.os_name,
                                                 } if env else {}, {
                                                     "sla_id": po.sla_id,
                                                     "sla_name": po.sla_name,
                                                     "sla_status": po.status,
                                                     "sla_compliance": po.sla_compliance,
                                                     "ext_parameters": po.ext_parameters,
                                                 } if po else {}, {
                                                     "identification_status": deses.identification_status,
                                                     "desesitization_status": deses.desesitization_status,
                                                     "desesitization_policy_id": deses.desesitization_policy_id,
                                                     "desesitization_policy_name": deses.desesitization_policy_name,
                                                 } if deses else {}]
        }

    @classmethod
    def __summary__(cls):
        environment = aliased(EnvironmentTable)
        return {
            "joins": {
                environment: environment.uuid == cls.root_uuid,
                ProtectedObject: ProtectedObject.resource_id == cls.uuid,
            },
            "alias": {
                "environment_uuid": environment.uuid,
                "%environment_name%": environment.name,
                "%environment_endpoint%": environment.endpoint,
                "%authorized_user%": environment.authorized_user,
                "sla_id": ProtectedObject.sla_id,
                "%sla_name%": ProtectedObject.sla_name,
                "sla_status": ProtectedObject.status,
                "sla_compliance": ProtectedObject.sla_compliance,
                "environment_os_type": environment.os_type,
            },
            "converter": lambda env, po: [{
                "environment_uuid": env.uuid,
                "environment_name": env.name,
                "environment_endpoint": env.endpoint,
                "environment_os_type": env.os_type,
            }, {
                "sla_id": po.sla_id,
                "sla_name": po.sla_name,
                "sla_status": po.status,
                "sla_compliance": po.sla_compliance,
                "ext_parameters": po.ext_parameters,
            } if po else {}]
        }

    @property
    def as_dict(self):
        temp = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return temp


class EnvironmentTable(ResourceTable):
    __tablename__ = 'environments'
    __discriminator__ = __tablename__
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(64), ForeignKey(ResourceTable.uuid, ondelete="CASCADE"), primary_key=True)
    endpoint = Column(String(128))
    port = Column(String(16))
    user_name = Column(String(256))
    password = Column(String(2048))
    link_status = Column(String(32))
    location = Column(String(128))
    os_type = Column(String(32))
    os_name = Column(String(128))
    is_cluster = Column(BOOLEAN, default=False)
    scan_interval = Column(Integer, default=3600)
    cert_name = Column(String(128))
    time_zone = Column(String(64))
    agent_version = Column(String(64))
    agent_timestamp = Column(String(64))

    __filter_fields__ = ["%name%", "!discriminator", "os_name%", "sub_type"]
    __mapper_args__ = {
        'polymorphic_identity': __discriminator__,
    }

    __relation__ = {
        "schema": EnvironmentSchema
    }

    __internal_relation__ = {
        "base": "__relation__",
        "schema": InternalEnvironmentSchema
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return temp


class ResourceCatalogTable(Base):
    """
    资源目录表，用于存放哪些资源在界面上显示，哪些不显示
    """
    __tablename__ = 'resource_catalog'
    catalog_id = Column(String(64), primary_key=True)
    catalog_name = Column(String(64))
    display_order = Column(Integer)
    show = Column(Boolean, default=False)
    parent_id = Column(String(64))
    label = Column(String(64))

    def as_dict(self):
        temp = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return temp


class ResourceDesesitizationTable(Base):
    __tablename__ = 'resource_desesitization'
    uuid = Column(String(64), ForeignKey(ResourceTable.uuid, ondelete="CASCADE"), primary_key=True)
    desesitization_status = Column(String(64))
    identification_status = Column(String(64))
    desesitization_job_id = Column(String(64))
    identification_job_id = Column(String(64))
    desesitization_policy_id = Column(String(64))
    desesitization_policy_name = Column(String(64))


class LinkStatusTable(Base):
    __tablename__ = 'link_status'
    uuid = Column(String(64), primary_key=True)
    source_role = Column(String(64), nullable=False)
    source_addr = Column(String(64), nullable=False)
    dest_role = Column(String(64), nullable=False)
    dest_addr = Column(String(64), nullable=False)
    state = Column(Integer, default=0, nullable=False)
    update_time = Column(Integer, default=0, nullable=False)


class ResExtendInfoTable(Base):
    """
    资源扩展表，KEY字段为scenario，并且值value为1表示为内置代理主机
    """
    __tablename__ = 'res_extend_info'
    uuid = Column(String(64), primary_key=True)
    resource_id = Column(String(64), nullable=False)
    key = Column(String(125), nullable=False)
    value = Column(Text, nullable=False)


class TClusterTarget(Base):
    __tablename__ = 't_cluster_target'
    cluster_id = Column(Integer, primary_key=True)
    cluster_name = Column(String(255))
    # 集群状态：27正常、28异常
    status = Column(Integer)
    cluster_ip = Column(String(255))
    cluster_port = Column(Integer)
    dest_vip = Column(String(255))
    remote_esn = Column(String(1024))
    username = Column(String(255))
    password = Column(String(255))
    verify_flag = Column(String(1))
    create_time = Column(BigInteger)
    last_update_time = Column(BigInteger)
    # 集群角色：复制（0）和 被管理（2）HostMigrateConstants.REPLICATION_CLUSTER_TYPE
    role = Column(Integer)
    enable_manage = Column(BOOLEAN)


class TClusterLocal(Base):
    __tablename__ = 't_cluster_local'
    cluster_id = Column(Integer, primary_key=True)
    cluster_name = Column(String(255))
    storage_esn = Column(String(1024))
    # 集群状态：27正常、28异常
    status = Column(Integer)
    cluster_ip = Column(String(255), nullable=False)
    user_name = Column(String(255))
    user_pwd = Column(String(255))
    create_time = Column(BigInteger)
    last_update_time = Column(BigInteger)
    used_capacity = Column(DECIMAL)
    capacity = Column(DECIMAL)
    db_service_ip = Column(String(255))


class UpgradeInfo(BaseModel):
    ip: str = Query(..., description="主机ip")
    port: str = Query(..., description="主机端口")
    download_link: str = Query(..., alias="downloadLink", description="下载链接")
    agent_id: str = Query(..., alias="agentId", description="AgentID")
    agent_name: str = Query(..., alias="agentName", description="Agent名称")
    connect_need_proxy: bool = Query(..., alias="canUseProxy", description="是否需要代理连接")
    certSecretKey: str = Query(..., alias="certSecretKey", description="证书密钥，已经加密")
    jobId: str = Query(..., alias="jobId", description="任务id")
    new_package_size: int = Query(..., alias="newPackageSize", description="新的安装包大小(kb)")
    packageType: str = Query(..., alias="packageType", description="安装命令")


class TDistributionStorage(Base):
    __tablename__ = 't_distribution_storage'
    uuid = Column(String(64), primary_key=True)
    name = Column(String(256), nullable=False)
    description = Column(String(1000), nullable=True)
    type = Column(String(256), nullable=False)
    cluster_id_list = Column(Text, nullable=True)
    storage_strategy_type = Column(Integer, nullable=True)
    timeout_period = Column(Integer, nullable=True)
    enable_parallel_storage = Column(Boolean, nullable=False)


class AgentTable(Base):
    __tablename__ = 't_host_agent_info'
    uuid = Column(String(64), primary_key=True)
    cpu_rate = Column(DECIMAL)
    mem_rate = Column(DECIMAL)
    last_update_time = Column(BigInteger)
    is_shared = Column(BOOLEAN)

