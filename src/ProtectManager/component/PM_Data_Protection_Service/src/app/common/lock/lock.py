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
import time
import uuid
import socket

from app.common.lock.curd_lock_object import curd_lock_object
from app.common.lock.lock_object import LockObject
from app.common.logger import get_logger

log = get_logger(__name__)


class Lock:
    """
    基于数据库实现的分布式锁，支持阻塞，非阻塞加锁， 超时自动释放。

    :param key: 锁关键字
    """

    def __init__(self, key):
        self.key = key

    def lock(self, timeout=600, sleep=0.5, blocking_timeout=None, desc=None):
        """
        申请分布式锁

        :param timeout: 锁超时时间, 超时后锁会自动失效。 单位 s
        :param sleep: 阻塞加锁循环等待时间。阻塞使用while true实现，每次循环都需要sleep, 支持设置sleep时间，默认0.5s 单位 s
        :param blocking_timeout: 阻塞等待时间 单位 s, 设置后会在时间周期内循环获取错，超时后返回False, 如果为None，则为非阻塞加锁，尝试加锁后直接返回结果。
        :param desc: 锁描述
        """
        stop_lock_time = time.time()
        if blocking_timeout is not None:
            stop_lock_time = time.time() + blocking_timeout
        while True:
            if self._do_acquire(timeout, desc):
                log.info(f'lock success. key: {self.key}')
                return True
            # 非阻塞直接返回
            if blocking_timeout is None:
                log.info(f'lock failed. key: {self.key}')
                return False
            next_lock_time = time.time() + sleep
            # 阻塞在超时时间内循环获取锁
            if next_lock_time > stop_lock_time:
                log.info(f'lock block timeout. key: {self.key}')
                return False
            time.sleep(sleep)

    def unlock(self):
        """
        解除分布式锁
        """
        curd_lock_object.delete_by_key(self.key)
        log.info(f'unlock success. key: {self.key}')

    def is_locked(self):
        """
        查询当前关键字是否已锁定
        """
        lock_object = curd_lock_object.get_by_key(self.key)
        if lock_object is None:
            return False
        else:
            if lock_object.unlock_time < datetime.datetime.utcnow():
                curd_lock_object.delete(lock_object.id)
                return False
            else:
                return True

    def _do_acquire(self, timeout, desc=None):
        lock_object = curd_lock_object.get_by_key(self.key)
        if lock_object is None:
            # 如果数据库中不存记录直接插入Lock数据加锁
            return curd_lock_object.create(self._generate_lock_object(timeout, desc))
        else:
            # 判断数据库中已存在的记录是否已超时，超时删除记录
            if lock_object.unlock_time < datetime.datetime.utcnow():
                curd_lock_object.delete(lock_object.id)
            return False

    def _generate_lock_object(self, timeout, desc=None):
        local_ip = socket.gethostbyname(socket.gethostname())
        release_time = datetime.datetime.utcnow() + datetime.timedelta(seconds=timeout)
        return LockObject(id=str(uuid.uuid1()), key=self.key, owner=local_ip, description=desc,
                          unlock_time=release_time, lock_time=datetime.datetime.utcnow())
