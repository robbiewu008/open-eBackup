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
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, DBRetryException
from app.common.redis_session import redis_session
from app.common.util.retry.retryer import retry
from app.resource_lock.common import consts
from app.resource_lock.db import db_tables
from app.resource_lock.db.db_class import DBClass
from app.resource_lock.kafka.rollback_utils import sync_unlock_deduplicate, sync_lock_deduplicate
from app.resource_lock.schemas.lock_schemas import LockRequest
from app.resource_lock.service import resource_group

LOGGER = logger.get_logger(__name__)
lock_session = resource_group.ResourceGroup()


@sync_lock_deduplicate()
@retry(exceptions=DBRetryException, tries=5, wait=60, backoff=1, logger=LOGGER)
def lock(lock_request: LockRequest):
    """
    资源同步加锁

    :param lock_request 资源锁加锁请求
    """
    LOGGER.info(f"lock resources request_id: {lock_request.request_id} lock_id: {lock_request.lock_id}, "
                f"resources: {lock_request.resources}")
    with lock_session.start_session(lock_request.lock_id, session_type=consts.LOCK) as session:
        request_dict = lock_request.dict()
        LOGGER.info(f"lock resources request_id, resource dict: {request_dict}")
        try:
            session.update_resources(request_dict.get("resources"), lock_request.priority)
            result, resource_id = session.try_lock()
            if result is False:
                session.release_all()
            else:
                toolkit.send_resource_redis_lock_request(
                    resources=[vars(resource) for resource in lock_request.resources],
                    lock_id=lock_request.lock_id)
            return result, resource_id
        except DBRetryException as db_exception:
            raise db_exception
        except Exception as e:
            LOGGER.exception(
                f"lock resource failed request_id: {lock_request.request_id}, lock_id: {lock_request.lock_id}.")
            # 加锁过程中出现异常，直接把锁释放
            session.release_all()
            raise EmeiStorBizException(error=CommonErrorCodes.OPERATION_FAILED,
                                       error_message="lock resource failed") from e


@sync_unlock_deduplicate()
@retry(exceptions=DBRetryException, tries=5, wait=60, backoff=1, logger=LOGGER)
def unlock(request_id: str, lock_id: str):
    """
    资源同步解锁

    :param request_id: 任务请求id
    :param lock_id: 任务的资源锁id，需要跟加锁时的id一致，否则无法解锁
    """
    LOGGER.info(f"unlock resource request_id: {request_id} lock_id: {lock_id}.")
    with lock_session.start_session(lock_id, session_type=consts.UNLOCK) as session:
        try:
            session.release_all()
        except DBRetryException as db_exception:
            raise db_exception
        except Exception as e:
            LOGGER.exception(
                f"unlock resource failed request_id: {request_id} lock_id: {lock_id}.")
            raise EmeiStorBizException(error=CommonErrorCodes.OPERATION_FAILED,
                                       error_message="unlock resource failed") from e


@retry(exceptions=DBRetryException, tries=5, wait=60, backoff=1, logger=LOGGER)
def clear():
    """
    清空资源锁

    """
    LOGGER.warn("clear all lock resources.")
    with DBClass().db_session() as session:
        try:
            session.clear(db_tables.DBResource)
            session.flush()
        except DBRetryException as db_exception:
            raise db_exception
        except Exception as e:
            LOGGER.exception(
                f"clear lock resources failed.")
            raise EmeiStorBizException(error=CommonErrorCodes.OPERATION_FAILED,
                                       error_message="clear lock resources failed") from e
