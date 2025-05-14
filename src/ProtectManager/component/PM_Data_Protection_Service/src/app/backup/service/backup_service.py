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
import json
import time
from datetime import datetime, timedelta

from app.backup.client.archive_client import ArchiveClient
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.clients.protection_client import ProtectionClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.deploy_type import DeployType
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.enums.copy_enum import GenerationType
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_id
from app.protection.object.client.dee_client import get_replication_pair_by_id
from app.resource.service.common import domain_resource_object_service
from app.restore.client.copy_client import CopyClient

from app.backup.client.job_client import JobClient
from app.backup.client.resource_client import ResourceClient
from app.backup.client.scheduler_client import SchedulerClient
from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.common.constant import ProtectionConstant, AntiRansomwareAlarm
from app.backup.common.validators.sla_validator import time_interval_switcher, ParamsValidator
from app.backup.redis.context import Context
from app.common import logger
from app.common.enums.job_enum import JobStatus, JobLogLevel, JobType
from app.common.enums.protected_object_enum import Status
from app.common.enums.resource_enum import ResourceSubTypeEnum, LinkStatusEnum, OcDeviceTypeEnum
from app.common.enums.schedule_enum import ExecuteType, ScheduleTypes
from app.common.enums.sla_enum import BackupTimeUnit, PolicyActionEnum, TriggerEnum, BackupTypeEnum
from app.common.event_messages.Flows.backup import BackupDone
from app.common.event_messages.Rlm.rlm import UnlockRequest, LockRequest
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.toolkit import modify_task_log, JobMessage
from app.protection.object.common import db_config
from app.protection.object.db import projected_object
from app.protection.object.models.projected_object import ProtectedObject

log = logger.get_logger(__name__)


def backup_complete_protect_object(context, status: JobStatus):
    if not status.success():
        return
    policy = context.get(BackupWorkflowConstants.POLICY, dict)
    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    backup_type_change_reason = context.get(
        BackupWorkflowConstants.CAUSE_OF_BACKUP_TYPE_CHANGE)
    # 1. 更新保护对象的完整性标识
    if policy.get("action", "") == PolicyActionEnum.full:
        with db_config.get_session() as session:
            projected_object.update_by_params(
                db=session,
                resource_id=resource_id,
                update_conditions={
                    ProtectedObject.consistent_status: ProtectionConstant.CONSISTENT
                }
            )
        log.info(f"Resource id: {resource_id}'s status "
                 f"has been updated to consistent.")
        protect_obj = context.get(BackupWorkflowConstants.PROTECTED_OBJECT, dict)
        # 全备后清除下次备份信息
        clean_next_backup_info(protect_obj)
        return
    # 2. 恢复保护对象中的扩展参数
    if backup_type_change_reason != BackupWorkflowConstants.BY_NEXT_BACKUP:
        return
    protect_obj = context.get(BackupWorkflowConstants.PROTECTED_OBJECT, dict)
    clean_next_backup_info(protect_obj)


def get_end_of_time_window(policy):
    """
    获取本次备份时间窗的开始，结束时间
    :param policy: 执行的备份策略
    :return:
    """
    schedule = policy["schedule"]
    window_start = schedule["window_start"]
    window_end = schedule["window_end"]
    log.info(f"get time window:window_start={window_start},window_end={window_end}")
    datetime_now = datetime.now()
    # 获取当前的日期
    start_date = ParamsValidator.parse_time_str(window_start, datetime_now)
    end_date = ParamsValidator.parse_time_str(window_end, datetime_now)
    if end_date <= start_date:
        end_date = end_date + timedelta(days=1)
    log.info(f"already get start_date={start_date},end_date={end_date}")
    return start_date, end_date


def check_need_delay_schedule(policy) -> bool:
    """
    根据时间窗判断，是否需要创建时间窗超时校验的延时任务
        1. 日志备份没有时间窗，不需要校验
        2. 如果时间窗持续时间为24小时，说明全天都可以备份，不存在超时
    :param policy: 执行的备份策略
    :return:
    """
    if PolicyActionEnum(policy["action"]) is PolicyActionEnum.log:
        # 日志备份校验时间窗
        return False

    start_time, end_time = get_end_of_time_window(policy)
    log.info(f"check need delay schedule：star_time={start_time},end_time={end_time}")
    return end_time - start_time != timedelta(days=1)


def get_next_execute_time(policy):
    """
    获取同一个策略的下次备份执行时间
    :param policy: 备份策略
    :return:
    """
    schedule = policy["schedule"]
    if schedule["trigger"] == TriggerEnum.customize_interval:
        schedule["interval_unit"] = "d"
        schedule["interval"] = 1
    interval = schedule["interval"]
    interval_unit = BackupTimeUnit(schedule["interval_unit"])
    second_of_interval = time_interval_switcher[interval_unit](interval)
    return datetime.now() + timedelta(seconds=second_of_interval)


def check_nfs(resource_obj):
    return True


def check_vmware(resource_obj):
    return True


def check_oracle(resource_obj):
    return True


resource_type_switcher = {
    ResourceSubTypeEnum.Oracle.value: check_oracle,
    ResourceSubTypeEnum.VirtualMachine.value: check_vmware,
    ResourceSubTypeEnum.NasFileSystem.value: check_nfs
}


def check_backup_by_resource_type(resource_obj):
    # 根据资源类型限制检查备份是否可以执行
    if resource_obj.get("sub_type") in resource_type_switcher.keys():
        return resource_type_switcher[resource_obj.get("sub_type")](resource_obj)
    return True


def check_sla_active(protected_obj):
    sla_id = protected_obj.get("sla_id")
    sla = ProtectionClient.query_sla(sla_id)
    if sla is None or not sla.get("enabled"):
        log.info(f"sla {sla_id} is deactive")
        return False
    return True


def check_can_be_backup_for_anti_ransomware(request_id, resource_obj):
    # 根据部署形态检测是否可以执行备份
    if not check_can_be_backup_by_deploy_type(resource_obj.get('uuid')):
        log.warning("This is the secondary end of a HyperMetro pai.")
        return False
    # 主存防勒索配置了smart mobility，不能备份，自动备份时触发告警
    if not check_smart_mobility_can_be_backup(resource_obj.get('uuid')):
        log.warning(f"This fs: {resource_obj.get('name')} has smart mobility and send alarm")
        timestamp = int(time.time())
        AlarmClient.send_alarm(SendAlarmReq(
            alarmId=AntiRansomwareAlarm.SMART_MOBILITY_CAN_NOT_BACKUP_ALARM_ID,
            userId=resource_obj.get("user_id", None),
            params=resource_obj.get("name", ""),
            alarmSource=AlarmSourceType.RESOURCE,
            createTime=timestamp,
            sequence=timestamp,
            sourceType=AlarmSourceType.RESOURCE
        ))
        return False
    # 校验Dorado远程复制从端不调度周期性侦测任务调度；
    if not check_replication_by_deploy_type(resource_obj.get('uuid')):
        log.warning(f"This is the secondary end of a Replication pair, request_id:{request_id}.")
        return False
    return True


def backup_pre_check(request_id, execute_type, policy, protected_obj, resource_obj,
                     is_first_backup) -> bool:
    """
    任务执行前校验
        1. 校验保护对象是否存在
        2. 根据资源类型校验是否需要进行本次备份
        3. 如果是页面手动触发备份，不需要判断时间窗及创建延时校验任务
        4. 手动备份校验
        5. 校验保护对象状态
        6. 校验首次备份不执行日志备份
        7. 校验时间窗：
            3.1 校验当前时间是否在今天时间窗范围内
            3.2 如果时间窗非全天，则创建执行时间为时间窗结束时间的延时任务，校验时间窗关闭前任务是否执行完成

    :param request_id: 请求id
    :param protected_obj: 保护对象
    :param policy: 本次执行备份策略
    :param execute_type: 执行类型
    :param resource_obj: 资源对象
    :param is_first_backup: 是否是首次备份
    :return:
    """
    # 查询保护对象
    if not protected_obj:
        # 如果不存在保护对象，则不执行备份
        log.warn(
            f"[initialize backup], request_id={request_id}, protected object not exist, workflow stop")
        return False
    # 根据资源类型进行校验
    if resource_obj is None:
        log.warn(f"[initialize backup], request_id={request_id}, resource info is None, workflow stop")
        return False
    if not check_sla_active(protected_obj):
        log.info(
            f"[initialize backup], request_id={request_id}, sla is deactive, workflow stop")
        return False
    if not check_backup_by_resource_type(resource_obj):
        log.warn(
            f"[initialize backup], request_id={request_id}, resource link_status is offline, workflow stop")
        return False
    # 手动备份放开某些限制
    if execute_type == ExecuteType.MANUAL.value:
        log.warn(
            f"[initialize backup], request_id={request_id}, [MANUAL] backup don't need check")
        return True
    # 任务类型为自动备份时
    if protected_obj.get("status", Status.Inactive.value) != Status.Active.value:
        # 如果保护对象 是未激活状态，则不执行备份
        log.warn(
            f"[initialize backup], request_id={request_id}, protected object status is inactive, workflow stop")
        return False
    if PolicyActionEnum(policy["action"]) is PolicyActionEnum.log:
        if is_first_backup:
            # 日志备份不能是首次备份
            log.warn(
                f"[initialize backup], request_id={request_id}, first backup type can't be backup, workflow stop")
            return False
        else:
            return True
    # 主存防勒索和安全一体机的校验
    if not check_can_be_backup_for_anti_ransomware(request_id, resource_obj):
        return False
    schedule = policy["schedule"]
    current_time = datetime.now()

    if not ParamsValidator.check_time_window(schedule["window_start"], schedule["window_end"], current_time):
        # 当前时间不在时间窗范围，不执行调度策略
        log.warn(
            f"[initialize backup], request_id={request_id}, current time is not in time window, workflow stop")
        return False

    return True


def backup_execute_check(request_id: str, job_id: str, resource_id: str):
    resource_info = ResourceClient.query_resource(resource_id)
    if resource_info.get("sub_type") == ResourceSubTypeEnum.VirtualMachine.value:
        return check_vcenter_link_status(job_id, request_id, resource_info)
    return True


def check_vcenter_link_status(job_id, request_id, resource_info):
    env_id = resource_info.get("root_uuid")
    resource_env = ResourceClient.query_resource(env_id)
    # 检查vcenter是否离线
    check_retry_count = 5
    while check_retry_count > 0:
        if resource_env.get("link_status") == LinkStatusEnum.Offline.value:
            log.info(f"request_id: {request_id},vcenter: {env_id} status is offline, retry count: {check_retry_count}")
            time.sleep(30)
            check_retry_count -= 1
        else:
            return True
    # vcenter资源离线,记录任务错误日志
    modify_task_log(request_id, job_id, {
        "jobLogs": [{
            "jobId": job_id,
            "logInfoParam": ["job_status_fail_label"],
            "logDetail": ResourceErrorCodes.RESOURCE_LINKSTATUS_OFFLINE.get("code"),
            "logDetailParam": [],
            "level": JobLogLevel.ERROR.value,
            "startTime": int(datetime.now().timestamp() * 1000),
            "logInfo": "job_log_protection_backup_execute_check_failed_label"
        }]
    })
    log.warn(
        f"[backup check], request_id={request_id},job_id={job_id},vcenter={env_id} is offline")
    return False


def log_backup_job_lock_failed(request_id: str, job_id: str):
    modify_task_log(request_id, job_id, {
        "jobLogs": [{
            "jobId": job_id,
            "logInfoParam": ["job_status_fail_label"],
            "logDetail": ResourceErrorCodes.BACKUP_RESOURCE_IS_LOCKED.get("code"),
            "logDetailParam": [],
            "level": JobLogLevel.ERROR.value,
            "startTime": int(datetime.now().timestamp() * 1000),
            "logInfo": "lock_pending_label"
        }]
    })


def update_compliance(request_id: str, status: JobStatus):
    """
    根据执行结果更新保护对象SLA遵从度
    :param request_id:  请求id
    :param status: 备份结果状态
    :return:
    """
    context = Context(request_id)
    if not context.exist() or len(context.exist()) == 1:
        log.info(
            f"backup workflow [backup done], request_id={request_id}, context not exist, abort")
        return
    resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
    execute_type = context.get(BackupWorkflowConstants.EXECUTE_TYPE)
    timeout_time = context.get(BackupWorkflowConstants.TIMEOUT_TIME)
    first_backup = context.get(BackupWorkflowConstants.FIRST_BACKUP)
    if execute_type == ExecuteType.MANUAL.value:
        # 手动执行结果不影响遵从度
        log.info(
            f"backup workflow [backup done], request_id={request_id}, [MANUAL] backup don't update compliance.")
        return
    # 备份结果不是成功时，更新保护对象遵从度为不遵从
    compliance = status.success() and int(time.time()) <= int(float(timeout_time)) + 1
    log.info(f"backup workflow [backup done], request_id={request_id}, datetime now={int(time.time())}, "
             f"timeout={timeout_time}, compliance={compliance}")
    # 是否是首次备份，首次备份失败不影响SLA遵从度
    if first_backup == 'True' and not compliance:
        log.info(
            f"backup workflow [backup done], request_id={request_id}, first backup don't update compliance.")
        return
    ResourceClient.update_protected_object_compliance(resource_id=str(resource_id), compliance=compliance)


def get_backup_job_priority(backup_action: str):
    """
    获取备份任务action对应的优先级
    :param backup_action:
    :return:
    """
    backup_action_priority = {
        PolicyActionEnum.full: 4,
        PolicyActionEnum.cumulative_increment: 1,
        PolicyActionEnum.difference_increment: 1,
        PolicyActionEnum.log: 0,
        PolicyActionEnum.snapshot: 1
    }
    action = PolicyActionEnum(backup_action)
    return backup_action_priority.get(action)


def create_backup_job(request_id: str, user_id: str, resource: dict, message: JobMessage, execute_type: str,
                      storage_id: str, job_extend_params=None, is_override=False):
    """
    创建备份任务
    """
    domain_id_list = domain_resource_object_service.get_domain_id_list(resource.get('uuid'))
    callback_data = {
        BackupWorkflowConstants.JOB_CANCEL_CALLBACK_URL:
            f"/v1/internal/workflows/backup/result",
        BackupWorkflowConstants.JOB_CANCEL_CALLBACK_EXECUTE_TYPE: execute_type
    }
    storage_obj = {}
    if storage_id:
        storage_obj = ArchiveClient().query_storage_info(storage_id)
    job_id = JobClient.create_job(
        request_id=request_id,
        user_id=user_id,
        domain_id_list=domain_id_list,
        resource_obj=resource,
        job_type=JobType.BACKUP.value,
        message=message,
        enable_stop=True,
        data=callback_data,
        target_name=storage_obj.get("storageName", "Local"),
        target_location=storage_obj.get("endpoint", "Local"),
        job_extend_params=job_extend_params,
        is_override=is_override
    )

    if job_id is None:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, parameters=[],
                                   error_message=f"backup workflow [create job], request_id={request_id}, failed.")
    return job_id


def unlock_resource_list(request_id, job_id):
    # Send message to RLM
    msg = UnlockRequest(
        request_id=request_id, lock_id=job_id,
    )
    log.info(f"backup workflow [unlock resource], request_id={request_id}, job_id={job_id}")
    producer.produce(msg)


def lock_resource_list(request_id, resource_id, policy, resource_sub_type=None, is_resumable_backup=False):
    # Send message to RLM
    resources = get_backup_lock_resource_ids(resource_id, policy, resource_sub_type, request_id, is_resumable_backup)
    msg = LockRequest(
        request_id=request_id,
        resources=resources,
        wait_timeout=30,
        priority=2,
        lock_id=request_id,
        response_topic=ProtectionConstant.TOPIC_BACKUP_LOCKED
    )
    producer.produce(msg)


def notify_backup_fail(request_id, job_id):
    """
    发送备份失败通知
    """
    backup_done_msg = BackupDone(
        copy_ids=[], request_id=request_id, job_id=job_id, status=0)
    producer.produce(backup_done_msg)


def create_backup_timeout_task(context, execute_type, policy, request_id, resource_id):
    if check_need_delay_schedule(policy) and execute_type == ExecuteType.AUTOMATIC.value:
        start_time, end_time = get_end_of_time_window(policy)
        context.set(BackupWorkflowConstants.TIME_WINDOW_START,
                    start_time.strftime(ProtectionConstant.DATE_TIME_FORMATTER))
        context.set(BackupWorkflowConstants.TIME_WINDOW_END,
                    end_time.strftime(ProtectionConstant.DATE_TIME_FORMATTER))
        # 创建延迟任务，校验任务是否在时间窗结束是否执行完，执行时间为时间窗结束
        schedule_req = {
            "schedule_type": ScheduleTypes.delayed.value,
            "action": ProtectionConstant.TOPIC_BACKUP_TIMEOUT_CHECK,
            "params": json.dumps({
                "request_id": request_id,
                "resource_id": resource_id
            }),
            'start_date': end_time.strftime(ProtectionConstant.DATE_TIME_FORMATTER)
        }
        SchedulerClient.create_delay_schedule(schedule_req)


def workflow_cancel_callback(resource_id):
    """
    备份流程取消回调：更新保护对象遵从度为不遵从
    :param resource_id: 资源id
    :return:
    """
    ResourceClient.update_protected_object_compliance(
        resource_id=resource_id, compliance=False)


def get_backup_lock_resource_ids(resource_id, policy, resource_sub_type, request_id=None, is_resumable_backup=False):
    lock_resource_id = resource_id
    if ResourceClient.is_support_data_and_log_parallel_backup(resource_id) \
            and policy.get("action") == PolicyActionEnum.log.value:
        log.info(f'Resource id: {resource_id}, resource sub type: {resource_sub_type}, support parallel backup')
        lock_resource_id += "@log"
    lock_resource_ids = [{"id": lock_resource_id, "lock_type": "w"}]
    if is_resumable_backup:
        copy = query_copy_by_id(request_id)
        if copy and copy.status == CopyStatus.INVALID and copy.extend_type == "checkPoint":
            lock_resource_ids.append({"id": request_id, "lock_type": "r"})
    # 获取自定义加锁资源列表
    lock_resource_ids.extend(ResourceClient.query_custom_resource(resource_id))
    backup_type = get_backup_type(policy)
    # 全量备份不依赖其它副本
    if backup_type == BackupTypeEnum.full.name:
        return get_unique_resource_id(lock_resource_ids)
    # 永久增量不依赖其它副本
    if backup_type == BackupTypeEnum.permanent_increment.name:
        return get_unique_resource_id(lock_resource_ids)
    # 快照备份不依赖其他副本
    if backup_type == BackupTypeEnum.snapshot.name:
        return get_unique_resource_id(lock_resource_ids)
    # vmware的增量都是永久增量，不用加锁
    if resource_sub_type == ResourceSubTypeEnum.VirtualMachine:
        return get_unique_resource_id(lock_resource_ids)

    latest_copy = query_latest_copy_by_resource_id(resource_id)
    # 第一次备份，没有副本
    if not latest_copy:
        return get_unique_resource_id(lock_resource_ids)
    # 只做过一次全量备份
    if latest_copy['backup_type'] == BackupTypeEnum.full.value:
        lock_resource_ids.append({"id": latest_copy['uuid'], "lock_type": "r"})
        return get_unique_resource_id(lock_resource_ids)
    lock_resource_ids.extend(get_difference_increment_copy_ids(latest_copy))

    return get_unique_resource_id(lock_resource_ids)


def get_unique_resource_id(lock_resource_ids):
    # 插件返回的值和框架里面的值去重
    unique_lock_resource_ids = []
    seen_ids = set()
    for item in lock_resource_ids:
        resource_id = item.get("id")
        if resource_id not in seen_ids:
            unique_lock_resource_ids.append(item)
            seen_ids.add(resource_id)
    return unique_lock_resource_ids


def get_backup_type(policy):
    backup_type = policy['action']
    if backup_type in (BackupTypeEnum.full.name, BackupTypeEnum.log.name):
        return BackupTypeEnum.full.name
    if policy.get('ext_parameters') and policy.get('ext_parameters').get('permanent_increment'):
        return BackupTypeEnum.full.name
    return backup_type


def get_difference_increment_copy_ids(latest_copy):
    lock_id_list = []
    while latest_copy:
        # 只对备份副本加锁，不对归档、挂载等副本加锁
        if latest_copy['generated_by'] == GenerationType.BY_BACKUP.value:
            log.debug(f"lock_id_list copy uuid: {latest_copy['uuid']}")
            lock_id_list.append({"id": latest_copy['uuid'], "lock_type": "r"})
        if latest_copy.get('prev_copy_id'):
            latest_copy = CopyClient.query_copy_info(latest_copy.get('prev_copy_id'))
            if latest_copy and latest_copy['backup_type'] == BackupTypeEnum.full.value:
                lock_id_list.append({"id": latest_copy['uuid'], "lock_type": "r"})
                break
        else:
            break
    return lock_id_list


def query_latest_copy_by_resource_id(resource_id):
    page_query_res = CopyClient.query_copies(0, 1, {'resource_id': resource_id}, '-display_timestamp')
    if not page_query_res:
        log.error(f"query latest copy by resource id:{resource_id} failed")
        raise EmeiStorBizException(
            CommonErrorCodes.SYSTEM_ERROR,
            message=f"query latest copy by resource id:{resource_id} failed"
        )
    if page_query_res.get('items'):
        return page_query_res.get('items')[0]
    return None


def check_smart_mobility(resource_id):
    resource = ResourceClient.query_resource(resource_id=resource_id)
    # 如果资源/设备信息被删除, 则无法获取设备信息, 此处校验通过, 由后续已有备份流程抛相应错误
    if resource is None or resource == {}:
        log.info(f"resource not exist, skip smart mobility check")
        return ""
    file_system_ids = resource.get("ext_parameters", {}).get("file_system_ids", [])
    log.info(f"file_system_ids:{file_system_ids}")
    if not file_system_ids:
        log.warning("not exists file_system_id.")
        return ""
    filesystem_info = SystemBaseClient.query_filesystem(file_system_ids[0])
    if not filesystem_info:
        log.warning("not exists filesystem_info.")
        return ""
    if filesystem_info.get("hasSmartMobility", "") == "1":
        return filesystem_info.get("name", "")
    return ""


def check_hyper_metro_pair(resource_id):
    protected_obj = ResourceClient.query_protected_object(resource_id=resource_id)

    file_system_id = protected_obj.get("ext_parameters", {}).get("file_system_ids", [])
    log.info(f"file_system_id:{file_system_id}")
    if not file_system_id:
        log.warning("not exists file_system_id.")
        return ""

    filesystem_info = SystemBaseClient.query_filesystem(file_system_id[0])
    hyper_metro_pair_ids = filesystem_info.get("hyperMetroPairIds", [])
    for hyper_metro_pair_id in hyper_metro_pair_ids:
        hyper_metro_pair_info = SystemBaseClient.query_hyper_metro_pair(hyper_metro_pair_id)

        if not hyper_metro_pair_info:
            log.info("not hyper_metro_pair_info")
            return ""

        # 双活从端不允许备份
        log.info(f"hyper_metro_pair_info:{hyper_metro_pair_info}")
        if not hyper_metro_pair_info.get("primary"):
            return filesystem_info.get("name", "")
    return ""


def check_cyber_engine_metro_pair(resource_id):
    resource = ResourceClient.query_resource(resource_id=resource_id)
    # 如果资源/设备信息被删除, 则无法获取设备信息, 此处校验通过, 由后续已有备份流程抛相应错误
    if resource is None or resource == {}:
        log.info(f"resource not exist, skip metro pair check")
        return ""
    env = ResourceClient.query_resource(resource_id=resource.get("root_uuid"))
    if env.get("link_status") == LinkStatusEnum.Offline.value:
        raise EmeiStorBizException(CommonErrorCodes.ERROR_DEVICE_OFFLINE,
                                   message="Device status is offline.")
    file_system_ids = resource.get("ext_parameters", {}).get("file_system_ids", [])
    log.info(f"file_system_ids:{file_system_ids}")
    if not file_system_ids:
        log.warning("not exists file_system_id.")
        return ""
    filesystem_info = SystemBaseClient.query_remote_storage_filesystem(resource.get("root_uuid"), file_system_ids[0])
    if not filesystem_info:
        log.warn("not exists filesystem_info.")
        return ""
    hyper_metro_pair_ids = filesystem_info.get("hyperMetroPairIds", [])
    for hyper_metro_pair_id in hyper_metro_pair_ids:
        hyper_metro_pair_info = SystemBaseClient.query_remote_storage_hyper_metro_pair(resource.get("root_uuid"),
                                                                                       hyper_metro_pair_id)
        if not hyper_metro_pair_info:
            log.info("not hyper_metro_pair_info")
            return ""
        # 双活从端不允许备份
        if not hyper_metro_pair_info.get("primary"):
            return filesystem_info.get("name", "")
    return ""


def check_cyber_engine_replication_pair(resource_id):
    resource = ResourceClient.query_resource(resource_id=resource_id)
    # 如果资源/设备信息被删除, 则无法获取设备信息, 此处校验通过, 由后续已有备份流程抛相应错误
    if resource is None or resource == {}:
        log.warning(f"resource not exist, skip replication pair check")
        return ""
    env = ResourceClient.query_resource(resource_id=resource.get("root_uuid"))
    if env.get("link_status") == LinkStatusEnum.Offline.value:
        raise EmeiStorBizException(CommonErrorCodes.ERROR_DEVICE_OFFLINE,
                                   message="Device status is offline.")
    # Pacific和OP存储设备类型远程复制从端允许打快照、创建/修改共享操作，此处校验通过
    if env.get("sub_type") == OcDeviceTypeEnum.Pacific.value \
            or env.get("sub_type") == OcDeviceTypeEnum.OceanProtect.value:
        log.info(f"Pacific or OceanProtect skip replication pair check")
        return ""
    file_system_ids = resource.get("ext_parameters", {}).get("file_system_ids", [])
    log.info(f"file_system_ids:{file_system_ids}")
    if not file_system_ids:
        log.warning("not exists file_system_id.")
        return ""
    replication_pair_info = get_replication_pair_by_id(
        resource.get("root_uuid"), resource.get("parent_uuid"), file_system_ids[0])
    if not replication_pair_info:
        log.info("not replication_pair_info")
        return ""
    # Dorado远程复制从端不允许进行手动侦测或周期性侦测（远程复制租户pair下的远程复制从端校验失败）
    is_primary = replication_pair_info.get("ISPRIMARY")
    vstore_pair_id = replication_pair_info.get("VSTOREPAIRID")
    if not is_primary and vstore_pair_id != "----":
        log.info(f"OC replication pair secondary check failed, ISPRIMARY:{is_primary}, VSTOREPAIRID:{vstore_pair_id}.")
        return resource.get("name")
    log.info(f"OC replication pair secondary check success, ISPRIMARY:{is_primary}, VSTOREPAIRID:{vstore_pair_id}.")
    return ""


def check_replication_by_deploy_type(resource_id):
    if DeployType().is_cyber_engine_deploy_type():
        filesystem_name = check_cyber_engine_replication_pair(resource_id)
        if filesystem_name:
            return False
    return True


def check_can_be_backup_by_deploy_type(resource_id):
    if DeployType().is_hyper_detect_deploy_type():
        filesystem_name = check_hyper_metro_pair(resource_id)
        if filesystem_name:
            return False
    if DeployType().is_cyber_engine_deploy_type():
        filesystem_name = check_cyber_engine_metro_pair(resource_id)
        if filesystem_name:
            return False
    return True


def check_smart_mobility_can_be_backup(resource_id):
    if DeployType().is_hyper_detect_deploy_type():
        if check_smart_mobility(resource_id):
            return False
    return True


def clean_next_backup_info(protect_obj):
    if not protect_obj:
        log.error("Protect_obj is none.")
        return
    resource_id = protect_obj.get("resource_id")
    ResourceClient.clean_next_backup(resource_id)
    clean_next_backup_info_in_protect_obj(protect_obj)


def clean_next_backup_info_in_protect_obj(protect_obj):
    old_ext_parameters = protect_obj.get("ext_parameters")
    resource_id = protect_obj.get("resource_id")
    if not old_ext_parameters:
        return
    # 为了适配1.2.1升1.3.0的场景，将保护对象中的下次备份相关信息清理掉
    if old_ext_parameters.get(BackupWorkflowConstants.KEY_NEXT_BACKUP_TYPE) or \
            old_ext_parameters.get(BackupWorkflowConstants.KEY_NEXT_BACKUP_CAUSE):
        old_ext_parameters[BackupWorkflowConstants.KEY_NEXT_BACKUP_TYPE] = None
        old_ext_parameters[BackupWorkflowConstants.KEY_NEXT_BACKUP_CAUSE] = None
        with db_config.get_session() as session:
            projected_object.update_by_params(
                db=session,
                resource_id=resource_id,
                update_conditions={
                    ProtectedObject.ext_parameters: json.dumps(old_ext_parameters)
                }
            )
        log.info(f"Resource id: {resource_id}'s old_ext_parameters has been recovered.")


