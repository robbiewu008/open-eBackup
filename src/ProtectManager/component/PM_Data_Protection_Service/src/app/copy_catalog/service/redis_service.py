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

from app.common import logger
from app.common.deploy_type import DeployType
from app.common.enums.sla_enum import RetentionTypeEnum, RetentionTimeUnit, add_time
from app.common.redis.redis_lock import redis_lock
from app.common.redis_session import redis_session
from app.common.toolkit import json2dict
from app.copy_catalog.common.common import ResourceStatus
from app.copy_catalog.common.constant import RedisKey
from app.copy_catalog.util.copy_util import get_future_time, pick

COPY_CACHE_LOCK = "cache_recent_expiring_copy"

log = logger.get_logger(__name__)


def get_context_value(context, name):
    return redis_session.hget(context, name)


def cache_recent_expiring_copy(copy_id, timestamp):
    redis_session.zadd(RedisKey.COPY_CACHE_EXPIRE, {copy_id: timestamp})


def update_copy_cache(expiration_time, copy_id, temporary):
    decorator = redis_lock(COPY_CACHE_LOCK)
    if temporary:
        limit = get_future_time(hours=3)
        if expiration_time < limit:
            cache = decorator(cache_recent_expiring_copy)
            cache(copy_id, expiration_time.timestamp())
        else:
            delete = decorator(delete_cached_copy)
            delete(copy_id)
    else:
        delete = decorator(delete_cached_copy)
        delete(copy_id)


def delete_cached_copy(copy_id: str):
    if redis_session.zrem(RedisKey.COPY_CACHE_EXPIRE, copy_id):
        return True
    else:
        log.warning(f"remove copy '{copy_id}' failed from redis key- {RedisKey.COPY_CACHE_EXPIRE}.")
        return False


def build_resource_info(copy, request_id):
    resource_properties = get_context_value(request_id, "resource")
    resource = json2dict(resource_properties)
    resource_info = pick(resource, {
        "uuid": "resource_id",
        "name": "resource_name",
        "type": "resource_type",
        "sub_type": "resource_sub_type",
        "path": "resource_location",
        "environment_name": "resource_environment_name",
        "environment_ip": "resource_environment_ip"
    }, default="")
    resource_info["resource_status"] = ResourceStatus.EXIST.value
    resource_info["resource_properties"] = resource_properties
    copy.update(resource_info)


def get_retention(request_id: str = ''):
    backup_type = get_context_value(request_id, "backup_type")
    str_sla = get_context_value(request_id, "sla")
    dic_sla = json2dict(str_sla)
    policy_list = dic_sla["policy_list"]
    retention_type = None
    duration = None
    unit = None
    for element in policy_list:
        if element["name"] == backup_type:
            retention_type = element["retention"]["retention_type"]
            duration = element["retention"]["retention_duration"]
            unit = element["retention"]["duration_unit"]
            return retention_type, duration, unit
    return retention_type, duration, unit


def get_retention_cyber_engine(request_id: str = ''):
    policy = get_context_value(request_id, "policy")
    retention_type = policy["retention"]["retention_type"]
    duration = policy["retention"]["retention_duration"]
    unit = policy["retention"]["duration_unit"]
    return retention_type, duration, unit


def build_retention_info(copy, request_id, timestamp: datetime.datetime):
    # 安全一体机从缓存的policy取保留时间, 而不是从sla的第一个备份策略取
    if DeployType().is_cyber_engine_deploy_type():
        retention_type, retention_duration, duration_unit = get_retention_cyber_engine(request_id)
    else:
        retention_type, retention_duration, duration_unit = get_retention(request_id)
    if retention_type is not None and RetentionTypeEnum.temporary.value == retention_type:
        retention_time_unit = RetentionTimeUnit(duration_unit)
        expiration_time = add_time(timestamp, retention_duration, retention_time_unit)
        log.info(f"timestamp: {timestamp}, time unit: {retention_time_unit}, expiration: {expiration_time}")
    else:
        retention_type = RetentionTypeEnum.permanent.value
        duration_unit = None
        retention_duration = None
        expiration_time = None
    copy.update({
        'retention_type': retention_type,
        'retention_duration': retention_duration,
        'duration_unit': duration_unit,
        'expiration_time': expiration_time
    })


def get_job_id(request_id: str = ''):
    value = get_context_value(request_id, "job_id")
    return value


def build_sla_info(copy_information: dict, request_id: str = ''):
    sla_properties = get_context_value(request_id, "sla")
    sla = json2dict(sla_properties)
    data = {
        'sla_name': sla.get('name'),
        'sla_properties': sla_properties
    }
    copy_information.update(data)
