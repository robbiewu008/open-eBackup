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
import datetime
import json
import math
import uuid
from typing import List

from sqlalchemy.orm import Query

from app.backup.client.rbac_client import RBACClient
from app.common.deploy_type import DeployType
from app.common import logger
from app.common.enums.copy_enum import GenerationType
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.event_messages.Flows.backup import BackupDone
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.extension import to_orm
from app.common.license import validate_license_by_resource_type
from app.common.redis_session import redis_session
from app.common.schemas.common_schemas import UuidObject
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo
from app.common.toolkit import json2list, complete_job_center_task
from app.copy_catalog.client.cluster_client import check_is_need_to_forward, send_create_index_forward_request
from app.copy_catalog.common.common import GenIndexType
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.common.copy_topics import CopyTopic
from app.copy_catalog.models.tables_and_sessions import database, CopyTable, CopyProtectionTable, CopyArchiveMapTable, \
    ReplicatedCopiesTable
from app.copy_catalog.schemas import CopyInfoSchema, CopySchema, CopyProtectionSchema, CopyArchiveMapSchema, \
    ReplicatedCopiesSchema
from app.copy_catalog.service.curd.copy_query_service import check_copy_whether_exist
from app.copy_catalog.service.redis_service import get_context_value, get_job_id, build_retention_info, \
    build_resource_info, build_sla_info
from app.copy_catalog.util.copy_util import get_copy_default_name
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id

log = logger.get_logger(__name__)


def save_copies(request_id: str):
    copies = json2list(get_context_value(request_id, "copy_info"))
    copy_ids = []
    if len(copies) == 0:
        log.error(f"There is no copy. request_id={request_id}")
        message = BackupDone(copy_ids, request_id, get_job_id(request_id), 0)
        producer.produce(message)
        return
    try:
        for copy in copies:
            copy_id = save_copy(copy, request_id)
            if copy_id:
                copy_ids.append(copy_id)
        status = 1
    except Exception as _:
        log.exception(f"save copy to database failed.(request_id={request_id})")
        status = 0
    finally:
        pass

    response_topic = get_context_value(request_id, "response_topic")
    response_topic = response_topic if response_topic else "protection.backup.done"
    message = CommonEvent(response_topic,
                          request_id=request_id,
                          job_id=get_job_id(request_id),
                          copy_ids=copy_ids,
                          status=status)
    producer.produce(message)


def save_copy(copy: dict, request_id):
    new_copy_id_in_db = None
    display_timestamp = copy.pop('display_timestamp')
    timestamp = datetime.datetime.fromtimestamp(float(display_timestamp) / 1000)
    copy['display_timestamp'] = timestamp
    chain_id = copy.get("chain_id")
    if not copy.get("user_id") and not copy.get("resource_properties"):
        resource_properties = json.loads(copy.get("resource_properties"))
        copy["user_id"] = resource_properties.get("user_id")
    with database.session() as session:
        count = session.query(CopyTable.uuid).filter(
            CopyTable.chain_id == chain_id,
            CopyTable.timestamp == copy['timestamp']
        ).count()
        if count > 0:
            return new_copy_id_in_db
        copy_id = str(uuid.uuid4())
        copy.update({
            "uuid": copy_id,
            "gn": CopyTable.copy_sequence.next_value(),
            "generation": max(copy.get("generation", 1), 1)
        })
        build_retention_info(copy, request_id, timestamp)
        build_resource_info(copy, request_id)
        build_sla_info(copy, request_id)
        _build_copy_chain_id(copy)
        insert_data = CopyTable(**copy)
        session.add(insert_data)
        check_copy_message = update_copy_link(chain_id, copy_id, insert_data)
        log.info(f"save copy success. copy_id:{copy_id}")
    send_copy_save_event(copy_id)
    if check_copy_message is not None:
        producer.produce(check_copy_message)
    job_id = redis_session.hget(request_id, "job_id")
    complete_job_center_task(copy_id, job_id, {
        "copyId": copy_id,
        "copyTime": math.floor(timestamp.timestamp() * 1000),
        "progress": 100
    })
    return copy_id


def _build_copy_chain_id(copy: CopyTable):
    if DeployType().is_cloud_backup_type():
        return
    if copy.generated_by != GenerationType.BY_BACKUP.value:
        return
    if copy.backup_type != BackupTypeEnum.full.value:
        return
    # 备份全量副本时更新chain
    chain_id = str(uuid.uuid4())
    origin_chain_id = copy.chain_id
    copy.chain_id = chain_id
    with database.session() as session:
        resource_id = copy.resource_id
        if resource_id is not None and len(resource_id):
            session.query(ProtectedObject).filter(ProtectedObject.resource_id == resource_id).update(
                {ProtectedObject.chain_id: chain_id})
    log.info(f"Update chain id: {chain_id}, copy uuid: {copy.uuid}, origin chain id: {origin_chain_id}")


def save_copy_domain_relation(copy_info):
    if DeployType().is_cyber_engine_deploy_type():
        domain_id_list = []
        if copy_info.user_id:
            domain_id = get_domain_id_by_user_id(copy_info.user_id)
            domain_id_list = [domain_id]
        log.info(f"domain_id_list:{domain_id_list}")
        resource_set_relation_info = ResourceSetRelationInfo(
            resource_object_id=copy_info.uuid,
            resource_set_type=ResourceSetTypeEnum.COPY.value,
            scope_module=ResourceSetScopeModuleEnum.COPY.value,
            sub_type=ResourceSubTypeEnum.COPY.value,
            domain_id_list=domain_id_list
        )
        RBACClient.add_resource_set_relation(resource_set_relation_info)
        log.warning(f"This is cyber engine environment, manually add copy:{copy_info.uuid} to domain relation list")


def save_copy_info(copy_info: CopyInfoSchema, override: bool = False):
    resource_properties = json.loads(copy_info.resource_properties)
    if not copy_info.user_id:
        copy_info.user_id = resource_properties.get("user_id")
    with database.session() as session:
        copy_obj_exist = check_copy_whether_exist(copy_info=copy_info, session=session)
        if copy_obj_exist:
            if override:
                session.delete(copy_obj_exist)
                session.flush()
            else:
                return UuidObject(uuid=copy_obj_exist.uuid)
        copy = to_orm(copy_info, CopyTable)
        if copy.generated_time is None:
            copy.generated_time = copy.display_timestamp
        if copy.name is None:
            copy.name = get_copy_default_name(copy.resource_name, copy.timestamp)
        copy.generation = max(copy.generation, 1)
        copy.resource_type = resource_properties.get("type")
        copy.resource_sub_type = resource_properties.get("sub_type")
        _build_copy_chain_id(copy)
        check_copy_message = update_copy_link(copy.chain_id, copy.uuid, copy)
        session.add(copy)
    if check_copy_message is not None:
        producer.produce(check_copy_message)
    save_copy_domain_relation(copy_info)
    send_copy_save_event(copy.uuid)
    return UuidObject(uuid=copy.uuid)


def update_copy_link(chain_id, copy_id, copy):
    with database.session() as session:
        query: Query = session.query(CopyTable).filter(CopyTable.chain_id == chain_id)
        query = query.order_by(CopyTable.timestamp.desc()).limit(1)
        prev_copy: CopyTable = query.one_or_none()
        message = None
        if prev_copy is not None:
            if prev_copy.deleted:
                prev_copy_id = prev_copy.prev_copy_id
                message = CommonEvent(CopyTopic.COPY_CHECK_TOPIC, copy_id=prev_copy.uuid)
            else:
                prev_copy_id = prev_copy.uuid
            if isinstance(copy, dict):
                copy["prev_copy_id"] = prev_copy_id
            else:
                setattr(copy, "prev_copy_id", prev_copy_id)
            if prev_copy_id:
                session.query(CopyTable).filter(CopyTable.uuid == prev_copy_id).update({
                    CopyTable.next_copy_id: copy_id
                })
    return message


def save_copy_list(copy_infos: List[CopySchema]):
    if len(copy_infos) == 0:
        return
    copies = []
    with database.session() as session:
        for copy_info in copy_infos:
            copy_info.generated_by = GenerationType.BY_IMPORTED.value
            # CopyTable 无prev_copy_gn、next_copy_gn字段
            del copy_info.prev_copy_gn
            del copy_info.next_copy_gn
            del copy_info.cluster_name
            copy = to_orm(copy_info, CopyTable)
            if copy.name is None:
                copy.name = get_copy_default_name(copy.resource_name, copy.timestamp)
            # 此处uuid保持不变
            copy.uuid = copy_info.uuid
            copies.append(copy)
            resource_set_relation_info = ResourceSetRelationInfo(
                resource_object_id=copy.uuid,
                resource_set_type=ResourceSetTypeEnum.COPY.value,
                scope_module=ResourceSetScopeModuleEnum.COPY.value,
                sub_type=ResourceSubTypeEnum.COPY.value,
            )
            RBACClient.add_resource_set_relation(resource_set_relation_info)
        session.add_all(copies)


def send_copy_save_event_if_need_forward(copy_id, gen_index=GenIndexType.AUTO):
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if check_is_need_to_forward(copy_obj.device_esn):
            log.info(f"copy is in remote device, try to forward request to remote esn: {copy_obj.device_esn}")
            send_create_index_forward_request(copy_id, copy_obj.device_esn)
            return
        status = copy_obj.status
        # 创建索引时，如果副本状态不正常，则抛出异常；（前端已拦截，后端加固）
        if status != CopyStatus.NORMAL:
            error_message = f'copy with id {copy_id} status is not normal'
            raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR, error_message=error_message)
    send_copy_save_event(copy_id, gen_index)


def send_copy_save_event(copy_id, gen_index=GenIndexType.AUTO):
    log.info(f"send_copy_save_event begin, copy_id={copy_id}")
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if not copy_obj:
            error_message = f'copy with id {copy_id} does not exist'
            raise EmeiStorBizException(CommonErrorCodes.OBJ_NOT_EXIST, error_message=error_message)
        generation_type = copy_obj.generated_by
        status = copy_obj.status
        resource_sub_type = copy_obj.resource_sub_type
        check_is_encrypted(copy_obj, resource_sub_type)
    if (generation_type == GenerationType.BY_LIVE_MOUNTE and gen_index == GenIndexType.AUTO) \
            or status != CopyStatus.NORMAL:
        return
    log.info(f"send_copy_save_event, copy_id={copy_id}, generation_type={generation_type}")
    if generation_type in [GenerationType.BY_REPLICATED.value, GenerationType.BY_CASCADED_REPLICATION.value]:
        protection = session.query(CopyProtectionTable) \
            .filter(CopyProtectionTable.protected_resource_id == copy_obj.resource_id).one_or_none()
        if protection is not None:
            log.info(f"send_copy_save_event. send message to copy.replica.success topic,"
                     f"copy_id = {copy_id}, resource_id = {copy_obj.resource_id},"
                     f"sla_id = {protection.protected_sla_id}")
            producer.produce(CommonEvent(topic=CopyTopic.COPY_REPLICA_SUCCESS, copy_id=copy_id,
                                         resource_id=copy_obj.resource_id,
                                         sla_id=protection.protected_sla_id))
        if gen_index == GenIndexType.AUTO:
            log.info(f"Copy {copy_id} is generated by replicated, dont create copy index automatically.")
            return
    # 只发送文件集、vmwware、cloudBackUp本地文件系统、nas文件系统、nas共享、HDFSFileSet、FC、HCS的索引请求
    if not is_support_index(resource_sub_type):
        return
    try:
        if not DeployType().is_cyber_engine_deploy_type():
            validate_license_by_resource_type(function=FunctionEnum.DATA_MANANGE_GLOBALE_SEARCH,
                                              resource=resource_sub_type)

    except EmeiStorBizException as es:
        if gen_index == GenIndexType.AUTO:
            log.info(f"license check failed {es}.")
            return
        raise es
    finally:
        pass
    producer.produce(CommonEvent(topic=CopyTopic.COPY_SAVE_EVENT, copy_id=copy_id, gen_index=gen_index))


def is_support_index(resource_sub_type):
    if resource_sub_type not in [ResourceSubTypeEnum.Fileset, ResourceSubTypeEnum.VirtualMachine,
                                 ResourceSubTypeEnum.CloudBackupFileSystem, ResourceSubTypeEnum.NasFileSystem,
                                 ResourceSubTypeEnum.NasShare, ResourceSubTypeEnum.HDFSFileset,
                                 ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.HCSCloudHost,
                                 ResourceSubTypeEnum.Volume, ResourceSubTypeEnum.OBJECT_SET,
                                 ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER, ResourceSubTypeEnum.APSARA_STACK_INSTANCE,
                                 ResourceSubTypeEnum.CNWARE_VM, ResourceSubTypeEnum.NUTANIX_VM,
                                 ResourceSubTypeEnum.HYPER_V_VM,
                                 ResourceSubTypeEnum.NdmpBackupSet, ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
                                 ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE, ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP,
                                 ResourceSubTypeEnum.MONGODB_SINGLE, ResourceSubTypeEnum.MONGODB_CLUSTER,
                                 ResourceSubTypeEnum.TiDB_CLUSTER, ResourceSubTypeEnum.TiDB_DATABASE,
                                 ResourceSubTypeEnum.TiDB_TABLE]:
        log.info(f"Resource Sub Type {resource_sub_type} doesn't support indexing.")
        return False
    return True


def check_is_encrypted(copy_obj, resource_sub_type):
    if resource_sub_type in [ResourceSubTypeEnum.HCSCloudHost]:
        # 加密盘不支持副本索引
        vol_list = json.loads(copy_obj.properties).get("volList")
        for vol in vol_list:
            system_encrypted = json.loads(vol.get("extendInfo")).get("systemEncrypted")
            if system_encrypted == "1":
                # 只要存在一个加密盘，则不能副本索引
                error_message = "Enc rypted disks do not support copy indexes."
                log.info(error_message)
                raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=error_message)


def create_copy_protection(copy_protection: CopyProtectionSchema):
    with database.session() as session:
        query = session.query(CopyProtectionTable) \
            .filter(CopyProtectionTable.protected_resource_id == copy_protection.protected_resource_id)
        protection = query.one_or_none()
        if protection is not None:
            query.update({
                CopyProtectionTable.protected_object_uuid: copy_protection.protected_object_uuid,
                CopyProtectionTable.protected_sla_id: copy_protection.protected_sla_id,
                CopyProtectionTable.protected_sla_name: copy_protection.protected_sla_name,
                CopyProtectionTable.protected_status: copy_protection.protected_status},
                synchronize_session=False)
        else:
            session.add(CopyProtectionTable(
                protected_resource_id=copy_protection.protected_resource_id,
                protected_object_uuid=copy_protection.protected_object_uuid,
                protected_sla_id=copy_protection.protected_sla_id,
                protected_sla_name=copy_protection.protected_sla_name,
                protected_status=copy_protection.protected_status
            ))


def generate_copy_archive_map(copy_archive_map: CopyArchiveMapSchema):
    with database.session() as session:
        archive_map_in_db = session.query(CopyArchiveMapTable).filter(
            CopyArchiveMapTable.copy_id == copy_archive_map.copy_id,
            CopyArchiveMapTable.storage_id == copy_archive_map.storage_id).one_or_none()
        if archive_map_in_db is None:
            session.add(CopyArchiveMapTable(
                copy_id=copy_archive_map.copy_id,
                storage_id=copy_archive_map.storage_id,
                resource_id=copy_archive_map.resource_id
            ))


def delete_copy_archive_map(copy_archive_map: CopyArchiveMapSchema):
    with database.session() as session:
        archive_map_in_db = session.query(CopyArchiveMapTable).filter(
            CopyArchiveMapTable.copy_id == copy_archive_map.copy_id,
            CopyArchiveMapTable.storage_id == copy_archive_map.storage_id).one_or_none()
        if archive_map_in_db:
            session.delete(archive_map_in_db)
            session.flush()


def create_replicated_copies(replicated_copies_schema: ReplicatedCopiesSchema):
    log.info(f"create_replicated_copies, copy_id:{replicated_copies_schema.copy_id},"
             f"esn:{replicated_copies_schema.esn}")
    with database.session() as session:
        replicated_copies = session.query(ReplicatedCopiesTable).filter(
            ReplicatedCopiesTable.copy_id == replicated_copies_schema.copy_id,
            ReplicatedCopiesTable.resource_id == replicated_copies_schema.resource_id,
            ReplicatedCopiesTable.esn == replicated_copies_schema.esn).one_or_none()
        if replicated_copies is None:
            session.add(ReplicatedCopiesTable(
                copy_id=replicated_copies_schema.copy_id,
                resource_id=replicated_copies_schema.resource_id,
                esn=replicated_copies_schema.esn
            ))
