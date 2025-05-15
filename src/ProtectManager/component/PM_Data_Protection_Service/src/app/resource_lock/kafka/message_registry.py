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

from app.resource_lock.db.db_class import DBClass
from app.resource_lock.db.db_tables import DBMsgRegistryObj


class AbstractMessageRegistry(object):
    def push(self, key: str, msg: dict) -> None:
        raise NotImplementedError

    def pop(self, key: str) -> dict:
        raise NotImplementedError

    def get(self, key: str) -> dict:
        raise NotImplementedError

    def all_key(self) -> list:
        raise NotImplementedError


class MessageRegistry(AbstractMessageRegistry, DBClass):
    """消息库数据库
    """

    def push(self, key: str, msg: dict):
        """
        插入消息

        :param key: key
        :param msg: 附加信息
        :return:
        """
        with self.db_session() as session:
            query = session.query(
                db_type=DBMsgRegistryObj, filter_by={'key': key})
            if len(query) > 0:
                return
            data = json.dumps(msg)
            session.add(DBMsgRegistryObj(key, data))

    def pop(self, key: str) -> dict:
        """
        移除消息

        :param key: key
        :return:
        """
        with self.db_session() as session:
            try:
                query = session.query(
                    db_type=DBMsgRegistryObj, filter_by={'key': key})
            except Exception as e:
                raise e
            finally:
                session.delete_key(db_type=DBMsgRegistryObj, key=key)
            if len(query) == 0:
                return {}
            if len(query) > 1:
                msg = f'duplicate msg registry key key={key} query={query}'
                raise Exception(msg)
            return json.loads(query[0].value)

    def get(self, key: str) -> dict:
        """
        移除消息

        :param key: key
        :return:
        """
        with self.db_session() as session:
            try:
                query = session.query(
                    db_type=DBMsgRegistryObj, filter_by={'key': key})
            except Exception as e:
                raise e
            if len(query) == 0:
                return {}
            if len(query) > 1:
                msg = f'duplicate msg registry key key={key} query={query}'
                raise Exception(msg)
            return json.loads(query[0].value)

    def all_key(self) -> list:
        """
        获取全部key

        :return: key队列
        """
        with self.db_session() as session:
            return [m.key for m in session.query(db_type=DBMsgRegistryObj)]
