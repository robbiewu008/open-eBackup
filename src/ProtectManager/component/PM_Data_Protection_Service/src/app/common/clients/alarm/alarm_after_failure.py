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
from time import time

from app.backup.common.backup_workflow_constant import BackupWorkflowConstants
from app.base.resource_consts import ResourceConstant
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.logger import get_logger
from app.resource.service.common import resource_service

log = get_logger(__name__)


def build_alarm_req(params, resource_id, alarm_id):
    return SendAlarmReq(
        alarmId=alarm_id,
        params=params,
        alarmSource="resource",
        createTime=int(time()),
        sequence=int(time()),
        sourceType=AlarmSourceType.RESOURCE,
        resouceId=resource_id
    )


def send_task_alarm_when_failed_or_partial_success(resource_name, resource_id, status, task_type):
    if status == ResourceConstant.COMMON_FAIL_LABEL:
        params = resource_name + "," + task_type + "," + status + "," + resource_id
        alarm_id = ResourceConstant.RESOURCE_TASK_ALARM_AFTER_FAILURE
    else:
        params = resource_name + "," + task_type + "," + resource_id
        alarm_id = ResourceConstant.RESOURCE_TASK_ALARM_AFTER_PARTIAL_SUCCESS
    log.info(f'[Task alarm]: send alarm params:{params}')
    try:
        AlarmClient.send_alarm(build_alarm_req(params, resource_id, alarm_id))
    except Exception as e:
        log.exception(
            f"[Task alarm]: resource: {resource_id} task alarm alarm : send error: {e}")
    finally:
        pass


def clear_task_alarm_when_success(resource_name, resource_id, task_type):
    try:
        params_fail = resource_name + "," + task_type + "," + ResourceConstant.COMMON_FAIL_LABEL + "," + resource_id
        AlarmClient.clear_entity_alarm(build_alarm_req(params_fail, resource_id,
            ResourceConstant.RESOURCE_TASK_ALARM_AFTER_FAILURE))
        params_partial_success = resource_name + "," + task_type + "," + resource_id
        AlarmClient.clear_entity_alarm(build_alarm_req(params_partial_success, resource_id,
            ResourceConstant.RESOURCE_TASK_ALARM_AFTER_PARTIAL_SUCCESS))
    except Exception as e:
        log.exception(
            f"[Task alarm]: resource: {resource_id} task alarm alarm clear error: {e}")
    finally:
        pass


def alarm_after_failure(context, status, resource_obj: dict = None):
    """
    context: redis 上下文，如果要调用，必须保证redis里面存在策略，资源对象存入的是json。
    status: 0 失败,1成功,2部分成功，3终止
    resource_obj: 资源对象字典
    """
    policy = context.get(BackupWorkflowConstants.POLICY, dict)

    if not policy:
        log.exception("No policy in context")
        return

    # 策略里面配置了失败告警参数为True时才开启失败告警
    if not policy.get("ext_parameters", {}).get("alarm_after_failure", False):
        log.exception("No policy.ext_parameters in context")
        return

    # 如果上层存在资源对象直接取
    if resource_obj:
        resource_id = resource_obj.get('uuid', "")
        resource_name = resource_obj.get("name", "")
    else:
        resource_id = context.get(BackupWorkflowConstants.RESOURCE_ID)
        resource_name = context.get(BackupWorkflowConstants.RESOURCE_NAME)
    log.info(f"[ARCHIVE_TASK YKP ALARM]: resource_Name:{resource_name},resourceId:{resource_id}")

    status = ResourceConstant.transition_status.get(status, "")
    sts = context.get(BackupWorkflowConstants.JOB_TYPE)
    # 如果上下文中查询不到数据, 取策略中的类型
    if not sts:
        sts = policy.get("type", "")
    task_type = ResourceConstant.transition_task_type.get(sts)
    if not resource_id:
        resource_obj = context.get(BackupWorkflowConstants.RESOURCE, dict)
        resource_id = resource_obj.get("uuid", "")

    # 如果都不存在找资源对象里面的值,如果资源对象不存在，根据资源id查询
    if not resource_name:
        resource_obj = resource_service.query_resource_by_id(resource_id)
        if resource_obj:
            resource_name = resource_obj.dict().get("name", "")
        else:
            resource_name = ""

    if status in [ResourceConstant.COMMON_FAIL_LABEL, ResourceConstant.COMMON_PARTIAL_SUCCESS_LABEL]:
        send_task_alarm_when_failed_or_partial_success(resource_name, resource_id, status, task_type)
    if status == ResourceConstant.COMMON_SUCCESS_LABEL:
        clear_task_alarm_when_success(resource_name, resource_id, task_type)
    return
