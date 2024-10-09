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

from common.common import convert_timestamp_to_time
from tidb.logger import log

from tidb.schemas.tidb_schemas import SqliteTable
from tidb.common.const import SqliteServiceField


class TidbSqliteService(object):
    @staticmethod
    def write_metadata_to_sqlite_file(path, obj_name, obj_type, parent_path, data_size):
        # sqlite 保存备份数据表/数据库的元数据到指定的自定义存储路径(父路径)
        copy_metadata_sqlite_path = os.path.join(path, "sqlite")
        if not os.path.exists(copy_metadata_sqlite_path):
            log.info("Start make copy metadata sqlite file ...")
            os.makedirs(copy_metadata_sqlite_path)
        # 创建sqlite数据库，并添加副本的数据库和数据表元数据信息
        TidbSqliteService.init_db(copy_metadata_sqlite_path, obj_name, obj_type, parent_path, data_size)

    # 将表数据写入数据库
    @staticmethod
    def init_db(path, obj_name, obj_type, parent_path, data_size):
        sqlite_db_name = SqliteServiceField.SQLITE_DATABASE_NAME.value
        db_path = os.path.join(path, sqlite_db_name)
        conn = sqlite3.connect(db_path)

        # 创建sqlite数据表T_COPY_METADATA存储副本下的数据库、数据表信息
        t_copy_metadata = SqliteServiceField.SQLITE_DATATABLE_NAME.value
        create_sql_table = TidbSqliteService.create_table_struct(t_copy_metadata)
        conn.execute(create_sql_table)
        conn.commit()

        # 数据库信息是否已经存在，用NAME+TYPE+PARENT_PATH字段过滤，不存在则插入数据库结构信息
        statement = f'INSERT INTO {t_copy_metadata} VALUES(?,?,?,?,?,?,?,?,?,?,?)'
        if obj_name is not None:
            is_exist_obj = TidbSqliteService.is_exist_sql(conn, t_copy_metadata, obj_name, obj_type, parent_path)
            if not is_exist_obj:
                database_data = TidbSqliteService.init_sql_params(conn, obj_name, obj_type, parent_path, data_size)
                conn.executemany(statement, database_data)
                conn.commit()
        conn.close()

    @staticmethod
    def is_exist_sql(conn, table_name, obj_name, obj_type, parent_path):
        exist_statement = \
            f"SELECT count(*) from '{table_name}' where NAME = '{obj_name}'and TYPE = '{obj_type}' \
                and PARENT_PATH = '{parent_path}'"
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
                SIZE TEXT,
                CREATE_TIME varchar(64), -- 创建时间
                MODIFY_TIME varchar(64), -- 修改时间时间
                EXTEND_INFO TEXT,
                RES_TYPE varchar (64),
                RES_SUB_TYPE varchar (64));'''

    @staticmethod
    def get_parent_guid(conn, obj_type, parent_path):
        t_copy_metadata = SqliteServiceField.SQLITE_DATATABLE_NAME.value
        if obj_type == SqliteServiceField.TYPE_DATABASE.value:
            get_statement = f"SELECT UUID from '{t_copy_metadata}' WHERE PARENT_PATH = '/' LIMIT 1;"
        elif obj_type == SqliteServiceField.TYPE_TABLE.value:
            parent_paths = [item for item in parent_path.split('/') if item]
            db_name = parent_paths[1]
            cluster_name = '/' + parent_paths[0]
            get_statement = \
                f"SELECT UUID from '{t_copy_metadata}' WHERE NAME = '{db_name}' AND " \
                f"PARENT_PATH = '{cluster_name}' AND TYPE = '{SqliteServiceField.TYPE_DATABASE.value}' LIMIT 1;"
        else:
            return ""
        get_cursor = conn.execute(get_statement)
        conn.commit()
        fit_items = get_cursor.fetchall()
        if len(fit_items) != 0 and fit_items[0][0] != 0:
            return fit_items[0][0]
        else:
            return ""

    @staticmethod
    def init_sql_params(conn, obj_name, obj_type, parent_path, data_size):
        sqlite_table_params = SqliteTable()
        create_time = convert_timestamp_to_time(time.time())
        modify_time = convert_timestamp_to_time(time.time())
        parent_guid = TidbSqliteService.get_parent_guid(conn, obj_type, parent_path)
        parent_guid = parent_guid if parent_guid else sqlite_table_params.parent_uuid
        return [(str(uuid.uuid1()), obj_name, obj_type, parent_path, parent_guid, data_size, create_time, modify_time,
                 sqlite_table_params.extend_info, sqlite_table_params.res_type, sqlite_table_params.res_sub_type)]
