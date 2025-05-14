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
from app.common import logger, toolkit
from app.common.exception.unified_exception import DBRetryException

from app.resource_lock.common import consts
from app.resource_lock.kafka import messaging_utils
from app.resource_lock.kafka.rollback_utils import deduplication
from app.resource_lock.service import resource_group
from app.common.util.retry.retryer import retry

log = logger.get_logger(__name__)
backend = resource_group.ResourceGroup()


@deduplication(api=consts.LOCK)
@retry(exceptions=DBRetryException, tries=5, wait=60, backoff=1, logger=log)
def lock(request, resources, timeout, lock_id, priority):
    """
    申请分布式资源锁

    :param lock_id: 分布式锁ID
    :param request: 请求
    :param resources: 资源对象列表(id:资源ID,lock_type:r读锁/w写锁)
    :param priority: 申请锁的优先级，值越小优先级越高
    :param timeout: 等锁时间（单位:秒）
    :return:
    """
    msg = f"lock context_id={request.request_id} lock_id={lock_id}"
    log.info(f"async lock resources lock_id: {lock_id} resources: {resources} priority: {priority}")
    messaging_utils.push_to_msg_registry(lock_id, consts.LOCK, request)
    with backend.start_session(lock_id, session_type=consts.LOCK) as session:
        try:
            session.update_resources(resources, priority)
            result, resource_id = session.try_lock()
            log.info(f'log result, isOk={result}, resource_id={resource_id}')
            if result is True:
                log.info(f"{msg} {consts.PROGRESS_SUCCEEDED}.")
                toolkit.send_resource_redis_lock_request(resources=resources,
                                                         lock_id=lock_id)
                messaging_utils.handle_response(
                    lock_id, consts.LOCK, request, consts.PROGRESS_SUCCEEDED)
                return True
            if timeout > 0:
                return messaging_utils.handle_pending(lock_id, timeout)
            log.error(f"{msg} {consts.PROGRESS_FAILED}.")
            session.release_all()
            messaging_utils.handle_response(
                lock_id, consts.LOCK, request, consts.PROGRESS_FAILED, resource_id)
            return False
        except DBRetryException as db_exception:
            raise db_exception
        except Exception:
            log.exception(f"{msg}.")
            session.release_all()
            return False


@deduplication(api=consts.UNLOCK)
@retry(exceptions=Exception, tries=30, wait=60, backoff=1, logger=log)
def unlock(request, lock_id: str):
    """
    解除分布式锁（无论该锁处于锁定状态还是等锁状态）解除后触发,锁传递判断

    :param lock_id: 分布式锁ID
    :param request: 请求
    :return:  transferred_lock_id_list 资源锁释放后,加锁成功的锁ID列表
    """
    msg = f"unlock context_id={request.request_id} lock_id={lock_id}"
    log.info(f"{msg} {consts.PROGRESS_START}.")
    messaging_utils.handle_unlock_response(lock_id)
    messaging_utils.push_to_msg_registry(lock_id, consts.UNLOCK, request)
    with backend.start_session(lock_id, session_type=consts.UNLOCK) as session:
        session.release_all()
        log.info(f"{msg} {consts.PROGRESS_SUCCEEDED}.")
        locks_list = session.transfer_resources()
        log.info(f"{msg} transfer {locks_list}.")
    with backend.start_session(lock_id+'@log', session_type=consts.UNLOCK) as session:
        session.release_all()
        log.info(f"{msg}@log {consts.PROGRESS_SUCCEEDED}.")
        locks_list = session.transfer_resources()
        log.info(f"{msg}@log transfer {locks_list}.")
    messaging_utils.handle_transferred_response(request, lock_id, locks_list)
    return True


def has_lock(request, lock_id: str, release_pending: bool):
    """
    查询指定分布式锁状态

    :param request:请求
    :param lock_id:分布式锁ID
    :param release_pending:释放pending锁
    :return: 分布式锁ID对应状态
    """
    msg = f"has lock context_id={request.request_id} lock_id={lock_id}"
    log.info(f"{msg} {consts.PROGRESS_START}.")
    resource_id = None
    with backend.start_session(lock_id, session_type=consts.QUERY) as session:
        state = session.get_lock_state()
        if state == consts.STATE_PENDING and not isinstance(release_pending, bool):
            is_locked, resource_id = session.try_lock()
            if is_locked:
                state = consts.STATE_LOCKED
            elif int(release_pending) > 0:
                messaging_utils.handle_pending(lock_id, release_pending)
                return
            else:
                session.release_all()
                state = consts.STATE_UNLOCK
    messaging_utils.handle_has_lock_response(request, lock_id, state, resource_id)
