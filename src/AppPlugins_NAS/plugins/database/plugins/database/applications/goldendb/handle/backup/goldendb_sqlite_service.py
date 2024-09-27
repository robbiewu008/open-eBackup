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

import os
import uuid
import time
import sqlite3

from goldendb.logger import log
from common.common import convert_timestamp_to_time
from common.util.exec_utils import exec_mkdir_cmd

from goldendb.schemas.glodendb_schemas import SqliteTable
from goldendb.handle.common.const import SqliteServiceField


class GoldenDBSqliteService(object):
    @staticmethod
    def write_metadata_to_sqlite_file(path, db_name):
        log.info('insert into sqlite')
        # sqlite 保存备份数据表/数据库的元数据到指定的自定义存储路径(父路径)
        copy_metadata_sqlite_path = os.path.join(path, "sqlite")
        if not os.path.exists(copy_metadata_sqlite_path):
            log.info("Start make copy metadata sqlite file ...")
            exec_mkdir_cmd(copy_metadata_sqlite_path)
        # 创建sqlite数据库，并添加副本的数据库和数据表元数据信息
        GoldenDBSqliteService.init_db(copy_metadata_sqlite_path, db_name)

    # 将表数据写入数据库
    @staticmethod
    def init_db(path, db_name):
        sqlite_db_name = SqliteServiceField.SQLITE_DATABASE_NAME.value
        db_path = os.path.join(path, sqlite_db_name)
        conn = sqlite3.connect(db_path)

        # 创建sqlite数据表T_COPY_METADATA存储副本下的数据库、数据表信息
        t_copy_metadata = SqliteServiceField.SQLITE_DATATABLE_NAME.value
        create_sql_table = GoldenDBSqliteService.create_table_struct(t_copy_metadata)
        conn.execute(create_sql_table)
        conn.commit()

        # 数据库信息是否已经存在，用NAME+TYPE字段过滤，不存在则插入数据库结构信息
        statement = f'INSERT INTO {t_copy_metadata} VALUES(?,?,?,?,?,?,?,?,?,?,?)'
        if db_name is not None:
            is_exist_database = GoldenDBSqliteService. \
                is_exist_sql(conn, t_copy_metadata, db_name, SqliteServiceField.TYPE_DIR.value, "/")
            if not is_exist_database:
                database_data = GoldenDBSqliteService.init_sql_params(db_name, SqliteServiceField.TYPE_DIR.value, "/")
                conn.executemany(statement, database_data)
                conn.commit()
        conn.close()

    @staticmethod
    def is_exist_sql(conn, table_name, field_name, field_type, field_path):
        exist_statement = \
            f"SELECT count(*) from '{table_name}' where NAME = '{field_name}'and TYPE = '{field_type}' \
                and PARENT_PATH = '{field_path}'"
        exist_cursor = conn.execute(exist_statement)
        conn.commit()
        exist_database_count = exist_cursor.fetchall()
        # 返回字段是否已经存在数据表中
        return len(exist_database_count) != 0 and exist_database_count[0][0] != 0

    @staticmethod
    def create_table_struct(t_copy_metadata):
        return f'''create table IF NOT EXISTS '{t_copy_metadata}'(
                UUID varchar(512) PRIMARY KEY NOT NULL, -- 唯一标识符，UUID格式
                NAME TEXT,  -- 数据库名、数据表明
                TYPE varchar(32), -- 类型：文件夹-d、文件-f
                PARENT_PATH TEXT,
                PARENT_UUID varchar(512),
                SIZE BIGINT,
                CREATE_TIME varchar(64), -- 创建时间
                MODIFY_TIME varchar(64), -- 修改时间时间
                EXTEND_INFO TEXT,
                RES_TYPE varchar (64),
                RES_SUB_TYPE varchar (64));'''

    @staticmethod
    def init_sql_params(file_name, file_type, parent_path):
        sqlite_table_params = SqliteTable()
        create_time = convert_timestamp_to_time(time.time())
        modify_time = convert_timestamp_to_time(time.time())
        return [(str(uuid.uuid1()), file_name, file_type, parent_path, sqlite_table_params.parent_uuid,
                 sqlite_table_params.size, create_time, modify_time, sqlite_table_params.extend_info,
                 sqlite_table_params.res_type, sqlite_table_params.res_sub_type)]
