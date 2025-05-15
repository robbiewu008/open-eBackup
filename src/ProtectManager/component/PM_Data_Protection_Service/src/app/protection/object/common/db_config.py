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
from typing import Generator

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import QueuePool

from app.common.config import settings
from app.common.constants.constant import GaussdbCertConfig
from app.common.database import build_engine_url

DB_NAME = 'protect_manager'

engine = create_engine(build_engine_url(database=DB_NAME),
                       poolclass=QueuePool,
                       max_overflow=30,
                       pool_size=10,
                       pool_timeout=30,
                       pool_recycle=7200,
                       pool_pre_ping=True,
                       isolation_level=GaussdbCertConfig.AUTOCOMMIT,
                       connect_args=GaussdbCertConfig.CONN_ARGS
                       )

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)


def get_engine():
    return engine


@contextmanager
def session_wrapper(session):
    try:
        yield session
        session.commit()
    except Exception as e:
        session.rollback()
        raise e
    finally:
        session.close()


def get_db_session() -> Generator:
    session = SessionLocal()
    try:
        yield session
        session.commit()
    except Exception as e:
        session.rollback()
        raise e
    finally:
        session.close()


def get_session():
    session = SessionLocal()
    return session_wrapper(session)
