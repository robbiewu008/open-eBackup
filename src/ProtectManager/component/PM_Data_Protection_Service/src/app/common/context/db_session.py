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
import logging
import os
import threading
from contextlib import contextmanager

import redis
from sqlalchemy import (
    create_engine,
    Column,
    String,
    BINARY,
    exc,
)
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from sqlalchemy_utils import database_exists, create_database

from ..config import settings
from ..constants.constant import GaussdbCertConfig

log = logging.getLogger(__name__)

Base = declarative_base()
Session = sessionmaker(autoflush=False)


def get_engine_url(hostname, port, db_type):
    if db_type == "sqlite":
        return 'sqlite:///context.db'
    else:
        db_username = os.getenv('DB_USERNAME', '')
        db_password = os.getenv('DB_PASSWORD', '')
        return f'postgresql://{db_username}:{db_password}@{hostname}:{port}/WORKFLOW'


class ContextItem(Base):
    __tablename__ = 'CONTEXT'
    key = Column(String, primary_key=True)
    val = Column(BINARY)

    def __repr__(self):
        return "<Context(key=%s, val='%s')>" % (self.key, self.val,)


class SessionManager:
    def __init__(self, db_type):
        host = settings.POSTGRES_HOST
        port = settings.POSTGRES_PORT
        db_encoding = settings.POSTGRES_ENCODING
        self.echo_sql = False
        if os.getenv('ECHO_SQL_COMMANDS', 'False') == 'True':
            self.echo_sql = True
        self.engine = None
        self.setup = 'STARTING'
        try:
            self.engine = create_engine(
                get_engine_url(host, port, db_type),
                echo=self.echo_sql,
                isolation_level=GaussdbCertConfig.AUTOCOMMIT,
                connect_args=GaussdbCertConfig.CONN_ARGS
            )
            self.setup = 'ENGINE_CREATED'
            if not database_exists(self.engine.url):
                create_database(self.engine.url, encoding=db_encoding, template="template0")
            self.setup = 'DATABASE_CREATED'

            Session.configure(bind=self.engine)

            Base.metadata.create_all(bind=self.engine)
            self.setup = 'TABLES_CREATED'
            if db_type == 'sqlite':
                self.setup = 'DUMMY'
            else:
                self.setup = 'REAL'
        except exc.SQLAlchemyError as e:
            log.error(f'After {self.setup} , will rollback after exception e={e}.')

    @contextmanager
    def session_scope(self):
        """Provide a transactional scope around a series of operations."""
        session = Session()
        try:
            yield session
            session.commit()

        except exc.SQLAlchemyError as e:
            log.error(f'Will rollback after exception e={e}.')
            session.rollback()
            raise
        finally:
            session.close()


class SqlDB:
    def __init__(self, db_type):
        self.session_mgr = SessionManager(db_type)

    @staticmethod
    def encode(obj):
        if isinstance(obj, str):
            return bytes(obj, encoding='utf-8')
        elif isinstance(obj, int):
            return bytes([obj])
        else:
            return bytes(obj)

    def set(self, key, val):
        with self.session_mgr.session_scope() as session:
            stmt = session.query(ContextItem).filter(ContextItem.key == key)

            if stmt.one_or_none() is None:
                ctx_item = ContextItem(key=key, val=self.encode(val))
                session.add(ctx_item)
            else:
                stmt.update({'val': self.encode(val)})

    def delete(self, key):
        with self.session_mgr.session_scope() as session:
            session.query(ContextItem).filter(ContextItem.key == key).delete()

    def get(self, key):
        with self.session_mgr.session_scope() as session:
            obj = (
                session.query(ContextItem).filter(ContextItem.key == key).one_or_none()
            )
            if obj is not None:
                return obj.val

    def query_by_filter(self, key_prefix):
        with self.session_mgr.session_scope() as session:
            objs = (
                session.query(ContextItem)
                .filter(ContextItem.key.like(f'{key_prefix}%'))
                .all()
            )
            ret_list = []
            for obj in objs:
                item = ContextItem(key=obj.key, val=obj.val)
                ret_list.append(item)
            return ret_list


class RedisDB(redis.Redis):
    def __init__(self):
        redis_url = os.getenv('REDIS_URL', 'redis-master')
        super().__init__(host=redis_url, port=6369, db=0)

    def query_by_filter(self, key_prefix):
        keys = [str(k, encoding='utf-8') for k in self.scan_iter(f'{key_prefix}*')]
        ret_list = []
        for key in keys:
            val = self.get(key)
            item = ContextItem(key=key, val=val)
            ret_list.append(item)
        return ret_list


db = None
db_lock = threading.Lock()
g_db_type = None


def get_db_instance(db_type):
    global db
    global g_db_type
    db_lock.acquire()
    if db is None or g_db_type != db_type:
        log.info(f'Creating database connection to {db_type}.')
        if db_type == 'redis':
            db = RedisDB()
        else:
            db = SqlDB(db_type)
        g_db_type = db_type
    db_lock.release()
    return db
