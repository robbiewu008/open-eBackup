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
import ast
import datetime
import json
import uuid

from app.archive.client.archive_client import ArchiveClient
from app.backup.client.archive_client import ArchiveClient as sc
from app.archive.schemas.archive_request import ArchiveMsg, ArchiveRequest, ArchiveStorageInfo, StorageProtocolEnum
from app.archive.service.archive_scheduler import ArchiveMessage
from app.archive.service.archive_scheduler import ArchiveScheduler
from app.archive.service.check_policy import CheckPolicy, ImportCopy
from app.archive.service.constant import ArchiveConstant, ArchiveErrorCode, ArchiveErrorCodeDict
from app.common.clients.alarm.alarm_after_failure import alarm_after_failure
from app.common.config import settings
from app.common.deploy_type import DeployType
from app.common.enums.copy_enum import GenerationType, CopyFormatEnum
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.schedule_enum import ExecuteType
from app.common.event_messages.Rlm.rlm import LockRequest
from app.common.event_messages.event import EventBase
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.toolkit import modify_job_lock_id
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.models.req_param import NoArchiveReq
from app.copy_catalog.service.curd.copy_query_service import get_resource_latest_copy
from app.copy_catalog.service.import_copy_service import query_copy_info_by_copy_id
from app.copy_catalog.util.copy_util import check_exist_copy_job, is_worm_copy, get_present_full_copy, \
    is_archive_copy_exist
from app.kafka import client
from app.backup.client.job_client import JobClient
from app.backup.client.protection_client import ProtectionClient
from app.backup.client.resource_client import ResourceClient
from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.common.constant import ProtectionConstant
from app.backup.redis.context import Context
from app.backup.service.backup_service import unlock_resource_list
from app.backup.service.backup_workflow import status_switcher
from app.protection.object.service.projected_copy_object_service import ProtectedCopyObjectService
from app.restore.client.copy_client import CopyClient
from app.common import license
from app.common import logger
from app.common.enums.job_enum import JobStatus, JobType
from app.common.enums.protected_object_enum import Status
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum, ArchiveTypeEnum, ArchiveScope, BackupTypeEnum
from app.common.enums.sla_enum import TriggerEnum
from app.common.events import producer
from app.common.events.consumer import EsEvent
from app.common.redis_session import redis_session
from app.common.util.decorators.callbacks import error_callback
from app.common.util.retry.retryer import retry

log = logger.get_logger(__name__)

FILTER_EXIST_COPY_PREFIX = 'archive_'


@retry(exceptions=Exception, tries=5, wait=60, backoff=1, logger=log)
def archive_error_callback(request, **params):
    request_id = request.request_id
    log.info(f"[ARCHIVE_TASK]:archive error callback begin: request_id:{request_id}")
    # job_id和request_id一致
    job_id = request_id
    JobClient.update_job(request_id, job_id, status_switcher[0])
    unlock_resource_list(request_id, job_id)
    redis_session.delete(request_id)
    log.info(f"[ARCHIVE_TASK]:archive error callback end: request_id:{request_id}")


@exter_attack
@client.topic_handler(ProtectionConstant.TOPIC_BACKUP_SUCCESS)
def back_success_handler(request: EsEvent, **kwargs):
    request_id = request.request_id
    copy_ids = kwargs.get("copy_ids")
    resource_id = kwargs.get("resource_id")
    execute_type = kwargs.get("execute_type", ExecuteType.AUTOMATIC.value)
    log.info(f"[ARCHIVE_TASK]:start handle backup success message request_id:{request_id}, copy_ids:{copy_ids},"
             f"resource id: {resource_id}, execute_type:{execute_type}")
    is_process_archive = anti_ransomware_check(request_id, kwargs["sla"], resource_id, kwargs["user_id"],
                                               copy_ids, execute_type)
    log.info(f"[ARCHIVE_TASK]:anti ransomware check before archiving result is:{is_process_archive}, "
             f"copy_ides:{copy_ids}, resource id: {resource_id}")
    if is_process_archive:
        process_archive(kwargs["sla"], kwargs["resource_id"], kwargs["user_id"], copy_ids, kwargs["execute_type"])


@exter_attack
@client.topic_handler(ProtectionConstant.TOPIC_REPLICA_SUCCESS)
def replica_success_handler(request: EsEvent, **kwargs):
    request_id = request.request_id
    copy_id = kwargs["copy_id"]
    resource_id = kwargs["resource_id"]
    sla_id = kwargs["sla_id"]
    execute_type = kwargs.get("execute_type", ExecuteType.AUTOMATIC.value)
    log.info(f"[ARCHIVE_TASK]:start handle replica success: copy_id:{copy_id}, resource_id:{resource_id}")
    sla = ProtectionClient.query_sla(sla_id)
    is_process_archive = anti_ransomware_check(request_id, sla, resource_id, None, [copy_id],
                                               execute_type)
    log.info(f"[ARCHIVE_TASK]:anti ransomware check before archiving result is:{is_process_archive}, copy_id:{copy_id},"
             f" resource id: {resource_id}")
    if is_process_archive:
        process_archive(sla, resource_id, None, [copy_id], execute_type)


def fill_resource_info_from_copy(context, copy_info, job_id):
    if not copy_info:
        log.info(f"[ARCHIVE_TASK]:fill resource info job_id:{job_id}, copy_info is empty, skip.")
        return
    copy_resource_id = copy_info.get(BackupWorkflowConstants.RESOURCE_ID)
    copy_resource_name = copy_info.get(BackupWorkflowConstants.RESOURCE_NAME)
    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    resource_name = context.get(BackupWorkflowConstants.RESOURCE_NAME)
    log.info(f"[ARCHIVE_TASK]:fill resource info, resource_id:{resource_id}, resource_name:{resource_name},"
             f" copy_resource_id:{copy_resource_id}, copy_resource_name:{copy_resource_name}, job_id:{job_id}")
    if not resource_id:
        context.set(BackupWorkflowConstants.RESOURCE_ID, copy_resource_id)
    if not resource_name:
        context.set(BackupWorkflowConstants.RESOURCE_NAME, copy_resource_name)


@exter_attack
@client.topic_handler(ArchiveConstant.TASK_ARCHIVE_DONE_TOPIC)
def archive_done_handler(request, job_id, status, copy_id, auto_retry_times):
    request_id = request.request_id
    log.info(f"[ARCHIVE_TASK]:receive topic[protection.archive.done]: request_id:{request_id},copy_id:{copy_id}")
    conditions = {"uuid": copy_id}
    copy_info = CopyClient.query_copies(0, 1, conditions).get("items")
    copy_info = copy_info if not copy_info else copy_info[0]
    context = Context(request_id)
    fill_resource_info_from_copy(context, copy_info, job_id)
    alarm_after_failure(context, status)
    resource_id = context.get("resource_id")
    if auto_retry_times == 0 or status_switcher[status] == JobStatus.ABORTED or not copy_info:
        log.info(f"[ARCHIVE_TASK]:start to delete AUTO_RETRY_FLAG, resource_id：{resource_id}")
        redis_session.delete('AUTO_RETRY_FLAG' + resource_id)
    # 释放资源锁
    unlock_resource_list(request_id, job_id)
    JobClient.update_job(request_id, job_id, status_switcher[status])

    context = Context(request_id)

    policy = json.loads(context.get(BackupWorkflowConstants.POLICY))
    sla_name = context.get("sla_name")
    context.delete_all()
    if not copy_info:
        return
    if status_switcher[status] == JobStatus.SUCCESS or status_switcher[status] == JobStatus.PARTIAL_SUCCESS:
        process_archive_success(copy_info, policy)
    elif status_switcher[status] == JobStatus.FAIL:
        process_archive_failed(auto_retry_times, policy, copy_info, sla_name)
    else:
        process_archive_aborted(policy, copy_info)


def process_archive_success(copy_info, policy):
    copy_id = copy_info.get("uuid")
    log.info(f"[ARCHIVE_TASK] Archive copy:{copy_id} success")
    ext_parameters = policy.get("ext_parameters", None)
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}")
    delete_import_copy = None if not ext_parameters else ext_parameters.get("delete_import_copy", None)
    resource_id = copy_info["resource_id"]
    # 只针对导入副本
    if copy_info.get("resource_sub_type") == ResourceSubTypeEnum.ImportCopy.value and delete_import_copy:
        ImportCopy(copy_info).delete_copy()
    else:
        ArchiveClient.create_copy_archive_map(resource_id, storage_id, copy_id)
    # 获取该资源下一个待归档的副本
    is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
    archive_copy = get_next_archive_copy(resource_id, storage_id, is_query_log_copy)
    if archive_copy is not None:
        submit_archive_schedule(TriggerEnum.backup_complete, archive_copy["param"], archive_copy["copy"],
                                storage_id)
    return


def process_archive_failed(auto_retry_times, policy, copy_info, sla_name):
    resource_id = copy_info["resource_id"]
    ext_parameters = policy.get("ext_parameters", None)
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}")
    if auto_retry_times == 0:
        # 获取该资源下一个待归档的副本
        archive_copy = get_next_archive_copy(resource_id, storage_id, is_query_log_copy)
        # 获取当前副本链中所有副本，将失败副本之后的副本都跳过归档，待下一个归档周期再进行归档
        copy_dependency = ArchiveClient.query_dependency_copy([copy_info.get('uuid', '')])
        if copy_dependency and isinstance(copy_dependency, list):
            if archive_copy.get("copy", {}).get("uuid", "") in copy_dependency[0].get("dependencyCopyUuidList", []):
                log.info(
                    f"Copy:{copy_info.get('uuid', '')} archive fail!,"
                    f" dependency copy:{archive_copy.get('copy', {}).get('uuid', '')} skip current archive schedule")
                return
        if archive_copy is not None:
            submit_archive_schedule(TriggerEnum.backup_complete, archive_copy["param"], archive_copy["copy"],
                                    storage_id)
        return
    delay_minutes = ext_parameters.get("auto_retry_wait_minutes")
    start_date = (datetime.datetime.now() + datetime.timedelta(minutes=delay_minutes)).strftime(
        ProtectionConstant.DATE_TIME_FORMATTER)
    resource_sub_type = copy_info["resource_sub_type"]
    resource_type = copy_info["resource_type"]
    schedule_param = {
        "copy_id": copy_info["uuid"],
        "resource_id": resource_id,
        "policy": json.dumps(policy),
        "resource_sub_type": resource_sub_type,
        "resource_type": resource_type,
        "auto_retry_times": auto_retry_times - 1,
        "sla_name": sla_name,
        "storage_id": storage_id,
        "auto_retry_flag": True,
        "start_date": start_date
    }
    log.info(f"[ARCHIVE_TASK]:start to auto retry")
    schedule_id = ArchiveScheduler.create_delay_schedule(schedule_param)
    if schedule_id is not None:
        # 针对高级备份的副本归档， 只有当自动重试次数结束后，才可以进行下一次的归档，此处来标记自动重试
        log.info(f"[ARCHIVE_TASK]:start to auto retry")
        redis_session.set('AUTO_RETRY_FLAG' + resource_id, "AUTO_RETRY_FLAG")


def process_archive_aborted(policy, copy_info):
    resource_id = copy_info["resource_id"]
    ext_parameters = policy.get("ext_parameters", None)
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    log.info(
        f"[ARCHIVE_TASK]:start to delete all copies in redis:resource_id:{resource_id},storage_id:{storage_id}")
    redis_session.hdel(resource_id, storage_id)
    return


def process_archive(sla_info, resource_id, user_id, copy_id_list=None, execute_type=ExecuteType.AUTOMATIC.value):
    policy_list = sla_info.get("policy_list")
    archiving_policy_list = [policy for policy in policy_list if policy.get("type") == PolicyTypeEnum.archiving.value]
    if len(archiving_policy_list) == 0:
        return
    process_archive_by_policy(archiving_policy_list, resource_id, sla_info, user_id, copy_id_list, execute_type)


def process_archive_by_policy(policy_list, resource_id, sla_info, user_id, copy_id_list=None,
                              execute_type=ExecuteType.AUTOMATIC.value):
    for policy in policy_list:
        is_log_archive = False
        if copy_id_list:
            copy_dict = query_copy_info_by_copy_id(copy_id_list[0]).__dict__
            ext_parameters = policy.get("ext_parameters")
            is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
            log.info(f"[ARCHIVE] check copy:{copy_id_list} can archive:{is_query_log_copy}.")
            is_log_archive = is_allow_archive(copy_dict) or is_query_log_copy
        schedule_type = policy.get("schedule").get("trigger")
        if schedule_type == TriggerEnum.interval:
            continue
        if not is_log_archive:
            continue
        params = {
            "resource_id": resource_id,
            "sla_id": sla_info["uuid"],
            "policy": policy,
            "schedule_type": schedule_type,
            "user_id": user_id,
            "execute_type": execute_type
        }
        message = ArchiveMessage('schedule.archiving', request_id=str(uuid.uuid4()), params=params)
        producer.produce(message)


def anti_ransomware_check(request_id, sla_info, resource_id, user_id, copy_ids, execute_type):
    log.info(f"Start anti ransomware check before archiving, copy ids: {copy_ids}, resource id: {resource_id}")
    policy_list = sla_info.get("policy_list")
    archiving_policy_list = [policy for policy in policy_list if policy.get("type") == PolicyTypeEnum.archiving.value]
    if len(archiving_policy_list) == 0:
        return True
    protected_obj = None
    for policy in policy_list:
        schedule_type = policy.get("schedule", {}).get("trigger")
        archive_target_type = policy.get("ext_parameters", {}).get("archive_target_type")
        archiving_scope = policy.get("ext_parameters", {}).get("archiving_scope")
        # 备份后立即完成和归档最新副本才去执行勒索检测
        if (schedule_type != TriggerEnum.backup_complete or archive_target_type != ArchiveTypeEnum.all_copy
                or archiving_scope != ArchiveScope.latest):
            continue
        if protected_obj is None:
            protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
        enable_security_archive = get_ext_parameters_from_protected_obj(sla_info, resource_id).get(
            "enable_security_archive", False)
        if enable_security_archive:
            params = {
                "resource_id": resource_id,
                "sla_id": sla_info["uuid"],
                "policy": policy,
                "schedule_type": schedule_type,
                "user_id": user_id,
                "copy_ids": copy_ids,
                "execute_type": execute_type
            }
            message = ArchiveMessage('schedule.archiving.anti.ransomware.check',
                                     request_id=str(uuid.uuid4()), params=params)
            log.info(f"End anti ransomware check before archiving, copies:{copy_ids} send "
                     f"message to anti ransomware, resource id: {resource_id}")
            producer.produce(message)
            return False
    return True


def get_ext_parameters_from_protected_obj(sla, resource_id):
    if sla.get("application") == ResourceSubTypeEnum.Replica:
        copy_protected_obj = ProtectedCopyObjectService.query_copy_protected_dict(resource_id=resource_id)
        if not copy_protected_obj:
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, copy protected object is none")
        else:
            return copy_protected_obj.get("ext_parameters", {})
    else:
        protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
        if not protected_obj:
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, protected object is none")
        else:
            return protected_obj.get("ext_parameters", {})
    return {}


def anti_ransomware_check_result(request, **params):
    sla_id = params.get('slaId')
    copy_id = params.get('copyId')
    sla = ProtectionClient.query_sla(sla_id)
    user_id = params.get('userId')
    resource_id = params.get('resourceId')
    is_detected = params.get('isDetected')
    execute_type = params.get('execute_type', ExecuteType.AUTOMATIC.value)
    log.info(f"[ARCHIVE_TASK] Start handle anti ransomware check result message. resource id: {resource_id},"
             f",is_detected:{is_detected}, execute_type:{execute_type}")
    if is_detected:
        process_archive(sla, resource_id, user_id, [copy_id], execute_type)
    else:
        log.info(f"[ARCHIVE_TASK] Anti ransomware check result is not detected, no need archive,"
                 f" resource id: {resource_id}")


def get_next_archive_copy(resource_id, storage_id, is_query_log_copy):
    log.info(f"[ARCHIVE_TASK]:start to get next archive copy.resource_id:{resource_id}, storage_id:{storage_id}")
    if redis_session.hexists(resource_id, storage_id):
        context = ast.literal_eval(redis_session.hget(resource_id, storage_id))
        if len(context) > 0:
            archive_copy = context.pop(0)
            copy_context = ast.literal_eval(redis_session.hget(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id))
            if copy_context and archive_copy["copy"] in copy_context:
                copy_context.remove(archive_copy["copy"])
                redis_session.hset(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id, str(copy_context))
            redis_session.hset(resource_id, storage_id, str(context))
            req_param = NoArchiveReq(resource_id=resource_id, generated_by=None,
                                     storage_id=storage_id, copy_id=archive_copy["copy"], user_id=None,
                                     is_query_log_copy=is_query_log_copy)
            copy_info = ArchiveClient.get_no_archive_copy_list(req_param)
            del context
            del copy_context
            if len(copy_info) == 1:
                return {"copy": copy_info[0], "param": archive_copy["param"]}
            else:
                return get_next_archive_copy(resource_id, storage_id, is_query_log_copy)
        else:
            redis_session.hdel(resource_id, storage_id)
            redis_session.hdel(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id)
            log.info(f"[ARCHIVE_TASK]:there is no copy archiving. resource_id:{resource_id},storage_id:{storage_id}")
            return None
    else:
        log.info(f"[ARCHIVE_TASK]:there is no copy archiving. resource_id:{resource_id}, storage_id:{storage_id}")
        return None


def deal_batch_copy(copy_list_sorted, schedule_param):
    resource_id = schedule_param["resource_id"]
    ext_parameters = json.loads(schedule_param["policy"]).get("ext_parameters")
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}")
    if redis_session.hexists(resource_id, storage_id):
        if not redis_session.hexists(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id):
            redis_session.hset(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id, str([]))
        copy_context = ast.literal_eval(redis_session.hget(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id))
        param_context = ast.literal_eval(redis_session.hget(resource_id, storage_id))
        for copy in copy_list_sorted:
            if copy["uuid"] not in copy_context:
                copy_context.append(copy["uuid"])
                param_context.append({"copy": copy["uuid"], "param": schedule_param})
        log.info(f"[ARCHIVE_TASK]:copy nums:{len(copy_context)}, param num:{len(param_context)}")
        redis_session.hset(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id, str(copy_context))
        redis_session.hset(resource_id, storage_id, str(param_context))
    else:
        copy_queue = list()
        param_queue = list()
        for copy in copy_list_sorted:
            copy_queue.append(copy["uuid"])
            param_queue.append({"copy": copy["uuid"], "param": schedule_param})
        log.info(f"[ARCHIVE_TASK]:create new copy queue, copy nums:{len(copy_queue)}, param nums:{len(param_queue)}")
        redis_session.hset(FILTER_EXIST_COPY_PREFIX + resource_id, storage_id, str(copy_queue))
        redis_session.hset(resource_id, storage_id, str(param_queue))


@error_callback(callback=archive_error_callback, logger=log)
def add_resource_lock(request, **params):
    log.info(f"[ARCHIVE_TASK]:add lock [params]: {params}")
    context = Context(request.request_id)
    copy_id = context.get("copy_id")
    job_id = context.get("job_id")
    context.set("request_id", job_id)
    msg = LockRequest(
        request_id=request.request_id,
        resources=[{
            "id": copy_id,
            "lock_type": "r"
        }],
        wait_timeout=-1,
        priority=2,
        lock_id=job_id,
        response_topic=ArchiveConstant.ARCHIVE_LOCK_RESPONSE
    )
    producer.produce(msg)
    log.info(
        f"[ARCHIVE_TASK]:send topic：{ArchiveConstant.ARCHIVE_LOCK_RESPONSE} end, job_id:{job_id},copy_id:{copy_id}")


# 检查是否加锁成功，发送kafka消息
@error_callback(callback=archive_error_callback, logger=log)
def start_archive(request, error_desc, status):
    log.info(f"[ARCHIVE_TASK]:receive topic [Archive_LockResponse],error_desc:{error_desc}")
    request_id = request.request_id
    context = Context(request_id)
    job_id = context.get("job_id")
    resource_sub_type = context.get("resource_sub_type")
    if status != "success":
        log.warning(f"[ARCHIVE_TASK]:archive workflow [resource locked], lock failed")
        JobClient.update_job(request_id, job_id, status_switcher[0])
        unlock_resource_list(request_id, job_id)
        redis_session.delete(request.request_id)
        return

    context.set('lock_id', job_id)
    modify_job_lock_id(context)
    check_result = license.validate_license_by_resource_type(FunctionEnum.ARCHIVE, resource_sub_type, job_id,
                                                             request_id, strict=False)
    if not check_result:
        log.warning(f"[ARCHIVE_TASK]:archive workflow [check license], license unavailable")
        JobClient.update_job(request_id, job_id, status_switcher[0])
        unlock_resource_list(request_id, job_id)
        redis_session.delete(request.request_id)
        return
    log.info(f'[ARCHIVE_TASK]:Send ARCHIVE Request To DataMover')
    message = EventBase(request_id=request.request_id, default_publish_topic='protection.archive')
    producer.produce(message)


def is_allow_create_archive_job(resource_id: str, storage_id: str, copy_id: str, is_query_log_copy: bool):
    copy = query_copy_info_by_copy_id(copy_id)
    if not copy:
        log.error(f"Copy: {copy_id} not exist.")
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   error_message="Copy not exist.")
    req_param = NoArchiveReq(resource_id=resource_id, generated_by=copy.generated_by,
                             storage_id=storage_id, copy_id=copy_id, user_id=None,
                             is_query_log_copy=is_query_log_copy)
    no_archive_copy_list = ArchiveClient.get_no_archive_copy_list(req_param)
    if copy.generated_by == GenerationType.BY_REPLICATED.value:
        req_param = NoArchiveReq(resource_id=resource_id, generated_by=GenerationType.BY_CASCADED_REPLICATION.value,
                                 storage_id=storage_id, copy_id=copy_id, user_id=None,
                                 is_query_log_copy=is_query_log_copy)
        cascaded_copy_list = ArchiveClient.get_no_archive_copy_list(req_param)
        no_archive_copy_list.extend(cascaded_copy_list)
    if not no_archive_copy_list:
        log.info(
            f"resource_id:{resource_id}, storage_id:{storage_id} copy_id:{copy_id} is not allow create archive job")
        return False
    return True


def prepare_archive(param: dict):
    prepare_res = None
    log.info(f"[ARCHIVE_TASK]:start to pre-check archive job")
    resource_id = param["resource_id"]
    storage_id = param["storage_id"]
    copy_id = param["copy_id"]
    req_param = NoArchiveReq(resource_id=resource_id, generated_by=None,
                             storage_id=storage_id, copy_id=copy_id, user_id=None,
                             is_query_log_copy=True)
    copy_info = ArchiveClient.get_no_archive_copy_list(req_param)
    if copy_info is None or len(copy_info) != 1:
        log.info(f"[ARCHIVE_TASK]:copy has already archived：{copy_id},task cancelled")
        return {
            "status": "cancelled",
            "cause": [ArchiveErrorCode.CANCELLED_ERROR_CODE.value]
        }
    if redis_session.exists("AUTO_RETRY_FLAG" + resource_id) == 1:
        return {
            "status": "pending"
        }
    return prepare_res


def is_protected_obj_and_active(sla, resource_id, execute_type):
    if sla.get("application") == ResourceSubTypeEnum.Replica:
        copy_protected_obj = ProtectedCopyObjectService.query_copy_protected_dict(resource_id=resource_id)
        if not copy_protected_obj:
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, copy protected object is none")
            return False
        if not copy_protected_obj.get("protected_status", False):
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, copy protected object status is inactive")
            return False
    else:
        protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
        if not protected_obj:
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, protected object is none")
            return False
        if (protected_obj.get("status", Status.Inactive.value) != Status.Active.value
                and execute_type != ExecuteType.MANUAL.value):
            log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, protected object status is inactive")
            return False
    return True


def handle_schedule_archiving(msg):
    params = msg.params
    request_id = msg.request_id
    log.info(f"[ARCHIVE_TASK]:receive schedule.archiving, request_id：{request_id}."
             f"resource_id:{params['resource_id']}")
    resource_id = params["resource_id"]
    sla = ProtectionClient.query_sla(params["sla_id"])
    if sla is None:
        log.warning(f"[ARCHIVE_TASK]:sla：{params['sla_id']} not exist")
        return
    log.info(f"[INTERVAL_ARCHIVE]:resource_id:{resource_id},application:{sla.get('application')}")
    execute_type = params.get("execute_type", ExecuteType.AUTOMATIC.value)
    if not is_protected_obj_and_active(sla, resource_id, execute_type):
        return
    generated_by = GenerationType.get_generated_by_sub_type(sla.get("application"))
    ext_parameters = params["policy"].get("ext_parameters")
    log.info(
        f"[INTERVAL_ARCHIVE] request_id：{request_id}, support log archive:{ext_parameters.get('log_archive')}")
    #  查出所有未归档的副本
    copy_list_archive = query_no_archive_copy(resource_id, generated_by, ext_parameters, None, sla)
    if len(copy_list_archive) < 1:
        log.info(f"[ARCHIVE_TASK]:there is no copy archiving")
        return
    auto_retry_times = (ext_parameters.get("auto_retry_times") if ext_parameters.get("auto_retry") else 0)
    params["policy"]["sla_id"] = params["sla_id"]
    schedule_param = {
        "resource_id": resource_id,
        "policy": json.dumps(params["policy"]),
        "auto_retry_times": auto_retry_times,
        "storage_id": ext_parameters.get("storage_id"),
        "storage_list": ext_parameters.get("storage_list"),
        "sla_name": sla.get("name")
    }
    schedule_type = (
        TriggerEnum.backup_complete if not params.__contains__("schedule_type") else params["schedule_type"])
    if schedule_type == TriggerEnum.after_backup_complete:
        delay_days = params.get("policy").get("schedule").get("interval")
        schedule_param["start_date"] = (
                datetime.datetime.now() + datetime.timedelta(hours=delay_days * 24)).strftime(
            ProtectionConstant.DATE_TIME_FORMATTER)
    if ext_parameters.get("archive_target_type", ArchiveTypeEnum.all_copy) == ArchiveTypeEnum.all_copy:
        all_copy(ext_parameters, copy_list_archive, schedule_param, resource_id, schedule_type)
    else:
        specified_copy(ext_parameters, copy_list_archive, schedule_param, resource_id, schedule_type)


def interval_archive(request, **params):
    log.info(f"[ARCHIVE_TASK] Interval archive start. request id: {request.request_id}")
    sla_id = params.get("sla_id")
    sla = ProtectionClient.query_sla(sla_id)
    if sla is None or not sla.get("enabled"):
        log.info(f"sla {sla_id} is deactive, archive stop.")
        return
    archive_msg = ArchiveMsg(
        request_id=request.request_id,
        params=params
    )
    if DeployType().is_dependent():
        handle_schedule_archiving(archive_msg)
    else:
        ArchiveClient.dispatch_archive(archive_msg)


def is_have_log_copy(copy_list_archive):
    for copy in copy_list_archive:
        if copy.get('backup_type') == BackupTypeEnum.log.value:
            return True
    return False


def all_copy(ext_parameters, copy_list_archive, schedule_param, resource_id, schedule_type):
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}")
    if ext_parameters.get("archiving_scope") == ArchiveScope.all_no_archiving:
        log.info(f"[ARCHIVE_TASK] Get archive policy:{ArchiveScope.all_no_archiving}.")
        schedule_need_archive_copy(copy_list_archive, schedule_param, ext_parameters, resource_id, schedule_type)
    else:
        log.info(f"[ARCHIVE_TASK] Get specified copy archive policy.")
        copy_info_latest = copy_list_archive[-1]
        copy_info_latest_uuid = copy_info_latest.get('uuid')
        resource_last_copy_id = get_resource_latest_copy(copy_info_latest.get("resource_id"),
                                                         copy_info_latest.get("generated_by"))
        if resource_last_copy_id != copy_info_latest_uuid:
            log.warning(f"[ARCHIVE_TASK] {copy_info_latest_uuid} is not last copy.")
            return
        if copy_info_latest.get('backup_type') == BackupTypeEnum.log.value:
            need_archive_copy_list = get_copy_chain_when_log_archive(copy_list_archive, copy_info_latest)
            schedule_need_archive_copy(need_archive_copy_list, schedule_param, ext_parameters, resource_id,
                                       schedule_type)
        else:
            submit_archive_schedule(schedule_type, schedule_param, copy_info_latest, storage_id)


def schedule_need_archive_copy(need_archive_copy_list, schedule_param, ext_parameters, resource_id, schedule_type):
    deal_batch_copy(need_archive_copy_list, schedule_param)
    is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    archive_copy = get_next_archive_copy(resource_id, storage_id, is_query_log_copy)
    if archive_copy is None:
        return
    copy = archive_copy.get("copy", None)
    if copy is None:
        return
    submit_archive_schedule(schedule_type, schedule_param, copy, storage_id)


def get_copy_chain_when_log_archive(copy_list_archive, copy_info_latest):
    """
    查出所有未归档副本
    1：找到最新未归档的日志副本，判断最新未归档副本是否是资源最新日志副本，是则归档副本日志副本链上的所有副本。
    2：如果最新的副本是全量副本，判断最新未归档副本是否是资源最新副本，是则归档副本链上的所有副本。
    """
    present_full_copy = get_present_full_copy(copy_info_latest)
    start_gn = present_full_copy.gn
    end_gn = int(copy_info_latest.get("gn"))
    return [copy for copy in copy_list_archive if start_gn <= int(copy.get("gn")) <= end_gn]


def add_non_full_copy(copy_dict, non_full_copy_uuid_list, result):
    # 如果存在非全量副本
    if non_full_copy_uuid_list:
        for non_full_copy_uuid in non_full_copy_uuid_list:
            present_full_copy = get_present_full_copy(copy_dict.get(non_full_copy_uuid))
            if present_full_copy and is_archive_copy_exist(present_full_copy):
                result.append(copy_dict.get(non_full_copy_uuid))


def get_copy_when_archive_specify_copy(copy_list_archive, specified_scope):
    """
    查出所有未归档副本
    如果有全量：
    最早一个全量之前的所有副本都是可能需要归档的副本，找到这些副本依赖的全量副本，判断全量副本是否已经归档且归档副本是否存在，若都满足则将这些副本加到队列中
    全量以及全量之后的所有副本都是需要归档的副本，加到队列中
    没有全量：
    查到副本关联的全量副本，判断全量副本是否已经归档且归档副本是否存在，若都满足则将这些副本加到队列中
    """
    copies = [copy.get("uuid", "") for copy in copy_list_archive]
    log.info(f"Start to get need to archive copy from copies:{copies}")
    # 格式{uuid:index}
    full_copy_dict = dict()
    copy_dict = {copy.get("uuid", ""): copy for copy in copy_list_archive}
    for index, copy in enumerate(copy_list_archive):
        if copy.get("backup_type") == 1:
            full_copy_dict.update({copy.get("uuid"): index})
    result = []
    if full_copy_dict:
        # 获取全量副本的UUID列表
        full_copy_uuid_list = list(full_copy_dict.keys())
        full_copy_list = [copy_dict.get(copy_uuid) for copy_uuid in full_copy_uuid_list]
        check_policy = CheckPolicy(specified_scope)
        copy_filters = check_policy.filter_copies(full_copy_list)
        if copy_filters:
            start_index = full_copy_dict.get(copy_filters[0].get('uuid'))
            # 结束索引应在过滤后最后一个全量的下一个全量位置，若没有下一个全量。则结束索引为空
            filter_full_copy_uuid = copy_filters[-1].get('uuid')
            filter_full_copy_index = full_copy_uuid_list.index(filter_full_copy_uuid)
            end_index = full_copy_dict.get(full_copy_uuid_list[filter_full_copy_index + 1]) \
                if filter_full_copy_index < len(full_copy_uuid_list) - 1 else None
            # 获取非全量副本UUID列表（副本在第一个全量副本之前）
            non_full_copy_list = copy_list_archive[0:start_index]
        else:
            # 过滤出来没有全量副本，开始索引和结束索引都取0
            start_index = 0
            end_index = 0
            non_full_copy_list = copy_list_archive[0:full_copy_dict.get(full_copy_uuid_list[0])]
        non_full_copy_uuid_list = [copy.get("uuid", "") for copy in non_full_copy_list]
        # 如果存在非全量副本
        add_non_full_copy(copy_dict, non_full_copy_uuid_list, result)

        # 处理剩下的全量以及非全量副本，获取需要的副本列表
        latest_and_after_copy_list = copy_list_archive[start_index:end_index]

        # 将结果添加到最终结果中
        result.extend(latest_and_after_copy_list)
    else:
        for copy in copy_list_archive:
            present_full_copy = get_present_full_copy(copy)
            if present_full_copy and is_archive_copy_exist(present_full_copy):
                result.append(copy)
    log.info(f"End to get need to archive copy:{[copy.get('uuid', '') for copy in result]}")
    return result


def specified_copy(ext_parameters, copy_list_archive, schedule_param, resource_id, schedule_type):
    log.info(f"[ARCHIVE_TASK] Get archive policy.")
    specified_scope = ext_parameters.get("specified_scope", [])
    check_policy = CheckPolicy(specified_scope)
    is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
    if is_query_log_copy:
        copy_filters = get_copy_when_archive_specify_copy(copy_list_archive, specified_scope)
    else:
        copy_filters = check_policy.filter_copies(copy_list_archive)

    if not copy_filters:
        log.info("[ARCHIVE_TASK] Can not find a copy.")
        return
    # 将资源对应的副本信息入对应的队列
    deal_batch_copy(copy_filters, schedule_param)
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}, is_query_log_copy:{is_query_log_copy}")
    archive_copy = get_next_archive_copy(resource_id, storage_id, is_query_log_copy)
    copy = archive_copy.get("copy", None)
    if copy is None:
        return
    submit_archive_schedule(schedule_type, schedule_param, copy, storage_id)


# 非原生的非全量的副本不允许归档 无效副本不允许归档
def is_allow_archive(copy):
    back_type = copy.get("backup_type")
    status = copy.get("status")
    copy_format = json.loads(copy.get("properties")).get("format")
    if status in ArchiveConstant.NOT_ALLOW_ARCHIVE_COPY_STATUS:
        return False
    if back_type == BackupTypeEnum.full.value or copy_format is None:
        return True
    if copy_format in [CopyFormatEnum.INNER_SNAPSHOT.value, CopyFormatEnum.EXTERNAL.value]:
        return True
    return False


def submit_archive_schedule(schedule_type: TriggerEnum, schedule_param, copy, storage_id=None):
    job_status = [JobStatus.READY.value, JobStatus.PENDING.value, JobStatus.RUNNING.value,
                  JobStatus.ABORTING.value, JobStatus.SUCCESS.value]
    if check_exist_copy_job(copy["uuid"], [str(JobType.ARCHIVE.value)], job_status, storage_id):
        return
    log.info(f"[ARCHIVE_TASK]:start to submit archive schedule,schedule_type:{schedule_type}")
    schedule_param["copy_id"] = copy["uuid"]
    schedule_param["resource_sub_type"] = copy["resource_sub_type"]
    schedule_param["resource_type"] = copy["resource_type"]
    schedule_param["gn"] = copy["gn"]
    # 下发调度任务时，将resource_name从copy信息中取出，提前放到上下文中
    schedule_param["resource_name"] = copy["resource_name"]

    archive_schedule_task_switcher.get(schedule_type)(schedule_param)


def query_no_archive_copy(resource_id, generated_by, ext_parameters, user_id, sla):
    log.info(f"[ARCHIVE_TASK]:start to query no-archived copies")
    copy_list = []
    storage_id = None if not ext_parameters else ext_parameters.get("storage_id", None)
    if storage_id is None:
        return copy_list
    log.info(f"[ARCHIVE_TASK] local storage id: {storage_id}")
    is_query_log_copy = None if not ext_parameters else ext_parameters.get("log_archive", False)
    req_param = NoArchiveReq(resource_id=resource_id, generated_by=generated_by, storage_id=storage_id, copy_id=None,
                             user_id=user_id, is_query_log_copy=is_query_log_copy)
    copy_list = ArchiveClient.get_no_archive_copy_list(req_param)
    if generated_by == GenerationType.BY_REPLICATED.value:
        req_param = NoArchiveReq(resource_id=resource_id, generated_by=GenerationType.BY_CASCADED_REPLICATION.value,
                                 storage_id=storage_id, copy_id=None, user_id=user_id,
                                 is_query_log_copy=is_query_log_copy)
        cascaded_copy_list = ArchiveClient.get_no_archive_copy_list(req_param)
        copy_list.extend(cascaded_copy_list)
    log.info(f"[ARCHIVE_TASK] copy_list len: {len(copy_list)}")
    if DeployType().is_dependent():
        # 需要按存储单元过滤
        local_copy_list = copy_list
    else:
        local_esn = get_backup_cluster_esn()
        log.info(f"[ARCHIVE_TASK] local esn is: {local_esn}")
        local_copy_list = list(filter(lambda item: item["device_esn"] == local_esn, copy_list))
    log.info(f"[ARCHIVE_TASK] local_copy_list len: {len(local_copy_list)}")
    if local_copy_list is None or len(local_copy_list) < 1:
        log.info(f"[ARCHIVE_TASK]:there is no copy archiving")
        return []
    new_copy_list = []
    if sla.get("application") not in [ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE, ResourceSubTypeEnum.TDSQL_CLUSTER,
                                      ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP, ResourceSubTypeEnum.TDSQL]:
        for copy in local_copy_list:
            if is_allow_archive(copy):
                new_copy_list.append(copy)
    else:
        new_copy_list = local_copy_list
    copy_list_sorted = sorted(new_copy_list, key=lambda item: item["gn"])
    log.info(f"[ARCHIVE_TASK]:query no-archived copies len:{len(copy_list_sorted)}")
    copy_list_sorted = handle_no_archive_copy_with_copy_infected(resource_id, copy_list_sorted, sla)
    return copy_list_sorted


def handle_no_archive_copy_with_copy_infected(resource_id, copy_list_sorted, sla):
    if len(copy_list_sorted) < 1:
        return copy_list_sorted

    enable_security_archive = get_ext_parameters_from_protected_obj(sla, resource_id).get(
        "enable_security_archive", False)
    if enable_security_archive is False:
        log.info(f"[ARCHIVE_TASK]:resource_id:{resource_id}, enable security archive option is false")
        return copy_list_sorted

    log.info(f"[ARCHIVE_TASK]:resource_id:{resource_id}, enable security archive option is true")

    copy_list = [copy.get("uuid") for copy in copy_list_sorted]
    res_map = CopyClient().query_copy_infected(copy_list)
    if res_map is None:
        return copy_list_sorted
    filter_copy_list = []
    infected_copy_id_list = []
    for copy in copy_list_sorted:
        if res_map.get(copy.get("uuid"), False):
            infected_copy_id_list.append(copy.get("uuid"))
        else:
            filter_copy_list.append(copy)
    if infected_copy_id_list:
        log.info(f"These copies are infected, can not archive. copy ids: {infected_copy_id_list}, "
                 f"resource_id:{resource_id}")
    return filter_copy_list


def query_root_resource(resource_id):
    resource = ArchiveClient.query_resource(resource_id)
    if resource is None:
        log.warning(f"[ARCHIVE_TASK]:resource_id:{resource_id}, resource not exist")
        return None
    if resource.get("uuid") == resource.get("root_uuid"):
        return resource
    root_resource = ArchiveClient.query_resource(resource.get("root_uuid"))
    if root_resource is None:
        log.warning(f"[ARCHIVE_TASK]:root_resource_id:{resource_id}] root resource not exist")
        return None
    return root_resource


def get_backup_cluster_esn():
    return settings.get_esn_from_sys_base()


# 手动归档前检查副本是否可以归档
def check_archive_copy(archive_request, copy):
    if not copy:
        log.error(f"Copy: {archive_request.copy_id} not exist.")
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   error_message="Copy not exist.")
    back_type = copy.backup_type
    status = copy.status
    copy_format = json.loads(copy.properties).get("format")
    # 状态为无效的副本不允许归档
    if status in ArchiveConstant.NOT_ALLOW_ARCHIVE_COPY_STATUS:
        log.error(f"Copy: {archive_request.copy_id} status: {status} is invalid, cannot be archived.")
        raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR,
                                   error_message="Copy is invalid.")
    # 非原生非全量副本不允许归档 增量,差异和日志副本不允许手动归档
    if back_type != BackupTypeEnum.full.value and copy_format == CopyFormatEnum.INNER_DIRECTORY.value:
        log.error(f"Copy: {archive_request.copy_id} is not a full copy or a native copy, cannot be archived.")
        raise EmeiStorBizException(error=CommonErrorCodes.NOT_A_FULL_OR_NATIVE_COPY,
                                   error_message="The copy is not a full copy or a native copy.")


def manual_archive(archive_request):
    copy = query_copy_info_by_copy_id(archive_request.copy_id)
    check_archive_copy(archive_request, copy)
    copy_exist_list = []
    job_exist_list = []
    tape_index_list = []
    archive_storage_list = []
    for storage in archive_request.storage_list:
        storage_info = sc().query_storage_info(storage_id=storage.storage_id)
        if not storage_info:
            log.error(f"Storage: {storage.storage_id} not exist.")
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       error_message="Storage not exist.")
        if (storage.protocol == StorageProtocolEnum.TAPE and archive_request.auto_index
                and copy.resource_sub_type not in ArchiveConstant.APPLICATION_SUPPORT_TAPE_AUTO_INDEX):
            log.info(f"Copy:{archive_request.copy_id}, Resource sub type: {copy.resource_sub_type}, "
                     f"Tape: {storage.storage_id} archive not support auto index!")
            tape_index_list.append({"copy": archive_request.copy_id, "storage": storage.storage_id})
            raise EmeiStorBizException(CommonErrorCodes.APPLICATION_NOT_SUPPORT_TAPE_ARCHIVE_AUTO_INDEX,
                                       copy.resource_sub_type,
                                       error_message="Application not support tape archive auto index.")

        req_param = NoArchiveReq(resource_id=copy.resource_id, generated_by=copy.generated_by,
                                 storage_id=storage.storage_id, copy_id=copy.uuid, user_id=None,
                                 is_query_log_copy=False)
        no_archive_copy_list = ArchiveClient().get_no_archive_copy_list(req_param)
        storage_name = storage_info.get('storageName', '') or storage_info.get('mediaSetName', '')
        if not no_archive_copy_list:
            log.info(f"Copy: {archive_request.copy_id} has archive to storage: {storage_name}.")
            copy_exist_list.append({"copy": archive_request.copy_id, "storage": storage_name})
            continue
        archive_flag, new_copy_list = is_skip_archive(no_archive_copy_list)
        if archive_flag:
            continue
        # 利用redis单线程机制，处理手动归档时并发执行的问题.创建任务后再清理redis
        lock_key = f"{copy.uuid + '_' + storage.storage_id}"
        if not redis_session.set(lock_key, 1, nx=True, ex=10):
            log.info(f"Copy: {copy.uuid} archiving to storage: {storage_name}, archive fail!")
            job_exist_list.append({"copy": archive_request.copy_id, "storage": storage_name})
            continue
        archive_storage_list.append({"copy": new_copy_list, "storage": storage})
    if copy_exist_list:
        archived_storage_list = [item.get("storage") for item in copy_exist_list if "storage" in item]
        raise EmeiStorBizException(CommonErrorCodes.COPY_HAS_ARCHIVED_TO_STORAGE,
                                   archive_request.copy_id, ','.join(archived_storage_list),
                                   error_message="The copy has archived to storage.")
    if job_exist_list:
        running_storage_list = [item.get("storage") for item in job_exist_list if "storage" in item]
        raise EmeiStorBizException(CommonErrorCodes.ARCHIVE_TO_STORAGE_JOB_EXISTED,
                                   archive_request.copy_id, ','.join(running_storage_list),
                                   error_message="Archive to storage job existed.")
    start_archive_schedule(archive_request, archive_storage_list, copy.resource_id)


def start_archive_schedule(archive_request, archive_storage_list, resource_id):
    for item in archive_storage_list:
        storage = item.get("storage")
        new_copy_list = item.get("copy")
        schedule_param = {
            "manual_archive": True,
            "resource_id": resource_id, "policy": json.dumps(get_manual_policy(archive_request, storage)),
            "auto_retry_times": 0,
            "storage_list": json.dumps([item.__dict__ for item in archive_request.storage_list]),
            "storage_id": storage.storage_id
        }
        submit_archive_schedule(TriggerEnum.backup_complete, schedule_param, new_copy_list[0],
                                storage.storage_id)


def is_skip_archive(no_archive_copy_list: list):
    copy = no_archive_copy_list[0]
    new_copy_list = filter_no_archive_copy(no_archive_copy_list)
    if not new_copy_list:
        log.info(f"Copy: {copy.get('uuid', '')} can not archive.")
        return True, []
    return False, new_copy_list


def get_manual_policy(archive_request: ArchiveRequest, storage: ArchiveStorageInfo):
    if storage.protocol is StorageProtocolEnum.TAPE and archive_request.auto_index:
        log.error(f"Copy: {archive_request.copy_id} not support index")
    policy = {
        "type": "archiving",
        "action": "archiving",
        "retention": {
            "retention_type": archive_request.retention_type,
            "retention_duration": archive_request.retention_duration,
            "duration_unit": archive_request.duration_unit
        },
        "ext_parameters": {
            "qos_id": archive_request.qos_id,
            "auto_index": archive_request.auto_index,
            "network_access": archive_request.network_access,
            "storage_id": storage.storage_id,
            "protocol": storage.protocol,
            "retention_type": archive_request.retention_type,
            "duration_unit": archive_request.duration_unit,
            "retention_duration": archive_request.retention_duration,
            "storage_list": json.dumps([item.__dict__ for item in archive_request.storage_list]),
            "driverCount": archive_request.driver_count
        }
    }
    return policy


def filter_no_archive_copy(no_archive_copy_list: []):
    new_copy_list = []
    for copy in no_archive_copy_list:
        if is_allow_archive(copy):
            new_copy_list.append(copy)
    return new_copy_list


archive_schedule_task_switcher = {
    TriggerEnum.after_backup_complete: ArchiveScheduler.create_delay_schedule,
    TriggerEnum.backup_complete: ArchiveScheduler.create_immediate_schedule
}
