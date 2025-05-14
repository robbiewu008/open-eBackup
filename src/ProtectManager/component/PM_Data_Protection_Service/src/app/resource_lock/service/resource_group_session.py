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
import time

from app.common import logger, toolkit
from app.common.redis_session import redis_session
from app.resource_lock.common import consts
from app.resource_lock.common.consts import REDIS_RESOURCE_LOCK_PREFIX
from app.resource_lock.db import db_tables
from app.resource_lock.db.db_tables import DBResource

log = logger.get_logger(__name__)


class ResourceLockSessionException(Exception):
    pass


class ResourceLockSession(object):
    def update_resources(self, resource_list, priority: int) -> None:
        raise NotImplementedError

    def try_lock(self):
        raise NotImplementedError

    def release_all(self) -> None:
        raise NotImplementedError

    def get_lock_state(self) -> str:
        raise NotImplementedError

    def transfer_resources(self) -> list:
        raise NotImplementedError


class ResourceGroupSession(ResourceLockSession):
    """资源分布式锁数据库会话封装
    """

    def __init__(self, db_session, lock_id: str):
        """
        初始化时从数据库查询对应锁的关联资源

        :param db_session: 数据库会话
        :param lock_id: 锁ID
        """
        self.db_session = db_session
        self.lock_id = lock_id
        self.resources = self._query_resource_by_lock_id(
            self.lock_id, lock=True)

    @staticmethod
    def _is_locked(waiters: list) -> bool:
        """
        查询指定资源是已锁定，状态不一致时抛出异常

        :param waiters: 指定锁资源队列，均为同一锁
        :return: True:已锁定，False:未锁定
        """
        locked = [w.lock_state == consts.STATE_LOCKED for w in waiters]
        if not any(locked):
            # 全部为False,未加锁
            return False
        if all(locked):
            # 全部为True,已加锁
            return True
        res = [(r.resource_id, r.lock_id, r.lock_state) for r in waiters]
        msg = f"Error in logging state partial lock {res}"
        raise ResourceLockSessionException(msg)

    def update_resources(self, resources_in, priority: int) -> None:
        """
        校验资源后生成资源信息，并写入数据库

        :param resources_in: 资源对象列表（id：资源ID，lock_type：r读锁/w写锁）
        :param priority: 申请锁的优先级，值越小优先级越高
        :return:
        """
        if self._validate_resources_match_db_resources(resources_in):
            return
        support = [consts.READ_LOCK, consts.WRITE_LOCK]
        if any([a.get(consts.LOCK_TYPE) not in support for a in resources_in]):
            msg = f'invalid locking type resources_in={resources_in}'
            raise ResourceLockSessionException(msg)
        now_time = time.time()
        for resource in resources_in:
            db_resource = db_tables.DBResource(
                resource_id=resource.get(consts.RESOURCE_ID),
                lock_id=self.lock_id,
                lock_type=resource.get(consts.LOCK_TYPE),
                lock_state=consts.STATE_PENDING,
                timestamp=int(now_time),
                priority=priority,
            )
            self.db_session.add(db_resource)
            self.resources.append(db_resource)
        self.db_session.flush()

    def try_lock(self):
        """
        尝试加锁初始传入锁ID和其资源

         :return: True:加锁成功，False:加锁失败
        """
        return self._try_lock(self.lock_id, self.resources)

    def release_all(self) -> None:
        """
        资源存在时释放资源锁

        :return:
        """
        if not self.resources:
            return
        try:
            toolkit.send_resource_redis_unlock_request(lock_id=self.lock_id)
        except Exception as ex:
            log.info(f'Error occurred redis unlock ,lock_id {self.lock_id}')
            pass
        self.db_session.delete_lock_id(
            db_type=db_tables.DBResource, lock_id=self.lock_id)
        self.db_session.flush()

    def transfer_resources(self) -> list:
        """
        释放资源锁后，传递加锁（获取释放资源的可加锁ID列表，尝试加锁）

        :return: 释放资源锁后，传递加锁成功ID列表
        """
        candidate_lock_ids = set()
        for obj in self.resources:
            next_lock = self._get_next_lock(obj.resource_id)
            candidate_lock_ids = candidate_lock_ids | set(next_lock)
        transferred_lock_ids = []
        for candidate_id in list(candidate_lock_ids):
            wait_lockers = self._query_resource_by_lock_id(candidate_id)
            result, resource_id = self._try_lock(candidate_id, wait_lockers)
            if result:
                transferred_lock_ids.append(candidate_id)
        return transferred_lock_ids

    def get_lock_state(self) -> str:
        """
        查询传入锁的资源锁状态

        :return:
        """
        if not self.resources:
            return consts.STATE_UNLOCK
        elif self._is_locked(self.resources):
            return consts.STATE_LOCKED
        return consts.STATE_PENDING

    def commit(self):
        return self.db_session.commit()

    def rollback(self):
        return self.db_session.rollback()

    def flush(self):
        return self.db_session.flush()

    def close(self):
        return self.db_session.close()

    def _query_resource_by_lock_id(self, lock_id: str, lock=False):
        """
        通过锁ID,获取该锁的所有资源锁列表

        :param lock_id: 锁ID
        :return:
        """
        _filter = {db_tables.DB_COLUMN_LOCK_ID: lock_id}
        return self.db_session.query(
            db_type=db_tables.DBResource, filter_by=_filter, lock=lock)

    def _validate_resources_match_db_resources(self, resources_in):
        """
        校验传入的资源列表和数据库获取的资源列表是否一致

        :param resources_in: 资源对象列表（id：资源ID，lock_type：r读锁/w写锁）
        :return:
        """
        if not self.resources:
            return False
        db_resource_ids = sorted([r.resource_id for r in self.resources])
        input_ids = sorted([r.get(consts.RESOURCE_ID) for r in resources_in])
        if db_resource_ids != input_ids:
            _msg = "Suspected partial DB update for"
            _lock_id = f"self.lock_id={self.lock_id}"
            _db_resource_ids = f"db_resource_ids={db_resource_ids}"
            _input_resource_ids = f"input_resource_ids={input_ids}"
            msg = f"{_msg} {_lock_id} {_db_resource_ids} {_input_resource_ids}"
            raise ResourceLockSessionException(msg)
        return True

    def _try_lock(self, lock_id: str, waiters: list):
        """
        尝试加锁指定锁ID,存在资源并且可以在加锁的锁ID列中时进行加锁

        :param lock_id: 锁ID
        :param waiters: 等锁资源队列，均为同一锁
        :return: True:加锁成功; False:加锁失败,并返回资源id
        """
        if not waiters:
            msg = f"Lock {lock_id} not exist resource"
            raise ResourceLockSessionException(msg)
        if self._is_locked(waiters):
            # 该锁资源已被lock_id锁定
            log.info(f"Same lock {lock_id} has Locked, lock success!")
            return True, ''
        for obj in waiters:
            resource_id = obj.resource_id
            next_lock = self._get_next_lock(resource_id)
            if lock_id not in next_lock:
                log.info(f"Lock {lock_id} can not get lock, [{obj}] is locked by [{next_lock}]")
                # lock_id不在可以获取锁的队列中
                return False, obj.resource_id
        filter_by = {db_tables.DB_COLUMN_LOCK_ID: lock_id}
        update_map = {db_tables.DB_COLUMN_LOCK_STATE: consts.STATE_LOCKED}
        self.db_session.apply_update_on_query(waiters, filter_by, update_map)
        self.db_session.flush()
        return True, ''

    def _get_next_lock(self, resource_id: str) -> list:
        """
        通过priority和timestamp排序后获取优先级最高,时间最早的锁ID

        :param resource_id: 待锁定的资源ID
        :return: 可以获取锁的ID列表(共同读时为多个锁)
        """
        resources = self._query_resources_by_resource_id(resource_id)
        waiters = [r for r in resources if r.lock_state != consts.STATE_LOCKED]
        if len(waiters) == 0:
            # 资源不存在等待锁定的锁
            return []
        holders = [r for r in resources if r.lock_state == consts.STATE_LOCKED]
        if any(w.lock_type == consts.WRITE_LOCK for w in holders):
            if len(holders) != 1:
                msg = f"resource {holders} exist write at same time"
                raise ResourceLockSessionException(msg)
            # 存在锁定的写锁，不允许添加任何锁
            return []
        waiters.sort(key=lambda x: (x.priority, x.timestamp), reverse=True)
        waiter = waiters.pop()
        res = [waiter.lock_id]
        if waiter.lock_type == consts.WRITE_LOCK:
            # 已经存在锁定读锁的情况下，不能添加写锁
            return res if not holders else []
        # 添加读锁(可以共同读)
        while len(waiters) > 0 and waiters[-1].lock_type != consts.WRITE_LOCK:
            waiter = waiters.pop()
            res.append(waiter.lock_id)
        return res

    def _query_resources_by_resource_id(self, resource_id: str) -> list:
        """
        通过资源ID,获取资源的所有资源锁列表

        :param resource_id:
        :return:
        """
        _filter = {db_tables.DB_COLUMN_RESOURCE_ID: resource_id}
        resources = self.db_session.query(
            db_type=db_tables.DBResource, filter_by=_filter)
        self._query_redis_resource_lock(resources, resource_id)
        return resources

    def _query_redis_resource_lock(self, resources: list, resource_id: str):
        try:
            log.info(f'start query redis resource lock ,lock id :{self.lock_id}')
            # 查询Redis中的锁信息和数据库中的锁信息
            db_lock_ids = {resource.lock_id for resource in resources}
            redis_lock_ids = {str(lock_id).replace(REDIS_RESOURCE_LOCK_PREFIX, '')
                              for lock_id in redis_session.smembers('LOCK_ID_REDIS_SET_KEY') if
                              self._filter_redis_id(str(lock_id), resource_id)}
            # 找出Redis中存在但数据库中不存在的锁信息
            new_lock_ids = redis_lock_ids - db_lock_ids
            for lock_id in new_lock_ids:
                log.info(f'Redis has lock that DB doesnt, lock id:{REDIS_RESOURCE_LOCK_PREFIX}{lock_id}')
                redis_resources = json.loads(redis_session.get(f'{REDIS_RESOURCE_LOCK_PREFIX}{lock_id}'))
                # 批量添加锁信息到资源列表
                resources.extend([DBResource(lock_id=lock_id, resource_id=redis_res.get('resourceId'),
                                             lock_state=consts.STATE_LOCKED, lock_type=redis_res.get('lockType'),
                                             priority=3, timestamp=0) for redis_res in redis_resources])
        except Exception:
            log.error(f'Error querying Redis resource lock: {self.lock_id}')

    def _filter_redis_id(self, lock_id: str, resource_id: str):
        log.debug(f'Start filter redis resource lock ,lock id :{self.lock_id}')
        resources = json.loads(str(redis_session.get(lock_id)))
        for res in resources:
            if res.get('resource_id') == resource_id:
                return True
        log.debug(f'Ignore redis lock id :{lock_id}')
        return False
