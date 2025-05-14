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
from sqlalchemy.orm import create_session
from app.common import logger

log = logger.get_logger(__name__)


def matches_filter(obj, filter_by):
    for key in filter_by:
        if obj.__dict__[key] != filter_by[key]:
            return False
    return True


class PostgresDBSession(object):
    engine = None

    def __init__(self):
        self.session = create_session(bind=self.engine)
        self.session.begin()

    @staticmethod
    def query_filter_from_dict(base_query, db_type, filter_dict):
        """
        为查询语句添加过滤条件

        :param base_query: 基本查询语句
        :param db_type: 数据表
        :param filter_dict: 过滤条件 采用key in value过滤
        :return:
        """
        query = base_query
        for attr in filter_dict.keys():
            value = filter_dict[attr]
            query = query.filter(getattr(db_type, attr).in_([value]))
        return query

    @staticmethod
    def apply_update_on_query(query_results, filter_by, update_map):
        """
        基于查询结果，过滤并更新指对应值

        :param query_results: 查询结果
        :param filter_by: 过滤条件 采用key == value过滤
        :param update_map: 更新健值
        :return:
        """
        update_list = [r for r in query_results if matches_filter(r, filter_by)]
        for obj in update_list:
            for key in update_map:
                obj.__setattr__(key, update_map[key])

    def add(self, *args):
        self.session.add(*args)

    def query(self, db_type, filter_by=None, lock=False):
        """
        按照过滤条件查询数据库

        :param db_type: 数据表
        :param filter_by: 过滤条件 采用key in value过滤
        :param lock: 查询并更新标记 True or False
        :return: 查询结果
        """
        base_query = self.session.query(db_type)
        if lock:
            base_query = base_query.with_for_update()

        if not filter_by:
            res = base_query.all()
        else:
            query = self.query_filter_from_dict(base_query, db_type, filter_by)
            res = query.all()
        return res

    def delete_lock_id(self, db_type, lock_id):
        self.session.query(db_type).filter_by(lock_id=lock_id).delete()

    def delete_key(self, db_type, key):
        return self.session.query(db_type).filter_by(key=key).delete()

    def clear(self, db_type):
        self.session.query(db_type).delete()

    def commit(self):
        self.session.commit()

    def flush(self):
        self.session.flush()

    def close(self):
        self.session.close()

    def rollback(self):
        self.session.rollback()


def get_db_session(bind=None):
    PostgresDBSession.engine = bind
    return PostgresDBSession
