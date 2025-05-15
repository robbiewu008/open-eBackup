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
import inspect
import json
import re
import uuid
from datetime import datetime
from http import HTTPStatus
from inspect import _empty
from typing import Callable, Any, Tuple, List, Dict, TypeVar

from fastapi import APIRouter, Body
from pydantic import Field
from pydantic.main import BaseModel

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, is_response_status_ok
from app.common.enums.job_enum import JobType, JobStatus, JobLogLevel
from app.common.exception.unified_exception import DBRetryException, EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.http import LONG_RETRY_POLICY
from app.common.sensitive.sensitive_word_filter_util import sensitive_word_filter
from app.resource_lock.common import consts
from app.resource_lock.common.consts import REDIS_RESOURCE_LOCK_PREFIX

log = logger.get_logger(__name__)
T = TypeVar('T')


class JobMessage(BaseModel):
    topic: str = Field(description="任务消息主题")
    payload: dict = Field(description="任务消息内容")
    traffic: dict = Field(None, description="任务流量配置")
    abolish: list = Field(None, description="任务废除日志")
    context: bool = Field(False, description="上下文数据标识")


def create_job_center_task(request_id: str, params, message: JobMessage = None):
    """
    创建任务

    :param request_id: request id
    :param params: params
    :param message: message
    :return: 任务ID
    """
    params['message'] = message.dict() if message else None
    log.debug(f'invoke api to create job, request body is {sensitive_word_filter(params)}')
    request_id = request_id if request_id else str(uuid.uuid4())
    params['requestId'] = request_id
    start_time = datetime.now()
    response = SystemBaseHttpsClient().request(
        "POST", f"/v1/internal/jobs", body=json.dumps(params))
    end_time = datetime.now()
    cost = end_time - start_time
    if response.status == HTTPStatus.OK:
        job_task_id = decode_response_data(response.data)
        log.info(f'Success create job for request_id={request_id}, job_task_id={job_task_id}, cost={cost}')
        return job_task_id
    else:
        log.error(f'Failed to create job for request_id={request_id}, cost={cost}')
        error_msg = json.loads(decode_response_data(response.data))
        error = {"code": error_msg["errorCode"]}
        if error_msg["errorMessage"]:
            error["message"] = error_msg["errorMessage"]
        else:
            error["message"] = ""
        if error_msg["parameters"]:
            parameters = error_msg["parameters"]
        else:
            parameters = []
        raise EmeiStorBizException(error, *parameters,
                                   error_message=error_msg["errorMessage"])


@exter_attack
def query_job_list(params):
    """
    任务信息分页查询
    :param params: 请求过滤条件参数
    :return: 查询结果
    """
    log.info(f'invoke api to query job, request is {sensitive_word_filter(params)}')
    response = SystemBaseHttpsClient().request(
        "GET", f"/v1/internal/jobs", fields=params)
    if response.status == HTTPStatus.OK:
        log.info(f'query some job info')
        return decode_response_data(response.data)
    else:
        log.info(f'Failed to query some job info')


def complete_job_center_task(request_id: str, job_id: str, params):
    """
    更新任务为结束状态

    :param request_id: request id
    :param job_id: job id
    :param params: params
    :return: None
    """
    params["endTime"] = int(datetime.now().timestamp() * 1000)
    log.info(f'invoke api to complete job, request body is {params}')
    start_time = datetime.now()
    response = SystemBaseHttpsClient().request(
        "PUT", f"/v1/internal/jobs/{job_id}/action/update",
        body=json.dumps(params))
    end_time = datetime.now()
    cost = end_time - start_time
    if response.status == HTTPStatus.OK:
        log.info(f'invoke api to complete job success job_id={job_id} request_id={request_id}, cost={cost}')
    else:
        log.info(f'invoke api to complete job failed, response.data={response.data}, cost={cost}')


def modify_task_log(request_id: str, job_id: str, params):
    log.info(f'invoke api to update job, request body is {params}')
    response = SystemBaseHttpsClient().request(
        "PUT", f"/v1/internal/jobs/{job_id}/action/update",
        body=json.dumps(params))
    if response.status == HTTPStatus.OK:
        log.info(f'invoke api to update job success job_id={job_id} request_id={request_id}')
    else:
        log.info(f'invoke api to update job failed, response.data={response.data}')


def send_resource_redis_lock_request(resources: list, lock_id: str):
    log.info(f'invoke api to redis lock, lock_id is {lock_id}')
    param = {
        "resources": [{"resourceId": resource.get(consts.RESOURCE_ID), "lockType": resource.get(consts.LOCK_TYPE)} for
                      resource in resources],
        "lockId": REDIS_RESOURCE_LOCK_PREFIX + lock_id
    }
    try:
        response = SystemBaseHttpsClient().request(
            "PUT", f"/v1/internal/resource/redis/lock", body=json.dumps(param))
        if response.status == HTTPStatus.OK:
            log.info(f'invoke api to redis lock success, lock_id is={lock_id}')
        else:
            log.info(f'invoke api to redis lock failed, lock_id is={lock_id}, '
                     f'response status is {response.status}')
    except Exception:
        log.error(f'Redis resource lock failed,lock id :{lock_id}')


def send_resource_redis_unlock_request(lock_id: str):
    log.info(f'invoke api to redis unlock, lock_id is {lock_id}')
    param = {
        "lockId": REDIS_RESOURCE_LOCK_PREFIX + lock_id
    }
    response = SystemBaseHttpsClient().request(
        "DELETE", f"/v1/internal/resource/redis/lock", body=json.dumps(param))
    if response.status == HTTPStatus.OK:
        log.info(f'invoke api to redis unlock success, lock_id is={lock_id}')
    else:
        log.info(f'invoke api to redis unlock failed, lock_id is={lock_id}, '
                 f'response status is {response.status}')


def modify_job_lock_id(context):
    lock_id = context.get("lock_id")
    request_id = getattr(context, 'request_id', getattr(context, 'name', None))
    if request_id is not None:
        modify_task_log(request_id, request_id, {
            "data": {"lock_id": lock_id}
        })


def build_update_job_log_request(job_id: str, log_label: str, log_level: JobLogLevel, log_info_param,
                                 log_detail: str = None,
                                 log_detail_param: str = None):
    if not log_info_param:
        log_info_param = []
    return {
        "jobLogs": [{
            "jobId": job_id,
            "startTime": int(datetime.now().timestamp() * 1000),
            "logInfo": log_label,
            "logInfoParam": log_info_param,
            "level": log_level.value,
            "logDetail": log_detail,
            "logDetailParam": log_detail_param
        }]
    }


def send_job_schedule_config(job_type: JobType, *rules):
    rules = [rule for rule in rules if rule] if rules else []
    param = {
        "job_type": job_type.value,
        "rules": rules
    }
    log.info(f"job type:{job_type}, param:{param}")
    response = SystemBaseHttpsClient(retries=LONG_RETRY_POLICY).request(
        "PUT", "/v1/internal/jobs/action/update-schedule-policy", body=json.dumps(param))
    if is_response_status_ok(response):
        log.info(f"{job_type} schedule config success.")
    else:
        log.error(f"response.data:{response.data}")


class JobScheduleOption(BaseModel):
    scope: Any = Field(None)
    global_job_limit: Dict[str, int] = Field(None)
    scope_job_limit: int = Field(0)
    major_priority: int = Field(0)
    minor_priorities: List[str] = Field(None)
    resume_status: JobStatus = Field(None)
    pending_window: int = Field(0)
    strict_scope: bool = Field(True)


class JobScheduleConfig:
    def __init__(self, examine: Tuple[str, str, Callable[[dict], Any]] = None,
                 options: JobScheduleOption = None):
        self.examine = examine
        self.options = options

    @classmethod
    def default(
            cls,
            examine: Tuple[str, str, Callable[[dict], Any]] = None,
            options: JobScheduleOption = JobScheduleOption(
                global_job_limit={"common": 20}
            )
    ):
        return JobScheduleConfig(examine=examine, options=options)


def count_task_in_status(params):
    log.info(f'invoke api to count jobs in status list, request body is {params}')
    response = SystemBaseHttpsClient().request(
        "GET", f"/v1/internal/jobs/status/count", body=json.dumps(params))
    if response.status == HTTPStatus.OK:
        log.info(f'invoke api to count jobs in status list success, response.data={response.data}')
        number = int(decode_response_data(response.data))
        return number
    else:
        log.info(f'invoke api to count jobs in status list failed, response.data={response.data}')


def json2list(string: str = ''):
    try:
        data = json.loads(string)
        if data is None:
            return []
        elif isinstance(data, (list, tuple)):
            return list(data)
        else:
            return [data]
    except:
        return []


def json2dict(string: str = ''):
    try:
        dic = json.loads(string)
        if isinstance(dic, dict):
            return dic
        else:
            return {}
    except:
        return {}


def get_value_by_path(obj, path, offset=0):
    index = path.find('.', offset)
    if index == -1:
        return get_obj_val_by_key(obj, path[offset:])
    name = path[offset:index]
    matches = re.match("^[*]+$", name)
    if matches:
        items = get_value_items(obj)
        all_pattern = matches.group()
        results = [get_value_by_path(item, path, index + len(all_pattern)) for item in items]
        if name == '**':
            items = [result if isinstance(result, (list, tuple)) else result for result in results]
            return [item for each in items for item in each]
        return results
    results = get_obj_val_by_key(obj, name)
    return get_value_by_path(results, path, index + 1)


def get_value_items(obj):
    if isinstance(obj, dict):
        return obj.values()
    return obj


def get_obj_val_by_key(obj, key):
    if obj is None:
        return None
    if isinstance(obj, dict):
        if key == '*':
            return obj.values()
        return obj.get(key)
    if isinstance(obj, (list, tuple)):
        if key == '*':
            return obj
        return obj[int(key)]
    return getattr(obj, key, None)


def to_collection(obj):
    """
    将对象转换为集合

    :param obj: 对象
    :return: 集合
    """
    if obj is None:
        return []
    if isinstance(obj, (list, tuple, set)):
        return obj
    return [obj]


class Method:
    def __init__(self, func):
        """
        对原函数进行拦截，过滤掉原函数未定义的传参

        :param func: 原函数
        """
        self.func = func
        signature = inspect.signature(func)
        self.original_signature = signature
        self.signature = signature

    def append_parameter(self, name: str, default=_empty, annotation=_empty):
        signature = self.signature
        if name in signature.parameters:
            return self
        items = list(signature.parameters.values())
        items.append(inspect.Parameter(name, inspect.Parameter.KEYWORD_ONLY, default=default, annotation=annotation))
        self.signature = signature.replace(parameters=items)
        return self

    def __call__(self, *args, **kwargs):
        arguments = {}
        parameters = self.original_signature.parameters
        for key, value in kwargs.items():
            if key in parameters:
                arguments[key] = value

        return self.func(*args, **arguments)


def listify(data):
    return [] if not data else list(data) if isinstance(data, (list, tuple, set)) else [data]


def combine_multiple_method_as_chain_method(*methods):
    """
    combine multiple method as chain method

    :param methods: method list
    :return: chain method
    """

    def function(args):
        result = args
        for method in methods:
            result = method(result)
        return result

    return function
