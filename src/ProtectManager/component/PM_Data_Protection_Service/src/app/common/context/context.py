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
import ast
import json
from json import JSONDecodeError
from typing import TypeVar, Type

from app.common.redis_session import redis_session

T = TypeVar('T')
STACK = "stack"


class Context:
    def __init__(self, request_id):
        self.db = redis_session
        self.request_id = request_id

    def exist(self) -> bool:
        return self.db.hgetall(self.request_id)

    def has(self, key: str):
        return self.db.exists(key)

    def hash_exist(self, key: str) -> bool:
        return self.db.hexists(self.request_id, key)

    def get(self, key: str, t: Type[T] = str) -> T:
        if t is str:
            return self.db.hget(self.request_id, key)
        elif t is dict:
            value = self.db.hget(self.request_id, key)
            if value is None:
                return
            try:
                return json.loads(value)
            except JSONDecodeError as e:
                if e.msg == 'Expecting property name enclosed in double quotes':
                    return ast.literal_eval(value)
                raise
        elif t is None:
            value = self.db.hget(self.request_id, key)
            if value is None:
                return None
            try:
                return json.loads(value)
            except:
                return value

    def set(self, key, val):
        if isinstance(val, str):
            self.db.hset(self.request_id, key, val)
        else:
            self.db.hset(self.request_id, key, json.dumps(val))

    def delete(self, key):
        self.db.hdel(self.request_id, key)

    def pop(self, key, t: Type[T] = None) -> T:
        value = self.get(key, t)
        self.delete(key)
        return value

    def delete_all(self):
        self.db.delete(self.request_id)

    def data(self):
        keys = self.db.hkeys(self.request_id)
        return {key: self.get(key, None) for key in keys}

    def pop_stack(self, topic):
        stack = self.get(STACK, dict)
        if stack is None or stack.get('topic') != topic:
            return None
        old_stack = stack.get(STACK)
        if old_stack is not None:
            self.db.set(STACK, json.dumps(old_stack))
        else:
            self.db.delete(STACK)
        return stack.get('message')

    def push_stack(self, topic, message):
        stack = self.get(STACK, dict)
        self.db.hset(self.request_id, STACK, json.dumps({
            "topic": topic,
            "message": message,
            "stack": stack
        }))

    def increment(self, key, amount):
        return self.db.hincrby(self.request_id, key, amount)

    def expire(self, time):
        self.db.expire(self.request_id, time)
