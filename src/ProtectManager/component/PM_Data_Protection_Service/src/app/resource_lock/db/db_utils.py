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
import os

from pydantic import PostgresDsn

from app.common import logger
from sqlalchemy import create_engine
from sqlalchemy_utils import database_exists, create_database

from app.resource_lock.db.db_tables import metadata
from app.resource_lock.common.consts import GaussdbCertConfig
from app.common.config import settings

log = logger.get_logger(__name__)

db_encoding = os.getenv('DB_ENCODING', 'utf8')


def get_db_engine(bring_up_db=True):
    engine = create_engine(build_engine_url('protect_manager'),
                           max_overflow=settings.DATABASE_MAX_OVERFLOW,
                           pool_size=settings.DATABASE_POOL_SIZE,
                           pool_timeout=settings.DATABASE_POOL_TIMEOUT,
                           pool_recycle=settings.DATABASE_POOL_RECYCLE,
                           pool_pre_ping=settings.DATABASE_POOL_PRE_PING,
                           isolation_level=GaussdbCertConfig.AUTOCOMMIT,
                           connect_args=GaussdbCertConfig.CONN_ARGS
                           )

    if bring_up_db:
        setup_db_and_tables_if_needed(engine)

    return engine


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


def setup_db_and_tables_if_needed(engine):
    metadata.bind = engine

    if not database_exists(engine.url):
        create_database(engine.url, encoding=db_encoding)
        log.info('Database(RLM) is created')
    else:
        log.info('Database(RLM) already exists')

    metadata.create_all()
