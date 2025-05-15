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
import calendar
import copy
import json
from datetime import timezone, datetime
import uuid
from itertools import groupby
from typing import List

from sqlalchemy import false, true
from sqlalchemy.orm import Session
from sqlalchemy.orm.attributes import flag_modified

from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.backup.service import backup_workflow, backup_service
from app.base import consts
from app.common import toolkit
from app.common.clients.scheduler_client import SchedulerClient
from app.common.constants.constant import PermanentBackupConstants
from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobType
from app.common.events.consumer import EsEvent
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.log.kernel import load_resource, convert_storage_type
from app.common.logger import get_logger
from app.common.toolkit import JobMessage
from app.common.util.cleaner import clear
from app.protection.object import db
from app.protection.object.models.projected_object import ProtectedTask, ProtectedObject

from app.protection.object.schemas.protected_object import \
    CurrentManualBackupRequest, ModifyProtectionSubmitReq, \
    ProtectedObjectSlaCompliance
from app.protection.object.service.protection_plugin_manager import ProtectionPluginManager

from app.protection.object.service.validator_manager import ValidatorManager
from app.resource.kafka.topics import SCAN_VM_UNDER_COMPUTE_RES
from app.resource.models.database_models import DatabaseTable
from app.resource.models.rbac_models import DomainResourceObjectTable
from app.resource.models.resource_models import EnvironmentTable, ResourceTable
from app.resource.rpc import hw_agent_rpc
from app.common.clients.protection_client import ProtectionClient
from app.common.clients.resource_client import ResourceClient
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum, ProtectionStatusEnum
from app.common.enums.schedule_enum import ExecuteType, ScheduleTypes
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum, TriggerActionEnum, TriggerEnum, WeekDaysEnum, \
    RetentionTypeEnum, MonthDayEnum, WormValidityTypeEnum
from app.common.enums.script_enum import ScriptWhiteListEnum
from app.common.event_messages.Discovery.discovery_rest import ResourceStatus, AuthType
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.rpc import system_base_rpc
from app.resource.service.common import resource_service, domain_resource_object_service
from app.copy_catalog.models.tables_and_sessions import (database, AntiRansomwarePolicyTable, PolicyTable,
                                                         StorageUnitTable, DistributionStorageUnitRelation,
                                                         AntiRansomwarePolicyResourceTable)
from app.protection.object.common.protection_enums import StorageInfoEnum

log = get_logger(__name__)
GROUP_BACKUP_TOPIC = "schedule.group_backup"


def generate_schedule_topic(topic, policy_type: str):
    if topic:
        return topic
    return "schedule." + policy_type


# 记录操作日志时获取资源名称和path地址
def get_endpoint_by_resource_ids(params):
    log.info("operation_log: get name and endpoint by resource_ids")

    res = []
    batch_create_req = params.get("batch_create_req")
    if batch_create_req is None:
        return "--, --"

    resources = batch_create_req.resources
    if resources is None:
        return "--, --"
    resource_ids = list(i.resource_id for i in resources if i is not None)

    # 调/v1/internal/resource/{uuid}查询
    datas = load_resource(resource_ids)

    if datas is None:
        return "--, --"

    for data in datas:
        if data is not None:
            try:
                name_endpoint = f"{data['name']}:{data['path']}"
            except KeyError:
                name_endpoint = f"{data['name']}:--"
            log.info(f"name_endpoint is list")
            res.append(name_endpoint)
    log_data = str(', '.join(res))
    return log_data


def get_batch_protection_log_data(params):
    batch_create_req = params.get("batch_create_req")
    resources = batch_create_req.resources
    resource_ids = list(resources.resource_id for resources in resources if resources is not None)
    log_data = get_log_data(resource_ids)
    post_action = batch_create_req.post_action
    if post_action:
        return log_data, "protection_and_manual_backup_label"
    return log_data, "protection_without_backup_label"


def get_resource_name_and_id(params):
    req = params.get("req")
    resource_ids = req.resource_ids
    log_data = get_log_data(resource_ids)
    return log_data


def get_resource_name_and_id_in_resource_group(params):
    req = params.get("req")
    resource_ids = req.resource_ids
    is_resource_group = req.is_resource_group
    if is_resource_group:
        log_data = get_log_data_for_resource_group(resource_ids)
    else:
        log_data = get_log_data(resource_ids)
    return log_data


def get_manual_backup_log_data(params):
    resource_id = params.get("resource_id")
    backup_req = params.get("backup_req")
    is_resource_group = backup_req.is_resource_group
    # 区分虚拟机和虚拟机组
    if is_resource_group:
        log_data = get_log_data_for_resource_group([resource_id])
    else:
        log_data = get_log_data([resource_id])
    return log_data


def get_modify_protection_log_data(params):
    submit_req = params.get("submit_req")
    resource_id = submit_req.resource_id
    log_data = get_log_data([resource_id])
    return log_data


def get_protection_cyber_log_data(params):
    batch_delete_req = params.get("req")
    resources = batch_delete_req.resource_ids
    # batch delete cyber 实际上是循环调用只有一个resource_id
    return get_log_data_cyber(resources[0])


def get_manual_backup_cyber_log_data(params):
    resource_id = params.get("resource_id")
    res = get_log_data_cyber(resource_id)
    # 特别设置，解决kernel中获取的结果为单个数组问题
    res.append("cyber-array-true")
    return res


def get_batch_create_cyber_protection_log_data(params):
    batch_create_req = params.get("batch_create_req")
    resources = batch_create_req.resources
    resource_ids = list(resources.resource_id for resources in resources if resources is not None)
    # batch create 实际上是循环调用只有一个resource_id
    return get_log_data_cyber(resource_ids[0])


def get_create_cyber_protection_log_data(params):
    create_req = params.get("create_req")
    resource_id = create_req.resource_id
    log_data = get_log_data_cyber(resource_id)
    return log_data


def get_modify_cyber_protection_log_data(params):
    submit_req = params.get("submit_req")
    resource_id = submit_req.resource_id
    log_data = get_log_data_cyber(resource_id)
    return log_data


def get_log_data_cyber(resource_id):
    # cyber 单个查询用户（{0}:{1}）对资源（存储设备名（{2}）、设备序列号（{3}）、设备类型（{4}）、租户名（{5}）、租户ID（{6}）、资源ID（{7}）、资源名（{8}））修改保护。
    resource = load_resource([resource_id])[0]
    res = []
    path = resource.get("path").split("/")
    res.append(path[1])
    res.append(resource.get("root_uuid"))
    res.append(convert_storage_type(path[0]))
    res.append(resource.get("parent_name"))
    res.append(resource.get("parent_uuid"))
    res.append(resource.get("uuid"))
    res.append(resource.get("name"))
    return res


def get_log_data(resource_ids):
    res = []
    # 调/v1/internal/resource/{uuid}查询
    datas = load_resource(resource_ids)
    for data in datas:
        name_id = f"{data.get('name')}:{data.get('uuid')}"
        res.append(name_id)
    log_data = ','.join(res)
    return log_data


def get_log_data_for_resource_group(resource_ids):
    log.info(f"get_log_data_for_resource_group...")
    res = []
    for resource_id in resource_ids:
        resource_group = resource_service.query_resource_group_by_id(resource_id)
        name_id = f"{resource_group.name}:{resource_group.uuid}"
        res.append(name_id)
    log_data = ','.join(res)
    return log_data


def create_schedule_and_build_task(policy, protected_obj):
    """
    创建调度任务并且构造保护对象任务
    :param policy:
    :param protected_obj:
    :return:
    """
    task_id = str(uuid.uuid4())
    SchedulerClient.create_interval_schedule(task_id, policy, ScheduleTypes.interval.value, protected_obj,
                                             protected_obj.chain_id)
    task = ProtectedTask(
        uuid=task_id,
        policy_id=policy["uuid"],
        protected_object_id=str(protected_obj.uuid),
        schedule_id=task_id)
    return task


def build_schedule_params(topic, resource_id, sla_id, chain_id, policy, execute_type) -> dict:
    """
    构造schedule 调度执行之后消息体数据(预留归档、复制扩展)
    :param topic: 执行调度发送的topic
    :param resource_id: 资源id
    :param sla_id: sla的id
    :param chain_id: 当前调度的链id
    :param policy: 当前调度对应的策略
    :param execute_type: 执行类型
    :return:
    """
    if topic in [PolicyTypeEnum.backup.value, GROUP_BACKUP_TOPIC]:
        ext_parameters = policy.get("ext_parameters")
        auto_retry = ext_parameters is not None and ext_parameters.get(
            "auto_retry")
        backup_params = {
            "resource_id": resource_id,
            "sla_id": sla_id,
            "chain_id": chain_id,
            "policy": policy,
            "execute_type": execute_type,
            "auto_retry": auto_retry
        }
        if auto_retry:
            backup_params["auto_retry_times"] = ext_parameters.get(
                "auto_retry_times")
            backup_params["auto_retry_wait_minutes"] = ext_parameters.get(
                "auto_retry_wait_minutes")
        return backup_params
    elif topic == PolicyTypeEnum.archiving:
        return {
            "resource_id": resource_id,
            "sla_id": sla_id,
            "chain_id": chain_id,
            "policy": policy,
            "execute_type": execute_type
        }
    elif topic == PolicyTypeEnum.replication:
        return {
            "resource_id": resource_id,
            "sla_id": sla_id,
            "chain_id": chain_id,
            "policy": policy,
            "execute_type": execute_type
        }
    elif topic == SCAN_VM_UNDER_COMPUTE_RES:
        return {
            "resource_id": resource_id
        }
    else:
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message=f"create schedule topic[{topic}] is not supported")


def _construct_schedule_start_time(schedule):
    constructed_schedule = copy.deepcopy(schedule)
    if schedule.get("trigger") == TriggerEnum.customize_interval and \
            schedule.get("trigger_action") == TriggerActionEnum.year:
        constructed_schedule["interval"] = "1"
        constructed_schedule["interval_unit"] = "Y"
    if schedule.get("trigger") == TriggerEnum.customize_interval and \
            schedule.get("trigger_action") == TriggerActionEnum.week:
        constructed_schedule["days_of_week"] = ",".join(
            list(WeekDaysEnum.get_value(key) for key in schedule.get("days_of_week")))
    return constructed_schedule


def build_protection_task(obj_id, policy, topic: str, schedule_params: dict, pre_task_id=None):
    """
    构造保护对象任务

    :param obj_id: 保护对象id
    :param policy: 对应sla中的策略
    :param topic: 保护任务对应的调度任务的topic
    :param schedule_params: 调度执行的参数
    :param pre_task_id: 原来已经存在的task_id
    :return:
    """
    task_id = pre_task_id if pre_task_id else str(uuid.uuid5(uuid.NAMESPACE_OID, obj_id + str(policy.get("uuid"))))
    policy_schedule = _construct_schedule_start_time(policy.get("schedule"))
    log.info(f"Create protected object, start time={policy_schedule.get('window_start')}, "
             f"end time={policy_schedule.get('window_end')}, task id: {task_id}")
    if policy_schedule.get("trigger") == 4 and policy_schedule.get("trigger_action") != "year":
        SchedulerClient.create_customize_interval_schedule(
            schedule_id=task_id,
            day_of_week=policy_schedule.get("days_of_week"),
            day_of_month=policy_schedule.get("days_of_month"),
            daily_start_time=policy_schedule.get("window_start"),
            daily_end_time=policy_schedule.get("window_end"),
            schedule_type=policy_schedule.get("trigger_action"),
            topic=topic,
            params=schedule_params
        )
        return ProtectedTask(
            uuid=task_id,
            policy_id=policy.get("uuid"),
            protected_object_id=obj_id,
            schedule_id=task_id)
    SchedulerClient.create_interval_schedule_new(
        task_id,
        str(policy_schedule.get("interval")) +
        policy_schedule.get("interval_unit"),
        ScheduleTypes.interval.value,
        policy_schedule.get("start_time"),
        topic,
        schedule_params
    )
    return ProtectedTask(
        uuid=task_id,
        policy_id=policy.get("uuid"),
        protected_object_id=obj_id,
        schedule_id=task_id)


def build_protection_task_and_schedule_param(policy, topic: str, schedule_params: dict):
    """
    构造保护对象任务

    :param policy: 对应sla中的策略
    :param topic: 保护任务对应的调度任务的topic
    :param schedule_params: 调度执行的参数
    :return:
    """
    policy_schedule = _construct_schedule_start_time(policy.get("schedule"))
    log.info(f"Create protected object, start time={policy_schedule.get('window_start')}, "
             f"end time={policy_schedule.get('window_end')}")
    if policy_schedule.get("trigger") == 4 and policy_schedule.get("trigger_action") != "year":
        schedule_req = {
            "schedule_name": "",
            "day_of_week": policy_schedule.get("days_of_week"),
            "day_of_month": policy_schedule.get("days_of_month"),
            "daily_start_time": policy_schedule.get("window_start"),
            "daily_end_time": policy_schedule.get("window_end"),
            "schedule_type": policy_schedule.get("trigger_action"),
            "action": topic,
            "params": json.dumps(schedule_params)
        }
    else:
        schedule_req = {
            "schedule_name": "",
            "interval": str(policy_schedule.get("interval")) + policy_schedule.get("interval_unit"),
            "schedule_type": ScheduleTypes.interval.value,
            "action": topic,
            "params": json.dumps(schedule_params),
            'start_date': policy_schedule.get("start_time")
        }
    return schedule_req


def build_protection_object(resource, sla, ext_parameters, topic: str = None,
                            create_req: ModifyProtectionSubmitReq = None):
    """
    构造保护对象

    :param resource: 资源信息
    :param sla: sla信息
    :param ext_parameters: 保护对象对应的高级参数
    :param topic: 保护任务对应的调度任务的topic
    :param create_req: 创建保护req
    :return:
    """
    if create_req:
        is_resource_group = create_req.is_resource_group
        is_group_sub_resource = create_req.is_group_sub_resource
        resource_group_id = create_req.resource_group_id or ""
    else:
        is_resource_group, is_group_sub_resource = False, False
        resource_group_id = ""
    resource_id = resource.get("uuid")
    chain_id = str(uuid.uuid4())
    sla_id = sla.get("uuid")
    # 针对资源组子资源或者资源组创建保护对象，需获取资源组id
    if is_group_sub_resource and not resource_group_id:
        resource_group_member = resource_service.query_resource_group_member_by_resource_id(resource_id)
        if resource_group_member:
            resource_group_id = resource_group_member.resource_group_id
    if is_resource_group:
        resource_group_id = resource_id
        topic = GROUP_BACKUP_TOPIC
    obj_params = {
        "uuid": resource_id,
        "resource_id": resource_id,
        "resource_group_id": resource_group_id,
        "sla_id": sla_id,
        "sla_name": sla.get("name"),
        "ext_parameters": ext_parameters.json(),
        "name": resource.get("name"),
        "path": resource.get("path"),
        "type": resource.get("type"),
        "sub_type": resource.get("sub_type"),
        "env_id": resource.get("root_uuid", ""),
        "chain_id": chain_id,
        "status": 1,
        "task_list": list(build_protection_task(
            resource_id,
            policy,
            generate_schedule_topic(topic, policy.get("type")),
            build_schedule_params(topic if topic else policy.get("type"),
                                  resource.get("uuid"), sla_id, chain_id, policy,
                                  ExecuteType.AUTOMATIC.value)
        ) for policy in sla["policy_list"] if filter_sla_policy(sla, policy, is_resource_group, is_group_sub_resource))
    }
    return ProtectedObject(**obj_params)


def build_sub_vm_protection_object(resource, sla, ext_parameters, resource_group_id):
    is_resource_group, is_group_sub_resource = False, True
    resource_id = resource.get("uuid")
    chain_id = str(uuid.uuid4())
    sla_id = sla.get("uuid")
    obj_params = {
        "uuid": resource_id,
        "resource_id": resource_id,
        "resource_group_id": resource_group_id,
        "sla_id": sla_id,
        "sla_name": sla.get("name"),
        "ext_parameters": ext_parameters.json(),
        "name": resource.get("name"),
        "path": resource.get("path"),
        "type": resource.get("type"),
        "sub_type": resource.get("sub_type"),
        "env_id": resource.get("root_uuid", ""),
        "chain_id": chain_id,
        "status": 1,
        "task_list": list(build_protection_task(
            resource_id,
            policy,
            "schedule." + policy.get("type"),
            build_schedule_params(policy.get("type"),
                                  resource.get("uuid"), sla_id, chain_id, policy,
                                  ExecuteType.AUTOMATIC.value)
        ) for policy in sla["policy_list"] if filter_sla_policy(sla, policy, is_resource_group, is_group_sub_resource))
    }
    return ProtectedObject(**obj_params)


def build_protection_object_without_task(resource, sla, ext_parameters):
    """
    构造保护对象

    :param resource: 资源信息
    :param sla: sla信息
    :param ext_parameters: 保护对象对应的高级参数
    :return:
    """
    resource_id = resource.get("uuid")
    chain_id = str(uuid.uuid4())
    sla_id = sla.get("uuid")
    obj_params = {
        "uuid": resource_id,
        "resource_id": resource_id,
        "sla_id": sla_id,
        "sla_name": sla.get("name"),
        "ext_parameters": ext_parameters.json(),
        "name": resource.get("name"),
        "path": resource.get("path"),
        "type": resource.get("type"),
        "sub_type": resource.get("sub_type"),
        "env_id": resource.get("root_uuid"),
        "chain_id": chain_id,
        "status": 1,
        "task_list": []
    }
    return ProtectedObject(**obj_params)


def filter_sla_policy(sla, policy, is_resource_group: bool = False, is_group_sub_resource: bool = False) -> bool:
    """
    判断SLA中的策略是否需要创建调度任务
    :param sla: sla信息
    :param policy: sla策略
    :param is_resource_group: 资源组创建保护对象场景
    :param is_group_sub_resource: 资源组子资源创建保护对象场景。
    :return:
    """
    if is_resource_group:
        return policy.get("type") == PolicyTypeEnum.backup.value

    if is_group_sub_resource:
        if policy.get("type") == PolicyTypeEnum.backup.value:
            return False

    if policy.get("type") == PolicyTypeEnum.backup.value:
        return True
    if policy.get("type") == PolicyTypeEnum.replication.value:
        if (policy.get("schedule") is not None) and (policy.get("schedule").get("trigger") == TriggerEnum.interval):
            return True
        return False
    if policy.get("type") == PolicyTypeEnum.archiving.value:
        if (policy.get("schedule") is not None) and (policy.get("schedule").get("trigger") == TriggerEnum.interval):
            return True
    return False


def check_os_type_script(os_type, value, resource_sub_type):
    if os_type == "windows" and not value.endswith(".bat"):
        raise EmeiStorBizException(error=ResourceErrorCodes.PROTECTED_OBJECT_SCRIPT_FORMAT_INCORRECT,
                                   message="The script is not a valid .bat file path")
    elif os_type in ["linux", "RedHat"]:
        if not value.endswith(".sh"):
            raise EmeiStorBizException(error=ResourceErrorCodes.PROTECTED_OBJECT_SCRIPT_FORMAT_INCORRECT,
                                       message="The script is not a valid .sh file path")


def check_protected_obj_script(resource_obj, ext_parameters):
    # VMWare先不校验 脚本 只有Oracle才会校验脚本
    if resource_obj["sub_type"] not in [ResourceSubTypeEnum.Oracle, ResourceSubTypeEnum.ORACLE_CLUSTER]:
        return
    if not ext_parameters.dict():
        return
    for key, value in ext_parameters.dict().items():
        # 主机卷在保护对象中有path这个选项  如果参数是代理主机或者是否删除归档日志，不做校验
        if key not in ScriptWhiteListEnum.__members__.values():
            continue
        if not value:
            continue
        if len(value) <= consts.SCRIPT_PATH_MAX_LENGTH:
            check_os_type_script(
                resource_obj["environment_os_type"], value, resource_obj["sub_type"])
        elif len(value) > consts.SCRIPT_PATH_MAX_LENGTH:
            raise EmeiStorBizException(error=ResourceErrorCodes.PROTECTED_OBJECT_SCRIPT_FORMAT_INCORRECT,
                                       message="The length of script can not more than " +
                                               str(consts.SCRIPT_PATH_MAX_LENGTH))


def check_is_container_resource(sub_type: str) -> bool:
    """
    校验是否是容器类资源
    :param sub_type: 资源子类型
    :return:
    """
    return ResourceSubTypeEnum(sub_type) in [ResourceSubTypeEnum.ClusterComputeResource,
                                             ResourceSubTypeEnum.HostSystem]


def check_is_resources_group(resource_group_id: str) -> bool:
    """
    校验是否是资源组类型资源
    :param resource_group_id： 保护对象资源组id
    :return:
    """
    return resource_group_id is not None and resource_group_id != ""


processor_manager = ValidatorManager()


def select_manual_backup_policies(policy):
    """
    选择可以执行的手动备份的SLA策略: 选择类型为备份，action非日志的策略
    :param policy:
    :return:
    """
    return policy["type"] == PolicyTypeEnum.backup.value


def extract_ids(projected_object_list: List[ProtectedObject]):
    """
    提取保护对象任务的id列表和调度任务id列表
    :param projected_object_list:
    :return:
    """
    task_lists = list(projected_object.task_list for projected_object in projected_object_list)
    task_list = list(task for tasks in task_lists for task in tasks)
    task_id_list = list(task.uuid for task in task_list)
    schedule_id_list = list(task.schedule_id for task in task_list)
    return task_id_list, schedule_id_list


def execute_manual_backup(protected_object: ProtectedObject, backup_req: CurrentManualBackupRequest):
    resource_id = protected_object.resource_id
    if not backup_req.is_resource_group and not backup_service.check_can_be_backup_by_deploy_type(resource_id):
        raise_metro_pair_cannot_backup_exception(resource_id)
    if not backup_req.is_resource_group and not backup_service.check_replication_by_deploy_type(resource_id):
        if DeployType().is_cyber_engine_deploy_type():
            raise EmeiStorBizException(ProtectionErrorCodes.CYBERENGINE_SYNCHRONOUS_REPLICATION_SECONDARY,
                                       backup_service.check_cyber_engine_replication_pair(resource_id))
    if not backup_req.is_resource_group and not backup_service.check_smart_mobility_can_be_backup(resource_id):
        raise_smart_mobility_cannot_backup_exception(resource_id)
    # 构造备份策略
    policy = build_policy_from_backup_req(protected_object, backup_req)

    # 获取保护对象与policy绑定关系中的chain_id
    chain_id = protected_object.chain_id
    schedule_params = build_schedule_params(policy.get("type"), resource_id, protected_object.sla_id,
                                            chain_id, policy, ExecuteType.MANUAL.value)
    if backup_req.copy_name:
        schedule_params[BackupWorkflowConstants.COPY_NAME] = backup_req.copy_name

    if backup_req.is_resource_group:
        schedule_params["resource_group_id"] = resource_id

    backup_job_id = ''
    try:
        schedule_params['timestamp'] = datetime.now(tz=timezone.utc).timestamp() * 1000
        schedule_params['user_id'] = backup_req.user_id
        if backup_req.is_resource_group:
            backup_job_id = backup_workflow.group_backup_start(request=EsEvent(str(uuid.uuid4())),
                                                               params=schedule_params)
        else:
            backup_job_id = backup_workflow.backup_start(request=EsEvent(str(uuid.uuid4())), params=schedule_params)
    except Exception as e:
        log.error(f"create backup job failed. resource_id:{resource_id} exception:{e}")
        raise e
    finally:
        pass
    return backup_job_id


def check_is_last_day():
    current_time = datetime.now(tz=timezone.utc)
    last_day = calendar.monthrange(datetime.now(tz=timezone.utc).year, datetime.now(tz=timezone.utc).month)[1]
    to_last_day = last_day - current_time.day
    return to_last_day == 0


def last_day_list_in_sla(policy_list):
    is_last_day_policy_list = [policy for policy in policy_list if check_last_day_policy(policy)]
    if len(is_last_day_policy_list) > 0:
        return is_last_day_policy_list[0]
    else:
        log.info(f"No policy with last day execution found in sla")
        return policy_list[0]


def check_last_day_policy(policy):
    schedule = policy.get("schedule", {})
    trigger_action = schedule.get("trigger_action")
    if trigger_action == TriggerActionEnum.month \
            and schedule.get("days_of_month") == MonthDayEnum.last_day_of_each_month:
        return True
    return False


def build_policy_from_backup_req(protected_object: ProtectedObject, backup_req: CurrentManualBackupRequest):
    # 筛选SLA中的policy可以手动备份的策略
    sla = ProtectionClient.query_sla(backup_req.sla_id)
    backup_policy_list = [policy for policy in sla.get("policy_list") if
                          select_manual_backup_policies(policy)]
    if len(backup_policy_list) <= 0:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   message="select sla policy not exist")
    execute_policy_list = list(policy for policy in backup_policy_list if
                               policy["action"] == backup_req.action)
    if len(execute_policy_list) <= 0 and backup_req.action == PolicyActionEnum.full:
        # 下发是全量，复用永久增量的policy
        execute_policy_list = list(policy for policy in backup_policy_list if
                                   policy["action"] == PolicyActionEnum.permanent_increment)
        # 有些应用的永久增量类型是difference_increment，这里对他们单独适配
        if (len(execute_policy_list) <= 0 and
                protected_object.sub_type in PermanentBackupConstants.DIFFERENCE_INCREMENT_APP_SUBTYPE):
            execute_policy_list = list(policy for policy in backup_policy_list if
                                       policy["action"] == PolicyActionEnum.difference_increment)
        # 保证执行的policy类型与下发备份一致
        _replace_backup_action(execute_policy_list, backup_req)

    if len(execute_policy_list) <= 0 and _is_permanent_backup_req(protected_object, backup_req):
        # 下发类型是永久增量类型, 复用全量的Policy
        execute_policy_list = list(policy for policy in backup_policy_list if policy["action"] == PolicyActionEnum.full)
        # 保证执行的policy类型与下发备份一致
        _replace_backup_action(execute_policy_list, backup_req)

    if len(execute_policy_list) > 0:
        # 如果今天是本月最后一天，并且sla中有配置为最后一天的策略 则使用这一个
        if check_is_last_day():
            policy = last_day_list_in_sla(execute_policy_list)
        else:
            policy = execute_policy_list[0]
    else:
        policy = backup_policy_list[0]
        policy['action'] = backup_req.action.value
        policy['name'] = backup_req.action.value
        if not policy['schedule']['window_start']:
            policy['schedule']['window_start'] = '00:00:00'
            policy['schedule']['window_end'] = '00:00:00'
        if "retention" not in policy:
            policy["retention"] = {}
        policy["retention"]["retention_type"] = RetentionTypeEnum.permanent
        policy["worm_validity_type"] = WormValidityTypeEnum.worm_not_open.value
    # 安全一体机, 手动快照场景, 是否立即检测字段固定为true, 未感染快照锁定和保留时间从入参获取
    if DeployType().is_cyber_engine_deploy_type():
        if "ext_parameters" in policy:
            policy["ext_parameters"]["need_detect"] = True
            policy["ext_parameters"]["is_security_snap"] = backup_req.is_security_snap
            policy["ext_parameters"]["is_backup_detect_enable"] = backup_req.is_backup_detect_enable
            policy["ext_parameters"]["upper_bound"] = backup_req.upper_bound
        if "retention" in policy:
            policy["retention"]["retention_type"] = backup_req.retention_type
            policy["retention"]["retention_duration"] = backup_req.retention_duration
            policy["retention"]["duration_unit"] = backup_req.duration_unit
    return policy


def _is_permanent_backup_req(protected_object, backup_req):
    """
    判断是不是永久增量
    """
    if backup_req.action == PolicyActionEnum.permanent_increment:
        return True
    if (backup_req.action == PolicyActionEnum.difference_increment
            and protected_object.sub_type in PermanentBackupConstants.DIFFERENCE_INCREMENT_APP_SUBTYPE):
        return True
    return False


def _replace_backup_action(execute_policy_list, backup_req: CurrentManualBackupRequest):
    if len(execute_policy_list) > 0:
        for policy in execute_policy_list:
            policy['action'] = backup_req.action.value


def virtual_resource_manual_backup(session: Session,
                                   protected_object: ProtectedObject,
                                   backup_req: CurrentManualBackupRequest):
    """
    虚拟化资源的手动备份
    :param session:
    :param protected_object:
    :param backup_req:
    :return:
    """
    plm = ProtectionPluginManager(ResourceSubTypeEnum(protected_object.sub_type))
    sub_resource_list = plm.query_sub_resources_only_vm(protected_object)
    if protected_object.sub_type == ResourceSubTypeEnum.FusionCompute \
            or protected_object.sub_type == ResourceSubTypeEnum.FUSION_ONE_COMPUTE:
        log.info(f"FC only backup VM, resource_id:{protected_object.resource_id}")
        sub_resource_list = [resource for resource in sub_resource_list if resource.get("type") == ResourceTypeEnum.VM]
    sub_resource_ids = list(sub_resource.get("uuid") for sub_resource in sub_resource_list)
    # 查询绑定了相同SLA的子资源
    sub_protected_object = db.projected_object.query_multi_by_params(db=session, conditions=[
        ProtectedObject.resource_id.in_(sub_resource_ids),
        ProtectedObject.sla_id == backup_req.sla_id
    ])
    backup_job_ids = []
    for obj in sub_protected_object:
        # 对子资源分别执行一次手动备份
        backup_job_id = execute_manual_backup(obj, backup_req)
        if backup_job_id:
            backup_job_ids.append(backup_job_id)
    return backup_job_ids


def test_auth(db_session, database_resource):
    filters = {EnvironmentTable.uuid == database_resource.root_uuid}
    env = db_session.query(EnvironmentTable).filter(*filters).first()
    db_name = database_resource.name
    host_ip = env.endpoint
    path = database_resource.path
    inst_name = database_resource.inst_name
    db_username = database_resource.db_username
    db_password = database_resource.db_password

    if env.is_cluster:
        # 查在线同名且数据认证的节点数据库
        children_database_list = db_session.query(DatabaseTable). \
            filter(DatabaseTable.root_uuid.in_(env.children_uuids),
                   DatabaseTable.discriminator == consts.DATABASES_CONST,
                   DatabaseTable.link_status == ResourceStatus.ON_LINE.value,
                   DatabaseTable.name == database_resource.name,
                   DatabaseTable.auth_type == AuthType.OS_OFF.value).all()
        if len(children_database_list) <= 0:
            # 没有数据库认证的在线数据库
            raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR, parameters=[])
        children_database = children_database_list[0]
        path = children_database.path
        inst_name = children_database.inst_name
        children_host = db_session.query(EnvironmentTable). \
            filter(EnvironmentTable.uuid == children_database.root_uuid).one()
        host_ip = children_host.endpoint

    try:
        db_password = system_base_rpc.decrypt(db_password)
        hw_agent_rpc.testconnection(
            host_ip, db_name, inst_name, path, db_username, db_password)
    except Exception:
        db_session.query(DatabaseTable).filter(DatabaseTable.uuid == database_resource.uuid). \
            update({DatabaseTable.verify_status: False})
        log.exception("Failed to test the login before the backup:{}".format(
            database_resource.uuid))
    finally:
        pass
    # 清除密码
    clear(db_password)


def common_resource_manual_backup(session: Session,
                                  protected_object: ProtectedObject,
                                  backup_req: CurrentManualBackupRequest):
    """
    通用资源类型手动备份
    :param session:
    :param protected_object:
    :param backup_req:
    :return:
    """
    backup_job_ids = []
    backup_job_id = execute_manual_backup(protected_object, backup_req)
    if backup_job_id:
        backup_job_ids.append(backup_job_id)
    return backup_job_ids


def is_need_generate_new_chain_id(resource_type: ResourceSubTypeEnum) -> bool:
    """
    判断是否需要生成新的chain_id，当应用支持索引的时候，需要生成新的chain_id
    :param resource_type:
    :return:
    """
    return resource_type in [ResourceSubTypeEnum.ClusterComputeResource, ResourceSubTypeEnum.HostSystem,
                             ResourceSubTypeEnum.VirtualMachine]


def check_action_and_small_file(policy_list, ext_parameters):
    """
    新增高参规则：永久增量备份SLA不能开启小文件聚合
    :param policy_list: 策略列表
    :param ext_parameters: 扩展参数
    :return:
    """
    is_small_file_aggregation_open = ext_parameters.small_file_aggregation
    if not is_small_file_aggregation_open:
        return
    for policy in policy_list:
        action = policy.get("action")
        if action == PolicyActionEnum.permanent_increment:
            log.error(f"SLA action: {action}, small_file_aggregation is open : {is_small_file_aggregation_open}")
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="small_file_aggregation and permanent_increment cant open at same time")


def has_children_need_backup(resource_type, resource_sub_type) -> bool:
    # 子资源是否需要备份，这个列表列举虚拟机第一层父资源和第二层父资源
    if resource_sub_type in [ResourceSubTypeEnum.ClusterComputeResource, ResourceSubTypeEnum.HostSystem,
                             ResourceSubTypeEnum.CNWARE_CLUSTER, ResourceSubTypeEnum.CNWARE_HOST,
                             ResourceSubTypeEnum.NUTANIX_CLUSTER, ResourceSubTypeEnum.NUTANIX_HOST,
                             ResourceSubTypeEnum.APSARA_STACK_ZONE, ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
                             ResourceSubTypeEnum.KubernetesNamespace, ResourceSubTypeEnum.HCSProject,
                             ResourceSubTypeEnum.OPENSTACK_PROJECT, ResourceSubTypeEnum.HYPER_V_HOST]:
        return True
    if resource_sub_type in [ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.FUSION_ONE_COMPUTE] \
            and resource_type in [ResourceTypeEnum.Cluster, ResourceTypeEnum.Host]:
        return True
    return False


def check_is_need_build_task(resource_type, resource_sub_type):
    # 自身是否需要创建任务调度，这个列表只列举虚拟机第一层父资源。
    if resource_sub_type in [ResourceSubTypeEnum.KubernetesNamespace, ResourceSubTypeEnum.HCSProject,
                             ResourceSubTypeEnum.OPENSTACK_PROJECT, ResourceSubTypeEnum.CNWARE_HOST,
                             ResourceSubTypeEnum.APSARA_STACK_ZONE, ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
                             ResourceSubTypeEnum.NUTANIX_HOST, ResourceSubTypeEnum.HYPER_V_HOST]:
        return False
    if resource_sub_type in [ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.FUSION_ONE_COMPUTE] \
            and resource_type in [ResourceTypeEnum.Cluster, ResourceTypeEnum.Host]:
        return False
    return True


def is_resources_group_sub_resource(projected_object: ProtectedObject):
    return projected_object.resource_group_id is not None and \
        projected_object.resource_group_id != "" and \
        projected_object.resource_group_id != projected_object.uuid


def build_new_task_list(projected_object, old_policy_schedule_id, sla, object_update_infos: dict):
    if not check_is_need_build_task(projected_object.type, projected_object.sub_type):
        return []
    if is_resources_group_sub_resource(projected_object):
        return []
    task_list = []
    for policy in sla.get("policy_list"):
        if not filter_sla_policy(sla, policy):
            continue
        policy_id = policy.get("uuid")
        pre_task_id = None
        if old_policy_schedule_id:
            pre_task_id = old_policy_schedule_id.get(policy_id)
        task_id = pre_task_id if pre_task_id else str(
            uuid.uuid5(uuid.NAMESPACE_OID, projected_object.uuid + str(policy_id)))
        protect_task = ProtectedTask(
            uuid=task_id,
            policy_id=policy_id,
            protected_object_id=projected_object.uuid,
            schedule_id=task_id)
        task_list.append(protect_task)
        object_update_info = {
            "resource_id": projected_object.uuid,
            "chain_id": projected_object.chain_id,
            "task_id": task_id
        }

        if policy_id in object_update_infos:
            object_update_infos.get(policy_id).append(object_update_info)
        else:
            object_update_infos[policy_id] = [object_update_info]
    return task_list


def build_schedule_param_for_topic(topic, policy, sla_id):
    schedule_param = build_schedule_params(topic if topic else policy.get("type"), None, sla_id, None, policy,
                                           ExecuteType.AUTOMATIC.value)
    return build_protection_task_and_schedule_param(policy, generate_schedule_topic(topic, policy.get("type")),
                                                    schedule_param)


def get_projected_object_topic(projected_object):
    topic = ""
    if check_is_resources_group(projected_object.resource_group_id):
        topic = GROUP_BACKUP_TOPIC
    if check_is_container_resource(projected_object.sub_type):
        topic = SCAN_VM_UNDER_COMPUTE_RES
    return topic


def create_job(sla: dict, user_id, resource_object: dict, message: JobMessage = None):
    request_id = str(uuid.uuid4())
    if message:
        message.payload['job_id'] = request_id
    # 增加extendField字段，包含SLA信息
    job_extend_params = {
        "sla_name": sla.get("name"),
        "sla_id": sla.get("uuid")
    }
    domain_id_list = domain_resource_object_service.get_domain_id_list(resource_object.get("uuid"))
    job_id = toolkit.create_job_center_task(request_id, {
        'requestId': request_id,
        'sourceId': resource_object.get("uuid"),
        'sourceName': resource_object.get("name"),
        'sourceType': resource_object.get("type"),
        'sourceSubType': resource_object.get("sub_type"),
        'sourceLocation': resource_object.get("path", ""),
        "type": JobType.RESOURCE_PROTECTION_MODIFY.value,
        'userId': user_id,
        'domainIdList': domain_id_list,
        'enableStop': False,
        "extendField": job_extend_params
    }, message)
    if not job_id:
        raise EmeiStorBizException(
            error=CommonErrorCodes.SYSTEM_ERROR, message="create resource protection modify job failed.")
    return job_id


def raise_metro_pair_cannot_backup_exception(resource_id):
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException(ProtectionErrorCodes.CYBERENGINE_HYPER_METRO_SECONDARY_CANNOT_BACKUP,
                                   backup_service.check_cyber_engine_metro_pair(resource_id))
    else:
        raise EmeiStorBizException(ProtectionErrorCodes.CLOUDBACKUP_SYNCHRONOUS_REPLICATION_SECONDARY,
                                   backup_service.check_hyper_metro_pair(resource_id))


def raise_smart_mobility_cannot_backup_exception(resource_id):
    raise EmeiStorBizException(ProtectionErrorCodes.CLOUDBACKUP_SMART_MOBILITY_CANNOT_BACKUP,
                               message="resource has smart mobility, can not backup.")


# 情况3：保护资源时，如果资源已被防勒索或worm保护，选择的sla包含本地盘则报错，更新保护时，选择的sla包含本地盘也报错
def check_resource_has_anti_or_worm(resource_id: str):
    if resource_id is None or len(resource_id) == 0:
        log.info("resource_id is null")
        return False
    with database.session() as session:
        anti_ransomware_policy_resources = session.query(AntiRansomwarePolicyResourceTable).filter(
            AntiRansomwarePolicyResourceTable.resource_id == resource_id).all()
        if anti_ransomware_policy_resources is None:
            log.info("anti ransomware policy resources is null")
            return False
        for anti_ransomware_policy_resource in anti_ransomware_policy_resources:
            policy_id = anti_ransomware_policy_resource.policy_id
            if policy_id is None:
                log.info("policy id is null")
                continue
            anti_ransomware_policy: AntiRansomwarePolicyTable = session.query(AntiRansomwarePolicyTable).filter(
                AntiRansomwarePolicyTable.id == policy_id).one_or_none()
            if anti_ransomware_policy is not None:
                return anti_ransomware_policy.need_detect or anti_ransomware_policy.set_worm
    return False


def get_policy_results(session, sla_id):
    if sla_id is None or len(sla_id) == 0:
        return []
    return session.query(PolicyTable).filter(PolicyTable.sla_id == sla_id).all()


def check_storage_support(storage_info):
    if storage_info is None:
        return
    if not is_support_worm_and_anti_ransom(storage_info):
        raise EmeiStorBizException(
            error=CommonErrorCodes.BASIC_DISK_NOT_SUPPORT_WORM_AND_ANTI,
            message="the local disk cannot support worm and antiransom."
        )


def check_is_support_worm_and_anti(sla_id: str):
    if sla_id is None or len(sla_id) == 0:
        return
    with database.session() as session:
        results = get_policy_results(session, sla_id)
        for policy in results:
            ext_parameters = policy.ext_parameters
            if ext_parameters is None or len(ext_parameters) == 0:
                continue
            log.info(f"ext_parameters: {ext_parameters}")
            ext_parameters_node = json.loads(ext_parameters)
            storage_info = ext_parameters_node.get(StorageInfoEnum.STORAGE_INFO)
            check_storage_support(storage_info)


def is_storage_unit(storage_info):
    storage_type = storage_info.get(StorageInfoEnum.STORAGE_TYPE).strip('"')
    return storage_type == StorageInfoEnum.STORAGE_UNIT


def is_storage_unit_group(storage_info):
    storage_type = storage_info.get(StorageInfoEnum.STORAGE_TYPE).strip('"')
    return storage_type == StorageInfoEnum.STORAGE_UNIT_GROUP


def fetch_storage_unit(storage_id):
    with database.session() as session:
        return session.query(StorageUnitTable).filter(StorageUnitTable.id == storage_id).one_or_none()


def fetch_unit_ids(storage_id):
    with database.session() as session:
        results = session.query(DistributionStorageUnitRelation).filter(
            DistributionStorageUnitRelation.distribution_id == storage_id).all()
        return [relation.unit_id for relation in results]


def is_support_worm_and_anti_ransom(storage_info):
    storage_id = storage_info.get(StorageInfoEnum.STORAGE_ID).strip('"')
    storage_type = storage_info.get(StorageInfoEnum.STORAGE_TYPE).strip('"')
    log.info(f"storage_id: {storage_id}, storage_type: {storage_type}")

    if is_storage_unit(storage_info):
        storage_unit = fetch_storage_unit(storage_id)
        if storage_unit and storage_unit.device_type == StorageInfoEnum.BASIC_DISK:
            return False

    if is_storage_unit_group(storage_info):
        unit_ids = fetch_unit_ids(storage_id)
        for unit_id in unit_ids:
            storage_unit = fetch_storage_unit(unit_id)
            if storage_unit and storage_unit.device_type == StorageInfoEnum.BASIC_DISK:
                return False

    return True


def check_sla_can_be_bounded(sla_obj, resource):
    policy_list = sla_obj.get("policy_list")
    for policy in policy_list:
        if policy.get("type") != PolicyTypeEnum.backup:
            continue
        storage_info = policy.get("extParameters").get("storage_info")
        if not storage_info:
            continue
    pass


def fill_protection_ext_parameters(sla_obj, ext_parameters):
    worm_switch = False
    for policy in sla_obj.get("policy_list"):
        worm_validity_type = policy.get("worm_validity_type", WormValidityTypeEnum.worm_not_open.value)
        if (policy.get("type") == PolicyTypeEnum.backup.value and
                worm_validity_type != WormValidityTypeEnum.worm_not_open.value):
            worm_switch = True
    ext_parameters.worm_switch = worm_switch


class ProtectedObjectService(object):

    @staticmethod
    def create_projected_object(
            session: Session,
            create_req: ModifyProtectionSubmitReq) -> str:
        # 查询当前是否存在相同的保护对象，resource_id和sla_id都相同
        resource_id = create_req.resource_id
        current_protected_obj = db.projected_object.query_one_by_resource_id(
            db=session, resource_id=resource_id)
        # 保护对象存在，sla_id也存在，多个sla不能绑定一个资源
        if current_protected_obj is not None and current_protected_obj.sla_id is not None:
            raise EmeiStorBizException(error=ResourceErrorCodes.RESOURCE_ALREADY_PROTECTED,
                                       message="resource already protected")
        # 保护资源时，如果资源已被防勒索或worm保护，选择的sla包含本地盘则报错
        if DeployType().is_dependent() and check_resource_has_anti_or_worm(resource_id):
            check_is_support_worm_and_anti(create_req.sla_id)
        # 查询资源详细信息
        if create_req.is_resource_group:
            resource = resource_service.query_resource_group_by_id(resource_id).as_dict()
            resource["type"] = resource.get("source_type", "")
            resource["sub_type"] = resource.get("source_sub_type", "")
        else:
            resource = ResourceClient.query_resource(resource_id)
        if not resource:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource [{resource_id}] is not exist")
        # 校验脚本是否正确
        if not create_req.is_resource_group:
            check_protected_obj_script(resource, create_req.ext_parameters)
        sla = ProtectionClient.query_sla(str(create_req.sla_id))
        # 填充保护对象的扩展字段
        fill_protection_ext_parameters(sla, create_req.ext_parameters)
        # 校验资源链路状态
        if not create_req.is_resource_group:
            first_os_type = None if 'os_type' not in resource else resource.get('os_type')
            from app.protection.object.service.batch_protection_service import batch_protect_pre_check
            batch_protect_pre_check(sla, resource, first_os_type, create_req.ext_parameters)
        # 校验该资源之前的备份副本所在的存储单元：1.等于当前SLA中选择的存储单元2.被包含在在当前SLA选择的存储单元组中
        ProtectionClient.check_exist_copies_location_before_protect(str(create_req.sla_id), resource_id)
        # 创建保护对象及调度任务
        projected_object = build_protection_object(resource, sla, create_req.ext_parameters, None, create_req)

        session.add(projected_object)
        # 更新资源表中的保护状态为已保护
        resource_service.update_protection_status(session=session, resource_id_list=[resource_id],
                                                  protection_status=ProtectionStatusEnum.protected,
                                                  is_resource_group=create_req.is_resource_group)
        return projected_object.uuid

    @staticmethod
    def modify_obj_and_schedules(
            session: Session,
            update_req: ModifyProtectionSubmitReq
    ) -> str:
        projected_object = db.projected_object.query_one_by_resource_id(
            db=session, resource_id=update_req.resource_id)
        if update_req.sla_id == projected_object.sla_id:
            # sla not change, only update metadata
            db.projected_object.update_ext_parameters(
                db=session, update_req=update_req)
        else:
            # sla changed, update sla id, delete schedules and tasks, recreate schedules and tasks use new sla
            ProtectedObjectService.batch_remove_protection(
                session, [projected_object])
            ProtectedObjectService.create_projected_object(session, update_req)
        return projected_object.uuid

    @staticmethod
    def batch_remove_protection(
            session: Session,
            projected_object_list: List[ProtectedObject]
    ):
        task_id_list, schedule_id_list = extract_ids(projected_object_list)

        # delete projected object's schedules and tasks
        if not SchedulerClient.batch_delete_schedules(schedule_id_list):
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message="fail to delete schedule task")
        # 删除保护对象
        obj_id_list = list(str(obj.uuid) for obj in projected_object_list)
        db.projected_object.batch_delete(session, obj_id_list)

    @staticmethod
    def manual_backup(session: Session, resource_id: str, backup_req: CurrentManualBackupRequest):
        """
        执行手动备份
        :param session:
        :param resource_id: 资源id
        :param backup_req: 备份请求
        :return:
        """
        protected_object = db.projected_object.query_one_by_resource_id(db=session, resource_id=resource_id)
        if protected_object is None:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message="projected object not exist", parameters=["resource_id"])
        if not backup_workflow.is_support_log_backup(resource_id, backup_req.action):
            # 抛出日志备份为首次备份的异常
            raise EmeiStorBizException(ProtectionErrorCodes.LOG_BACKUP_IS_THE_FIRST_BACKUP)
        if has_children_need_backup(protected_object.type, protected_object.sub_type):
            log.info(f"Resource has children need backup, resource_id:{protected_object.resource_id}")
            backup_job_ids = virtual_resource_manual_backup(
                session, protected_object, backup_req)
        else:
            backup_job_ids = common_resource_manual_backup(
                session, protected_object, backup_req)
        return backup_job_ids

    @staticmethod
    def sync_sla_info_change(session: Session, sla_id: str):
        """
        sla信息变更时，绑定了该SLA的保护对象及调度策略需要刷新
        1. 查询SLA绑定的所有保护对象
        2. 查询SLA的信息
        3. 找到保护对象需要删除的调度任务进行删除
            3.1 首先查找当前保护对象所有保护任务对应的policy_id和schedule_id
            3.2 查找修改sla过后，保护对象新的policy_id
            3.3 删除存在于原有保护任务，但是不存在于新保护任务中的schedule_id（policy_id对应一个schedule_id）
        4. 用新SLA信息重新创建调度任务
        5. 更新保护对象任务
            5.1 如果SLA中的策略id在protected_task存在，则更新schedule_id
            5.2 如果SLA中的策略id已经在protected_task不存在，则创建新的protected_task
        :param session: db session
        :param sla_id: SLA id
        :return:
        """
        obj_list = db.projected_object.query_obj_by_sla_id(
            db=session, sla_id=sla_id)
        sla = ProtectionClient.query_sla(sla_id)
        log.info(f"Sync sla info change, sla_id:{sla_id}")
        need_delete_schedule = []
        policy_update_dtos = []
        obj_list.sort(key=lambda pro_object: get_projected_object_topic(pro_object))
        pro_obj_group = groupby(obj_list, lambda pro_object: get_projected_object_topic(pro_object))
        for topic, grouped_obj in pro_obj_group:
            policy_obj_dict = {}
            for projected_object in grouped_obj:
                # 找到需要删除的schedule_id
                ProtectedObjectService.update_single_obj(need_delete_schedule, projected_object, sla, policy_obj_dict)
            # 不同topic、policy单独生成模板发给PM,减少数据量
            for policy in sla.get("policy_list"):
                if not filter_sla_policy(sla, policy):
                    continue
                policy_schedule = build_schedule_param_for_topic(topic, policy, sla.get("uuid"))
                policy_obj_info = policy_obj_dict.get(policy.get("uuid"))
                policy_update_dto = {
                    "policy_id": policy.get("uuid"),
                    "schedule": policy_schedule,
                    "protect_object_infos": policy_obj_info
                }
                policy_update_dtos.append(policy_update_dto)
        return {
            "need_delete_schedules": need_delete_schedule,
            "policy_update_info": policy_update_dtos
        }

    @staticmethod
    def update_single_obj(need_delete_schedule, projected_object, sla, policy_obj_dict: dict):
        old_policy_schedule_id = {task.policy_id: task.schedule_id for task in projected_object.task_list}
        new_policy_ids = {policy.get("uuid") for policy in sla.get("policy_list") if filter_sla_policy(sla, policy)}
        need_delete_schedule_ids = [
            old_policy_schedule_id.get(policy_id)
            for policy_id in old_policy_schedule_id.keys()
            if policy_id not in new_policy_ids
        ]
        worm_switch = False
        for policy in sla.get("policy_list"):
            worm_validity_type = policy.get("worm_validity_type", WormValidityTypeEnum.worm_not_open.value)
            if (policy.get("type") == PolicyTypeEnum.backup.value and
                    worm_validity_type != WormValidityTypeEnum.worm_not_open.value):
                worm_switch = True
        if isinstance(projected_object.ext_parameters, str):
            ext_parameters = json.loads(projected_object.ext_parameters)
        else:
            ext_parameters = projected_object.ext_parameters

        if ext_parameters.get("worm_switch") != worm_switch:
            ext_parameters["worm_switch"] = worm_switch
            projected_object.ext_parameters = ext_parameters
            flag_modified(projected_object, "ext_parameters")

        log.info(
            f"Change sla, old policy ids: {old_policy_schedule_id}, new policy ids: {new_policy_ids}, need delete "
            f"schedule ids: {need_delete_schedule_ids}.")
        need_delete_schedule.extend(need_delete_schedule_ids)
        task_list = build_new_task_list(projected_object, old_policy_schedule_id, sla, policy_obj_dict)
        projected_object.task_list = task_list
        projected_object.sla_id = sla.get("uuid")
        projected_object.sla_name = sla.get("name")
        if is_need_generate_new_chain_id(ResourceSubTypeEnum(projected_object.sub_type)):
            projected_object.chain_id = str(uuid.uuid4())

    @staticmethod
    def update_compliance(session: Session, resource_id: str, compliance: bool):
        pro_obj = db.projected_object.update_by_params(db=session, resource_id=resource_id, update_conditions={
            ProtectedObject.sla_compliance: compliance
        })
        return pro_obj

    @staticmethod
    def query_obj(
            session: Session,
            resource_id: str
    ):
        return db.projected_object.query_one_by_resource_id(db=session, resource_id=resource_id)

    @staticmethod
    def query_sla_compliance(session: Session, user_info):
        query = session.query(ProtectedObject).join(ResourceTable, ProtectedObject.resource_id == ResourceTable.uuid)
        if user_info:
            sub_query = session.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == user_info.get('domain-id')).subquery()
            query = query.filter(ResourceTable.uuid.in_(sub_query))
        in_compliance = query.filter(ProtectedObject.sla_compliance == true()).count()
        out_of_compliance = query.filter(ProtectedObject.sla_compliance == false()).count()
        return ProtectedObjectSlaCompliance(in_compliance=in_compliance,
                                            out_of_compliance=out_of_compliance)

    @staticmethod
    def count_by_sla_id(
            session: Session,
            sla_id: str,
            domain_id: str = None
    ) -> int:
        sla = ProtectionClient.query_sla(sla_id)
        if sla is not None and sla.get("application") in [ResourceSubTypeEnum.Replica]:
            obj_list = db.projected_object.query_obj_by_sla_id(db=session, sla_id=sla_id)
            return len(obj_list)
        else:
            obj_list = db.projected_object.query_by_sla_id(
                db=session, sla_id=sla_id, domain_id=domain_id)
            return len(obj_list)

    @staticmethod
    def query_projected_object_by_user_id(
            session: Session,
            domain_id: str = None) -> List:
        obj_list = db.projected_object.query_projected_object_by_user_id(db=session, domain_id=domain_id)
        return obj_list
