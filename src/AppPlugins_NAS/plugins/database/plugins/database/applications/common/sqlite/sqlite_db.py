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

import sqlite3


class SqliteDB:

    def __init__(self):
        self._db_connect = None

    def connect(self, db_name):
        """
        功能描述: 创建一个连接
        @param db_name: 数据库文件的绝对路径
        @return None
        @note: db_name不存在时会创建, 存在时会打开
        """
        self._db_connect = sqlite3.connect(db_name)

    def execute_sql(self, sql_str):
        """
        功能描述: 执行sql语句
        @param sql_str: 要执行的sql语句
        @return None
        @example1: 
        创建一个表
        sql_str:CREATE TABLE COMPANY(ID INT, NAME TEXT);
        @example2: 
        插入一条数据
        sql_str:INSERT INTO COMPANY (ID,NAME) VALUES (1, 'Paul');
        @example3:
        ... 
        """
        if self._db_connect:
            self._db_connect.execute(sql_str)
            try:
                self._db_connect.commit()
            except Exception:
                self._db_connect.rollback()
                raise

    def close_connect(self):
        """
        功能描述: 关闭连接
        """
        if self._db_connect:
            try:
                self._db_connect.commit()
            except Exception:
                self._db_connect.rollback()
                raise
            finally:
                self._db_connect.close()