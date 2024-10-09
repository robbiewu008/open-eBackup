#
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
#

import contextlib
import threading

from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine, exc


class SessionManager:
    def __init__(self, url):
        self._engine = None
        self._db_session = None
        self.create_engine(url)

    def create_engine(self, url):
        '''
            功能描述： 创建一个数据库库连接
            参数
            @url: 数据库链接的url、
            Example: in windows with sqlite3: 'sqlite:///D:/PRO_DME//test.db'
                    将在指定的路径下创建一个test.db sqlite数据库文件
        '''
        self._engine = create_engine(url, echo=False)
        self._db_session = sessionmaker(autoflush=False, expire_on_commit=False)
        self._db_session.configure(bind=self._engine)

    @contextlib.contextmanager
    def session_scope(self):
        """
            功能描述： 打开一个数据库会话、进行查询插入等操作
        """
        session = self._db_session()
        yield session
        try:
            session.commit()
        except exc.SQLAlchemyError as e:
            session.rollback()
            raise e

        finally:
            session.close()

    def create_table(self, base_model):
        """
        函数功能：创建继承 base_model 的数据表
        参数：@base_model：继承数据库表的model
        @note： 参见数据库测试用例
        """
        base_model.metadata.create_all(self._engine)


db = {}
db_lock = threading.Lock()


def get_db_instance(url):
    db_lock.acquire()
    if url not in db.keys():
        db[url] = SessionManager(url)
    db_lock.release()
    return db.get(url)
