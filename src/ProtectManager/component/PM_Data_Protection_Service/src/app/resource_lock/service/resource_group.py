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
from contextlib import contextmanager

from app.common import logger
from sqlalchemy_utils import database_exists

from app.resource_lock.common import consts
from app.resource_lock.service import resource_group_session
from app.resource_lock.db.db_class import DBClass

log = logger.get_logger(__name__)


class ResourceLockManager(object):
    @contextmanager
    def start_session(self, lock_id: str, session_type: str) -> None:
        raise NotImplementedError


class ResourceGroup(ResourceLockManager, DBClass):
    """资源分布式锁管理会话
    """

    def start_session(self, lock_id: str, session_type: str) -> None:
        """
        启动数据库会话

        :param lock_id: 锁ID
        :param session_type: 会话类型（lock:加锁，unlock：解锁，query:查询）
        :return:
        """
        if not database_exists(self.engine.url):
            msg = f'DB self.engine.url={self.engine.url} does not exist'
            raise resource_group_session.ResourceLockSessionException(msg)
        if session_type not in [consts.LOCK, consts.UNLOCK, consts.QUERY]:
            msg = f'invalid session type session_type={session_type}'
            raise resource_group_session.ResourceLockSessionException(msg)
        session = resource_group_session.ResourceGroupSession(
            lock_id=lock_id, db_session=self._db_session())
        return self.wrap_session_with_context_manager(session)
