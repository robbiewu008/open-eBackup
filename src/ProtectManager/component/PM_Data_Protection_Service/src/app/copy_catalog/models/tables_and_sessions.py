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
from sqlalchemy import (
    Column,
    Integer,
    BigInteger,
    String,
    DateTime,
    Sequence,
    SmallInteger,
    func,
    literal_column,
    BOOLEAN,
    Text, ForeignKey, DECIMAL, JSON
)
from sqlalchemy.dialects.postgresql import aggregate_order_by
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import aliased, relationship

from app.common.database import Database
from app.common.logger import get_logger
from app.common.type_decorator.time_type_decorator import TZDateTime
from app.copy_catalog.schemas import CopyResourceSummarySchema, CopyInfoQuerySchema

log = get_logger(__name__)

Base = declarative_base()


class CopyTable(Base):
    __tablename__ = 'copies'

    copy_sequence = Sequence('copy_generate_number', metadata=Base.metadata)
    uuid = Column(String(64), primary_key=True, index=True)
    chain_id = Column(String(128))
    timestamp = Column(String(64))
    display_timestamp = Column(TZDateTime)
    deletable = Column(BOOLEAN, default=True)
    deleted = Column(BOOLEAN, default=False)
    status = Column(String(32))  # Invalid, Normal, Restoring, Mounting, Deleting
    location = Column(String(1024))  # Local, etc...
    backup_type = Column(Integer, nullable=True)  # 1：完全备份 2：增量备份 3：差异备份 4：日志备份 6:快照备份
    generated_by = Column(String(32))  # Imported, Replicated, Backup, Archive, LiveMount
    generated_time = Column(TZDateTime)
    features = Column(Integer)  # 0: indexable, 1:restoreable, 2:instant-restoreable, 3:mountable
    indexed = Column(String(32), default="Unindexed")  # Unindexed, Indexed, Indexing
    gn = Column(BigInteger, copy_sequence, server_default=copy_sequence.next_value())  # generate number
    generation = Column(Integer, default=0)
    parent_copy_uuid = Column(String(64))
    prev_copy_id = Column(String(64), nullable=True)
    next_copy_id = Column(String(64), nullable=True)
    retention_type = Column(SmallInteger, nullable=False)
    retention_duration = Column(Integer)
    duration_unit = Column(String(4))
    expiration_time = Column(TZDateTime)
    properties = Column(Text, nullable=True)  # JSON
    resource_id = Column(String(64), nullable=False)
    resource_name = Column(String(512), nullable=False)
    resource_type = Column(String(64), nullable=False)
    resource_sub_type = Column(String(64), nullable=False)
    resource_location = Column(String(1024), nullable=False)
    resource_status = Column(String(32), nullable=False)
    resource_environment_name = Column(String(512), nullable=False)
    resource_environment_ip = Column(String(512), nullable=False)
    resource_properties = Column(Text, nullable=False)
    sla_name = Column(String(256), nullable=False)
    sla_properties = Column(Text, nullable=False)
    user_id = Column(String(255), nullable=True)
    is_archived = Column(BOOLEAN, default=False, nullable=True)
    is_replicated = Column(BOOLEAN, default=False, nullable=True)
    detail = Column(Text, nullable=True)
    name = Column(String(550), nullable=True)
    storage_id = Column(String(64), nullable=True)
    source_copy_type = Column(Integer, nullable=True)  # 1：完全备份 2：增量备份 3：差异备份 4：日志备份 6:快照备份
    worm_status = Column(Integer, default=1)  # 1：未设置，2：设置中，3：已设置，4：设置失败， 5：已过期
    device_esn = Column(String(256), nullable=True)  # 副本所在集群
    pool_id = Column(String(256), nullable=True)  # 副本所在存储单元
    origin_backup_id = Column(String(64), nullable=True)
    origin_copy_time_stamp = Column(TZDateTime, nullable=True)
    storage_unit_id = Column(String(64), nullable=True)
    storage_snapshot_flag = Column(BOOLEAN, default=False, nullable=True)
    extend_type = Column(String(128), nullable=True)  # 副本扩展信息字段。各应用可以自定义该参数值的含义，如AD域对应副本信息的备份数据字段。可用于副本筛选查询
    worm_validity_type = Column(Integer, default=1)  # worm有效期类型 1 同副本过期 2.自定义worm有效时间
    worm_retention_duration = Column(Integer)  # worm保留时间
    worm_duration_unit = Column(String(4))  # worm保留时间单位
    worm_expiration_time = Column(TZDateTime)  # worm过期时间
    storage_unit_status = Column(Integer, default=1)  # 副本所在的存储介质状态  备份 归档 复制 挂载
    browse_mounted = Column(String(32), nullable=True)  # Umount, Mounted, Mounting

    @classmethod
    def __relation__(cls):
        prev_copy_table = aliased(cls)
        next_copy_table = aliased(cls)

        def converter(prev_copy: cls, next_copy: cls, cluster: cls, storage_unit: cls):
            result = {}
            if prev_copy:
                result["prev_copy_id"] = prev_copy.uuid
                result["prev_copy_gn"] = prev_copy.gn
            if next_copy:
                result["next_copy_id"] = next_copy.uuid
                result["next_copy_gn"] = next_copy.gn
            if cluster:
                result["cluster_name"] = cluster.cluster_name
            if storage_unit:
                result["storage_unit_name"] = storage_unit.name
            return result

        return {
            "joins": {
                prev_copy_table: prev_copy_table.uuid == cls.prev_copy_id,
                next_copy_table: next_copy_table.uuid == cls.next_copy_id,
                ClusterMemberTable: ClusterMemberTable.remote_esn == cls.device_esn,
                StorageUnitTable: StorageUnitTable.id == cls.storage_unit_id,
            },
            "converter": converter,
            "orders": ["display_timestamp", "origin_copy_time_stamp", "uuid"],
            "filters": ['%location%', '%resource_name%', '%sla_name%', '%resource_location%',
                        '%name%', '!properties', '!resource_properties', '!sla_properties', '%storage_id%',
                        '%storage_unit_name%'],
            "schema": CopyInfoQuerySchema,
        }

    @classmethod
    def __resource_summary__(cls):
        copy_count = literal_column("copy_count")

        def field(name: str):
            return func.array_agg(aggregate_order_by(getattr(cls, name), CopyTable.display_timestamp.desc()))[1].label(
                name)

        return {
            "joins": {
                CopyProtectionTable: CopyProtectionTable.protected_resource_id == cls.resource_id
            },
            "fields": {
                "resource_id": cls.resource_id.label("resource_id"),
                "resource_name": field("resource_name"),
                "resource_type": field("resource_type"),
                "resource_sub_type": field("resource_sub_type"),
                "resource_location": field("resource_location"),
                "resource_status": field("resource_status"),
                "properties": field("properties"),
                "resource_properties": field("resource_properties"),
                "resource_environment_name": field("resource_environment_name"),
                "resource_environment_ip": field("resource_environment_ip"),
                "sla_name": field("sla_name"),
                "protected_resource_id": CopyProtectionTable.protected_resource_id,
                "protected_object_uuid": CopyProtectionTable.protected_object_uuid,
                "protected_sla_id": CopyProtectionTable.protected_sla_id,
                "protected_sla_name": CopyProtectionTable.protected_sla_name,
                "protected_status": CopyProtectionTable.protected_status,
                "copy_count": func.count(cls.uuid).label('copy_count'),
            },
            "alias": {
                "resource_id": literal_column("resource_id"),
                "%resource_name%": literal_column("resource_name"),
                "resource_type": literal_column("resource_type"),
                "resource_sub_type": literal_column("resource_sub_type"),
                "%resource_location%": literal_column("resource_location"),
                "resource_status": literal_column("resource_status"),
                "display_timestamp": literal_column("display_timestamp"),
                "%resource_environment_name%": literal_column("resource_environment_name"),
                "%resource_environment_ip%": literal_column("resource_environment_ip"),
                "%sla_name%": literal_column("sla_name"),
                "%protected_sla_name%": literal_column("protected_sla_name"),
                "protected_status": literal_column("protected_status"),
                "copy_count": copy_count,
                "protected_sla_id": literal_column("protected_sla_id"),
            },
            "initiator": lambda query: query.group_by(cls.resource_id),
            "converter": lambda po, **data: data if data else {},
            "orders": ["copy_count", "display_timestamp"],
            "filters": ['%resource_name%', '%resource_location%', '%sla_name%', '%resource_environment_name%',
                        '%resource_environment_ip%', '!properties', '!resource_properties', '!sla_properties',
                        '%protected_sla_name%', "protected_sla_id"],
            "schema": CopyResourceSummarySchema,
        }

    def __repr__(self):
        return (
            f"<Copy(id='{self.uuid}', chain_id='{self.chain_id}', timestamp='{self.timestamp}', gn='{self.gn}')>"
        )

    def as_dict(self):
        temp = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return temp


class ClusterMemberTable(Base):
    __tablename__ = 't_cluster_member'

    cluster_id = Column(Integer, primary_key=True, nullable=False)
    cluster_name = Column(String(64), nullable=False)
    status = Column(Integer, nullable=False)
    cluster_ip = Column(String(64))
    cluster_port = Column(Integer, nullable=False)
    remote_esn = Column(String(64), nullable=False)
    username = Column(String(64))
    password = Column(String(64))
    role = Column(Integer)
    net_plane_name = Column(String(255))
    net_plane_setting_status = Column(Integer)
    net_plane_setting_time = Column(DateTime)
    create_time = Column(BigInteger)
    last_update_time = Column(BigInteger)
    origin_net_plane_name = Column(String(255))

    def as_dict(self):
        cluster_member_table_dict = {col.name: getattr(self, col.name)
                                     for col in self.__table__.columns}
        return cluster_member_table_dict


class CopyProtectionTable(Base):
    __tablename__ = 'copies_protection'

    protected_resource_id = Column(String(64), primary_key=True, nullable=False)
    protected_object_uuid = Column(String(64), nullable=False)
    protected_sla_id = Column(String(64))
    protected_sla_name = Column(String(64))
    protected_status = Column(BOOLEAN, nullable=False, default=False)
    protected_type = Column(String(64), nullable=False)
    protected_sub_type = Column(String(64), nullable=False)
    protected_chain_id = Column(String(64), nullable=False)
    task_list = relationship('CopyProtectedTask', lazy="joined", cascade='all, delete-orphan', passive_deletes=True)
    ext_parameters = Column(JSON)

    def as_dict(self):
        copy_protected_object_dict = {col.name: getattr(self, col.name)
                                      for col in self.__table__.columns}
        return copy_protected_object_dict


class CopyProtectedTask(Base):
    __tablename__ = 'copy_protected_task'

    uuid = Column(String(64), primary_key=True)
    protected_resource_id = Column(String(64), ForeignKey(CopyProtectionTable.protected_resource_id), nullable=False)
    policy_id = Column(String(64), nullable=False)
    schedule_id = Column(String(64), nullable=False)


class CopyArchiveMapTable(Base):
    __tablename__ = 'copies_archive_map'

    copy_id = Column(String(64), primary_key=True, index=True)
    storage_id = Column(String(64), primary_key=True, index=True)
    resource_id = Column(String(64), nullable=False)


class ReplicatedCopiesTable(Base):
    __tablename__ = 'replicated_copies'

    copy_id = Column(String(64), primary_key=True, index=True)
    resource_id = Column(String(64), nullable=True)
    esn = Column(String(64), nullable=True)


class CopyRetentionTable(Base):
    __tablename__ = 'copy_retention'

    resource_id = Column(String(64), primary_key=True)
    retention_type = Column(SmallInteger, nullable=False)
    retention_duration = Column(Integer)
    duration_unit = Column(String(4))


class CopyAntiRansomwareTable(Base):
    __tablename__ = 'copies_anti_ransomware'

    copy_id = Column(String(64), primary_key=True, index=True)
    status = Column(Integer)  # UNDETECTED -1, PREPARE 0, DETECTING 1, UNINFECTING 2, INFECTING 3, ERROR 4
    model = Column(String(64), nullable=True)
    detection_start_time = Column(String(64), nullable=True)
    detection_end_time = Column(String(64), nullable=True)
    detection_duration = Column(Integer, nullable=True)
    report = Column(Text, nullable=True)
    tenant_id = Column(String(64), nullable=True)
    tenant_name = Column(String(512), nullable=True)
    total_file_size = Column(BigInteger, nullable=True)
    changed_file_count = Column(Integer, nullable=True)
    added_file_count = Column(Integer, nullable=True)
    deleted_file_count = Column(Integer, nullable=True)
    handle_detect_infect = Column(BOOLEAN, default=False)
    generate_type = Column(String(64), nullable=True)  # IO_DETECT, COPY_DETECT
    infected_file_count = Column(BigInteger, nullable=True)
    backup_software = Column(String(1024), nullable=True)
    infected_file_detect_duration = Column(BigInteger, nullable=True)


class AntiRansomwarePolicyResourceTable(Base):
    __tablename__ = 'anti_ransomware_policy_resource'

    id = Column(Integer, primary_key=True, index=True)
    policy_id = Column(Integer)
    schedule_id = Column(String(64), nullable=True)
    resource_id = Column(String(64))
    resource_name = Column(String(512))
    resource_sub_type = Column(String(64))
    resource_location = Column(String(1024))


class PolicyTable(Base):
    __tablename__ = 'policy'

    uuid = Column(String(64), primary_key=True, index=True)
    sla_id = Column(String(64), nullable=False)
    name = Column(String(128), nullable=False)
    type = Column(String(16), nullable=False)
    env_type = Column(String(16), nullable=True)
    object_type = Column(String(16), nullable=True)
    action = Column(String(32), nullable=True)
    active = Column(BOOLEAN, nullable=False)
    protection_mode = Column(String(16), nullable=True)
    ext_parameters = Column(JSON, nullable=True)
    worm_validity_type = Column(Integer, default=0)


class DistributionStorageUnitRelation(Base):
    __tablename__ = 't_distribution_storage_unit_relation'

    unit_id = Column(String(256), primary_key=True, index=True, nullable=False)
    distribution_id = Column(String(256), primary_key=True, index=True, nullable=False)
    available_capacity_ratio = Column(Integer, nullable=False)
    strategy_order = Column(Integer, nullable=True)


class AntiRansomwarePolicyTable(Base):
    __tablename__ = 'anti_ransomware_policy'

    id = Column(Integer, primary_key=True, index=True)
    policy_name = Column(String(256))
    description = Column(String(1000), nullable=True)
    schedule_interval = Column(Integer, nullable=True)
    schedule_interval_unit = Column(String(256), nullable=True)
    schedule_policy = Column(String(256))
    copy_time = Column(String(64), nullable=True)
    start_detection_time = Column(String(64), nullable=True)
    cluster_id = Column(Integer)
    data_source_type = Column(String(64))
    resource_sub_type = Column(String(64))
    detection_type = Column(Integer, nullable=True)
    set_worm = Column(BOOLEAN, default=False)
    need_detect = Column(BOOLEAN, default=True)


class ResourceAntiRansomwareTable(Base):
    __tablename__ = 'resource_anti_ransomware'

    id = Column(Integer, primary_key=True, index=True)
    resource_id = Column(String(64), nullable=False)
    status = Column(Integer)  # 最新的快照的最新检测状态, DETECTING 1, UNINFECTING 2, INFECTING 3
    copy_id = Column(String(64), nullable=True)
    copy_generated_time = Column(String(64), nullable=True)
    total_file_size = Column(BigInteger, nullable=True)
    changed_file_count = Column(Integer, nullable=True)
    added_file_count = Column(Integer, nullable=True)
    deleted_file_count = Column(Integer, nullable=True)
    copy_detected_time = Column(String(64), nullable=True)
    infected_file_count = Column(BigInteger, nullable=True)
    backup_software = Column(String(1024), nullable=True)
    generate_type = Column(String(64), nullable=True)


class StorageUnitTable(Base):
    __tablename__ = 't_storage_unit'

    id = Column(String(64), primary_key=True, index=True)
    name = Column(String(256), nullable=False)
    device_id = Column(String(64), nullable=False)
    pool_id = Column(String(64), nullable=False)
    device_type = Column(String(64), nullable=False)
    is_auto_added = Column(BOOLEAN, default=False)

    def as_dict(self):
        storage_unit_table_dict = {col.name: getattr(self, col.name)
                                   for col in self.__table__.columns}
        return storage_unit_table_dict


class ClusterTargetTable(Base):
    __tablename__ = 't_cluster_target'

    cluster_id = Column(Integer, primary_key=True, index=True)
    cluster_name = Column(String(256), nullable=False)
    status = Column(Integer, nullable=False)
    cluster_ip = Column(String(256), nullable=False)
    cluster_port = Column(Integer, nullable=False)
    dest_vip = Column(String(256), nullable=True)
    remote_esn = Column(String(1024), nullable=False)
    username = Column(String(256), nullable=False)
    password = Column(String(2048), nullable=False)
    verify_flag = Column(String(1), nullable=False)
    create_time = Column(BigInteger, nullable=False)
    last_update_time = Column(BigInteger, nullable=False)
    role = Column(Integer, nullable=True)
    enable_manage = Column(BOOLEAN, nullable=True)
    used_capacity = Column(DECIMAL, nullable=True)
    capacity = Column(DECIMAL, nullable=True, default=0)
    generated_type = Column(Integer, default=1)
    device_type = Column(String(64), nullable=False, default="")

    def as_dict(self):
        cluster_target_table_dict = {col.name: getattr(self, col.name)
                                     for col in self.__table__.columns}
        return cluster_target_table_dict


class SystemConfigEntity(Base):
    __tablename__ = "t_config"

    uuid = Column(String(64), nullable=False)
    key = Column(String(128), primary_key=True, nullable=False)
    value = Column(Text, nullable=True)


database = Database('protect_manager', Base)
