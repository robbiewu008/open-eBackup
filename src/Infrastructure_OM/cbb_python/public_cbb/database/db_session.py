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
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import threading
from contextlib import contextmanager
from urllib.parse import quote_plus as urlquote
from sqlalchemy import create_engine, exc
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import QueuePool

from public_cbb.exception.error_codes import GeneralErrorCodes
from public_cbb.exception.custom_exception import CustomException
from public_cbb.security.pwd_manage.pwd_manage import clear
from public_cbb.security.pwd_manage.cert_manage import CertManage
from public_cbb.log.logger import get_logger
from public_cbb.config.config_map import get_general_user
from public_cbb.config.global_config import get_settings
from public_cbb.security.anonym_utils.anonymity import Anonymity

log = get_logger()
settings = get_settings()

DATABASE_POOL_PARAMS = {
    'max_overflow': settings.DATABASE_MAX_OVERFLOW,
    'pool_size': settings.DATABASE_POOL_SIZE,
    'pool_timeout': settings.DATABASE_POOL_TIMEOUT,
    'pool_recycle': settings.DATABASE_POOL_RECYCLE,
    'pool_pre_ping': settings.DATABASE_POOL_PRE_PING
}

DATABASE_SSL_PARAMS = {
    'sslmode': 'verify-ca',
    'sslcert': settings.INTERNAL_CERT_DIR,
    'sslkey': settings.INTERNAL_KEY_DIR,
    'sslrootcert': settings.INTERNAL_CA_DIR
}


class DBInfo:
    encoding = "utf8"
    echo_sql = False
    db_name = ''
    host = ''
    port = ''
    user_name = ''
    user_pwd = ''


def build_engine_url(db_info: DBInfo, init=False):
    db_username, db_userpwd = get_general_user(init)
    name = db_info.db_name
    gaussdb_host = f"{settings.INFRA_DB_HOST}:{settings.INFRA_DB_PORT}"
    gaussdb_timeout = f"keepalives_idle={settings.KEEPALIVES_IDLE}&keepalives_interval={settings.KEEPALIVES_INTERVAL}" \
                      f"&keepalives_count={settings.KEEPALIVES_COUNT}&tcp_user_timeout={settings.TCP_USER_TIMEOUT}"
    url = f"postgresql://{db_username}:%s@{gaussdb_host}/{name}?{gaussdb_timeout}" % urlquote(db_userpwd)
    clear(db_userpwd)
    return url


class SessionManager:
    def __init__(self, db_info: DBInfo, verify=settings.OPEN_DATABASE_VERIFY):
        self.engine = None
        self.setup = 'STARTING'
        self.session = sessionmaker(autoflush=False, expire_on_commit=False)
        self.db_info = db_info
        self.verify = verify
        log.info(f'db_encoding:{db_info.encoding}, echo_sql:{db_info.echo_sql}')
        try:
            self.engine = self.create_db_engine(self.verify, init=True)
            self.session.configure(bind=self.engine)
        except exc.SQLAlchemyError as e:
            log.error(
                f'--- After self.setup={self.setup} in {self.db_info.db_name}, '
                f'will rollback after exception e:{Anonymity.process(e.__str__())}'
            )
            raise CustomException(GeneralErrorCodes.ERR_INTERNAL_ERROR) from e
        else:
            self.setup = 'ENGINE_CREATED'

    def create_db_engine(self, verify, init=False):
        try:
            db_url = build_engine_url(db_info=self.db_info, init=init)
        except Exception as e:
            raise exc.SQLAlchemyError(e)

        if verify:
            cnf_file = settings.INTERNAL_CNF_DIR
            engine = create_engine(
                db_url, echo=bool(self.db_info.echo_sql), poolclass=QueuePool,
                connect_args={**DATABASE_SSL_PARAMS,
                              'sslpassword': CertManage.get_cert_pwd(cnf_file)},
                **DATABASE_POOL_PARAMS)
        else:
            engine = create_engine(
                db_url, echo=bool(self.db_info.echo_sql), poolclass=QueuePool,
                **DATABASE_POOL_PARAMS)

        return engine

    def create_session(self):
        try:
            self.engine = self.create_db_engine(self.verify)
            self.session.configure(bind=self.engine)
            self.setup = 'ENGINE_CREATED'
            return True
        except exc.SQLAlchemyError as e:
            log.error(
                f'--- After self.setup={self.setup} in {self.db_info.db_name}, '
                f'will rollback after exception e:{Anonymity.process(e.__str__())}'
            )
            return False

    @contextmanager
    def session_scope(self):
        """Provide a transactional scope around a series of operations."""
        session = self.session()
        try:
            yield session
            session.commit()
        except exc.SQLAlchemyError as e:
            log.error(f'--- Will rollback in db_name:{self.db_info.db_name} '
                      f'after exception e:{Anonymity.process(e.__str__())}')
            session.rollback()
            if 'Invalid username/password,login denied.' in str(e):
                log.info('Login failed, try to create new session.')
                if self.create_session():
                    return
            raise CustomException(GeneralErrorCodes.ERR_INTERNAL_ERROR) from e
        finally:
            session.close()


db = {}
db_lock = threading.Lock()


def get_db_instance(db_name):
    db_lock.acquire()
    m_key = db_name
    if m_key not in db:
        log.info(f'Creating database connection to m_key:{m_key}...')
        db_info = DBInfo()
        db_info.db_name = db_name
        session_mgr = SessionManager(db_info)
        db[m_key] = session_mgr
    db_lock.release()
    return db[m_key]
