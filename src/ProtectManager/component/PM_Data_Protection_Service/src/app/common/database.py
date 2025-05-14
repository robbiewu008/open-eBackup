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
import math
import re
import threading
import time
from typing import TypeVar, List, Dict, Callable

from psycopg2 import errors, OperationalError
from pydantic import PostgresDsn
from sqlalchemy import (
    create_engine,
    exc, event)
from sqlalchemy.engine import Engine
from sqlalchemy.ext.declarative import DeclarativeMeta
from sqlalchemy.orm import sessionmaker, Session, Query
from sqlalchemy.pool import QueuePool
from sqlalchemy_utils import database_exists

from app.common.config import settings
from app.common.constants.constant import GaussdbCertConfig, RBACConstants
from app.common.exception.unified_exception import DBRetryException
from app.common.logger import get_logger
from app.common.schemas.common_schemas import BasePage

log = get_logger(__name__)

SESSION = 'session'
T = TypeVar('T')

local = threading.local()


def build_engine_url(database: str, host: str = settings.POSTGRES_HOST, port: str = settings.POSTGRES_PORT,
                     username: str = settings.POSTGRES_USERNAME,
                     password: str = settings.get_db_password()):
    return PostgresDsn.build(
        scheme="postgresql",
        user=username,
        password=password,
        host=host,
        port=port,
        path=f"/{database}",
    )


def init_order_config(entities, extra_fields, orders, query):
    orders = orders if orders is not None else []
    for order in orders:
        flag = order[0]
        if flag not in '+-':
            continue
        field = get_field(entities, order[1:], extra_fields)
        if flag == '+':
            query = query.order_by(field.asc())
        elif flag == '-':
            query = query.order_by(field.desc())
    return query


class Database:
    _session_maker: sessionmaker

    def __init__(self, name: str,
                 declaration: DeclarativeMeta = None, host: str = settings.POSTGRES_HOST,
                 port: str = settings.POSTGRES_PORT,
                 username: str = settings.POSTGRES_USERNAME,
                 password: str = settings.get_db_password(),
                 encoding: str = settings.POSTGRES_ENCODING,
                 echo: bool = settings.POSTGRES_ECHO):
        self._echo = echo
        log.debug(f'echo:{self._echo}')
        self._engine = None
        self._encoding = encoding
        self.status = 'STARTING'
        try:
            self._engine = create_engine(
                build_engine_url(
                    database=name,
                    host=host,
                    port=port,
                    username=username,
                    password=password
                ),
                poolclass=QueuePool,
                max_overflow=settings.DATABASE_MAX_OVERFLOW,
                pool_size=settings.DATABASE_POOL_SIZE,
                pool_timeout=settings.DATABASE_POOL_TIMEOUT,
                pool_recycle=settings.DATABASE_POOL_RECYCLE,
                pool_pre_ping=settings.DATABASE_POOL_PRE_PING,
                echo=self._echo,
                isolation_level=GaussdbCertConfig.AUTOCOMMIT,
                connect_args=GaussdbCertConfig.CONN_ARGS
            )
            self.status = 'ENGINE_CREATED'
            self.initialize(declaration)
            self._session_maker = sessionmaker(autoflush=False)
            self._session_maker.configure(bind=self._engine)
            # 请不要日志输出：self._engine.url，其中含有明文敏感数据
            log.info(f'Database and Session Factory existing!')
        except exc.SQLAlchemyError as e:
            log.error(f'Failed after {self.status} , will rollback after exception e={e}')

    def initialize(self, declaration: DeclarativeMeta = None):
        self.status = 'DATABASE_CONNECT'
        while True:
            try:
                if database_exists(self._engine.url):
                    break
            except exc.SQLAlchemyError as e:
                log.error(f'Failed after {self.status} , will rollback after exception e={e}')
            time.sleep(3)
        time.sleep(20)
        log.info(f'Database connect OK.')

    def session(self):
        return SessionWrapper(self._session_maker)

    def paginate(self,
                 *entities,
                 page: int,
                 size: int,
                 orders: List[str] = None,
                 conditions: Dict[str, any] = None,
                 initiator: Callable[[Query], Query] = None,
                 converter: Callable[[any], T] = None,
                 extra_fields: dict = None) -> BasePage[T]:
        if size <= 0 or page < 0:
            return BasePage(items=[], total=0, pages=0, page_no=page, page_size=size)
        if len(entities) > 1 and (initiator is None or converter is None):
            if initiator is None:
                raise Exception(f"initiator is missing: {entities[0]}")
            if converter is None:
                raise Exception(f"converter is missing: {entities[0]}")
        with self.session() as session:
            count = _create_query(entities, conditions, session, initiator, extra_fields).count()
            if count == 0:
                return BasePage(items=[], total=0, pages=0, page_no=page, page_size=0)
            query: Query = _create_query(entities, conditions, session, initiator, extra_fields)
            query = init_order_config(entities, extra_fields, orders, query)
            query = query.offset(page * size).limit(size)
            data = query.all()
            if converter is not None:
                data = [converter(item) for item in data]
            return BasePage(
                items=data,
                total=count,
                pages=math.ceil(count / size),
                page_no=page,
                page_size=len(data))


def _create_query(entities, conditions, session,
                  initiator: Callable[[Query], Query] = None,
                  extra_fields: dict = None) -> Query:
    query: Query = session.query(*entities)
    param = {}
    conditions = conditions if conditions else {}

    for key in conditions:
        value = conditions[key]
        if key in RBACConstants.IGNORE_FIELDS_LIST:
            continue
        if '%' in key:
            query = add_like_filter(entities, extra_fields, query, key, value)
        elif isinstance(value, (list, tuple)):
            query = add_list_filter(entities, extra_fields, query, key, value)
        else:
            query = add_value_filter(entities, extra_fields, query, key, value, param)
    if len(param) > 0:
        query = query.filter_by(**param)
    if initiator is not None:
        query = initiator(query)
    return query


def get_attr_by_field_name(entities, extra_fields, name, pattern):
    item = get_field(entities, name)
    if not item:
        item = extra_fields.get(pattern)
    return item


def add_like_filter(entities, extra_fields, query, key, value):
    value = value if isinstance(value, str) else str(value) if value is not None else ""
    if len(value) == 0:
        return query
    field = re.compile(r'^%|%$').sub('', key)
    for token in '#%?*_':
        value = value.replace(token, f"#{token}")
    value = key.replace(field, value)
    attr = get_attr_by_field_name(entities, extra_fields, field, key)
    if attr is None:
        return query
    return query.filter(attr.ilike(value, escape='#'))


def add_list_filter(entities, extra_fields, query, key, value):
    attr = get_attr_by_field_name(entities, extra_fields, key, key)
    if attr is None:
        return query
    return query.filter(attr.in_(value))


def add_value_filter(entities, extra_fields, query, key, value, param):
    attr = get_attr_by_field_name(entities, extra_fields, key, key)
    if attr is None:
        param[key] = value
    else:
        query = query.filter(attr == value)
    return query


def get_field(entities, key: str, extra_fields: dict = None):
    if '.' in key:
        index = key.index('.')
        type_name = key[0:index]
        attr_name = key[index + 1:]
        for entity in entities:
            if entity.__name__ == type_name:
                return getattr(entity, attr_name, None)
        return None
    for entity in entities:
        field = getattr(entity, key, None)
        if field is not None:
            return field
    if not extra_fields:
        return None
    reg = re.compile(r"^%|%$")
    fields = dict([(reg.sub('', k), v) for k, v in extra_fields.items()])
    return fields.get(key)


class SessionWrapper:
    session_maker: sessionmaker
    initial: bool
    session: Session

    def __init__(self, session_maker: sessionmaker):
        self.session_maker = session_maker

    def __enter__(self) -> Session:
        self.session = getattr(local, SESSION, None)
        self.initial = self.session is None
        if self.initial:
            self.session = self.session_maker(expire_on_commit=False)
            setattr(local, SESSION, self.session)
        return self.session

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            self.clean_local_data()
            self.rollback()
            raise
        try:
            self.commit()
        except exc.SQLAlchemyError:
            self.rollback()
            raise
        finally:
            self.clean_local_data()

    def clean_local_data(self):
        if self.initial:
            delattr(local, SESSION)

    def rollback(self):
        if self.initial:
            self.session.rollback()

    def commit(self):
        if self.initial:
            self.session.commit()


@event.listens_for(Engine, 'handle_error')
def receive_handle_error(exception_context):
    """ 截获特定的数据库异常，进行重试 """
    retry_errors = (errors.ConnectionFailure, errors.SqlclientUnableToEstablishSqlconnection,
                    errors.ConnectionDoesNotExist, errors.SqlserverRejectedEstablishmentOfSqlconnection,
                    errors.TransactionResolutionUnknown, errors.ProtocolViolation,
                    errors.OperatorIntervention, errors.QueryCanceled, errors.AdminShutdown,
                    errors.CrashShutdown, errors.CannotConnectNow, errors.DatabaseDropped)
    if type(exception_context.original_exception) == OperationalError \
            and exception_context.original_exception.pgcode is None:  # 处理连接尚未建立而发起的数据库操作报的异常
        log.error(f'Catch exception that need to retry, error_info:{str(exception_context.original_exception)}.')
        raise DBRetryException(f'Database is not ready, need to retry')
    if isinstance(exception_context.original_exception, retry_errors):  # 处理连接建立后数据库异常而报的异常
        log.error(f'Catch exception that need to retry:{str(exception_context.original_exception)}, '
                  f'pgcode:{exception_context.original_exception.pgcode}.')
        raise DBRetryException(f'Database is abnormal, need to retry')
