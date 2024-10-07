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
import sqlite3

from oceanbase.common.const import SqliteServiceName
from oceanbase.common.oceanbase_common import remove_dir
from oceanbase.logger import log


class OceanBaseSqliteService(object):
    @staticmethod
    def write_sqlite(path, tenant_name_list, database_tuple, table_tuple):
        log.info("start to init sqlite for copy display")
        sqlite_path = os.path.join(path, SqliteServiceName.SQLITE_DIR.value)
        if not os.path.exists(sqlite_path):
            log.info("Start make copy metadata sqlite file ...")
            os.makedirs(sqlite_path)
        sqlite_file = os.path.join(sqlite_path, SqliteServiceName.SQLITE_DATABASE_NAME.value)
        if os.path.exists(sqlite_file):
            remove_dir(sqlite_file)
            log.info("succeeded to remove old sqlite file")
        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()
        log.info("connect sqlite successfully")
        sql_str_create_table = OceanBaseSqliteService.create_table_struct()
        try:
            cursor.execute(sql_str_create_table)
            log.info("sql_str_create_table success")
        except Exception:
            log.error("fail to create sqlite table")
        log.info("start to insert data into sqlite")
        try:
            sql_str_list_tenant = OceanBaseSqliteService.oceanbase_tenant_name_to_sql_list(tenant_name_list)
            for sql_str in sql_str_list_tenant:
                cursor.execute(sql_str)
            sql_str_list_database = OceanBaseSqliteService.oceanbase_database_tuple_to_sql_list(database_tuple)
            for sql_str in sql_str_list_database:
                cursor.execute(sql_str)
            sql_str_list_table = OceanBaseSqliteService.oceanbase_table_tuple_to_sql_list(table_tuple)
            for sql_str in sql_str_list_table:
                cursor.execute(sql_str)
            log.info("succeed to insert data into sqlite")
        except Exception as err:
            log.error(f"fail to insert data into sqlite {err}")
        conn.commit()
        conn.close()

    @staticmethod
    def create_table_struct():
        """
        TABLE NAME = COPY_META_TABLE
        UUID 格式：TENANT_ID-DATABASE_NAME-TABLE_NAME   (用*填充，如果不涉及)
        NAME ：租户名、数据库名、数据表名
        TYPE ： 三种类型tenant/database/table
        PARENT_PATH： 上一级，tanent_name-*-*则为 "/"
        PARENT_UUID: 当前UUID的上一层设置为*，若为tenant级则设置为"null"
        其余六个字段均设置为0
        :return: CREATE SQLITE SQL STR
        """
        return f'''create table IF NOT EXISTS '{SqliteServiceName.SQLITE_TABLE_NAME.value}'(
                UUID varchar(512) PRIMARY KEY NOT NULL, 
                NAME TEXT,  
                TYPE varchar(32), 
                PARENT_PATH TEXT, 
                PARENT_UUID varchar(512),
                SIZE BIGINT,
                CREATE_TIME varchar(64), 
                MODIFY_TIME varchar(64),
                EXTEND_INFO TEXT,
                RES_TYPE varchar (64),
                RES_SUB_TYPE varchar (64));'''

    @staticmethod
    def oceanbase_table_tuple_to_sql_list(table_tuple):
        # example_tuple: ('tenant1','database1','table1'),('tenant2','database2','table1'))
        sql_str_list = []
        for table in table_tuple:
            sql_str = f'INSERT INTO {SqliteServiceName.SQLITE_TABLE_NAME.value} ' \
                      f'VALUES("{table[0]}-{table[1]}-{table[2]}", ' \
                      f'"{table[2]}", "{SqliteServiceName.TABLE_LEVEL.value}", '\
                      f'"/{table[0]}/{table[1]}", "{table[0]}-{table[1]}-*",NULL,NULL,NULL,NULL,NULL,NULL)'
            sql_str_list.append(sql_str)
        return sql_str_list

    @staticmethod
    def oceanbase_database_tuple_to_sql_list(database_tuple):
        sql_str_list = []
        for database in database_tuple:
            sql_str = f'INSERT INTO {SqliteServiceName.SQLITE_TABLE_NAME.value} ' \
                      f'VALUES("{database[0]}-{database[1]}-*", ' \
                      f'"{database[1]}", "{SqliteServiceName.DATABASE_LEVEL.value}", ' \
                      f'"/{database[0]}", "{database[0]}-*-*",NULL,NULL,NULL,NULL,NULL,NULL)'
            sql_str_list.append(sql_str)
        return sql_str_list

    @staticmethod
    def oceanbase_tenant_name_to_sql_list(tenant_name_list):
        sql_str_list = []
        for tenant_name in tenant_name_list:
            sql_str = f'INSERT INTO {SqliteServiceName.SQLITE_TABLE_NAME.value} ' \
                      f'VALUES("{tenant_name}-*-*",' \
                      f'"{tenant_name}","{SqliteServiceName.TENANT_LEVEL.value}", ' \
                      f'"/",NULL,NULL,NULL,NULL,NULL,NULL,NULL)'
            sql_str_list.append(sql_str)
        return sql_str_list
