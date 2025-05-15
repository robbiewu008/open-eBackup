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
import math
import time

from app.common.clients.protection_client import ProtectionClient
from app.common.clients.scheduler_client import SchedulerClient
from app.common.constants.constant import CommonConstants, DBRetryConstants
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.sla_enum import BackupTimeUnit, RetentionTimeUnit, TriggerEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, DBRetryException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.logger import get_logger
from app.common.util.retry.retryer import retry
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.service.common.domain_resource_object_service import get_domain_resource_object_relation

log = get_logger(__name__)

time_interval_switcher = {
    BackupTimeUnit.minutes.value: lambda x: x * 60.0,
    BackupTimeUnit.hours.value: lambda x: x * 60 * 60.0,
    BackupTimeUnit.days.value: lambda x: x * 24 * 60 * 60.0,
    BackupTimeUnit.weeks.value: lambda x: x * 7 * 24 * 60 * 60.0,
    RetentionTimeUnit.months.value: lambda x: x * 4 * 7 * 24 * 60 * 60.0,
    RetentionTimeUnit.years.value: lambda x: x * 12 * 4 * 7 * 24 * 60 * 60.0
}


def sync_time(db, resource_id):
    protect_obj = get_protect_obj(db, resource_id)
    if not protect_obj:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                   error_message="select protect obj not exist", parameters=[])
    if not protect_obj.earliest_time:
        db.query(ProtectedObject).filter(ProtectedObject.resource_id == resource_id).update(
            {ProtectedObject.earliest_time: datetime.datetime.now(),
             ProtectedObject.latest_time: datetime.datetime.now()})
    else:
        db.query(ProtectedObject).filter(ProtectedObject.resource_id == resource_id).update(
            {ProtectedObject.latest_time: datetime.datetime.now()})


def get_protect_obj(db, resource_id, domain_id=None):
    default_protect_obj = None
    log.debug(f'resource_id={resource_id}')
    protect_obj = db.query(ProtectedObject).filter(ProtectedObject.resource_id == resource_id).first()
    if not protect_obj:
        return default_protect_obj
    if domain_id:
        relation = get_domain_resource_object_relation(domain_id=domain_id, resource_object_id=resource_id,
                                                       resource_type_list=[ResourceSetTypeEnum.RESOURCE,
                                                                           ResourceSetTypeEnum.RESOURCE_GROUP])
        if not relation:
            log.error(f'Resource: {resource_id} not in user domain: {domain_id}.')
            raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
    return protect_obj


def get_pre_or_future_window_timestamp(window_timestamp, internal):
    one_day_timestamp = 24 * 60 * 60
    return window_timestamp + one_day_timestamp * internal


def get_today_window_timestamp(window_start):
    now_time = datetime.datetime.now()
    today_time_array = time.strptime(now_time.strftime("%Y-%m-%d") + " " + window_start,
                                     CommonConstants.COMMON_DATE_FORMAT)
    return time.mktime(today_time_array)


def get_backup_duration(backup_interval, interval_unit):
    return time_interval_switcher.get(interval_unit)(backup_interval)


def calculate_time_by_schedule(schedules):
    now_timestamp = time.mktime(datetime.datetime.now().timetuple())
    # 首次开始时间
    start_time = time.mktime(datetime.datetime.strptime(schedules['start_time'], "%Y-%m-%dT%H:%M:%S").timetuple())

    if start_time >= now_timestamp:
        # 开始时间在未来，直接返回时间窗的开始日期时间作为下次执行时间
        return start_time

    # 1. start_time < now，开始时间在当前时间之前
    return get_execute_timestamp(schedules, now_timestamp, start_time)


def get_execute_timestamp(schedules, now_timestamp, start_time):
    # 1.1 将当前时间的前一天作为时间窗
    pre_window_timestamp = get_today_window_timestamp(schedules['window_start'])
    next_window_timestamp = get_next_window_timestamp(schedules, pre_window_timestamp)
    pre_window_timestamp = get_pre_or_future_window_timestamp(pre_window_timestamp, -1)
    # 1.1.2 如果window start time >= window end time
    # start time -1天作为window start time， end time作为window end time, 则不改变

    # 1.2 找到第一次时间大于window start time
    # 备份频率
    backup_duration = get_backup_duration(schedules['interval'], schedules['interval_unit'])
    for count in range(367):
        cycle_number = math.ceil((pre_window_timestamp - start_time) / backup_duration)
        cycle_number = cycle_number if cycle_number > 0 else 0

        # 1.2.1 判断时间是否大于当前时间，不满足，再加一个执行周期
        while start_time + cycle_number * backup_duration < pre_window_timestamp or \
                start_time + cycle_number * backup_duration <= now_timestamp:
            cycle_number = cycle_number + 1
        # 1.2.2 判断时间是否小于window end time，
        policy_timestamp = start_time + cycle_number * backup_duration
        if policy_timestamp <= next_window_timestamp:
            # 如果满足返回时间作为下次执行时间;
            return start_time + cycle_number * backup_duration

        # 如果循环达到最大次数367次，我们将选择第367次时到达的日期，
        # 该循环主要针对分钟，小时的频率
        if count == 366:
            log.info(
                f"find next execute time count over 367, now_timestamp: {now_timestamp},\
                pre_window_timestamp: {pre_window_timestamp}")
            return pre_window_timestamp

        # 如果不满足，把window start time和window end time全部加一天，当前这次时间作为start time，从1.2步骤执行循环
        pre_window_timestamp = get_pre_or_future_window_timestamp(pre_window_timestamp, 1)
        next_window_timestamp = get_pre_or_future_window_timestamp(next_window_timestamp, 1)
        start_time = policy_timestamp


def get_next_window_timestamp(schedules, pre_window_timestamp):
    next_window_timestamp = get_today_window_timestamp(schedules['window_end'])
    # 1.1.1 如果window start time < window end time
    # start time -1 天作为window start time, end time - 1天作为window end time
    if pre_window_timestamp < next_window_timestamp:
        next_window_timestamp = get_pre_or_future_window_timestamp(next_window_timestamp, -1)
    return next_window_timestamp


@retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
       backoff=DBRetryConstants.BACKOFF, logger=log)
def get_next_time(db, uuid):
    """
    如果是资源组的资源备份任务, 取所属资源组的调度时间
    否则, 查询自身的调度时间
    """
    log.info(f"get next time of resource={uuid}")
    scheduled_protect_obj = get_protect_obj(db, uuid)
    if scheduled_protect_obj.resource_group_id:
        scheduled_protect_obj = get_protect_obj(db, scheduled_protect_obj.resource_group_id)
        log.info(f"get next time of the resource group id={scheduled_protect_obj.resource_group_id}")
    next_time = None
    for task in scheduled_protect_obj.task_list:
        policy_id = task.policy_id
        policy = ProtectionClient.query_policy(policy_id)
        policy_time = get_backup_policy_time(policy)
        if not policy_time:
            # 如果是周期性调度的备份策略，需要根据时间窗来计算下次执行时间
            policy_time = SchedulerClient.get_schedule_next_time(task.uuid)
        if not next_time:
            next_time = policy_time
        log.info(f"get next time: {next_time} and policy time: {policy_time}")
        next_time = min(next_time, policy_time)
    return next_time


def get_backup_policy_time(policy):
    backup_date = None
    if policy.get("type") != "backup":
        return backup_date

    if policy.get("action") == "log":
        return backup_date

    if policy.get("schedule").get("trigger") == TriggerEnum.customize_interval:
        return backup_date

    if policy.get("schedule").get("window_end") == policy.get("schedule").get("window_start"):
        return backup_date

    if policy.get("schedule").get("interval_unit") not in [BackupTimeUnit.minutes.value,
                                                           BackupTimeUnit.hours.value]:
        return backup_date

    # 根据不同的触发类型，计算下次备份开始时间
    schedule = policy.get("schedule")
    trigger_action = schedule.get("trigger")
    if trigger_action == TriggerEnum.interval:
        # 获取周期性调度的备份时间
        return time.strftime(CommonConstants.COMMON_DATE_FORMAT, time.localtime(calculate_time_by_schedule(schedule)))
    return backup_date
