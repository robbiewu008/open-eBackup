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
import logging
from typing import TypeVar

from app.common.redis_session import redis_session

T = TypeVar('T')

logger = logging.getLogger(__name__)


class Context:
    def __init__(self, request_id):
        self.db = redis_session
        self.name = request_id
        # 上下文默认过期时间30天
        self.db.expire(request_id, 30 * 24 * 60 * 60)

    def exist(self) -> bool:
        try:
            return self.db.hgetall(self.name)
        except Exception:
            logger.exception("get context from redis error.")
            return False

    def get(self, key: str, t: T = str):
        if t is str:
            return self.db.hget(self.name, key)
        elif t is dict:
            return json.loads(self.db.hget(self.name, key))
        else:
            return self.db.hget(self.name, key)

    def set(self, key, val):
        if isinstance(val, str):
            self.db.hset(self.name, key, val)
        elif isinstance(val, dict):
            self.db.hset(self.name, key, json.dumps(val))
        else:
            self.db.hset(self.name, key, val)

    def delete(self, key):
        self.db.hdel(self.name, key)

    def delete_all(self):
        self.db.delete(self.name)
