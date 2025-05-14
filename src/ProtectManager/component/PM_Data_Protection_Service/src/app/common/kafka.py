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
import asyncio
import inspect
import json
import re
import threading
import uuid
from asyncio import Future
from datetime import datetime
from http import HTTPStatus
from typing import Callable, Union, Dict

from confluent_kafka.admin import AdminClient
from confluent_kafka.cimpl import Consumer, Message, KafkaException, NewTopic, KafkaError, TopicPartition

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient
from app.common.concurrency import MILLI_SECONDS, to_async_method
from app.common.config import settings
from app.common.constants.constant import ServiceConstants, MultiClusterKafkaTopicConstants
from app.common.context.context import Context
from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobStatus, JobLogLevel
from app.common.event_messages.Rlm.rlm import LockRequest, UnlockRequest
from app.common.event_messages.event import CommonEvent
from app.common.events import producer
from app.common.events.config import get_default_consumer_config
from app.common.events.consumer import EsEvent
from app.common.events.topics import get_admin_client
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.sensitive.sensitive_word_filter_util import sensitive_word_filter
from app.common.toolkit import (
    get_value_by_path,
    modify_task_log,
    to_collection,
    complete_job_center_task,
    listify,
    modify_job_lock_id,
)

JOB_ID = "job_id"

JOB_STATUS = "job_status"

UNLOCK_ACTION_FLAG = "unlock_action_flag"

STATUS = "job_status"

LOCK_ID = "lock_id"

RUNNING_STATE_COUNT = "RunningStateCount"

ABORTING_STATE_COUNT = "AbortingStateCount"

log = logger.get_logger(__name__)


def normalize_target(target):
    return target if isinstance(target, dict) else {"topic": str(target), "message": "{payload}"}


def replace_place_holder(template, param):
    patten = re.compile(r'{([^{}:"]+)}')
    while True:
        match = patten.search(template)
        if match is None:
            break
        expr = match.group(1)
        value = get_value_by_place_holder(param, expr)
        data = value if isinstance(value, str) else json.dumps(value)
        end = match.end()
        template = template[0:match.start()] + data + template[end:]
    template = template.lower() if template.endswith('_label') else template
    return template


def get_value_by_place_holder(param, expr: str):
    for each in expr.split('|'):
        value = get_value_by_path(param, each)
        if value is not None:
            return value
    return None


class CallableDict(dict):
    function: Callable

    def __init__(self, function: Callable, *args, **kwargs):
        super(CallableDict, self).__init__(*args, **kwargs)
        self.function = function

    def __call__(self, *args, **kwargs):
        return self.function(*args, **kwargs)


def unpack_event(message):
    """ parse event from kafka message value """
    value = message.value()
    try:
        data = json.loads(value)
        event = EsEvent(**data)
        log.debug(
            f'unpack event: topic={message.topic()}, '
            f'request_id={event.request_id}',
        )
        return event
    except ValueError:
        log.exception(
            f'Parse failed. Topic({message.topic()}, {message.partition()}, {message.offset()}'
        )
        return None


def poll(consumer: Consumer):
    message = consumer.poll(0)
    if message is None:
        return None
    error = message.error()
    topic = message.topic()
    partition = message.partition()
    offset = message.offset()
    if error:
        log.error(f'Consume failed. Topic({topic}[{partition}]) at offset {offset}, error={error}')
        message = None
    else:
        log.info(f'Consume success. Topic({topic}[{partition}]) at offset {offset}')
    return message


class Topic:
    def __init__(
            self,
            name: str,
            group: str,
            auto_offset_reset: str,
            num_partitions: int = 4,
            replication_factor: int = 1,
            retryable: bool = True,
            on_assign: Callable[[Consumer, TopicPartition], None] = None,
            on_revoke: Callable[[Consumer, TopicPartition], None] = None
    ):
        self.name = name
        self.group = group
        self.auto_offset_reset = auto_offset_reset
        self.num_partitions = max(num_partitions, 1)
        self.replication_factor = replication_factor
        self.retryable = retryable
        self.on_assign = on_assign
        self.on_revoke = on_revoke

    def match(self, name: str, group: str):
        return self.name == name and self.group == group

    def create(self):
        # X9000系列kafka topic分区数为8，其余为4
        if DeployType().is_x9000_type():
            self.num_partitions = 8
        return NewTopic(self.name, num_partitions=self.num_partitions, replication_factor=self.replication_factor)

    def callback(self, name: str, *builtin_callback_list):
        if not name.startswith('on_'):
            return None

        custom_callback_method = getattr(self, name)
        if custom_callback_method is None and not builtin_callback_list:
            return None

        def callback(*args, **kwargs):
            for builtin_callback_method in builtin_callback_list:
                builtin_callback_method(*args, **kwargs)
            if custom_callback_method is not None:
                custom_callback_method(*args, **kwargs)

        callback.__name__ = name
        return callback

    def create_consumer(self, config: Dict[str, any]):
        consumer = Consumer(config)
        options = dict(
            on_assign=self.callback('on_assign'),
            on_revoke=self.callback('on_revoke')
        )
        options = {key: value for key, value in options.items() if callable(value)}
        consumer.subscribe([self.name], **options)
        log.info(f"create consumer for {self} success.")
        return consumer

    def __str__(self):
        return f"Topic(name:{self.name}, group:{self.group})"


async def dispatch(event: EsEvent, topic: Topic, handler, start_time: datetime):
    try:
        request_id = event.request_id
        context = Context(request_id)
        # 业务开始运行，更新RUNNING_STATE_COUNT
        if context.hash_exist(RUNNING_STATE_COUNT):
            context.increment(RUNNING_STATE_COUNT, 1)
            log.info(
                f'Job is running, RUNNING_STATE_COUNT equals {context.get(RUNNING_STATE_COUNT)}.'
                f'JobId: {request_id}')
        # 如果存在ABORTING_STATE_COUNT，检查是否要调用或者已调用任务中止
        if context.hash_exist(ABORTING_STATE_COUNT):
            if check_send_delete_job_or_not(event):
                return
        result = handler(request=event, **event.message)
        if inspect.isawaitable(result):
            await result
        return
    except TypeError as ex:
        log.exception(f'params type mismatch: {topic}, event:{sensitive_word_filter(event.__dict__)}')
        return ex
    except Exception as ex:
        log.exception(f'unexpect exception: {topic}, event:{sensitive_word_filter(event.__dict__)}')
        return ex
    finally:
        end_time = datetime.now()
        time_cost = (end_time - start_time) / MILLI_SECONDS
        log.debug(
            f"{topic}, event:{sensitive_word_filter(event.__dict__)}, "
            f"start time:{start_time}, end time:{end_time}, time cost:{time_cost}ms"
        )
        try:
            send_delete_job_message(event, False)
        except:
            log.exception(f'unexpect exception: {topic}, event:{sensitive_word_filter(event.__dict__)}')


def check_send_delete_job_or_not(event: EsEvent):
    request_id = event.request_id
    context = Context(request_id)
    # 如果任务中止计数器ABORTING_STATE_COUNT为0，说明已发送任务中止消息，直接返回，不执行当前业务
    if context.get(ABORTING_STATE_COUNT) == "0":
        log.info(f'ABORTING_STATE_COUNT equals 0, message delete job has been sent. JobId: {request_id}')
        return True
    # 如果还有其他线程在运行，不发送任务中止的消息
    if context.get(RUNNING_STATE_COUNT) != "1":
        return False
    # 如果任务中止计数器ABORTING_STATE_COUNT为1，发送删除任务上下文的消息,返回
    if context.get(ABORTING_STATE_COUNT) == "1":
        context.increment(RUNNING_STATE_COUNT, -1)
        send_delete_job_message(event, True)
        return True
    return False


def send_delete_job_message(event: EsEvent, force: bool):
    request_id = event.request_id
    if len(request_id) == 0:
        return
    context = Context(request_id)
    if not context.exist():
        return
    if not context.hash_exist(RUNNING_STATE_COUNT):
        return
    if force:
        # 当前业务开始前，获取RUNNING_STATE_COUNT--
        running_state_count = int(context.get(RUNNING_STATE_COUNT)) - 1
    else:
        # 当前业务结束后，更新RUNNING_STATE_COUNT，并获取返回值
        running_state_count = int(context.increment(RUNNING_STATE_COUNT, -1))
    # 没有正在运行的业务，且ABORTING_STATE_COUNT为1，则删除任务上下文
    if running_state_count <= 0 and context.get(ABORTING_STATE_COUNT) == "1":
        aborting_stat_count = str(context.increment(ABORTING_STATE_COUNT, -1))
        log.info(f'RunningStateCount: {running_state_count}, AbortingStateCount: {aborting_stat_count}')
        if aborting_stat_count == "0":
            log.info(f'Calling delete job when there is no running job. JobId: {request_id}')
            response = SystemBaseHttpsClient().request(
                "PUT", f"/v1/internal/jobs/{request_id}/action/abort")
            if response.status == HTTPStatus.OK:
                log.info(f'invoke api to delete job success job_id={request_id}')
            else:
                log.info(f'invoke api to delete job failed, response.data={response.data}')


async def consume(event, handler, start_time, topic, times, interval):
    success = False
    for i in range(times):
        error = await dispatch(event, topic, handler, start_time)
        if error is None:
            success = True
            break
        elif topic.retryable:
            n = i + 1
            log.error(f'process failed at {n}. {topic}, event:{sensitive_word_filter(event.__dict__)}')
            if n < times:
                await asyncio.sleep(interval)
        else:
            times = 1
    if not success:
        log.error(
            f'process failed after retry {times} times. '
            f'{topic}, event:{sensitive_word_filter(event.__dict__)}'
        )


update_task_log = to_async_method(modify_task_log)
complete_job_task = to_async_method(complete_job_center_task)
update_job_lock_id = to_async_method(modify_job_lock_id)


class Process:
    def __init__(self, topic, group, lock, unlock, handler, job_log=None, failures=None, wait_timeout=30,
                 priority=2, load=None, terminate=False, prepare=None, status=()):
        self.topic = topic
        self.group = group
        self.lock = lock
        self.unlock = unlock
        self.handler = handler
        self.job_log = job_log
        self.failures = failures
        self.wait_timeout = wait_timeout
        self.priority = priority
        self.load = load
        self.terminate = terminate
        self.prepare = prepare
        self.process_topic = topic + ".process"
        self.finish_topic = topic + ".finish"
        self.status = listify(status)

    def intercept(self, method):
        prepare = self.prepare
        topic = self.topic
        if not callable(prepare):
            return to_async_method(method)

        async def interceptor(*args, **kwargs):
            result = await to_async_method(prepare)(*args, **kwargs)
            if isinstance(result, dict):
                kwargs.update(result)
                result = True
            elif isinstance(result, tuple):
                kwargs.update(result[1])
                result = result[0]
            else:
                result = result if isinstance(result, bool) else True
            if not result:
                log.info(f"topic {topic} is not activated.")
                return
            return await method(*args, **kwargs)

        return interceptor

    async def record_job_log(
            self,
            context: Context,
            payload,
            fail: bool = False,
            log_params=None,
            status_fields=(),
            error: Exception = None
    ):
        log_params = listify(log_params or self.job_log)
        job_id = context.get(JOB_ID)
        if len(log_params) == 0 or job_id is None:
            return
        data = {
            "payload": payload,
            "context": context.data(),
            "status": "FAIL" if fail else "SUCCESS"
        }
        if fail:
            context.set(JOB_STATUS, "FAIL")
        results = [get_value_by_path(data, field) for field in status_fields]
        if fail:
            results.append("FAIL")
        level = JobLogLevel.ERROR if "FAIL" in results else JobLogLevel.INFO
        log_info = {
            "jobId": job_id,
            "startTime": int(datetime.now().timestamp() * 1000),
            "logInfo": log_params[0],
            "logInfoParam": [replace_place_holder(log_param, data) for log_param in log_params[1:]],
            "level": level.value,
            "unique": True
        }
        if isinstance(error, EmeiStorBizException):
            log_info['logDetail'] = error.error_code
            log_info['logDetailParam'] = error.parameter_list
        await update_task_log(context.request_id, job_id, {
            "jobLogs": [log_info]
        })

    async def complete(self, context: Context, stack: dict):
        fail = stack.get('fail') if stack else None
        if fail:
            job_status = "FAIL"
        else:
            job_status = normalize_job(context)
        if self.terminate:
            job_id = context.get(JOB_ID)
            if job_status == "ABORTED":
                job_complete_param = {
                    "status": JobStatus.ABORTED.value,
                }
            elif job_status == "FAIL":
                job_complete_param = {
                    "status": JobStatus.FAIL.value,
                }
            elif job_status == "PARTIAL_SUCCESS":
                job_complete_param = {
                    "status": JobStatus.PARTIAL_SUCCESS.value,
                }
            else:
                job_complete_param = {
                    "status": JobStatus.SUCCESS.value,
                    "progress": 100,
                }
            if isinstance(self.terminate, dict):
                self.init_terminate_job_copy_param(self.terminate, context, job_complete_param)
            elif callable(self.terminate):
                job_complete_param.update(self.terminate(context.data()))
            await complete_job_task(context.request_id, job_id, job_complete_param)
        if fail:
            self.send_message_by_config(context, stack.get('payload'))
        returns = stack.get('returns')
        send_next_topic_message(context, returns)
        if self.terminate:
            context.delete_all()

    def init_terminate_job_copy_param(self, terminate_config, context: Context, job_complete_param):
        job_copy_config = terminate_config.get("copy", {})
        if isinstance(job_copy_config, str):
            job_copy_config = {
                "copyId": f"{self.terminate}.copyId",
                "copyTime": f"{job_copy_config}.copyTime"
            }
        elif isinstance(job_copy_config, dict):
            job_copy_config = {
                "copyId": terminate_config.get('copyId', 'copy.copyId'),
                "copyTime": terminate_config.get('copyTime', 'copy.copyTime')
            }
        else:
            return
        data = context.data()
        job_complete_param["copyId"] = get_value_by_path(data, job_copy_config['copyId'])
        job_complete_param["copyTime"] = get_value_by_path(data, job_copy_config['copyTime'])

    async def lock_request(self, request: EsEvent, **payload):
        request_id = request.request_id
        # 保证在加锁阶段任务不被中止
        context = Context(request_id)
        if context.hash_exist(RUNNING_STATE_COUNT):
            context.increment(RUNNING_STATE_COUNT, 1)
            log.info(
                f'Lock request, RUNNING_STATE_COUNT equals {context.get(RUNNING_STATE_COUNT)}.'
                f'JobId: {request_id}')
        if self.load:
            loaded_data = self.load(request, payload)
            if isinstance(loaded_data, dict):
                payload.update(loaded_data)
        arguments = {"request_id": request_id, **payload}
        resources = resolve_resources(self.lock, arguments)
        context = Context(request_id)
        context.push_stack(self.process_topic, {
            "payload": payload,
        })
        if not resources:
            return await self.on_lock_response(request, "", "success")
        lock_id = request_id + self.process_topic + "@ignore"
        msg = LockRequest(
            request_id=request_id,
            resources=resources,
            wait_timeout=self.wait_timeout,
            priority=self.priority,
            lock_id=lock_id,
            response_topic=self.process_topic
        )
        context.set(LOCK_ID, lock_id)
        producer.produce(msg)
        await self.record_job_log(context, payload, log_params='lock_running_label')

    async def on_lock_response(self, request: EsEvent, error_desc, status):
        request_id = request.request_id
        context = Context(request_id)
        stack = context.pop_stack(self.process_topic)
        if stack is None:
            log.warn(f"workflow context stack data is missing. request id: {request_id}")
            return
        log.info(
            f"backup workflow [resource locked], request_id={request_id}, status={status}, error_code={error_desc}")
        if status != "success":
            log.warn(f"workflow lock failed. request id: {request_id}")
            stack_payload = stack.get('payload')
            await self.record_job_log(context, stack_payload, True, log_params='lock_failed_label')
            await self.complete(context, {**stack, "fail": True})
            self.send_message_by_config(context, stack_payload)
            return
        payload = stack.get('payload')
        await self.record_job_log(context, payload, log_params='lock_succeeded_label')
        await update_job_lock_id(context)
        if context.hash_exist(RUNNING_STATE_COUNT):
            context.increment(RUNNING_STATE_COUNT, -1)
        await self.wrapper(request, **payload)

    async def wrapper(self, request: EsEvent, **payload):
        request_id = request.request_id
        context = Context(request_id)
        try:
            returns = await self.handler(request, **payload)
            fail = returns is not None and returns.get('success') is False
            await self.record_job_log(context, payload, fail, status_fields=self.status)
            await self.unlock_request(context, payload, returns=returns)
        except Exception as e:
            log.exception(f"process topic '{self.topic}' is failed. request id:{request_id}")
            await self.record_job_log(context, payload, True, status_fields=self.status, error=e)
            unlock_request_result = await self.unlock_request(context, payload, fail=True)
            if not unlock_request_result:
                self.send_message_by_config(context, payload, error=e)

    def send_message_by_config(self, context: Context, payload, error=None):
        if self.failures:
            send_message_by_config(self.failures, context, payload)
        elif error is not None:
            raise error

    async def unlock_request(self, context: Context, payload, returns=None, fail=False):
        lock_id = context.get(LOCK_ID)
        request_id = context.request_id
        if context.hash_exist(RUNNING_STATE_COUNT):
            context.increment(RUNNING_STATE_COUNT, 1)
            log.info(
                f'Unlock request, RUNNING_STATE_COUNT equals {context.get(RUNNING_STATE_COUNT)}.'
                f'JobId: {request_id}')
        unlocking = self.unlock and lock_id
        if unlocking:
            push_stack(context, self.finish_topic, payload, returns, fail)
            context.set(UNLOCK_ACTION_FLAG, "true")
            await self.record_job_log(context, payload, log_params='unlock_running_label')
            message = UnlockRequest(
                request_id=request_id,
                lock_id=lock_id,
                response_topic=self.finish_topic
            )
            producer.produce(message)
            return True
        else:
            if self.terminate:
                await self.complete(context, {"fail": fail, "payload": payload, "returns": returns})
                return True
            else:
                send_next_topic_message(context, returns)

        return False

    async def unlock_response(self, request: EsEvent, **payload):
        request_id = request.request_id
        context = Context(request_id)
        stack = context.pop_stack(self.finish_topic)
        flag = context.pop(UNLOCK_ACTION_FLAG)
        if not flag:
            await self.complete(context, stack)
            return
        status = payload.get('status')
        error_desc = payload.get('error_desc')
        if normalize_job(status) == "FAIL":
            log.error(f"topic: {self.finish_topic}, request id: {request_id}, error: {error_desc}")
            stack_payload = stack.get('payload')
            self.send_message_by_config(context, stack_payload)
            await self.record_job_log(context, payload, True, log_params='lock_failed_label')
            return
        await self.record_job_log(context, payload, log_params='unlock_succeeded_label')
        context.delete(LOCK_ID)
        await update_job_lock_id(context)
        if context.hash_exist(RUNNING_STATE_COUNT):
            context.increment(RUNNING_STATE_COUNT, -1)
        await self.complete(context, stack)


def build_message_payload(configuration, params):
    if configuration[0] == '{' and configuration[-1] == '}':
        result = replace_place_holder(configuration, params)
        return json.loads(result)
    return get_value_by_path(params, configuration)


def normalize_job(status: Union[Context, str]):
    if isinstance(status, str):
        return status.upper()
    value = status.get(STATUS)
    if value is None:
        return
    return value.upper()


def normalize_message(configuration, params):
    if isinstance(configuration, str):
        index = configuration.find(':')
        configuration = [configuration[:index], *configuration[index + 1:].split(',')] if index != -1 else [
            configuration, "payload"]
    message_topic = configuration[0]
    message_payload = {}
    for item in configuration[1:]:
        data = build_message_payload(item, params)
        if isinstance(data, dict):
            message_payload.update(data)
    if message_topic and message_payload:
        return message_topic, message_payload
    return None


def send_message_by_config(configuration, context: Context, payload):
    configuration = to_collection(configuration)
    params = {
        "context": context.data(),
        "payload": payload
    }
    messages = [normalize_message(config, params) for config in configuration]
    for message in messages:
        if message:
            event = CommonEvent(message[0], context.request_id, **message[1])
            producer.produce(event)


def send_next_topic_message(context: Context, returns):
    if not valid_next_topic_message(returns):
        return
    return_topic = returns.get('topic')
    return_message = {**returns.get('message')}
    request_id = return_message.pop("request_id") if "request_id" in return_message else context.request_id
    message = CommonEvent(return_topic, request_id, **return_message)
    producer.produce(message)


def valid_next_topic_message(data):
    if not isinstance(data, dict):
        return False
    topic = data.get('topic')
    if topic is None:
        return False
    message = data.get('message')
    return isinstance(message, dict)


def resolve_resources(lock_config, params: dict):
    lock_config = lock_config if isinstance(lock_config, (list, tuple)) else [lock_config]
    if not lock_config:
        return []

    def builtin_resolve(data, *fields):
        items = [resolve(data, field) for field in fields]
        return [{"lock_type": "w", **each} for item in items for each in item if each and 'id' in each]

    def resolve(data, field):
        index = field.find(":")
        result: list
        if index == -1:
            value = get_value_by_path(data, field)
            if isinstance(value, (list, set, tuple)):
                result = list(value)
            elif isinstance(value, dict):
                result = [value]
            else:
                result = [{
                    "id": value,
                    "lock_type": "w"
                }]
        else:
            result = [{
                "id": get_value_by_path(data, field[index + 1:]),
                "lock_type": field[0:index]
            }]
        return result

    if not callable(lock_config[0]):
        return builtin_resolve(params, *lock_config)
    return lock_config[0](CallableDict(builtin_resolve, **params), *lock_config[1:])


def push_stack(context: Context, message_topic, message_payload, returns=None, fail=False):
    context.push_stack(message_topic, {
        "payload": message_payload,
        "returns": returns,
        "fail": fail
    })


class KafkaClient:
    group: str
    admin: AdminClient
    handlers: Dict[Topic, Callable]
    running: bool

    def __init__(self, group: str = 'default', admin: AdminClient = None, commit_retry: int = 3):
        self.group = group
        self.admin = admin or get_admin_client()
        self.handlers = {}
        self.running = False
        self.commit_retries = max(commit_retry, 1)

    def topic_handler(
            self,
            topic: str,
            group: str = None,
            lock=None,
            unlock=False,
            job_log=None,
            failure=None,
            load=None,
            terminate=False,
            prepare=None,
            status=(),
            lock_timeout=30,
            auto_offset_reset: str = 'earliest'
    ):
        """
        topic handler decorator

        :param topic: topic
        :param group: group
        :param lock: lock info
        :param unlock: unlock flag
        :param job_log: jog log info
        :param failure: failure message config
        :param load: data loader
        :param terminate: terminate handler flag
        :param prepare: prepare function
        :param status: handler status
        :param lock_timeout: 资源锁加锁超时时间
        :param auto_offset_reset: auto offset reset config; earliest, latest, or none; default earliest
        :return: decorated handler
        """

        def decorator(handler):
            process = Process(
                topic=topic,
                group=group,
                lock=lock,
                unlock=unlock,
                handler=to_async_method(handler),
                job_log=job_log,
                failures=failure,
                load=load,
                terminate=terminate,
                prepare=prepare,
                status=status,
                wait_timeout=lock_timeout
            )
            if process.lock:
                self.register_topic_handler(
                    group=process.group,
                    handlers={
                        process.topic: process.intercept(process.lock_request),
                        process.process_topic: process.on_lock_response,
                    },
                    auto_offset_reset=auto_offset_reset
                )
            else:
                self.register_topic_handler(
                    topic=process.topic,
                    handler=process.intercept(process.wrapper),
                    group=process.group,
                    auto_offset_reset=auto_offset_reset
                )
            if process.unlock:
                self.register_topic_handler(
                    topic=process.finish_topic,
                    handler=process.unlock_response,
                    group=process.group,
                    auto_offset_reset=auto_offset_reset
                )
            return process.wrapper

        return decorator

    def register_topic_handler(
            self,
            handlers: Dict[str, callable] = None,
            topic: str = None,
            handler=None,
            group: str = None,
            auto_offset_reset: str = 'earliest'
    ):
        group = group or self.group or "default"
        if topic is not None and callable(handler):
            self.handlers[Topic(topic, group, auto_offset_reset)] = to_async_method(handler)
        if handlers:
            for key, value in handlers.items():
                self.register_topic_handler(topic=key, handler=value, group=group)
        return self

    def run(self):
        if self.running:
            return
        self.running = True
        log.info(f"Current thread: {threading.current_thread().name}, "
                 f"id: {threading.current_thread().ident} of group: {self.group} is running")
        try:
            self.create_all_topic()
            asyncio.run(self.process())
        finally:
            self.running = False
            log.warning(f"Current thread: {threading.current_thread().name}, "
                        f"id: {threading.current_thread().ident} of group: {self.group} is not running")

    def create_all_topic(self):
        # e1000集群模式下topic副本数默认6，其余模式默认4
        is_zk_cluster_enabled = settings.get_key_from_config_map(ServiceConstants.MULTI_CLUSTER_CONF,
                                                                 ServiceConstants.ZK_CLUSTER)
        if is_zk_cluster_enabled:
            topic_list = [
                NewTopic(topic.name, num_partitions=MultiClusterKafkaTopicConstants.KAFKA_DEFAULT_TOPIC_PARTITIONS,
                         replication_factor=MultiClusterKafkaTopicConstants.KAFKA_DEFAULT_TOPIC_REPLICATION_FACTOR,
                         config={'min.insync.replicas': MultiClusterKafkaTopicConstants.MIN_IN_SYNC_REPLICAS})
                for topic in self.handlers.keys()
            ]
        else:
            topic_list = list({topic.name: topic.create() for topic in self.handlers.keys()}.values())
        futures: Dict[str, Future] = self.admin.create_topics(topic_list)
        for topic_name, future in futures.items():
            try:
                future.result()
                log.info(f'Topic {topic_name} created')
            except KafkaException as ex:
                kafka_error = ex.args[0]
                if kafka_error.code() == KafkaError.TOPIC_ALREADY_EXISTS:
                    log.info(f'Topic {topic_name} already exists')
                else:
                    log.error(f'Topic {topic_name} not created. error={kafka_error.name()}')

    def start(self):
        if not self.running:
            thread = threading.Thread(
                target=self.run,
                args=(),
                name=f"kafka-consume-thread",
                daemon=True,
            )
            thread.start()
            log.info(f"consumer thread '{self.group}' is started")
            return thread

    def pause(self):
        self.running = False
        log.warning(f"Consumer: {self.group} is paused.")

    async def process(self):
        futures = []
        for topic in self.handlers.keys():
            for _ in range(topic.num_partitions):
                futures.append(self.handle(topic))
        log.info(f"There is {len(futures)} task(s) created.")
        future: Future = await asyncio.gather(*futures)
        if hasattr(future, "exception") and future.exception() is not None:
            log.exception("consume process failed", exc_info=future.exception())

    async def handle(self, topic: Topic):
        handler: callable = self.handlers.get(topic)
        config = get_default_consumer_config(topic.group, **{'auto.offset.reset': topic.auto_offset_reset})
        consumer: Consumer = topic.create_consumer(config)
        await asyncio.sleep(0)
        try:
            while True:
                if not self.running:
                    break
                message = poll(consumer)
                if message is not None:
                    await self.consume(consumer, handler, message, topic)
                await asyncio.sleep(0.1)
        finally:
            consumer.close()
            log.info(f"Consumer: {self.group} of topic: {topic.name} is closed.")

    async def consume(self, consumer: Consumer, handler, message, topic: Topic):
        start_time = datetime.now()
        event = unpack_event(message)
        if event and 'message.retry.failed' not in event.message:
            await consume(event, handler, start_time, topic, times=5, interval=120)
        await self.commit(consumer, topic, message, start_time)

    async def commit(self, consumer: Consumer, topic, message: Message, start_time: datetime):
        """
        Retry to commit message. Exception will raised when failed to commit after n retries
        """
        partition = message.partition()
        value = sensitive_word_filter(message.value())
        for times in range(self.commit_retries):
            end_time = datetime.now()
            time_cost = (end_time - start_time) / MILLI_SECONDS
            try:
                offsets = consumer.commit(message=message, asynchronous=False)
                log.debug(
                    f'Commit done. {topic} partition={partition}, offsets={offsets}, '
                    f'message={value}, cost={time_cost}ms'
                )
                return
            except KafkaException:
                log.exception(
                    f'Commit failed. {topic} partition={partition}, offset={message.offset()}, '
                    f'message={value}, times={times}, cost={time_cost}ms'
                )
                await asyncio.sleep(1)
        log.error(f"Commit message failed. {topic} partition={partition}, offset={message.offset()}")

    def get_group_topic_list(self, group: str, name: bool = False):
        return [topic.name if name else topic for topic in self.handlers.keys() if topic.group == group]
