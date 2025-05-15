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
import json
from collections import namedtuple

import urllib3

from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok
from app.common.event_messages.Rlm import rlm
from app.common.events import producer
from app.common import logger
from app.common.enums.schedule_enum import ScheduleTypes

from app.resource_lock.common import consts
from app.resource_lock.kafka import message_registry
from app.resource_lock.kafka.progress import notify_progress

log = logger.get_logger(__name__)
msg_registry = message_registry.MessageRegistry()
api_response = {
    consts.LOCK: rlm.LockResponse,
    consts.UNLOCK: rlm.UnlockResponse,
    consts.QUERY: rlm.HasLockResponse
}
msg_status = {
    consts.PROGRESS_SUCCEEDED: consts.MESSAGE_OK,
    consts.PROGRESS_FAILED: consts.MESSAGE_FAIL
}
Request = namedtuple('Request', ('request_id', 'response_topic'))


def push_to_msg_registry(lock_id, api, request):
    """
    向消息库添加请求

    :param lock_id: 锁ID
    :param api: 接口(lock:加锁请求,unlock:解锁请求)
    :param request: 请求ID
    :return:
    """
    if api not in [consts.LOCK, consts.UNLOCK]:
        return
    value = {consts.REQUEST_API: api,
             consts.REQUEST_ID: request.request_id,
             consts.REQUEST_RESPONSE: request.response_topic}
    log.info(f'push lock_id: {lock_id} value: {value}')
    if lock_id is None:
        if request.response_topic.strip() != "":
            msg = api_response.get(api)(request.request_id,
                                        status=msg_status.get(consts.PROGRESS_FAILED))
            producer.produce(msg, request.response_topic)
        return
    msg_registry.push(lock_id, value)
    notify_progress(request.request_id, consts.PROGRESS_START, api, lock_id)


def handle_response(lock_id, operation, request, status, resource_id=None):
    """
    通用请求处理

    :param operation: 操作
    :param lock_id: 锁ID
    :param request: 请求
    :param status: 状态
    :param resource_id: 资源id,加锁失败的时候值不为空,其余情况值为空
    :return:
    """
    log.info(f"handle_response lock_id: {lock_id}")
    msg = msg_registry.get(lock_id)
    api = msg.get(consts.REQUEST_API, operation)
    if not api or api != operation:
        return
    if lock_id is None:
        return
    request_id = msg.get(consts.REQUEST_ID, request.request_id)
    topic = msg.get(consts.REQUEST_RESPONSE, request.response_topic)
    msg = api_response.get(api)(
        request_id, status=msg_status.get(status))
    notify_progress(request_id, status, api, lock_id, resource_id)
    msg_registry.pop(lock_id)
    producer.produce(msg, topic)


def handle_unlock_response(lock_id):
    """
    解锁消息入库前，判断加锁消息是否存在，如果存在则强制加锁失败

    :param lock_id: 锁ID
    :return:
    """
    msg = msg_registry.get(lock_id)
    if not msg or msg.get(consts.REQUEST_API) != consts.LOCK:
        return
    request = Request(msg.get(consts.REQUEST_ID),
                      msg.get(consts.REQUEST_RESPONSE))
    handle_response(lock_id, consts.LOCK, request, consts.PROGRESS_FAILED)


def handle_lock_state(lock_id, lock_state, resource_id=None):
    """
    根据锁状态处理锁请求

    :param lock_id: 锁ID
    :param lock_state: 锁ID对应状态
    :param resource_id: 资源id,加锁失败的时候值不为空,其余情况值为空
    :return:
    """
    msg = msg_registry.get(lock_id)
    if not msg:
        return

    api = msg.get(consts.REQUEST_API)
    request = Request(msg.get(consts.REQUEST_ID),
                      msg.get(consts.REQUEST_RESPONSE))
    if (lock_state != consts.STATE_UNLOCK and api == consts.UNLOCK) \
            or (lock_state == consts.STATE_UNLOCK and api != consts.UNLOCK):
        handle_response(
            lock_id, api, request, consts.PROGRESS_FAILED, resource_id)
    elif lock_state != consts.STATE_PENDING:
        handle_response(
            lock_id, api, request, consts.PROGRESS_SUCCEEDED, resource_id)


def handle_has_lock_response(request, lock_id, lock_state, resource_id=None):
    """
    处理查询锁状态请求,根据锁状态处理消息

    :param request: 请求
    :param lock_id: 锁ID
    :param lock_state: 锁ID对应状态
    :param resource_id: resource_id: 资源id,加锁失败的时候值不为空,其余情况值为空
    :return:
    """
    handle_lock_state(lock_id, lock_state, resource_id)
    if not request:
        return
    msg = api_response.get(consts.QUERY)(request.request_id, status=lock_state)
    producer.produce(msg, request.response_topic)


def handle_transferred_response(request, unlock, locked):
    """
    处理释放锁时，传递资源锁消息

    :param request: 请求
    :param unlock: 释放的资源锁ID
    :param locked: 资源锁释放后，加锁成功的锁ID列表
    :return:
    """
    handle_response(unlock, consts.UNLOCK, request, consts.PROGRESS_SUCCEEDED)
    for lock in locked:
        # 解当前锁会传递加锁，加锁成功要通知加锁成功
        request = Request(None, None)
        handle_response(lock, consts.LOCK, request, consts.PROGRESS_SUCCEEDED)


def recover_unsent_messages(lock_service):
    """
    异常恢复时，队列中未回包消息，根据锁状态进行处理

    :param lock_service: 分布式锁服务
    :return:
    """
    lock_id_set = msg_registry.all_key()
    for lock_id in lock_id_set:
        lock_service.has_lock(None, lock_id)


def handle_pending(lock_id, timeout):
    """
    异步等锁调度任务

    :param lock_id: 分布式锁ID
    :param timeout: 超时时间
    :return:
    """
    log.info(f"handle_pending lock_id: {lock_id}")
    schedule = datetime.datetime.now() + datetime.timedelta(seconds=30)
    url = f"/v1/schedules"
    req_data = {
        "schedule_type": ScheduleTypes.immediate.value,
        "schedule_name": f"pendinglock_{lock_id}",
        "action": rlm.HasLockRequest.default_topic,
        "params": json.dumps({
            "lock_id": lock_id,
            "realse_pending": timeout-30
        }),
        "start_date": schedule.strftime('%Y-%m-%dT%H:%M:%S.%fZ')
    }
    response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(req_data))
    if not is_response_status_ok(response):
        raise urllib3.exceptions.HTTPError()

