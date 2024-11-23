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
import pwd
import time

from common.common import write_content_to_file
from mysql import log
from mysql.src.common.constant import SystemServiceType, MySQLStrConstant, ExecCmdResult
from mysql.src.utils.common_func import SQLParam, exec_sql_without_binlog, get_version_from_sql, exec_mysql_sql_cmd
from mysql.src.utils.mysql_utils import MysqlUtils


def drop_database(sql_param: SQLParam, database):
    sql_param.sql = f"drop database if exists `{database}`"
    return exec_sql_without_binlog(sql_param)


def create_database(sql_param: SQLParam, database):
    sql_param.sql = f"create database if not exists `{database}`"
    return exec_sql_without_binlog(sql_param)


def get_tables_from_database(sql_param: SQLParam, database):
    sql_param.sql = f"select table_name from information_schema.tables where table_schema='{database}'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    table_names = []
    for table_name in output:
        table_names.append(table_name[0])
    return table_names


def get_dump_file(copy_path, copy_database):
    for path in os.listdir(copy_path):
        new_path = os.path.join(copy_path, path)
        if copy_database + ".sql" == path:
            return True, new_path
    return False, ""


def generate_discard_sql(sql_param: SQLParam, table_schema: str, outfile_path: str):
    file_content = ""
    table_names = get_tables_from_database(sql_param, table_schema)
    for table_name in table_names:
        file_content += f"alter table `{table_schema}`.`{table_name}` discard tablespace;\n"
    write_content_to_file(outfile_path, file_content)


def generate_import_sql(sql_param: SQLParam, table_schema: str, outfile_path: str):
    file_content = ""
    table_names = get_tables_from_database(sql_param, table_schema)
    for table_name in table_names:
        file_content += f"alter table `{table_schema}`.`{table_name}` import tablespace;\n"
    write_content_to_file(outfile_path, file_content)


def generate_rename_sql(sql_param: SQLParam, from_dt, to_dt, outfile_path: str):
    file_content = ""
    table_names = get_tables_from_database(sql_param, from_dt)
    for table_name in table_names:
        file_content += f"rename table  `{from_dt}`.`{table_name}` to `{to_dt}`.`{table_name}`;\n"
    write_content_to_file(outfile_path, file_content)


def get_data_dir_user(datadir: str):
    stat_info = os.stat(datadir)
    uid = stat_info.st_uid
    user = pwd.getpwuid(uid).pw_name
    log.info(f"Get basedir user name: {user}")
    return user


def set_foreign_key_checks(sql_param: SQLParam, value: str):
    sql_param.sql = f"SET FOREIGN_KEY_CHECKS = {value};"
    try:
        exec_sql_without_binlog(sql_param)
        log.info(f"set foreign key checks {value} success")
        return True
    except Exception as exception_str:
        log.warn(f"set foreign key checks {value} failed, exception_str:{exception_str}")
        return False


def get_pxc_strict_mode(sql_param: SQLParam):
    sql_param.sql = "show variables like 'pxc_strict_mode'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return ""
    return output[0][1]


def set_pxc_strict_mode(sql_param: SQLParam, mode_str):
    sql_param.sql = f"SET GLOBAL pxc_strict_mode = {mode_str}"
    return exec_sql_without_binlog(sql_param)


def restart_mysql(is_pxc_master, service_name, service_type, my_cnf_path, port):
    if is_pxc_master:
        if service_type == SystemServiceType.SYSTEMCTL:
            cmd_str = f"systemctl start {MySQLStrConstant.MYSQLPXCSERVICES}"
        else:
            cmd_str = f"service {MySQLStrConstant.MYSQLPXCSERVICES} start"
    elif service_type == "manual":
        user_name = MysqlUtils.get_data_path_user_name(my_cnf_path)
        MysqlUtils.kill_mysql_process()
        cmd_str = f"mysqld --defaults-file={my_cnf_path} --user={user_name} --port={port} &"
    else:
        if service_type == SystemServiceType.SYSTEMCTL:
            cmd_str = f"systemctl restart {service_name}"
        else:
            cmd_str = f"service {service_name} restart"
    # execute_cmd 在linux red hat 8.7及以上执行命令存在阻塞问题。
    ret = os.system(f"{cmd_str} 2>&1 > /dev/null")
    log.info(f"cmd_str:{cmd_str} end,ret:{ret}")
    if str(ret) != ExecCmdResult.SUCCESS:
        log.error(f"Exec systemctl restart mysql failed.")
        return False
    log.info(f"Start {service_name} success.")
    return True


def wait_mysql_ready(sql_param: SQLParam):
    for _ in range(30):
        version = get_version_from_sql(sql_param)
        if version:
            return True
        time.sleep(10)
    return False
