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
import functools

from app.common import logger
from app.common.deploy_type import DeployType
from app.common.enums.resource_enum import DeployTypeEnum
from app.common.redis_session import redis_session
from app.common.util.retry.retryer import retry

from app.resource_lock.common import consts
from app.resource_lock.kafka import messaging_utils

log = logger.get_logger(__name__)


def get_from_args(args: tuple, kwargs: dict, key: str, place_in_args: int):
    """
    获取参数值，字典参数中无此key时，获取列表指定值

    :param args: 列表参数
    :param kwargs: 字典参数
    :param key: 参数key名称
    :param place_in_args: 列表参数位置
    :return: 参数key的值
    """
    arg = kwargs.get(key, None)
    if arg is None and len(args) > place_in_args:
        arg = args[place_in_args]
    return arg


def rollback_on_exception(api: str, lock_id_in: int):
    """
    消息队列调用异常封装处理

    :param api: 调用api
    :param lock_id_in: lock_id参数位置
    :return: 异常处理装饰函数
    """
    def decorator(func):
        @functools.wraps(func)
        def rollback_wrapper(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except Exception as e:
                log.error(f'Caught exception e={e} rolling back')
                request = get_from_args(args, kwargs, 'request', 0)
                lock_id = get_from_args(args, kwargs, 'lock_id', lock_id_in)
                _msg = f'Rollback {api} request={request}, lock_id={lock_id}'
                log.error(_msg)
                if api in [consts.LOCK, consts.UNLOCK]:
                    messaging_utils.handle_response(
                        lock_id, api, request, consts.PROGRESS_FAILED)
        return rollback_wrapper

    return decorator


def deduplication(api: str):
    """
    消息去重
    :return: 处理方法
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapper(request, *args, **kwargs):
            if api not in [consts.LOCK, consts.UNLOCK]:
                return False
            request_id = request.request_id
            # CloudBack
            locked_flag = f"{api}_{request_id}"
            if DeployType().get_deploy_type() in [DeployTypeEnum.X9000.value, DeployTypeEnum.X8000.value,
                                                  DeployTypeEnum.A8000.value,
                                                  DeployTypeEnum.X6000.value, DeployTypeEnum.X3000.value,
                                                  DeployTypeEnum.PACIFIC, DeployTypeEnum.DEPENDENT]:
                # 有及时挂载功能的都要加上
                lock_id = args[0] if api == consts.UNLOCK else args[2]
                locked_flag = f"{locked_flag}_{lock_id}"
            log.info(f"locked_flag: {locked_flag}, job id: {request_id} ")
            try:
                msg_repeat = is_repeat(locked_flag)
            except Exception as ex:
                log.error(ex)
                return False
            # 若该任务已经加锁/解锁，不再重复消费消息
            if msg_repeat:
                log.warn(f"The job({request_id}) has been {api} and does not need to be reprocessed.")
                return True
            result = func(request, *args, **kwargs)
            if result:
                # 过期时间1天
                redis_session.setex(locked_flag, 1 * 24 * 60 * 60, locked_flag)
                # 解锁了就把已加锁删除, 加锁了就把已解锁删除
                api_set = {consts.UNLOCK, consts.LOCK}
                if api in api_set:
                    api_set.remove(api)
                    rm_flag = locked_flag.replace(api, api_set.pop(), 1)
                    log.info(f"rm flag {rm_flag} from redis")
                    redis_session.delete(rm_flag)
            return result

        return wrapper

    return decorator


def sync_unlock_deduplicate():
    """
    同步资源锁消息去重
    : return: 处理函数
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            request_id = args[0]
            # CloudBackup
            locked_flag = f"unlock_{request_id}"
            if DeployType().is_support_multi_job_deploy_type():
                # X8000
                lock_id = args[1]
                locked_flag = f"{locked_flag}_{lock_id}"
            log.info(f"locked_flag: {locked_flag}, job id: {request_id} ")
            try:
                is_msg_repeat = is_repeat(locked_flag)
            except Exception as ex:
                log.error(ex)
                return

            # 若该任务已经加锁/解锁，不再重复消费消息
            if is_msg_repeat:
                log.warn(f"The job({request_id}) has been unlock and does not need to be reprocessed.")
                return
            func(*args, **kwargs)
            log.info(f"Sync unlock success, set redis locked flag: {locked_flag}")
            redis_session.setex(locked_flag, 1 * 24 * 60 * 60, locked_flag)
        return wrapper

    return decorator


def sync_lock_deduplicate():
    """
    同步资源锁消息去重
    : return: 处理函数
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            request_id = args[0].request_id
            # CloudBackup
            locked_flag = f"lock_{request_id}"
            if DeployType().is_support_multi_job_deploy_type():
                # X8000
                lock_id = args[0].lock_id
                locked_flag = f"{locked_flag}_{lock_id}"
            try:
                is_msg_repeat = is_repeat(locked_flag)
            except Exception as ex:
                log.error(ex)
                return False, ''

            # 若该任务已经加锁/解锁，不再重复消费消息
            if is_msg_repeat:
                log.warn(f"The job({request_id}) has been locked and does not need to be reprocessed.")
                return True, ''

            result, result_id = func(*args, **kwargs)
            if result:
                log.info(f"Sync lock success, set redis locked flag: {locked_flag}")
                redis_session.setex(locked_flag, 1 * 24 * 60 * 60, locked_flag)
            return result, result_id
        return wrapper

    return decorator


@retry(exceptions=Exception, tries=5, wait=60, logger=log)
def is_repeat(key: str):
    return redis_session.exists(key)
