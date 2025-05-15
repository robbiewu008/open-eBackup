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
from typing import Type
from sqlalchemy.exc import IntegrityError

from app.base.db_base import database
from app.common.lock.lock_object import LockObject
from app.common.logger import get_logger

log = get_logger(__name__)


class CRUDLockObject(object):

    def __init__(self, model: Type[LockObject]):
        self.model = model

    @staticmethod
    def create(lock_object: LockObject):
        with database.session() as session:
            session.add(lock_object)
            try:
                session.flush([lock_object])
            except IntegrityError as error:
                log.warning(f'create lock object error. code: {error.args}')
                session.rollback()
                return False
        return True

    @staticmethod
    def get_by_key(key):
        with database.session() as session:
            return session.query(LockObject).filter(LockObject.key == key).first()

    @staticmethod
    def delete_by_key(key):
        with database.session() as session:
            session.query(LockObject).filter(LockObject.key == key).delete()

    @staticmethod
    def delete(lock_id):
        with database.session() as session:
            session.query(LockObject).filter(LockObject.id == lock_id).delete()


curd_lock_object = CRUDLockObject(LockObject)
