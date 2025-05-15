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
import copy
import datetime
import json
import time
from typing import List
import uuid

from app.backup.client.job_client import JobClient
from app.backup.client.protection_client import ProtectionClient
from app.backup.client.resource_client import ResourceClient
from app.backup.client.scheduler_client import SchedulerClient
from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.common.config.db_config import SessionLocal
from app.backup.common.constant import ProtectionConstant, ProtectionBackupJobSteps
from app.backup.common.validators.sla_validator import ParamsValidator
from app.backup.models.sla_table import PolicyModel
from app.backup.redis.context import Context
from app.backup.service import backup_service
from app.backup.service.backup_service import create_backup_job, unlock_resource_list, lock_resource_list, \
    notify_backup_fail, create_backup_timeout_task, log_backup_job_lock_failed
from app.base.db_base import database
from app.common import toolkit, logger, license
from app.common.clients import job_center_client
from app.common.clients.alarm.alarm_after_failure import alarm_after_failure
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.clients.job_center_client import get_job_queue_scope
from app.common.config import settings
from app.common.constants.resource_type_mapping import SubTypeMapper
from app.common.deploy_type import DeployType
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.enums.copy_enum import CopyFormatEnum, CopyVerifyStatusEnum, GenerationType
from app.common.enums.job_enum import JobStatus, JobType, JobLogLevel
from app.common.enums.license_enum import FunctionEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.schedule_enum import ExecuteType, ScheduleTypes
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum
from app.common.event_messages.Eam.eam import BackupRequest
from app.common.event_messages.Flows.backup import BackupDone, BackupInit, BackupScheduleRequest, BackupComplete
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.log.event_client import EventClient
from app.common.log.event_schemas import SendEventReq, LogRank
from app.common.log.kernel import convert_storage_type
from app.common.schemas.backup_schemas import BackupWorkFlowSchema
from app.common.toolkit import JobMessage, modify_job_lock_id, count_task_in_status
from app.common.util.decorators.callbacks import error_callback
from app.common.verify_copy_client import send_verify_copy
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_id, query_copy_count_by_resource_id, \
    query_last_copy_by_resource_id
from app.resource.models.resource_group_models import ResourceGroup, ResourceGroupMember
from app.resource.service.common import resource_service, domain_resource_object_service
from app.resource.service.common.protect_obj_service import get_next_time

log = logger.get_logger(__name__)

NAME = "backup_manager"


def backup_error_callback(request, **params):
    """
    发送备份失败通知
    """

    request_id = request.request_id
    # 记录任务日志
    if isinstance(params.get("exception"), EmeiStorBizException):
        record_job_step(request_id, request_id, ProtectionBackupJobSteps.BACKUP_CHECK_FAILED, JobLogLevel.ERROR,
                        ["job_status_fail_label"], params.get("exception").error_code)
    else:
        record_job_step(request_id, request_id, ProtectionBackupJobSteps.BACKUP_CHECK_FAILED, JobLogLevel.ERROR,
                        ["job_status_fail_label"], CommonErrorCodes.STATUS_ERROR.get("code"))

    context = Context(request_id)
    if not context.exist() or len(context.exist()) == 1:
        return
    log.info(f"backup workflow [error callback], request_id: {request_id}")
    backup_done_msg = BackupDone(
        copy_ids=[], request_id=request_id, job_id=request_id, status=0)
    producer.produce(backup_done_msg)


def consume_backup_start(request, **params):
    """
    备份任务检查及启动
    :param request: request
    :param params: params
    :return: job id
    """
    start_time_millisecond = int(round(time.time() * 1000))
    job_id = ''
    try:
        log.info(f"Consume backup start.")
        job_id = backup_start(request, params=params)
    except EmeiStorBizException as ex:
        if CommonErrorCodes.USER_FUNCTION_CHECK_FAILED.get("code") == ex.error_code:
            log.warning(f"User-related functions are disabled, request id is {request.request_id}. Auto backup end.")
        else:
            log.error(f"Consume backup start occurs error: {ex}")
    except Exception as ex:
        # 出现异常这条消息也消费掉，防止
        log.error(f"Consume backup start occurs error: {ex}")
    finally:
        cost_time = int(round(time.time() * 1000)) - start_time_millisecond
        log.info(f"Consume backup start cost time: {cost_time}ms, job id: {job_id}")


def backup_start(request, **params):
    """
    备份任务检查及启动
    :param request:
    :param params:
    :return:
    """
    job_id = ""
    if params.get('params'):
        params = params.get('params')
    request_id = request.request_id
    resource_id = params.get("resource_id", "")
    protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
    if protected_obj is None:
        log.warn(f"[backup start], request_id: {request_id}, protect_obj not exist, workflow stop")
        return job_id
    is_resumable_backup, last_job_id = check_resumable_backup(protected_obj)
    if is_resumable_backup:
        request_id = last_job_id
    log.info(f"backup workflow [initialize backup], request_id: {request_id}")
    policy = params.get("policy")
    execute_type = params.get("execute_type", ExecuteType.AUTOMATIC.value)
    resource_obj = ResourceClient.query_resource(resource_id=resource_id)
    # 日志备份是首次备份或全量副本被删除，不创建任务
    if not is_support_log_backup(resource_id, policy.get("action")):
        log.info(f"[backup start], request_id: {request_id}, there is no copy supports log backup")
        return job_id
    # 获取最早的备份执行时间并设置到上下文，后续判断是否是首次备份使用
    earliest_time = protected_obj.get("earliest_time")
    is_first_backup = not earliest_time
    job_message = build_job_message(params, policy, resource_obj)
    if not backup_service.backup_pre_check(request_id, execute_type, policy, protected_obj, resource_obj,
                                           is_first_backup):
        log.warn(f"[backup start], request_id: {request_id}, precheck failed, workflow stop")
        return job_id
    ext_parameters, job_extend_params = _build_job_extend_param(execute_type, params, policy, request_id, resource_obj)
    job_id = create_backup_job(request_id, params.get("user_id", None), resource_obj, job_message, execute_type,
                               ext_parameters.get("storage_id", ""), job_extend_params=job_extend_params,
                               is_override=is_resumable_backup)
    return str(job_id)


def _build_job_extend_param(execute_type, params, policy, request_id, resource_obj):
    ext_parameters = policy.get("ext_parameters", {})
    if isinstance(ext_parameters, str):
        ext_parameters = json.loads(ext_parameters)
    sla_id = _get_sla_id_from_policy(policy)
    sla = ProtectionClient.query_sla(sla_id)
    # 增加extendField字段，包含SLA信息
    job_extend_params = {
        "sla_name": sla.get("name"),
        "sla_id": sla_id,
        "sla_application": sla.get("application"),
        "policy_id": policy.get("uuid"),
        "ai_sorter": ext_parameters["ai_sorter"] if "ai_sorter" in ext_parameters else False,
        "skip_mutual_exclusion": _need_skip_mutual_exclusion(resource_obj.get("sub_type"), policy.get("action")),
        "policy_action": policy.get("action"), "backup_type": policy.get("action"),
        "execute_type": execute_type
    }
    is_group_member_backup = params.get("is_group_member_backup", False)
    # 如果是资源组备份场景，需要将父任务id 加入到extendInfo中
    if is_group_member_backup:
        job_extend_params.update({"groupBackupJobId": params.get("group_backup_job_id"),
                                  "resourceGroupId": params.get("resource_group_id")})
        log.info(f"Try to create sub backup job {request_id} for resource_group {params.get('resource_group_id')}, "
                 f"group_backup_job {params.get('group_backup_job_id')}.")
    return ext_parameters, job_extend_params


def _get_sla_id_from_policy(policy):
    sla_id = policy.get("sla_id", None)
    if sla_id:
        return sla_id
    policy_id = policy.get("uuid")
    with database.session() as session:
        db_policy = session.query(PolicyModel.sla_id).filter(PolicyModel.uuid == policy_id).one()
        if db_policy:
            return db_policy.sla_id
    log.warning(f"Can not find sla id from policy: {policy_id}")
    return None


def build_job_message(params, policy, resource_obj):
    job_priority = backup_service.get_backup_job_priority(policy.get("action"))
    job_message = JobMessage(
        topic=BackupInit.default_topic,
        payload=get_job_payload_queue_scope(params, resource_obj),
        traffic={
            "priority": job_priority
        }
    )
    return job_message


def check_resumable_backup(protected_object):
    """
    支持断点续备功能，新任务使用原任务ID（上次失败副本ID）
    Args:
        protected_object: 保护对象
    """
    ext_parameters = protected_object.get("ext_parameters", {})
    is_resumable_backup = ext_parameters.get("checkPoint", False)
    if not is_resumable_backup:
        return False, ''
    resource_id = protected_object.get("resource_id")
    last_backup_copy = query_last_copy_by_resource_id(resource_id, GenerationType.BY_BACKUP.value)
    if last_backup_copy and last_backup_copy.extend_type == "checkPoint" \
            and not check_resource_have_running_backup_job(resource_id):
        log.info(f"resumable backup start, job id:{last_backup_copy.uuid}")
        return True, last_backup_copy.uuid
    return False, ''


def check_resource_have_running_backup_job(resource_id):
    """
    检测资源有没有还在运行中的备份任务，有的话就不进行断点续备
    Args:
        resource_id: 资源ID
    """
    query_param = {
        "source_ids": [resource_id],
        "status_list": [JobStatus.READY.value, JobStatus.PENDING.value, JobStatus.RUNNING.value,
                        JobStatus.DISPATCHING.value, JobStatus.ABORTING.value],
        "types": [JobType.BACKUP.value]
    }
    count = count_task_in_status(query_param)
    return count and count > 0


def _need_skip_mutual_exclusion(resource_sub_type: str, backup_type: str):
    return backup_type == PolicyActionEnum.log.value and resource_sub_type in SubTypeMapper.skip_mutual_exclusion_types


def get_affinity_user_id(params, environment_resource, resource_obj):
    if params.get("user_id") is None:
        user_id = environment_resource.get('user_id')
        if not user_id:
            user_id = resource_obj.get('user_id')
        return user_id
    return params.get("user_id")


def get_job_payload_queue_scope(params, resource_obj):
    queue_scope = get_job_queue_scope(resource_obj.get("sub_type"), JobType.BACKUP.value)
    if queue_scope:
        params["queue_job_type"] = JobType.BACKUP.value
        params[queue_scope] = resource_obj.get("root_uuid")
    return params


def _is_in_consistency(protected_obj):
    consistent_status = protected_obj.get(ProtectionConstant.CONSISTENT_KEY, "")
    if consistent_status == ProtectionConstant.INCONSISTENT:
        consistent_results = protected_obj.get(ProtectionConstant.CONSISTENT_RESULTS, "")
        if consistent_results is None or consistent_results == "":
            # 升级场景， 只需要判断consistent_status就行
            return True
        local_device_esn = settings.get_key_from_config_map(ProtectionConstant.CLUSTER_CONFIG,
                                                            ProtectionConstant.CLUSTER_ESN)
        if local_device_esn is None:
            log.error("get local device esn error.")
            return False
        # 判断当前机器的文件一致性状态
        consistent_results_map = json.loads(consistent_results)
        local_device_consistent = consistent_results_map.get(local_device_esn)
        return ProtectionConstant.INCONSISTENT == local_device_consistent
    # 文件一致， 不需要转全量
    return False


def update_backup_policy_if_possible(request_id, policies, protected_obj, sla) -> str:
    """
    某些情况下需要更新备份策略
    1. 保护对象->非consistent
    2. 保护对象->ext_parameters->next_backup_type 指定了下一次备份类型

    :param request_id: 任务id
    :param policies: 备份策略
    :param protected_obj: 保护对象
    :param sla: sla
    :return: 更新保护策略原因，如果为更新则返回空字符串。
    """
    policy = policies[0]
    old_backup_type = policy.get("action", "")
    resource_sub_type = protected_obj.get("sub_type")
    copy_count = query_copy_count_by_resource_id(protected_obj.get("resource_id"))
    # 首次备份是增量时，获取policy的适配
    if old_backup_type in BackupWorkflowConstants.INCREMENT_BACKUP_TYPES and copy_count == 0 \
            and is_support_full_job(request_id, resource_sub_type):
        log.info(f"First backup, jobId: {request_id}.")
        policies[0] = get_next_backup_policy(policy, PolicyActionEnum.full.value, sla)
        record_first_increment_convert_to_full_job_step(old_backup_type, request_id)
        return BackupWorkflowConstants.BY_NEXT_BACKUP
    # 首次备份是日志时，转为全量备份
    if old_backup_type == PolicyActionEnum.log.value and copy_count == 0:
        log.info(f"First log backup, jobId: {request_id}.")
        policies[0] = get_next_backup_policy(policy, PolicyActionEnum.full.value, sla)
        # 打印首次日志备份转为全量备份的label
        record_job_step(request_id, request_id,
                        "job_first_backup_log_convert_full_label", JobLogLevel.WARNING)
        return BackupWorkflowConstants.BY_NEXT_BACKUP
    # 如果是增量备份，且保护对象不完整，则增量转为全量
    if old_backup_type in BackupWorkflowConstants.INCREMENT_BACKUP_TYPES and _is_in_consistency(protected_obj):
        record_job_step(request_id, request_id,
                        "job_log_protection_backup_increment_convert_full_label", JobLogLevel.WARNING)
        log.info(f"Backup turn from {old_backup_type} to full, jobId: {request_id}.")
        next_policy = get_next_backup_policy(policy, PolicyActionEnum.full.value, sla)
        policies[0] = next_policy
        return BackupWorkflowConstants.BY_IN_CONSISTENT
    else:
        # 查看保护对象扩展参数，如果指定了下次备份类型则变更备份类型。
        ext_parameters = get_next_backup_ext_parameter(protected_obj)
        next_backup_type = ext_parameters.get(BackupWorkflowConstants.KEY_NEXT_BACKUP_TYPE, '')
        if next_backup_type not in BackupWorkflowConstants.ALL_BACKUP_TYPES:
            log.info(f"Backup type not in all backup type, next: {next_backup_type}, jobId: {request_id}.")
            return ""
        if next_backup_type == old_backup_type:
            log.info(f"Backup not change, old: {old_backup_type}, jobId: {request_id}.")
            return BackupWorkflowConstants.BY_NEXT_BACKUP
        backup_cause_label = ext_parameters.get(BackupWorkflowConstants.KEY_NEXT_BACKUP_CAUSE, '')
        record_job_step(request_id, request_id, backup_cause_label, JobLogLevel.WARNING)
        log.info(f"Backup turn from {old_backup_type} to {next_backup_type}, "
                 f"because {backup_cause_label}, jobId: {request_id}.")
        next_policy = get_next_backup_policy(policy, next_backup_type, sla)
        policies[0] = next_policy
        return BackupWorkflowConstants.BY_NEXT_BACKUP


def is_support_full_job(request_id, resource_sub_type):
    if ResourceSubTypeEnum.NasFileSystem.value == resource_sub_type:
        log.info(f'Current job: {request_id} only supports permanent increment backup.')
        return False
    return True


def is_support_log_backup(resource_id, policy_action):
    if policy_action != PolicyActionEnum.log.value:
        return True
    if query_copy_count_by_resource_id(resource_id) != 0:
        return True
    return False


def record_first_increment_convert_to_full_job_step(old_backup_type, request_id):
    # 前端传递参数old_backup_type:
    # difference_increment为增量备份，cumulative_increment为差异备份（中英文对应错位，属于历史问题）
    convert_to_full_log_dict = {
        PolicyActionEnum.permanent_increment: "job_first_permanent_increment_convert_full_label",
        PolicyActionEnum.difference_increment: "job_first_backup_increment_convert_full_label",
        PolicyActionEnum.cumulative_increment: "job_first_difference_increment_convert_full_label"
    }
    # 打印首次增量备份转为全量备份的label
    record_job_step(request_id, request_id,
                    convert_to_full_log_dict.get(old_backup_type, "job_first_backup_increment_convert_full_label"),
                    JobLogLevel.WARNING)


def get_next_backup_policy(current_policy, next_backup_type, sla):
    policy_list = sla.get("policy_list")
    full_backup_policy_list = [policy for policy in policy_list if is_policy_backup_and_full(policy)]
    if len(full_backup_policy_list) > 0:
        return full_backup_policy_list[0]
    else:
        current_policy["action"] = next_backup_type
        return current_policy


def is_policy_backup_and_full(policy):
    return policy.get("type") == PolicyTypeEnum.backup.value and policy.get("action") == PolicyActionEnum.full.value


@error_callback(callback=backup_error_callback, logger=log)
def backup_context_initialize(request, **params):
    """
    备份任务上下文初始化
    1. 备份前置流程校验
    2. 同步备份执行时间
    3. 初始化上下文
    4. 锁定资源
    :param request:
    :param params:
    :return:
    """
    request_id = request.request_id
    if not job_center_client.query_is_job_present(request_id):
        return
    log.info(f"backup workflow [context initialize], request_id: {request_id}")
    policy = params.get("policy")
    execute_type = params.get("execute_type", ExecuteType.AUTOMATIC.value)
    sla_id = params.get("sla_id")
    sla = ProtectionClient.query_sla(sla_id=sla_id)
    # 手动备份会从数据库中查询最新的sla，只对自动备份做更新操作
    if execute_type == ExecuteType.AUTOMATIC.value:
        policy_uuid = policy.get("uuid")
        policy_model = [
            policy
            for policy in sla.get("policy_list")
            if policy["uuid"] == policy_uuid
        ]
        if policy_model and len(policy_model) == 1:
            policy = policy_model[0]
    resource_id = params.get("resource_id", "")
    resource_obj, protected_obj = get_resource_and_protection_object(resource_id, request_id)
    chain_id = protected_obj.get('chain_id', '')
    # 获取最早的备份执行时间并设置到上下文，后续判断是否是首次备份使用
    earliest_time = protected_obj.get("earliest_time")
    is_first_backup = not earliest_time

    # 同步备份时间
    ResourceClient.sync_protection_time(resource_id)
    # 设置上下文
    context = Context(request_id)
    context.set(BackupWorkflowConstants.JOB_TYPE, JobType.BACKUP.value)
    context.set(BackupWorkflowConstants.JOB_ID, request_id)
    context.set(BackupWorkflowConstants.CHAIN_ID, chain_id)
    context.set(BackupWorkflowConstants.RESOURCE_ID, resource_id)
    if params.get("is_group_member_backup", False):
        context.set(BackupWorkflowConstants.RESOURCE_GROUP_ID, params.get("resource_group_id"))
    context.set(BackupWorkflowConstants.RESOURCE_NAME, resource_obj["name"])
    context.set(BackupWorkflowConstants.BACKUP_TYPE, policy["action"])
    context.set(BackupWorkflowConstants.POLICY, policy)
    context.set(BackupWorkflowConstants.SLA_ID, sla_id)
    context.set(BackupWorkflowConstants.SLA, sla)
    context.set(BackupWorkflowConstants.PROTECTED_OBJECT, protected_obj)
    context.set(BackupWorkflowConstants.EXECUTE_TYPE, execute_type)

    timeout = get_next_time(SessionLocal(), resource_id)
    log.debug(f"backup workflow [initialize backup], timeout: {timeout}")
    if timeout:
        timeout = time.mktime(time.strptime(timeout, ProtectionConstant.DATE_TIME_FORMATTER))
    context.set(BackupWorkflowConstants.TIMEOUT_TIME, timeout)
    context.set(BackupWorkflowConstants.RESOURCE, json.dumps(resource_obj))
    context.set(BackupWorkflowConstants.FIRST_BACKUP, str(is_first_backup))
    context.set(BackupWorkflowConstants.AUTO_RETRY, str(params.get("auto_retry")))
    if params.get("auto_retry"):
        context.set(BackupWorkflowConstants.RETRY_TIMES, params.get("auto_retry_times"))
        context.set(BackupWorkflowConstants.WAIT_MINUTES, params.get("auto_retry_wait_minutes"))
    if params.get(BackupWorkflowConstants.COPY_NAME):
        context.set(BackupWorkflowConstants.COPY_NAME, params.get(BackupWorkflowConstants.COPY_NAME))

    policies = [policy]
    backup_type_change_reason = update_backup_policy_if_possible(request_id, policies, protected_obj, sla)
    # 列表中对policy的修改可能是指向了一个新的policy, 所以 policy 需要重新从列表中获取
    policy = policies[0]
    context.set(BackupWorkflowConstants.CAUSE_OF_BACKUP_TYPE_CHANGE, backup_type_change_reason)
    context.set(BackupWorkflowConstants.BACKUP_TYPE, policy["action"])
    context.set(BackupWorkflowConstants.POLICY, policy)
    # 锁定资源
    context.set("lock_id", request_id)
    modify_job_lock_id(context)
    ext_parameters = protected_obj.get("ext_parameters", {})
    is_resumable_backup = ext_parameters.get("checkPoint", False)
    lock_resource_list(request_id, resource_id, policy, resource_obj.get("sub_type"), is_resumable_backup)


@error_callback(callback=backup_error_callback, logger=log)
def resource_locked(request, **params):
    """
    资源锁定完成
    :param request:
    :param params:  锁定结果
    :return:
    """
    request_id = request.request_id
    if not job_center_client.query_is_job_present(request_id):
        return
    context = Context(request_id)
    if not context.exist() or len(context.exist()) == 1:
        log.info(f"Backup job:{request_id} context is not exists.")
        unlock_resource_list(request_id, request_id)
        return
    status = params.get("status")
    error_desc = params.get('error_desc')
    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    log.info(f"backup workflow [resource locked], request_id: {request_id}, status: {status}, error_code: {error_desc}")
    if status != "success":
        log.warn("backup workflow [resource locked], lock failed")
        # 获取资源锁失败，关闭自动重试
        context.set(BackupWorkflowConstants.AUTO_RETRY, str(False))
        # 加入资源锁备份日志
        log_backup_job_lock_failed(request_id, request_id)
        notify_backup_fail(request_id, request_id)
        return

    policies = [context.get(BackupWorkflowConstants.POLICY, dict)]
    execute_type = context.get(BackupWorkflowConstants.EXECUTE_TYPE)
    sla = context.get(BackupWorkflowConstants.SLA, dict)
    chain_id = context.get(BackupWorkflowConstants.CHAIN_ID)
    protected_obj = context.get(BackupWorkflowConstants.PROTECTED_OBJECT, dict)
    policy = policies[0]

    check_result = get_license_check_result(request_id, resource_id)
    if not check_result:
        notify_backup_fail(request_id, request_id)
        return
    if not backup_service.backup_execute_check(request_id, request_id, resource_id):
        notify_backup_fail(request_id, request_id)
        return
    create_backup_timeout_task(context, execute_type, policy, request_id, resource_id)
    msg = BackupRequest(request_id=request_id, task_id=request_id, resource_id=resource_id,
                        protected_object=protected_obj, chain_id=chain_id, backup_type=policy["action"], config={},
                        sla=sla)
    producer.produce(msg)


def get_license_check_result(request_id, resource_id):
    check_result = True
    # 安全一体机暂时跳过license校验
    if not DeployType().is_cyber_engine_deploy_type():
        check_result = license.validate_license_by_resource_uuid(FunctionEnum.BACKUP, resource_id, request_id,
                                                                 request_id, strict=False)
    if not check_result:
        log.warn("backup workflow [check license], license unavailable")
    return check_result


status_switcher = {
    # 失败
    0: JobStatus.FAIL,
    # 成功
    1: JobStatus.SUCCESS,
    # 部分成功
    2: JobStatus.PARTIAL_SUCCESS,
    # 终止
    3: JobStatus.ABORTED
}


def backup_done(request, copy_ids, job_id, status):
    """
    备份完成逻辑处理
    1.如果上下文已经删除，说明已经处理过，无需重复处理
    2.更新任务状态
    3.更新保护对象遵从度
    4.解锁资源
    5.发送备份完成通知
    6.删除缓存
    """
    request_id = request.request_id
    log.info(
        f"backup workflow [backup done], request_id: {request_id}, job_id: {request_id}, status: {status}")
    context = Context(request_id)
    if not context.exist() or len(context.exist()) == 1:
        # 如果上下文不存在，说明是重复消息，已经清理过上下文
        log.info(
            f'backup workflow [backup done], request_id: {request_id}, message has been processed repeated')
        unlock_resource_list(request_id, request_id)
        return
    if not job_center_client.query_is_job_present(request_id):
        return
    # 根据任务状态判断是否发送告警
    alarm_after_failure(context, status)

    backup_result = status_switcher.get(status)

    '''
    解决：
    如果先更新资源状态，则会将下个任务进入运行状态，在这种情况下，下个任务加锁时由于该任务的锁还未释放导致加锁失败。
    '''
    # 释放资源锁
    log.info(
        f'backup workflow [backup done], request_id: {request_id}, release resource lock')
    # 解锁资源
    unlock_resource(context, request_id)
    # 更新job结果
    JobClient.update_job(request_id, request_id, backup_result)

    # 更新保护对象遵从度
    backup_service.update_compliance(request_id, backup_result)
    # 发送备份完成通知
    backup_complete_notify(request_id, context, backup_result, copy_ids)
    backup_retry(request_id=request_id, context=context, status=backup_result)
    # 更新保护资源状态
    backup_service.backup_complete_protect_object(context, backup_result)
    # 删除缓存
    context.delete_all()


def backup_retry(request_id: str, context: Context, status: JobStatus):
    if not status.need_retry():
        log.warning(
            f'backup workflow [backup retry], request_id: {request_id}, '
            f'backup success, abort')
        return
    execute_type = context.get(BackupWorkflowConstants.EXECUTE_TYPE)
    auto_retry = context.get(BackupWorkflowConstants.AUTO_RETRY)
    retry_times = context.get(BackupWorkflowConstants.RETRY_TIMES)
    wait_minutes = context.get(BackupWorkflowConstants.WAIT_MINUTES)
    log.info(f'backup workflow [backup retry], request_id: {request_id}, auto_retry: {auto_retry}, '
             f'retry_times: {retry_times}, wait_minutes: {wait_minutes}')
    if not check_need_retry(request_id, execute_type, auto_retry, retry_times, wait_minutes):
        return
    start_date = (datetime.datetime.now() + datetime.timedelta(minutes=int(wait_minutes))
                  ).strftime(ProtectionConstant.DATE_TIME_FORMATTER)
    schedule_param = BackupWorkFlowSchema(
        resource_id=context.get(BackupWorkflowConstants.RESOURCE_ID),
        sla_id=context.get(BackupWorkflowConstants.SLA_ID),
        chain_id=context.get(BackupWorkflowConstants.CHAIN_ID),
        policy=context.get(BackupWorkflowConstants.POLICY, dict),
        execute_type=context.get(BackupWorkflowConstants.EXECUTE_TYPE),
        auto_retry=bool(auto_retry),
        auto_retry_times=int(retry_times) - 1,
        auto_retry_wait_minutes=int(wait_minutes)
    )
    schedule_req = {
        "schedule_name": f"{request_id}_retry_{retry_times}",
        "schedule_type": ScheduleTypes.delayed.value,
        "action": BackupScheduleRequest.default_topic,
        "params": json.dumps(schedule_param.dict()),
        'start_date': start_date
    }
    log.debug('backup workflow [backup retry], create retry task')
    SchedulerClient.create_delay_schedule(schedule_req)


def check_need_retry(request_id, execute_type, auto_retry, retry_times, wait_minutes) -> bool:
    """
    校验是否可以重试
    :param request_id: 请求id
    :param execute_type: 备份执行类型
    :param auto_retry:  是否开启重试
    :param retry_times: 当前重试次数
    :param wait_minutes: 重试间隔
    :return: True 需要重试/False 不需要重试
    """
    if execute_type == ExecuteType.MANUAL.value:
        log.warning(
            f'backup workflow [backup retry], request_id: {request_id}, '
            f'manual backup not execute retry, abort')
        return False
    if auto_retry != 'True':
        log.warning(
            f'backup workflow [backup retry], request_id: {request_id}, auto retry is close, abort')
        return False
    if retry_times is None or int(retry_times) <= 0:
        log.warning(
            f'backup workflow [backup retry], request_id: {request_id}, max retry times, abort')
        return False
    if wait_minutes is None:
        log.warning(
            f'backup workflow [backup retry], request_id: {request_id}, wait minutes is None, abort')
        return False
    return True


def backup_timeout_check(request, **params):
    """
    备份任务时间窗超时处理逻辑
    :param request:
    :param params:
    :return:
    """
    request_id = request.request_id
    if not job_center_client.query_is_job_present(request_id):
        return
    log.info(f'backup workflow [time window check], request_id: {request_id}')
    context = Context(request_id)
    if not context.exist() or len(context.exist()) == 1:
        # 如果上下文不存在，说明任务已经执行完，不处理
        log.info(
            f'backup workflow [time window check], request_id: {request_id}, workflow has been completed')
        return

    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    log.warn(
        f'backup workflow [time window check], request_id: {request_id},'
        + 'workflow exceeded the time window, will send alarm')
    send_backup_execution_exceed_time_window_event(context)
    # 时间窗超时，更新保护对象遵从度为不遵从
    ResourceClient.update_protected_object_compliance(
        resource_id=resource_id, compliance=False)


def send_backup_execution_exceed_time_window_event(context):
    resource = context.get(BackupWorkflowConstants.RESOURCE, dict)
    resource_name = context.get(BackupWorkflowConstants.RESOURCE_NAME)
    time_window_start = context.get(BackupWorkflowConstants.TIME_WINDOW_START)
    time_window_end = context.get(BackupWorkflowConstants.TIME_WINDOW_END)
    timestamp = int(time.time())
    env_info = ResourceClient.query_resource(resource_id=resource.get("root_uuid"))
    if DeployType().is_cyber_engine_deploy_type():
        event_id = ProtectionConstant.ALARM_SNAPSHOT_EXECUTION_EXCEEDS_TIME_WINDOW
        params = [
            env_info.get("name", ""),
            env_info.get("uuid", ""),
            convert_storage_type(env_info.get("sub_type", "")),
            resource.get("parent_name", ""),
            resource.get("parent_uuid", ""),
            resource.get("uuid", ""),
            resource_name,
            time_window_start,
            time_window_end
        ]
    else:
        event_id = ProtectionConstant.EVENT_BACKUP_EXECUTION_EXCEEDS_TIME_WINDOW
        params = [resource_name, time_window_start, time_window_end]

    policy = context.get(BackupWorkflowConstants.POLICY, dict)
    if policy.get("ext_parameters", {}).get("alarm_over_time_window", False):
        # 需要发送告警
        log.warn(f"resource {resource_name}  job {context.name} will send alarm.")
        params = resource_name + "," + context.name + "," + time_window_start + "," + time_window_end
        AlarmClient.send_alarm(SendAlarmReq(
            alarmId=ProtectionConstant.ALARM_BACKUP_EXECUTION_EXCEEDS_TIME_WINDOW,
            userId=env_info.get("user_id", None),
            params=params,
            alarmSource="localhost",
            createTime=timestamp,
            sequence=timestamp,
            sourceType=AlarmSourceType.SLA,
            severity=LogRank.WARNING.value
        ))
    else:
        EventClient.send_running_event(SendEventReq(
            userId=env_info.get("user_id", None),
            eventId=event_id,
            eventParam=params,
            eventTime=timestamp,
            eventLevel=LogRank.WARNING.value,
            sourceId=event_id,
            resourceId=context.get(BackupWorkflowConstants.RESOURCE_ID),
            sourceType=AlarmSourceType.PROTECTION,
            eventSequence=timestamp,
            isSuccess=False
        ))


def query_agents_by_resource_id(resource_id):
    protected_object = ResourceClient.query_protected_object(resource_id=resource_id)
    if protected_object and protected_object.get("ext_parameters"):
        return protected_object.get("ext_parameters").get("agents")
    return ""


def notify_verify_copy(copy_ids: list, user_id: str, policy: dict, resource_id: str):
    if not copy_ids:
        log.info("copy_id is null.")
        return
    ext_parameters = policy.get("ext_parameters")
    copy_id = copy_ids[0]
    log.info(f"start notify_verify_copy copy_id:{copy_id},userId:{user_id}")
    copy_info = query_copy_by_id(copy_id)
    agents = query_agents_by_resource_id(resource_id)
    # 从副本的扩展字段中获取获取副本校验的状态 默认3副本校验不存在
    verify_status = json.loads(copy_info.properties).get("verifyStatus",
                                                         CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST.value)
    log.info(f"notify_verify_copy copy_id:{copy_id},userId:{user_id},verify_status:{verify_status}")
    if verify_status != CopyVerifyStatusEnum.VERIFY_FILE_NOT_EXIST.value and ext_parameters.get("copy_verify", False):
        send_verify_copy(copy_id, user_id, agents)


def backup_complete_notify(request_id, context, status: JobStatus, copy_ids):
    """
    发送备份完成通知
    :param copy_ids: 副本ids
    :param status: 状态
    :param request_id: 本次流程的request_id
    :param context:  上下文对象
    """
    if not status.success():
        return
    if not context.exist() or len(context.exist()) == 1:
        log.info(
            f'backup workflow [complete notify], request_id: {request_id}, context not exist, abort')
        return
    policy = context.get(BackupWorkflowConstants.POLICY, dict)
    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    execute_type = context.get(BackupWorkflowConstants.EXECUTE_TYPE)
    sla = context.get(BackupWorkflowConstants.SLA, dict)
    resource = context.get(BackupWorkflowConstants.RESOURCE, dict)
    current_operate_user_id = context.get(
        BackupWorkflowConstants.CURRENT_OPERATE_USER_ID)
    # 通知复制和防勒索检测
    msg = BackupComplete(
        request_id=str(uuid.uuid4()),
        related_request_id=request_id,
        resource_id=resource_id,
        policy=policy,
        execute_type=execute_type,
        sla=sla,
        resource=resource,
        current_operate_user_id=current_operate_user_id,
        copy_ids=copy_ids
    )
    producer.produce(msg)

    # 通知归档
    archive_message = CommonEvent(
        ProtectionConstant.TOPIC_BACKUP_SUCCESS, sla=sla, resource_id=resource_id,
        user_id=current_operate_user_id, copy_ids=copy_ids, execute_type=execute_type)
    producer.produce(archive_message)
    # 通知副本校验
    notify_verify_copy(copy_ids, current_operate_user_id, policy, resource_id)


def unlock_resource(context, request_id):
    if context.get(BackupWorkflowConstants.RESOURCE_ID, str) \
            or context.get(BackupWorkflowConstants.PROTECTED_OBJECT, str):
        unlock_resource_list(request_id, request_id)


def record_job_step(job_id: str, request_id: str, job_step_label: str,
                    log_level: JobLogLevel = JobLogLevel.INFO,
                    log_info_param=None, log_detail=None, log_detail_param=None):
    log.info(f'record_job_step={request_id},job_step_label={job_step_label}')
    if not log_info_param:
        log_info_param = []
    log_step_req = toolkit.build_update_job_log_request(job_id,
                                                        job_step_label,
                                                        log_level,
                                                        log_info_param,
                                                        log_detail,
                                                        log_detail_param)
    log.info(f'log_step_req={log_step_req}')
    toolkit.modify_task_log(request_id, job_id, log_step_req)


def get_resource_and_protection_object(resource_id, request_id):
    resource_obj = ResourceClient.query_resource(resource_id=resource_id)
    if not resource_obj:
        raise EmeiStorBizException(
            error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[],
            error_message=f"backup workflow [initialize backup], request_id: {request_id}, failed.")

    protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)
    if not protected_obj:
        raise EmeiStorBizException(
            error=ResourceErrorCodes.RESOURCE_NOT_PROTECTED, parameters=[],
            error_message=f"backup workflow [initialize backup], request_id: {request_id}, failed.")
    return resource_obj, protected_obj


def get_next_backup_ext_parameter(protected_obj):
    resource_id = protected_obj.get("resource_id")
    ext_parameters = ResourceClient.query_next_backup_type_and_cause(resource_id)
    # 如果资源的扩展参数中包含有效的下次备份信息，直接返回；否则使用保护对象中的信息
    if ext_parameters and ext_parameters.get(BackupWorkflowConstants.KEY_NEXT_BACKUP_TYPE):
        return ext_parameters
    return protected_obj.get("ext_parameters", {})


def group_backup_start(request, **params):
    """
    资源组备份检查及启动
    : param request
    : param params:
    : return:
    """
    if params.get("params"):
        params = params.get("params")
    request_id = request.request_id

    # 查询组和组成员
    resource_group_id = params.get("resource_id")
    if not params.get("resource_group_id"):
        params["resource_group_id"] = resource_group_id
    if not resource_group_id:
        raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM, parameters=[],
                                   error_message=f"Can not find resource_group_id for group backup.")
    resource_group: ResourceGroup = resource_service.query_resource_group_by_id(resource_group_id)
    resource_group_member_ids = resource_service.query_group_resources_by_group_id(resource_group_id)

    # 对于自动备份需要检查备份窗口
    policy = params.get("policy")
    schedule = policy["schedule"]
    current_time = datetime.datetime.now()
    params.get("")
    execute_type = params.get("execute_type")
    if (not execute_type == ExecuteType.MANUAL.value
            and not ParamsValidator.check_time_window(schedule["window_start"], schedule["window_end"], current_time)):
        # 当前时间不在时间窗范围，不执行调度策略
        log.warning(
            f"[initialize backup], request_id={request_id}, current time is not in time window, workflow stop")
        return ""

    protected_obj = ResourceClient.query_protected_object(resource_id=resource_group_id)
    other_params = {
        "protected_obj": protected_obj,
        "policy": policy,
        "execute_type": execute_type
    }
    group_backup_job_id = create_group_backup_job(params, request_id, resource_group, **other_params)

    # 备份时间
    ResourceClient.sync_protection_time(resource_group_id)
    log.info(f"Updated last backup time for resource group {resource_group_id}.")

    try:
        start_group_backup(params, request, resource_group, resource_group_member_ids, group_backup_job_id)
    except Exception as e:
        log.error(f"Start resource group backup failed, resource_group_id {resource_group_id}, "
                  f"job_id {group_backup_job_id}, error msg {str(e)}.")
        record_job_step(group_backup_job_id, request_id, job_step_label="group_backup_job_fail_label",
                        log_level=JobLogLevel.ERROR)
        JobClient.update_job(request_id, group_backup_job_id, JobStatus.FAIL)
    return group_backup_job_id


def create_group_backup_job(params, request_id, resource_group, **kwargs):
    # 创建备份组任务
    resource_group_id = resource_group.uuid
    resource_group_dict = resource_group.as_dict()
    resource_group_dict["type"] = resource_group.source_type
    resource_group_dict["sub_type"] = resource_group.source_sub_type
    protected_obj = kwargs.get("protected_obj")
    policy = kwargs.get("policy")
    job_extend_params = {
        "sla_name": protected_obj.get("sla_name"),
        "sla_id": protected_obj.get("sla_id"),
        "policy_id": policy.get("uuid"),
        "policy_action": policy.get("action"), "backup_type": policy.get("action"),
        "execute_type": kwargs.get("execute_type"),
        "subJobsCreationDone": False,
        "resourceGroupId": resource_group_id
    }
    domain_id_list = domain_resource_object_service.get_domain_id_list(resource_group_id)
    group_backup_job_id = JobClient.create_job(
        request_id=request_id,
        user_id=params.get("user_id", None),
        domain_id_list=domain_id_list,
        resource_obj=resource_group_dict,
        job_type=JobType.GROUP_BACKUP.value,
        enable_stop=False,
        job_extend_params=job_extend_params
    )
    if group_backup_job_id is None:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                   error_message=f"backup workflow [create job], request_id={request_id} failed.")
    log.info(f"Create group backup job: {group_backup_job_id} for resource_group {resource_group_id}.")
    record_job_step(group_backup_job_id, request_id, job_step_label="group_backup_start_execute")
    JobClient.update_job(request_id, group_backup_job_id, JobStatus.RUNNING)
    return group_backup_job_id


def start_group_backup(params, request, resource_group: ResourceGroup,
                       resource_group_member_ids, group_backup_job_id: str):
    pre_copy_name = params.get("copy_name")
    if not pre_copy_name:
        pre_copy_name = f"{resource_group.name}_{int(time.time() * 1000)}"
    _handle_zero_member_when_start(group_backup_job_id, request, resource_group_member_ids)
    sub_job_create_failed_count = 0
    sub_resource_count = len(resource_group_member_ids)
    for i in range(0, sub_resource_count):
        group_member_id = resource_group_member_ids[i]
        # 创建子备份任务，复用父任务param，部分字段需要替换
        sub_backup_params = copy.deepcopy(params)
        resource_obj = resource_service.query_resource_by_id(group_member_id)
        sub_backup_params["copy_name"] = f"{pre_copy_name}_{resource_obj.name[:64]}_{resource_obj.uuid}"
        sub_backup_params["resource_id"] = group_member_id
        sub_backup_params["is_group_member_backup"] = True
        sub_backup_params["group_backup_job_id"] = group_backup_job_id
        is_first_sub_job = i == 0
        is_last_sub_job = i == (sub_resource_count - 1)
        try:
            # 由于request_id 会被作为job_id，这里未避免子任务与父任务id冲突，需要重置request_id
            sub_request = copy.deepcopy(request)
            sub_request.request_id = str(uuid.uuid4())
            sub_backup_job_id = backup_start(sub_request, params=sub_backup_params)
            if not sub_backup_job_id:
                raise EmeiStorBizException(error=CommonErrorCodes.OPERATION_FAILED, parameters=[],
                                           error_message=f"Failed to create sub job for group "
                                                         f"backup {group_backup_job_id}.")
            update_group_backup_job_when_sub_job_create(request.request_id, group_backup_job_id, resource_obj, False,
                                                        sub_backup_job_id, is_first_sub_job, is_last_sub_job, False)
            log.info(f"Create sub backup job: {sub_backup_job_id} for resource {group_member_id} "
                     f"and source group {params.get('resource_group_id')}.")
        except Exception as e:
            log.error(f"Failed to create sub job for resource group backup job {group_backup_job_id}, "
                      f"error info: {str(e)}.")
            sub_job_create_failed_count += 1
            update_group_backup_job_when_sub_job_create(request.request_id, group_backup_job_id, resource_obj, True, "",
                                                        is_first_sub_job, is_last_sub_job,
                                                        sub_job_create_failed_count == sub_resource_count)


def _handle_zero_member_when_start(group_backup_job_id, request, resource_group_members):
    if len(resource_group_members) == 0:
        update_group_job_params = dict()
        update_group_job_params["progress"] = 100
        update_group_job_params["status"] = JobStatus.SUCCESS.value
        update_group_job_params["jobLogs"] = [
            {"jobId": group_backup_job_id,
             "startTime": int(datetime.datetime.now().timestamp() * 1000),
             "logInfo": "group_backup_job_success_label",
             "logInfoParam": [],
             "level": JobLogLevel.INFO.value,
             "logDetail": None,
             "logDetailParam": None}
        ]
        toolkit.modify_task_log(request.request_id, group_backup_job_id, update_group_job_params)


def update_group_backup_job_when_sub_job_create(request_id, group_backup_job_id, resource_obj, is_create_fail,
                                                sub_job_id, is_first_sub_job, is_last_sub_job, all_failed):
    # 获取资源组备份任务
    response = toolkit.query_job_list(params={"jobId": group_backup_job_id})
    if not response:
        log.error(f"Failed to find group backup job with id: {group_backup_job_id}.")
        return
    records = json.loads(response).get("records")
    if not records:
        log.error(f"Failed to find group backup job with id: {group_backup_job_id}.")
        return
    job = records[0]
    extend_info = json.loads(job.get("extendStr", '{}'))

    update_group_job_params = {}

    # 更新extendInfo
    if is_last_sub_job:
        extend_info["subJobsCreationDone"] = True
        update_group_job_params.update({"extendStr": json.dumps(extend_info)})

    # 更新job_log
    update_group_job_params.update(
        _get_job_log_for_group_backup(group_backup_job_id, resource_obj, is_create_fail, sub_job_id))

    # 更新进度，创建一个进度 1%，全部创建进度为 5%
    if is_first_sub_job:
        update_group_job_params["progress"] = 1

    if is_last_sub_job:
        update_group_job_params["progress"] = 5

    # 检查是否全部创建失败，更新进度为 100%，并标记失败
    if all_failed:
        update_group_job_params["progress"] = 100
        update_group_job_params["status"] = JobStatus.FAIL.value
        update_group_job_params["jobLogs"] = [
            {"jobId": group_backup_job_id,
             "startTime": int(datetime.datetime.now().timestamp() * 1000),
             "logInfo": "resource_group_backup_job_fail_label",
             "logInfoParam": [],
             "level": JobLogLevel.INFO.value,
             "logDetail": None,
             "logDetailParam": None}
        ]

    toolkit.modify_task_log(request_id, group_backup_job_id, update_group_job_params)


def _get_job_log_for_group_backup(job_id, resource_obj, is_create_fail, sub_job_id):
    if not is_create_fail:
        log_label = "group_backup_create_sub_job_success"
        log_level = JobLogLevel.INFO.value
        log_info_param = [resource_obj.name, sub_job_id]
    else:
        log_label = "group_backup_create_sub_job_fail"
        log_level = JobLogLevel.ERROR.value
        log_info_param = [resource_obj.name]
    return {
        "jobLogs": [{
            "jobId": job_id,
            "startTime": int(datetime.datetime.now().timestamp() * 1000),
            "logInfo": log_label,
            "logInfoParam": log_info_param,
            "level": log_level,
            "logDetail": None,
            "logDetailParam": None
        }]
    }
