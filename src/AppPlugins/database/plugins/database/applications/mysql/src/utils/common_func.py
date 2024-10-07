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

import configparser
import os
import random
import re
import uuid
from configparser import ConfigParser

import pymysql

from common.common import execute_cmd_list_communicate, retry_when_exception
from common.exception.common_exception import ErrCodeException
from mysql import log
from mysql.src.common.constant import ExecCmdResult, MySQLType, MysqlExecSqlError
from mysql.src.common.error_code import MySQLErrorCode

SYSTEM_DATABASE = ["information_schema", "mysql", "performance_schema", "sys"]


class SQLParam(object):
    def __init__(self, host: str, port: str | int, user: str, passwd: str, sql="", charset="utf8mb4"):
        self.host = host
        self.port = port
        self.user = user
        self.passwd = passwd
        self.sql = sql
        self.charset = charset


def exec_mysql_sql_cmd(sql_param: SQLParam, ignore_sql_error=True):
    try:
        mysql_connection = pymysql.connect(user=sql_param.user, host=sql_param.host, password=sql_param.passwd,
                                           charset=sql_param.charset, port=int(sql_param.port))
    except pymysql.MySQLError as error:
        log.error(f"Connect MySQL service failed!,{error}")
        if MysqlExecSqlError.ERROR_ACCESS_DENINED in str(error):
            raise ErrCodeException(MySQLErrorCode.CHECK_AUTHENTICATION_INFO_FAILED, message=str(error)) from error
        raise ErrCodeException(MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING, message=str(error)) from error
    try:
        cursor = mysql_connection.cursor()
        cursor.execute(sql_param.sql)
        results = cursor.fetchall()
    except Exception as except_str:
        log.error(f"execute sql failed!,execute_sql:{sql_param.sql},{except_str}")
        if ignore_sql_error:
            return False, str(except_str)
        raise except_str
    finally:
        mysql_connection.close()
    return True, results


def validate_my_cnf(my_cnf_path):
    if not os.path.exists(my_cnf_path):
        return False
    if os.path.isdir(my_cnf_path):
        return False
    mysql_parser = ConfigParser(allow_no_value=True, strict=False)
    try:
        mysql_parser.read(my_cnf_path)
    except configparser.Error:
        return False
    return mysql_parser.has_section("mysqld")


def find_default_my_cnf_path():
    def parse_file():
        for line in output.splitlines(False):
            if "my.cnf" not in line:
                continue
            for i in line.split(" "):
                if os.access(i, os.F_OK):
                    return True, i
        return False, ""

    # 命令硬编码，使用subprocess.getstatusoutput接口无安全风险
    ret, output, _ = execute_cmd_list_communicate(["mysql --help", "grep -A 1 'Default options'", "grep my.cnf"])
    if ret == ExecCmdResult.SUCCESS:
        # 过滤一些警告信息
        result, file_path = parse_file()
        if result:
            return True, file_path
    if validate_my_cnf("/etc/my.cnf"):
        return True, "/etc/my.cnf"
    return False, ""


def find_log_bin_path_dir(sql_param: SQLParam, my_cnf_path):
    sql_param.sql = "show variables like 'log_bin_index'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if ret and output:
        log.info("query log_bin_index success")
        log_bin_index_path = output[0][1].strip()
        return os.path.dirname(log_bin_index_path)
    return find_log_bin_path_dir_from_cnf(my_cnf_path)


def find_log_bin_path_dir_from_cnf(my_cnf_path: str) -> str:
    bin_log_path = get_conf_by_key(my_cnf_path, "log_bin")
    log.info(f"bin_log_path:{bin_log_path}")
    if not bin_log_path:
        return get_conf_by_key(my_cnf_path, "datadir")
    if os.path.isabs(bin_log_path):
        if os.path.isdir(bin_log_path):
            return str(bin_log_path)
        return str(os.path.dirname(bin_log_path))
    datadir = get_conf_by_key(my_cnf_path, "datadir")
    log.info(f"datadir:{bin_log_path}")
    log_bin_dir = os.path.abspath(os.path.join(datadir, bin_log_path))
    if os.path.isdir(log_bin_dir):
        return str(log_bin_dir)
    return str(os.path.dirname(log_bin_dir))


def ignore_line_optionxform(option):
    # 忽略中划线，一律用下划线取值
    return option.replace('-', '_')


def get_conf_by_key(my_cnf_path, key: str, section: str = "mysqld") -> str:
    if os.path.exists(my_cnf_path):
        conf_path = my_cnf_path
    else:
        conf_path = find_default_my_cnf_path()
    mysql_parser = configparser.ConfigParser(allow_no_value=True, strict=False, delimiters="=",
                                             comment_prefixes="#", inline_comment_prefixes="#")
    mysql_parser.optionxform = ignore_line_optionxform
    try:
        mysql_parser.read(conf_path)
    except configparser.Error as err:
        log.error(f"parse config file error,config_path:{conf_path},error:{err}")
        return ""
    if not mysql_parser.has_section(section):
        log.error(f"config file:{conf_path} has no section:{section}")
        return ""
    if mysql_parser.has_option(section, key):
        return mysql_parser.get(section, key).strip()
    log.warn(f"config file:{conf_path} has no key:{key}")
    return ""


def get_version_from_sql(sql_param: SQLParam):
    sql_param.sql = "show variables like 'version'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return ""
    return output[0][1]


def get_data_dir_from_sql(sql_param: SQLParam):
    sql_param.sql = "show variables like 'datadir'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return ""
    return output[0][1]


def get_binlog_filenames(sql_param: SQLParam):
    sql_param.sql = "show binary logs"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret or not output:
        return []
    binlog_filenames = []
    for line in output:
        log_name = line[0]
        file_size = line[1]
        if int(file_size) > 0:
            binlog_filenames.append(log_name)
    return binlog_filenames


def get_data_dir_from_conf(my_cnf_path: str):
    return get_conf_by_key(my_cnf_path, "datadir")


def get_slave_info(sql_param: SQLParam):
    sql_param.sql = "show slave status"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return []
    if not output:
        return []
    info_line_array = []
    for line in output:
        info_line_array.append(line)
    return info_line_array


@retry_when_exception(retry_times=5, delay=random.randint(15, 30), logger=log)
def reset_slave_all(sql_param: SQLParam):
    sql_param.sql = "reset slave all"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if ret:
        return True
    raise Exception(f"reset slave all failed,{output}")


def get_master_ips(sql_param: SQLParam):
    result = get_slave_info(sql_param)
    master_ips = set()
    for info in result:
        master_ips.add(info[1])
    return master_ips


def get_master_info(sql_param: SQLParam):
    result = get_slave_info(sql_param)
    if not result:
        return []
    master_info_lst = []
    for master_info in result:
        # 1 3 两行分别是主机名和端口,如果能查询到slave信息，则有这两行
        master_host = str(master_info[1])
        master_port = str(master_info[3])
        master_info_lst.append({"host": master_host, "port": master_port})
    log.info(f"master_info_lst:{master_info_lst}")
    return master_info_lst


def check_cluster_sync_status(sql_param: SQLParam):
    result = get_slave_info(sql_param)
    for info in result:
        # 第35行，记录的是SLAVE IO ERROR列
        if info[35].strip():
            log.error("Data sync error. error:%s", info[35])
            return False, info[35]
    return True, ""


def get_log_bin_from_sql(sql_param: SQLParam):
    sql_param.sql = "show variables like 'log_bin'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        log.error("execute show variables like 'log_bin' error")
        return ""
    return output[0][1]


def get_databases_from_sql(sql_param: SQLParam, instance_id: str, instance_name: str):
    sql_param.sql = "show databases"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return []
    database_list = []
    for database_info in output:
        database = database_info[0]
        if database in SYSTEM_DATABASE:
            continue
        if database.__contains__('#mysql50#'):
            continue
        database_uuid = str(uuid.uuid5(uuid.NAMESPACE_X500, instance_id + MySQLType.SUBTYPE
                                       + instance_name + database))
        resource = {
            "id": database_uuid,
            "name": database,
            "parentId": instance_id,
            "parentName": instance_name
        }
        database_list.append(resource)
    return database_list


def get_privilege_from_sql(sql_param: SQLParam, host):
    sql_param.sql = f"show grants for '{sql_param.user}'@'{host}';"
    ret, privilege_out = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return []
    privilege_set = set()
    for privilege_str in privilege_out:
        match = re.search(r"GRANT\s+(.*?)\s+ON", privilege_str[0])
        if not match:
            continue
        privilege_array = match.group(1).split(",")
        for privilege in privilege_array:
            privilege_set.add(privilege.strip())
    return list(privilege_set)


def get_pxc_info(sql_param: SQLParam):
    sql_param.sql = "show status like 'wsrep_incoming_addresses'"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret or not output:
        return []
    pxc_info = list()
    for node in output[0][1].split(","):
        node_info = node.split(":")
        pxc_info.append({"ip": node_info[0], "port": node_info[1]})
    return pxc_info


def contains_all_elements(arr_total, arr_sub):
    for item in arr_sub:
        if item not in arr_total:
            return False
    return True


def contains_any_elements(arr_total, arr_sub):
    for item in arr_sub:
        if item in arr_total:
            return True
    return False


def build_body_param(code: int, body_error: int, message: str):
    params = {
        "code": code,
        "bodyErr": body_error,
        "message": message
    }
    return params


def get_value(param, key, default_value=""):
    return_value = param.get(key)
    if return_value:
        return return_value
    return default_value
