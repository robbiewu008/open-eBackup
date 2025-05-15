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
from app.resource_lock.common import consts
from app.resource_lock.kafka.rollback_utils import rollback_on_exception
from app.resource_lock.service import async_lock_service


@rollback_on_exception(api=consts.LOCK, lock_id_in=3)
def async_lock(request, resources, wait_timeout, lock_id,
               priority=consts.DEFAULT_PRIORITY):
    """
    申请分布式资源锁

    :param request: 请求ID
    :param resources: 资源对象列表（id：资源ID，lock_type：r读锁/w写锁）
    :param wait_timeout: 等锁时间（单位:秒）
    :param lock_id: 分布式锁ID
    :param priority: 申请锁的优先级，值越小优先级越高(默认优先级3)
    :return: 加锁结果（"succeeded":加锁成功，"failed":加锁失败）
    """
    return async_lock_service.lock(
        request, resources, wait_timeout, lock_id, priority)


@rollback_on_exception(api=consts.UNLOCK, lock_id_in=1)
def async_unlock(request, lock_id):
    """
    解除分布式锁（无论该锁处于锁定状态还是等锁状态）解除后触发，锁传递判断

    :param request: 请求ID
    :param lock_id: 分布式锁ID
    :return:  transferred_lock_id_list 资源锁释放后，加锁成功的锁ID列表
    """
    return async_lock_service.unlock(request, lock_id)


@rollback_on_exception(api=consts.QUERY, lock_id_in=1)
def has_lock(request, lock_id, **kwargs):
    """
    查询指定锁状态

    :param request:请求
    :param lock_id:分布式锁ID
    :param kwargs:更多参数
    :return: 布式锁ID对应状态
    """
    release_pending = kwargs.get("realse_pending", False)
    return async_lock_service.has_lock(request, lock_id, release_pending)
