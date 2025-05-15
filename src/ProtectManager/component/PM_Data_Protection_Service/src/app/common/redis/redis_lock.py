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
import time
import uuid

import redis

from redis.cluster import RedisCluster

from app.common.redis_session import redis_session


def get_lock(name, acquire_time=10, timeout=100):
    identifier = str(uuid.uuid4())
    return setnx(name, identifier, acquire_time, timeout)


def setnx(name, value, acquire_time=10, timeout=100):
    end = time.time() + acquire_time
    while time.time() < end:
        if redis_session.setnx(name, value):
            redis_session.expire(name, timeout)
            return value
        elif redis_session.ttl(name) == -1:
            redis_session.expire(name, timeout)
        time.sleep(0.001)
    return False


def get_lock_without_timeout(name, acquire_time=10):
    identifier = str(uuid.uuid4())
    return setnx_without_timeout(name, identifier, acquire_time)


def setnx_without_timeout(name, value, acquire_time=10):
    end = time.time() + acquire_time
    while time.time() < end:
        if redis_session.setnx(name, value):
            return value
        time.sleep(0.001)
    return ""


def release(lock_names, identifier):
    # redis集群使用Lua 脚本实现锁释放
    unlock_script = """
    if redis.call("get", KEYS[1]) == ARGV[1] then
        return redis.call("del", KEYS[1])
    else
        return 0
    end
    """
    if isinstance(redis_session, RedisCluster):
        return redis_session.eval(unlock_script, 1, lock_names, identifier)
    pipe = redis_session.pipeline(True)
    while True:
        try:
            # 通过watch命令监视某个键，当该键未被其他客户端修改值时，事务成功执行。当事务运行过程中，发现该值被其他客户端更新了值，任务失败
            pipe.watch(lock_names)
            if pipe.get(lock_names) == identifier:  # 检查客户端是否仍然持有该锁
                # multi命令用于开启一个事务，它总是返回ok
                # multi执行之后， 客户端可以继续向服务器发送任意多条命令， 这些命令不会立即被执行， 而是被放到一个队列中， 当 EXEC 命令被调用时， 所有队列中的命令才会被执行
                pipe.multi()
                # 删除键，释放锁
                pipe.delete(lock_names)
                # execute命令负责触发并执行事务中的所有命令
                pipe.execute()
                return True
            pipe.unwatch()
            break
        except redis.exceptions.WatchError:
            pass


def redis_lock(lock_names, acquire_time=100, time_out=1000):
    def _lock(func):
        def __lock(*args, **kwargs):
            identifier = get_lock(lock_names, acquire_time, time_out)
            if not identifier:
                raise Exception("acquire lock failed")
            try:
                return func(*args, **kwargs)
            finally:
                release(lock_names, identifier)

        return __lock

    return _lock
