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

import json
import os
import re
import shlex
import socket
import subprocess
import time

import pymysql

from common.cleaner import clear
from common.common import execute_cmd, check_command_injection
from common.const import ParamConstant
from common.util.backup import backup_files, query_progress
from common.util.exec_utils import exec_cat_cmd
from mysql import log
from mysql.src.common.constant import ExecCmdResult, MySQLStrConstant, DELETING_PATH_WHITE_LIST, \
    DEPLOY_OPERATING_SYSTEMS, MysqlExecSqlError, SystemConstant
from mysql.src.common.constant import FileBackToolRET
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.utils.common_func import exec_cmd_spawn


def exec_sql(exec_sql_parse_param, data_path=''):
    """连上数据库,执行一条sql命令,返回查询结果"""
    passwd = get_passwd(exec_sql_parse_param, data_path)
    if not passwd:
        return False, "get passwd failed"
    try:
        if exec_sql_parse_param.database_name:
            mysql_connection = pymysql.connect(host=exec_sql_parse_param.host_ip,
                                               port=int(exec_sql_parse_param.port),
                                               user=exec_sql_parse_param.user,
                                               password=passwd,
                                               database=exec_sql_parse_param.database_name,
                                               charset=exec_sql_parse_param.charset)
        else:
            mysql_connection = pymysql.connect(host=exec_sql_parse_param.host_ip,
                                               port=int(exec_sql_parse_param.port),
                                               user=exec_sql_parse_param.user,
                                               password=passwd,
                                               charset=exec_sql_parse_param.charset)
    except pymysql.Error as except_str:
        log.exception(except_str)
        log.error(f"Connect MySQL service failed!{except_str}")
        if MysqlExecSqlError.ERROR_ACCESS_DENINED in str(except_str):
            return False, MysqlExecSqlError.ERROR_ACCESS_DENINED
        return False, "raise except"
    finally:
        clear(passwd)
    try:
        cursor = mysql_connection.cursor()
        cursor.execute(exec_sql_parse_param.sql_str)
        results = cursor.fetchall()
    finally:
        mysql_connection.close()
    return True, results


def get_passwd(exec_sql_parse_param, data_path=''):
    if exec_sql_parse_param.passwd:
        return exec_sql_parse_param.passwd
    return safe_get_pwd(exec_sql_parse_param.passwd_env, data_path)


def mysql_get_status_output(cmd, passwd=""):
    log.info("mysql get status output")
    # 针对centos stream 9的场景，新增mysql默认的动态库路径到环境变量中，仅在执行mysql命令时生效
    env = os.environ.copy()
    ret, out = exec_cat_cmd("/etc/centos-release")
    if ret and SystemConstant.CENTOS_STREAM_9 in str(out):
        for var, value in env.items():
            if var == MySQLStrConstant.LD_LIBRARY_PATH:
                env[MySQLStrConstant.LD_LIBRARY_PATH] = MySQLStrConstant.LIB64 + ":" + value
    exitcode = 0
    try:
        data = subprocess.check_output(shlex.split(cmd), encoding="utf-8",
                                       shell=False, text=True, stderr=subprocess.STDOUT, env=env)
    except subprocess.CalledProcessError as ex:
        log.exception(ex)
        data = ex.output
        exitcode = ex.returncode
    except Exception as e:
        log.exception(e)
        data = "cmd error"
        exitcode = 127
    if data[-1:] == '\n':
        data = data[:-1]
    return exitcode, data


def exec_cmd_nowait(cmd):
    """异步执行shell命令"""
    # 针对centos stream 9的场景，新增mysql默认的动态库路径到环境变量中，仅在执行mysql命令时生效
    log.info("exec cmd no wait.")
    env = os.environ.copy()
    ret, out = exec_cat_cmd("/etc/centos-release")
    if ret and SystemConstant.CENTOS_STREAM_9 in str(out):
        for var, value in env.items():
            if var == MySQLStrConstant.LD_LIBRARY_PATH:
                env[MySQLStrConstant.LD_LIBRARY_PATH] = MySQLStrConstant.LIB64 + ":" + value
    process = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, encoding="utf-8", env=env)
    return process


def retry_cmd_list(passwd_env, cmds: [str]):
    try:
        passwd = safe_get_environ(passwd_env)
        for cmd in cmds:
            ret, output = exec_cmd_spawn(cmd, passwd)
            if ret == int(ExecCmdResult.SUCCESS):
                return True, output
        return False, ""
    finally:
        clear(passwd)


def exec_rc_tool_cmd(cmd, in_path, out_path):
    cmd = f"{os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh')} {cmd} {in_path} {out_path}"
    ret, out, err = execute_cmd(cmd)
    if ret != ExecCmdResult.SUCCESS:
        log.error(f"An error occur in execute cmd. ret:{ret} err:{err}")
        return False
    return True


def check_mysql_command_injection(param):
    """
    mysql检测是否含有特殊字符防止命令注入
    :param : shell 执行参数
    :return: bool
    """
    # 特别注意，要匹配反斜杠(\)需要使用(\\\\)
    expression = "[|;&$><`!\'\"\\\\%]"
    if re.search(expression, param):
        return True
    return False


STDIN_VALUE = ""


def safe_get_environ(key):
    global STDIN_VALUE
    if not STDIN_VALUE:
        STDIN_VALUE = input()
    try:
        value_dict = json.loads(STDIN_VALUE)
    except Exception:
        return ""
    value = value_dict.get(key)
    return value if isinstance(value, str) else ""


def safe_get_pwd(key, data_path):
    if not data_path:
        return safe_get_environ(key)
    connect_param_path = os.path.join(data_path, "connect_param.json")
    if not os.path.exists(connect_param_path):
        return safe_get_environ(key)
    try:
        connect_param = ReadFile.read_param_file(connect_param_path)
    except Exception as exception_str:
        log.debug(f"connect param not exists:{exception_str}")
        return safe_get_environ(key)
    return connect_param.get("passwd") if connect_param else ""


def mysql_backup_files(job_id, files, dir_path, write_queue_size=1000):
    interval = 0
    res = backup_files(job_id, files, dir_path, False, write_queue_size)
    if not res:
        log.error(f"Failed to start backup. jobId: {job_id}.")
        return False
    while True:
        interval += 1
        time.sleep(2)
        status, progress, data_size = query_progress(job_id)
        if status == FileBackToolRET.SUCCESS:
            log.info(f"Backup completed, jobId: {job_id}.")
            return True
        if status == FileBackToolRET.FAILED:
            log.error(f"Backup failed, jobId: {job_id}.")
            return False
        if interval % 10 == 0:
            log.info(f"Backing up, progress: {progress}, dataSize: {data_size}, jobId: {job_id}.")


def check_path_in_white_list(path_):
    """
    检测白名单
    :param : shell 执行参数
    :return: bool
    """
    try:
        real_path = os.path.realpath(path_)
    except Exception as e:
        return False

    if check_command_injection(real_path):
        return False

    for path in DELETING_PATH_WHITE_LIST:
        if real_path.find(path) == 0:
            return True
    if f"{real_path}/" in DELETING_PATH_WHITE_LIST:
        return True
    return False


def check_delete_mysql_bin_log_file(path_):
    """
    检查删除的mysql日志文件
    @param path_: 文件
    @return: 返回检查结果
    """
    try:
        real_path = os.path.realpath(path_)
    except Exception as ex:
        return False
    if not os.path.isfile(real_path):
        return False
    return True


def get_operating_system():
    """
    获取操作系统名称
    :param : None
    :return: deployOS
    """
    ret, output = exec_cat_cmd("/proc/version")
    if not ret:
        log.error("Get deploy operating system failed.")
        return DEPLOY_OPERATING_SYSTEMS[0]
    for system_name in DEPLOY_OPERATING_SYSTEMS[1:]:
        if system_name in str(output):
            log.info(f"Get deploy operating system：{system_name}")
            return system_name
    log.error("Get deploy operating system failed.")
    return DEPLOY_OPERATING_SYSTEMS[0]


def get_charset_from_instance(instance_param):
    charset = instance_param.get("extendInfo", {}).get("charset", "")
    if not charset:
        return SystemConstant.DEFAULT_CHARSET
    return charset


def get_config_value_from_instance(instance_param, key: str, default_value=""):
    value = instance_param.get("extendInfo", {}).get(key, "")
    if not value:
        return default_value
    return value


def get_config_value_from_job_param(job_param, key: str, default_value=""):
    try:
        # 从保护对象中拿
        value = job_param.get("job", {}).get("protectObject", {}).get("extendInfo", {}).get(key, "")
        if value:
            return value
        # 单实例，数据库 charset
        value = job_param.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get(key, "")
        if value:
            return value
        # 集群节点
        nodes = job_param.get("job", {}).get("protectEnv", {}).get("nodes", [])
        if not nodes:
            nodes = job_param.get("job", {}).get("targetEnv", {}).get("nodes", [])
        if not nodes:
            return default_value
        value = nodes[0].get("extendInfo", {}).get(key, "")
        if value:
            return value
        return default_value
    except Exception as e:
        log.info(f"parse error :{e}")
        return default_value


def is_certificate_file(filename):
    if os.path.isdir(filename):
        return False
    file_extension = os.path.splitext(filename)[1].lower()
    if file_extension not in ['.pem', '.crt', '.cer', '.der']:
        return False
    try:
        with open(filename, 'r') as cert_file:
            content = cert_file.read()
            if 'BEGIN CERTIFICATE' in content and 'END CERTIFICATE' in content:
                return True
            return "BEGIN RSA PRIVATE KEY" in content and 'END RSA PRIVATE KEY' in content
    except Exception as exception_str:
        log.debug(f"read error:{exception_str}")
        return False


def is_port_in_use(port, host='localhost'):
    """
    检查指定的端口是否正在被使用。
    :param port: 要检查的端口号
    :param host: 主机地址，默认为 localhost
    :return: 如果端口正在被使用返回True，否则返回False
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.bind((host, int(port)))
        sock.close()
        return False
    except socket.error as e:
        return True
    finally:
        sock.close()


def get_change_master_cmd(master_info):
    cmd_str = (f'change master to master_host=\'{master_info.get("master_host", "")}\','
               f' master_port={master_info.get("master_port", "")},'
               f' master_user=\'{master_info.get("master_user", "")}\','
               f' master_password=\'{master_info.get("master_password", "")}\','
               f' master_log_file=\'{master_info.get("master_binlog_file", "")}\','
               f' master_log_pos={master_info.get("master_binlog_pos", "")}')
    return cmd_str


def get_cluster_type_from_target_env(json_param):
    return json_param.get("job", {}).get("targetEnv", {}) \
        .get("extendInfo", {}).get("clusterType", "")


def match_greatsql(param):
    return re.search(MySQLStrConstant.GREATSQLAPPLICTATION, param)
