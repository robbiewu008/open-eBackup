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
import itertools
import json
import uuid
from typing import List

from sqlalchemy.orm import Session

from app.backup.client.job_client import JobClient
from app.common.clients.protection_client import ProtectionClient
from app.common.clients.scheduler_client import SchedulerClient
from app.common.constants.constant import ReplicationConstants
from app.common.constants.resource_type_mapping import SubTypeMapper
from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobType
from app.common.enums.protected_object_enum import Status
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.schedule_enum import ExecuteType, ScheduleTypes
from app.common.enums.sla_enum import PolicyTypeEnum, TriggerEnum, ReplicationTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.logger import get_logger
from app.common.toolkit import JobMessage
from app.copy_catalog.db.copies_db import count_by_user_id_and_resource_ids
from app.copy_catalog.models.tables_and_sessions import CopyProtectionTable, CopyProtectedTask
from app.copy_catalog.service.curd.copy_query_service import query_replicated_copy_by_resource_id, \
    query_last_copy_by_resource_id
from app.protection.object import db
from app.protection.object.common import db_config
from app.protection.object.models.projected_object import ProtectedObject
from app.protection.object.schemas.protected_copy_object import ProtectedCopyObjectUpdate, ManualReplicationReq, \
    ProtectedCopyBaseExtParam
from app.replication.client.replication_client import ReplicationClient
from app.resource.service.common import domain_resource_object_service
from app.resource.service.common.domain_resource_object_service import get_domain_id_list_by_resource_object_list

log = get_logger(__name__)


class ProtectedCopyObjectService(object):

    @staticmethod
    def create_protected_object(session: Session, create_req: ProtectedCopyObjectUpdate) -> str:
        copy = query_replicated_copy_by_resource_id(resource_id=create_req.resource_id)
        sla = ProtectionClient.query_sla(str(create_req.sla_id))
        copy_protection_object = db.copy_projected_object.query_one_by_resource_id(session=session,
                                                                                   resource_id=create_req.resource_id)
        check_copy_protect(copy, sla, copy_protection_object, create_req, True)
        # 创建副本保护对象及调度任务
        new_projected_object = build_copy_protection_object(copy, sla, create_req.ext_parameters)
        session.add(new_projected_object)
        return new_projected_object.protected_resource_id

    @staticmethod
    def modify_protection(session: Session, update_req: ProtectedCopyObjectUpdate) -> str:
        copy = query_replicated_copy_by_resource_id(resource_id=update_req.resource_id)
        copy_protection_object = db.copy_projected_object.query_one_by_resource_id(session=session,
                                                                                   resource_id=update_req.resource_id)
        sla = ProtectionClient.query_sla(str(update_req.sla_id))
        check_copy_protect(copy, sla, copy_protection_object, update_req, False)
        is_sla_changed = copy_protection_object.protected_sla_id != sla.get("uuid")
        if is_sla_changed:
            schedule_ids = list(task.uuid for task in copy_protection_object.task_list)
            SchedulerClient.batch_delete_schedules(schedule_ids)
            task_list = list(build_copy_protection_task(
                copy_protection_object.protected_resource_id,
                policy,
                "schedule." + policy.get("type"),
                {
                    "resource_id": copy_protection_object.protected_resource_id,
                    "resource_obj": get_cascaded_replication_resource(policy=policy, copy=copy),
                    "sla_id": sla.get("uuid"),
                    "chain_id": copy_protection_object.protected_chain_id,
                    "policy": policy,
                    "execute_type": ExecuteType.AUTOMATIC.value
                },
            ) for policy in sla.get("policy_list") if filter_sla_policy(sla, policy))
            copy_protection_object.task_list = task_list
            copy_protection_object.protected_sla_id = sla.get("uuid")
            copy_protection_object.protected_sla_name = sla.get("name")
            if update_req.ext_parameters:
                copy_protection_object.ext_parameters = update_req.ext_parameters.dict()
            session.add(copy_protection_object)
        else:
            if update_req.ext_parameters:
                copy_protection_object.ext_parameters = update_req.ext_parameters.dict()
                session.add(copy_protection_object)
        return copy_protection_object.protected_resource_id

    @staticmethod
    def batch_remove_protection(session: Session, resource_ids: List[str]) -> List[str]:
        copy_projected_object_list = db.copy_projected_object.query_by_resource_ids(session=session,
                                                                                    resource_ids=resource_ids)
        tasks = list(itertools.chain.from_iterable(list(obj.task_list for obj in copy_projected_object_list)))
        schedule_ids = list(task.uuid for task in tasks)
        SchedulerClient.batch_delete_schedules(schedule_ids)
        db.copy_projected_object.delete_by_condition(session=session, conditions=[
            CopyProtectionTable.protected_resource_id.in_(resource_ids)])
        return resource_ids

    @staticmethod
    def batch_activated(session: Session, resource_ids: List[str]):
        copy_projected_object_list = db.copy_projected_object.query_by_resource_ids(
            session=session, resource_ids=resource_ids)
        type_set = {
            copy_projected_object.protected_sub_type for copy_projected_object in copy_projected_object_list}
        if len(type_set) != 1:
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message="batch sub_type must consistent")
        db.copy_projected_object.update_status(
            session=session, resource_ids=resource_ids, protected_status=True)

    @staticmethod
    def batch_deactivate(session: Session, resource_ids: List[str]):
        copy_projected_object_list = db.copy_projected_object.query_by_resource_ids(
            session=session, resource_ids=resource_ids)
        type_set = {
            copy_projected_object.protected_sub_type for copy_projected_object in copy_projected_object_list}
        if len(type_set) != 1:
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message="batch sub_type must be consistent")
        db.copy_projected_object.update_status(
            session=session, resource_ids=resource_ids, protected_status=False)

    @staticmethod
    def sync_replica_sla_change(session: Session, sla_id: str):
        copy_obj_list = db.copy_projected_object.query_obj_by_sla_id(
            session=session, sla_id=sla_id)
        sla = ProtectionClient.query_sla(sla_id)
        for copy_projected_object in copy_obj_list:
            schedule_ids = list(task.uuid for task in copy_projected_object.task_list)
            SchedulerClient.batch_delete_schedules(schedule_ids)
            task_list = list(build_copy_protection_task(
                copy_projected_object.protected_resource_id,
                policy,
                "schedule." + policy.get("type"),
                {
                    "resource_id": copy_projected_object.protected_resource_id,
                    "sla_id": sla.get("uuid"),
                    "chain_id": copy_projected_object.protected_chain_id,
                    "policy": policy,
                    "execute_type": ExecuteType.AUTOMATIC.value
                },
            ) for policy in sla.get("policy_list") if filter_sla_policy(sla, policy))
            copy_projected_object.task_list = task_list
            copy_projected_object.sla_id = sla.get("uuid")
            copy_projected_object.sla_name = sla.get("name")
        session.add_all(copy_obj_list)

    @staticmethod
    def manual_replicate(user_id: str, manual_replicate_req: ManualReplicationReq, session: Session):
        """
        手动复制

        为资源创建复制任务；
        :param session: 数据库会话
        :param user_id: 用户id
        :param manual_replicate_req: 手动复制请求体
        """
        request_id = str(uuid.uuid4())
        log.info(f"Execute manual replication, resource id: {manual_replicate_req.resource_id}, "
                 f"request id: {request_id}, sla id: {manual_replicate_req.sla_id}")
        resource_obj = get_resource_obj_from_copy(manual_replicate_req.resource_id)
        copy_protected_obj = db.copy_projected_object.query_one_by_resource_id(
            session=session, resource_id=manual_replicate_req.resource_id)
        copy_protected_obj = copy_protected_obj.as_dict()
        if not copy_protected_obj:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS,
                                       "protected obj of " + manual_replicate_req.resource_id + " is null")
        payload = build_manual_replicate_payload(request_id=request_id, replicate_req=manual_replicate_req)
        policy = payload.get("policy")
        ext_parameters = payload.get("policy").get("ext_parameters")
        copy_dict = ReplicationClient.query_copy_statistic(
            manual_replicate_req.resource_id,
            policy)
        copy_format_map = copy_dict.get('copy_format_map', {})
        device_esn = copy_dict.get('device_esn', [])
        unit_id = copy_dict.get('unit_id', [])
        copy_id_esn_map = copy_dict.get('copy_id_esn_map', {})
        log.info(f'copy format:{copy_format_map}, copy esn:{device_esn}, unit_id:{unit_id}')
        if not copy_format_map:
            raise EmeiStorBizException(error=ResourceErrorCodes.MANUAL_REPLICATION_HAS_NO_COPIES,
                                       message=f"There are no copies that satisfy the replication rules.")
        for esn in device_esn:
            copy_id_list = copy_id_esn_map.get(esn, [])
            domain_id_list = get_domain_id_list_by_resource_object_list(copy_id_list)
            log.info(f"Copy id list:{copy_id_list} in esn:{esn} relation domain list:{domain_id_list}")
            param = {
                "copy_format_map": copy_format_map,
                "copy_protected_obj": copy_protected_obj,
                "user_id": user_id,
                "resource_obj": resource_obj,
                "payload": payload,
                "ext_parameters": ext_parameters,
                "manual_replicate_req": manual_replicate_req,
                "esn": esn,
                "request_id": request_id,
                "domain_id_list": domain_id_list
            }
            for unit in unit_id:
                param["unit"] = unit
                create_job_for_format(param)

    @staticmethod
    def query_copy_protected_dict(resource_id: str) -> dict:
        with db_config.get_session() as session:
            copy_projected_object = db.copy_projected_object.query_one_by_resource_id(session=session,
                                                                                      resource_id=resource_id)
            if copy_projected_object:
                return copy_projected_object.as_dict()
        return {}

    @staticmethod
    def count_by_sla_id(session: Session, sla_id: str) -> int:
        return db.copy_projected_object.count_obj_by_sla_id(session=session, sla_id=sla_id)

    @staticmethod
    def verify_protect_copy_objects_ownership(user_id: str, resource_uuid_list: List[str]):
        if not resource_uuid_list:
            return
        if not user_id:
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
        with db_config.get_session() as session:
            count = count_by_user_id_and_resource_ids(session=session, user_id=user_id,
                                                      resource_uuid_list=resource_uuid_list)
            log.info(f"count_by_user_id_and_resource_ids, count:{count}")
            if count == 0:
                raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)

    @staticmethod
    def query_copy_projected_object(session: Session) -> List:
        return db.copy_projected_object.query_copy_projected_object(session=session)


def convert_protected_obj(copy_protected_obj):
    status = Status.Active
    if not copy_protected_obj.get("protected_status"):
        status = Status.Inactive
    obj_params = {
        "uuid": copy_protected_obj.get("protected_resource_id"),
        "resource_id": copy_protected_obj.get("protected_resource_id"),
        "sla_id": copy_protected_obj.get("protected_sla_id"),
        "sla_name": copy_protected_obj.get("protected_sla_name"),
        "type": copy_protected_obj.get("protected_type"),
        "sub_type": copy_protected_obj.get("protected_sub_type"),
        "chain_id": copy_protected_obj.get("protected_chain_id"),
        "status": status
    }
    return ProtectedObject(**obj_params).as_dict()


def build_copy_protection_object(copy, sla, ext_parameters):
    resource_id = copy.get("resource_id")
    protected_chain_id = str(uuid.uuid4())
    sla_id = sla.get("uuid")
    if not ext_parameters:
        ext_parameters = ProtectedCopyBaseExtParam()
    obj_params = {
        "ext_parameters": ext_parameters.dict(),
        "protected_resource_id": resource_id,
        "protected_object_uuid": resource_id,
        "protected_sla_id": sla_id,
        "protected_sla_name": sla.get("name"),
        "protected_type": copy.get("resource_type"),
        "protected_sub_type": copy.get("resource_sub_type"),
        "protected_chain_id": protected_chain_id,
        "protected_status": True,
        "task_list": list(build_copy_protection_task(
            resource_id,
            policy,
            "schedule." + policy.get("type"),
            {
                "resource_id": resource_id,
                "resource_obj": get_cascaded_replication_resource(policy=policy, copy=copy),
                "sla_id": sla_id,
                "chain_id": protected_chain_id,
                "policy": policy,
                "execute_type": ExecuteType.AUTOMATIC.value
            },
        ) for policy in sla["policy_list"] if filter_sla_policy(sla, policy))
    }
    return CopyProtectionTable(**obj_params)


def get_cascaded_replication_resource(policy, copy):
    # 复制副本级联复制需要设置resource对象
    resource_obj = None
    if policy.get("type") == PolicyTypeEnum.replication.value:
        resource_obj = json.loads(copy.get("resource_properties"))
    return resource_obj


def build_copy_protection_task(obj_id, policy, topic: str, schedule_params: dict):
    """
    构造保护对象任务

    :param obj_id: 保护对象id
    :param policy: 对应sla中的策略
    :param topic: 保护任务对应的调度任务的topic
    :param schedule_params: 调度执行的参数
    :return:
    """
    task_id = str(uuid.uuid4())
    policy_schedule = policy.get("schedule")

    SchedulerClient.create_interval_schedule_new(
        task_id,
        str(policy_schedule.get("interval")) +
        policy_schedule.get("interval_unit"),
        ScheduleTypes.interval.value,
        policy_schedule.get("start_time"),
        topic,
        schedule_params
    )
    return CopyProtectedTask(
        uuid=task_id,
        policy_id=policy.get("uuid"),
        protected_resource_id=obj_id,
        schedule_id=task_id)


def filter_sla_policy(sla, policy) -> bool:
    if policy.get("type") not in [PolicyTypeEnum.archiving.value, PolicyTypeEnum.replication.value]:
        return False
    if sla.get("application") not in [ResourceSubTypeEnum.Replica]:
        return False
    if policy.get("schedule") is None:
        return False
    return policy.get("schedule").get("trigger") == TriggerEnum.interval


def get_resource_obj_from_copy(resource_id: str):
    copy = query_last_copy_by_resource_id(resource_id=resource_id, generated_by=None)
    if not copy:
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, f"resource: {resource_id} has no copy.")
    return json.loads(copy.as_dict().get("resource_properties"))


def build_manual_replicate_payload(request_id: str, replicate_req: ManualReplicationReq):
    sla = ProtectionClient.query_sla(sla_id=replicate_req.sla_id)
    if not sla:
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   error_message=f"sla of {replicate_req.sla_id} is null.")

    policy_list = sla.get("policy_list")
    selected_policy = [policy for policy in policy_list if policy.get("uuid") == replicate_req.policy_id]
    if len(selected_policy) == 0:
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   error_message=f"selected policy: {replicate_req.policy_id} is null.")
    payload = {
        "request_id": request_id,
        "resource_id": replicate_req.resource_id,
        "execute_type": ExecuteType.MANUAL.value,
        "sla_id": replicate_req.sla_id,
        "sla": sla,
        "policy": selected_policy[0]
    }
    return payload


def check_replicated_copy(copy: dict):
    if copy is None:
        raise EmeiStorBizException(error=ResourceErrorCodes.REPLICATION_PROTECT_CONDITION,
                                   message=f"Replication copies cannot be replication or archived in master")


def check_sla(sla: dict):
    if sla is None:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, message=f"sla not exist")
    sla_type = ResourceSubTypeEnum(sla.get("application")).value
    if sla_type != ResourceSubTypeEnum.Replica.value:
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS, message="SLA type is inconsistent")


def check_copy_support_archive(sla: dict, resource_sub_type: str):
    type_list = [policy['type'] for policy in sla.get("policy_list")]
    if PolicyTypeEnum.archiving.value in type_list and resource_sub_type not in SubTypeMapper.copy_archive_types:
        raise EmeiStorBizException(error=CommonErrorCodes.REP_COPY_CANNOT_ARCHIVE, message="rep copy can not archive.")


def check_copy_protect_status(copy_protection_object: CopyProtectionTable, is_create: bool):
    if is_create:
        if copy_protection_object is not None and copy_protection_object.protected_sla_id is not None:
            raise EmeiStorBizException(error=ResourceErrorCodes.RESOURCE_ALREADY_PROTECTED, message="copy is protected")
    else:
        if copy_protection_object is None:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, message="copy is not protected")


def check_copy_protect(copy: dict, sla: dict, copy_protection_object: CopyProtectionTable,
                       req: ProtectedCopyObjectUpdate, is_create: bool):
    check_replicated_copy(copy)
    check_sla(sla)
    check_copy_support_archive(sla, copy.get("resource_sub_type"))
    check_copy_protect_status(copy_protection_object, is_create)


def get_rep_mode(ext_parameters: dict):
    return ReplicationConstants.REP_MODE_ALL_COPY if ext_parameters.get(
        ReplicationConstants.REPLICATION_TARGET_TYPE,
        ReplicationTypeEnum.ALL_COPY) == ReplicationTypeEnum.ALL_COPY else ReplicationConstants.REP_MODE_SPECIFIED_COPY


def create_job_for_format(param: {}):
    request_id = param.get("request_id", "")
    domain_id_list = param.get("domain_id_list", [])
    copy_format_map = param.get("copy_format_map", {})
    user_id = param.get("user_id", "")
    resource_obj = param.get("resource_obj", {})
    payload = param.get("payload", {})
    ext_parameters = param.get("ext_parameters", {})
    unit = param.get("unit", [])
    manual_replicate_req = param.get("manual_replicate_req", ManualReplicationReq)
    esn = param.get("esn", "")
    copy_protected_obj = param.get("copy_protected_obj", {})
    for copy_format, copy_format_id_list in copy_format_map.items():
        log.info(f'manual replication, copy_format:{copy_format}, copy_format_id_list:{copy_format_id_list},'
                 f' request_id:{request_id}')
        if not copy_format_id_list:
            continue
        protected_obj = convert_protected_obj(copy_protected_obj)
        JobClient.create_job(request_id=request_id, user_id=user_id,
                             domain_id_list=domain_id_list,
                             resource_obj=resource_obj,
                             job_type=JobType.COPY_REPLICATION.value,
                             message=JobMessage(
                                 topic="initialize.replication",
                                 payload={
                                     **payload,
                                     "resource_obj": resource_obj,
                                     "protected_obj": protected_obj,
                                     "copy_type": copy_format, "rep_mode": get_rep_mode(ext_parameters),
                                     "unit": unit
                                 }
                             ),
                             job_extend_params={
                                 "multiClusterQueue": True,
                                 "slaId": manual_replicate_req.sla_id,
                                 "execute_type": ExecuteType.MANUAL.value
                             },
                             enable_stop=True, device_esn=esn, target_name=None,
                             target_location=None)
        request_id = str(uuid.uuid4())
