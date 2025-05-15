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

from app.resource_lock.db.db_session import get_db_session
from app.resource_lock.db.db_utils import get_db_engine


class DBClass(object):
    def __init__(self):
        self.engine = get_db_engine()
        self._db_session = get_db_session(bind=self.engine)

    @staticmethod
    @contextmanager
    def wrap_session_with_context_manager(session):
        yield session
        try:
            session.commit()
        except Exception as e:
            session.rollback()
            raise e
        finally:
            session.close()

    def get_db_session(self):
        return self._db_session()

    def db_session(self):
        session = self.get_db_session()
        return self.wrap_session_with_context_manager(session)
