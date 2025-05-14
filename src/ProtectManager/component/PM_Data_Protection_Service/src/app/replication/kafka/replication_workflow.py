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
import uuid
from datetime import datetime

from app.backup.client.job_client import JobClient
from app.common import toolkit
from app.backup.client.protection_client import ProtectionClient
from app.backup.client.resource_client import ResourceClient
from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.common.constant import ProtectionConstant
from app.backup.schemas.policy import Schedule
from app.common import logger
from app.common.clients.alarm.alarm_after_failure import alarm_after_failure
from app.common.config import settings
from app.common.constants.constant import ReplicationConstants
from app.common.context.context import Context
from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobType, JobLogLevel
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.protected_object_enum import Status
from app.common.enums.schedule_enum import ExecuteType
from app.common.enums.sla_enum import TriggerEnum, PolicyTypeEnum, PolicyActionEnum, ReplicationTypeEnum, \
    ReplicationModeEnum, ReplicationStorageTypeEnum, BackupTypeEnum
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.events.consumer import EsEvent
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.kafka import update_task_log
from app.common.license import validate_license_by_resource_type
from app.common.toolkit import JobMessage
from app.copy_catalog.service.import_copy_service import query_copy_info_by_copy_id
from app.kafka import client
from app.protection.object.service.projected_copy_object_service import convert_protected_obj, \
    ProtectedCopyObjectService
from app.protection.object.models.projected_object import ProtectedObject
from app.replication.client.replication_client import ReplicationClient
from app.replication.service.replication_copy_service import is_reverse_replication, get_resource_obj
from app.resource.service.common import domain_resource_object_service
from app.resource.service.common.domain_resource_object_service import get_domain_id_list_by_resource_object_list
from app.resource.service.common.resource_service import query_local_cluster

log = logger.get_logger(__name__)
SCHEDULE_REPLICATION_TOPIC = "schedule.replication"
INITIALIZE_REPLICATION_TOPIC = "initialize.replication"
REPLICATION_COMPLETE_TOPIC = "replication.complete"
SLA_CHANGED_REQUEST = "SlaChangedRequest"


def is_start_time_effect(schedule: Schedule):
    start_time = schedule.start_time
    log.info(f"schedule start_time is: {start_time}")
    now = datetime.now()
    return now > start_time


@exter_attack
@client.topic_handler(
    SCHEDULE_REPLICATION_TOPIC
)
def schedule_replication(request: EsEvent, **payload):
    request_id = request.request_id
    resource_id = payload.get("resource_id")
    protected_obj = get_protect_obj(resource_id)
    log.info(f"[REPLICATION_TASK]:receive schedule.replication, request_id：{request_id}."
             f"resource_id:{resource_id}")
    if not protected_obj:
        log.info(f'protect object not exist, resource id is {resource_id}')
        return
    status = protected_obj.get("status")
    active = get_active_status(status=status, policy=payload.get("policy"),
                               execute_type=payload.get("execute_type", ExecuteType.AUTOMATIC.value))
    policy = payload.get("policy")
    if not check_sla_active(protected_obj):
        return
    sla_id = protected_obj.get("sla_id")
    sla = ProtectionClient.query_sla(sla_id)
    policy_uuid = policy.get("uuid")
    policy_model = [
        policy
        for policy in sla.get("policy_list")
        if policy["uuid"] == policy_uuid
    ]
    if policy_model and len(policy_model) == 1:
        policy = policy_model[0]
    schedule = Schedule.parse_obj(policy['schedule'])
    ext_parameters = policy.get("ext_parameters")
    copy_dict = ReplicationClient.query_copy_statistic(resource_id, policy)
    copy_format_map = copy_dict.get('copy_format_map', {})
    device_esn = copy_dict.get('device_esn', [])
    unit_id = copy_dict.get('unit_id', [])
    copy_id_esn_map = copy_dict.get('copy_id_esn_map', {})
    log.info(f"[pre-check]request-id: {request_id}, resource_id: {resource_id}, "
             f"active: {active}, copy_format_map:{copy_format_map}, device_esn:{device_esn}, unit_id:{unit_id}")
    if active and copy_format_map:
        resource_obj = payload.get("resource_obj")
        resource_obj = get_resource_obj(resource_obj, resource_id)
        payload["resource_obj"] = resource_obj
        current_operate_user_id = payload.get(BackupWorkflowConstants.CURRENT_OPERATE_USER_ID)
        user_id = current_operate_user_id if current_operate_user_id else resource_obj.get("user_id")
        # 增加extendField字段，包含SLA信息
        job_extend_params = {
            "sla_name": protected_obj.get("sla_name"),
            "sla_id": protected_obj.get("sla_id"),
            "policy_id": policy.get("uuid"),
            "multiClusterQueue": True
        }
        for esn in device_esn:
            # 域内复制且复制到指定备份存储单元，如果副本所在esn和复制指定备份存储单元一致，不复制
            copy_id_list = copy_id_esn_map.get(esn, [])
            domain_id_list = get_domain_id_list_by_resource_object_list(copy_id_list)
            log.info(f"Copy id list:{copy_id_list} in esn:{esn} relation domain list:{domain_id_list}")
            if check_copy_in_replica_backup_unit(esn, ext_parameters):
                continue
            param = {
                "copy_format_map": copy_format_map,
                "user_id": user_id,
                "resource_obj": resource_obj,
                "payload": payload,
                "ext_parameters": ext_parameters,
                "esn": esn,
                "request_id": request_id,
                "job_extend_params": job_extend_params,
                "protected_obj": protected_obj,
                "domain_id_list": domain_id_list
            }
            create_job_by_deploy_type(param, unit_id)


def create_job_by_deploy_type(param: {}, unit_id: []):
    for unit in unit_id:
        param["unit"] = unit
        create_job_for_format(param)


def create_job_for_format(param: {}):
    request_id = param.get("request_id", "")
    domain_id_list = param.get("domain_id_list", [])
    copy_format_map = param.get("copy_format_map", {})
    user_id = param.get("user_id", "")
    resource_obj = param.get("resource_obj", {})
    payload = param.get("payload", {})
    ext_parameters = param.get("ext_parameters", {})
    unit = param.get("unit", [])
    esn = param.get("esn", "")
    job_extend_params = param.get("job_extend_params", {})
    protected_obj = param.get("protected_obj", {})
    for copy_format, copy_format_id_list in copy_format_map.items():
        log.info(f'schedule_replication, copy_format:{copy_format}, copy_format_id_list:{copy_format_id_list},'
                 f' request_id:{request_id}')
        if not copy_format_id_list:
            continue
        JobClient.create_job(
            device_esn=esn,
            request_id=str(uuid.uuid4()),
            user_id=user_id,
            domain_id_list=domain_id_list,
            resource_obj=resource_obj,
            job_type=JobType.COPY_REPLICATION.value,
            message=JobMessage(
                topic=INITIALIZE_REPLICATION_TOPIC,
                payload={
                    **payload,
                    "protected_obj": protected_obj, "copy_type": copy_format,
                    "rep_mode": get_rep_mode(ext_parameters),
                    "unit": unit
                }
            ),
            enable_stop=True,
            job_extend_params=job_extend_params,
            target_name=None,
            target_location=None
        )


def check_copy_in_replica_backup_unit(esn: str, ext_parameters: dict):
    replication_target_mode = ext_parameters.get("replication_target_mode", ReplicationModeEnum.EXTRA)
    if not esn:
        log.info(f'check_copy_in_replica esn is null!')
        return True
    if replication_target_mode == ReplicationModeEnum.INTRA and \
            ReplicationStorageTypeEnum.BACKUP_STORAGE_UNIT == ext_parameters.get(
            "replication_storage_type", ReplicationStorageTypeEnum.BACKUP_STORAGE_UNIT_GROUP):
        if 'replication_storage_id' in ext_parameters and ext_parameters.get('replication_storage_id'):
            replication_storage_id = ext_parameters.get('replication_storage_id')
            if replication_storage_id == esn:
                return True
    return False


def get_active_status(status: int, policy: dict, execute_type: str):
    schedule = Schedule.parse_obj(policy['schedule'])
    active = is_start_time_effect(schedule) and (
            execute_type == ExecuteType.MANUAL.value or status == Status.Active.value
    )
    return active


def get_rep_mode(ext_parameters: dict):
    return ReplicationConstants.REP_MODE_ALL_COPY if ext_parameters.get(
        ReplicationConstants.REPLICATION_TARGET_TYPE,
        ReplicationTypeEnum.ALL_COPY) == ReplicationTypeEnum.ALL_COPY else ReplicationConstants.REP_MODE_SPECIFIED_COPY


def get_protect_obj(resource_id):
    copy_protected_dict = ProtectedCopyObjectService.query_copy_protected_dict(resource_id=resource_id)
    if copy_protected_dict:
        protected_obj = convert_protected_obj(copy_protected_dict)
    else:
        protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
    return protected_obj


def check_sla_active(protected_obj: ProtectedObject):
    sla_id = protected_obj.get("sla_id")
    sla = ProtectionClient.query_sla(sla_id)
    if sla is None or not sla.get("enabled"):
        log.info(f"sla {sla_id} is deactive, replication stop.")
        return False
    return True


@exter_attack
@client.topic_handler(
    INITIALIZE_REPLICATION_TOPIC,
    job_log=(
            "job_log_copy_replication_schedule_label",
            "job_status_{payload.job_status|context.job_status|status}_label"
    ),
    failure=REPLICATION_COMPLETE_TOPIC
)
def initialize_replication(request: EsEvent, **payload):
    request_id = request.request_id
    resource_id = payload.get("resource_id")
    log.info(f"replication workflow [initialize replication], request_id: {request_id}, resource_id:{resource_id}")
    policy = payload.get("policy")
    execute_type = payload.get("execute_type", ExecuteType.AUTOMATIC.value)
    resource_obj = payload.get("resource_obj")
    resource_obj = get_resource_obj(resource_obj, resource_id)
    payload["resource_obj"] = resource_obj
    validate_license_by_resource_type(FunctionEnum.REPLICATION, resource_obj.get("sub_type"))
    copy_id = payload.get("copy_id")

    context = Context(request_id)
    if is_reverse_replication(copy_id):
        log.info(f"[REVERSED REPLICATION] Reverse replicate copy: {copy_id}")
        context.set(BackupWorkflowConstants.COPY_ID, copy_id)
    else:
        if 'sla' in payload:
            sla = payload.get("sla")
        else:
            sla_id = payload.get("sla_id")
            sla = ProtectionClient.query_sla(sla_id=sla_id)
        protected_obj = payload.get("protected_obj")
        context.set(BackupWorkflowConstants.SLA, sla)
        context.set(BackupWorkflowConstants.PROTECTED_OBJECT, protected_obj)
    context.set(BackupWorkflowConstants.POLICY, policy)
    context.set(BackupWorkflowConstants.RESOURCE, resource_obj)
    context.set(BackupWorkflowConstants.EXECUTE_TYPE, execute_type)
    context.set("copy_type", payload.get("copy_type"))
    context.set("unit", payload.get("unit"))
    job_status = payload.get("job_status")
    log.info(f"request_id: {request_id}, job_status: {job_status}")
    return {
        "topic": "protection.replication",
        "message": payload
    }


@exter_attack
@client.topic_handler(
    REPLICATION_COMPLETE_TOPIC,
    job_log=(
            "job_log_copy_replication_complete_label",
            "job_status_{payload.job_status|context.job_status|status}_label"
    ),
    status="context.job_status",
    terminate=True
)
def replication_complete(request: EsEvent, **_):
    request_id = request.request_id
    context = Context(request_id)
    copy_id = context.get("copy_id")
    job_status = context.get("job_status")
    log.info(f"received message [replication.complete]. request_id: {request_id}, job_status: {job_status}")
    resource_obj = context.get(BackupWorkflowConstants.RESOURCE, dict)
    # 根据任务状态判断是否发送告警
    alarm_after_failure(context, job_status, resource_obj)
    if copy_id:
        log.info(f"[REVERSED REPLICATION] Copy:{copy_id} reversed replication no need to handle.")
        return
    sla = context.get(BackupWorkflowConstants.SLA, dict)
    sla_id = sla.get('uuid')
    if not sla_id:
        toolkit.modify_task_log(request_id, request_id, {
            "jobLogs": [{
                "jobId": request_id,
                "level": JobLogLevel.WARNING.value,
                "startTime": int(datetime.now().timestamp() * 1000),
                "logInfo": "job_log_replication_sla_lost_label"
            }]
        })
    resource_id = resource_obj.get('uuid', '')
    current_resource_obj = ResourceClient.query_resource(resource_id=resource_id)
    if current_resource_obj is not None and sla_id:
        current_sla_id = current_resource_obj.get("sla_id")
        if sla_id != current_sla_id:
            log.info(f"sla of resource({resource_id}) is changed. raw:{sla_id}, now:{current_sla_id}")
            event = CommonEvent("protection.change.event", None, resource_id=resource_id, sla_id=sla_id)
            producer.produce(event)
        return
    log.info(f"replication complete current resource obj is None.")
    event = CommonEvent("replication.need.deleted", None, resource_id=resource_id)
    producer.produce(event)
    log.info(f"replication complete end.request id: {request_id}, job status: {job_status}")


@exter_attack
@client.topic_handler("protection.backup.completed")
def protection_backup_complete(request: EsEvent, **payload):
    log.info(f"received topic [protection.backup.completed]. request id: {request.request_id}")
    sla = payload.get("sla")
    policy_list = sla.get("policy_list")
    if not payload.get("copy_ids", []):
        log.info("Skip replication copy list empty.")
        return
    copy_id = payload.get("copy_ids", [])[0]
    copy = query_copy_info_by_copy_id(copy_id)
    for policy in policy_list:
        policy_type = policy["type"]
        action = policy["action"]
        if PolicyActionEnum.replication_log.value == action and copy.backup_type != BackupTypeEnum.log.value:
            log.info(f"Copy:{copy_id} is not log, policy action is {action} can not do replication!")
            continue
        if PolicyActionEnum.replication.value == action and copy.backup_type == BackupTypeEnum.log.value:
            log.info(f"Copy:{copy_id} is log, policy action is {action} can not do replication!")
            continue
        schedule_trigger = policy["schedule"]["trigger"]
        if PolicyTypeEnum.replication.value == policy_type and TriggerEnum.backup_complete.value == schedule_trigger:
            message = {
                **payload,
                "policy": policy
            }
            event = CommonEvent(SCHEDULE_REPLICATION_TOPIC, **message)
            log.info(f"backup request id: {request.request_id}, replication request id: {event.request_id}")
            producer.produce(event)


def start_log():
    log.info('Started Replication Workflow.')


def get_backup_cluster_esn():
    local_device_esn = settings.get_key_from_config_map(ProtectionConstant.CLUSTER_CONFIG,
                                                        ProtectionConstant.CLUSTER_ESN)
    if local_device_esn == "":
        local_cluster = query_local_cluster()
        if not local_cluster:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                       message="The local cluster does not exist.")
        local_device_esn = local_cluster.storage_esn
    return local_device_esn
