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
import shutil
import sqlite3
import stat
import time

import psutil
import pymysql

from common.common import check_path_legal, execute_cmd, output_execution_result, check_command_injection, \
    ismount_with_timeout
from common.const import SysData, ParamConstant, SubJobPriorityEnum
from oceanbase.common.const import RpcParamKey, CMDResult, GetIPConstant, SubJobStatusForSqlite
from oceanbase.logger import log


def write_progress_file(message: str, file_name: str):
    log.info(f'Write message into progress file: {message}')
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'a+') as out_file:
        out_file.write(message)
        out_file.write('\n')


def write_progress_file_ex(message: str, file_name: str):
    # 先将进度文件清空在写
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'w') as file_stream:
        file_stream.truncate(0)
        file_stream.write(message)
        file_stream.write('\n')


def output_execution_result_ex(file_path, payload):
    # 先将文件清空再写
    json_str = json.dumps(payload)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as file_stream:
        file_stream.truncate(0)
        file_stream.write(json_str)


def report_job_details(job_id: str, sub_job_details: dict):
    try:
        result_info = exec_rc_tool_cmd(job_id, "ReportJobDetails", sub_job_details)
    except Exception as err:
        log.error(f"Invoke rpc_tool interface exception, err: {err}.")
        return False
    if not result_info:
        return False
    ret_code = result_info.get("code", -1)
    if ret_code != int(CMDResult.SUCCESS):
        log.error(f"Invoke rpc_tool interface failed, result code: {ret_code}.")
        return False
    return True


def exec_rc_tool_cmd(unique_id, interface_name, param_dict):
    def clear_file(path):
        ret = check_path_legal(path, "")
        if not ret:
            return
        if os.path.isfile(path):
            os.remove(path)

    input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id)
    output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
    output_execution_result_ex(input_file_path, param_dict)

    cmd = f"sh {RpcParamKey.RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        # 执行命令后删除输入文件
        clear_file(input_file_path)

    if ret != CMDResult.SUCCESS:
        return {}

    # 读取文件成功后删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        clear_file(output_file_path)

    return result


def extract_ip():
    log.info("Start getting all local ips ...")
    local_ips = []
    try:
        ip_dict = psutil.net_if_addrs()
    except Exception as err:
        log.error(f"Get ip address err: {err}.")
        return local_ips
    for _, info_list in ip_dict.items():
        for i in info_list:
            if i[0] == GetIPConstant.ADDRESS_FAMILY_AF_INET and i[1] != GetIPConstant.LOCAL_HOST:
                local_ips.append(i[1])
    if not local_ips:
        local_ips.append(GetIPConstant.LOCAL_HOST)
    return local_ips


def get_env_variable(str_env_variable: str):
    env_variable = ''
    input_str = json.loads(SysData.SYS_STDIN)
    if input_str.get(str_env_variable):
        env_variable = input_str.get(str_env_variable)
    return env_variable


def parse_sql_result(cmd_result):
    result_list = cmd_result.strip().split('\r\n')
    info_list = []
    for info in result_list:
        if '|' in info:
            temp_list = info.split('|')
            temp_list = [i.strip() for i in temp_list if i.strip()]
            info_list.append(temp_list)
    cnum = len(info_list[0])
    nnum = len(info_list) - 1
    result = []
    for j in range(nnum):
        dict_info = dict()
        for i in range(cnum):
            dict_info[info_list[0][i]] = info_list[j + 1][i]
        result.append(dict_info)
    return result


def remove_dir(path):
    # 可删除目录或文件
    # 删除目录
    if os.path.isdir(path):
        try:
            shutil.rmtree(path)
        except Exception as e:
            log.error(f"Fail to delete dir {path} for {e}.")
    # 删除文件
    elif os.path.isfile(path):
        try:
            os.remove(path)
        except Exception as e:
            log.error(f"Fail to delete file {path} for {e}.")


def invoke_rpc_tool_interface(unique_id: str, interface_name: str, param_dict: dict):
    def clear_file(path):
        log.info(f"try to delete file {path}")
        if os.path.isfile(path):
            log.info(f"start to delete {path}")
            os.remove(path)

    cur_time = str(int((time.time())))
    input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id + cur_time)
    output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
    output_execution_result(input_file_path, param_dict)

    cmd = f"sh {RpcParamKey.RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        # 执行命令后删除输入文件
        clear_file(input_file_path)
    if ret != CMDResult.SUCCESS:
        return {}

    # 读取文件成功后删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        clear_file(output_file_path)
    return result


def get_dir_size(dir_path):
    return_code, out_info, err_info = execute_cmd(f"du -sh --block-size=1K {dir_path}")
    if return_code == CMDResult.SUCCESS:
        result = int(out_info.split('\t')[0])
    else:
        present_size = 0
        for root, _, files in os.walk(dir_path):
            present_size += sum([os.path.getsize(os.path.join(root, name)) for name in files])
        result = int(present_size / 1024)
    return result


def init_sqlite_file(sqlite_file_name, obclient_alive_num):
    sql_str_create_table = "create table obclient_status(sub_job_priority int primary key, status int, " \
                           "max_retry_time int);"
    sql_str_init_table = f"insert into obclient_status(sub_job_priority, status, max_retry_time) " \
                         f"values ({SubJobPriorityEnum.JOB_PRIORITY_1.value}, {SubJobStatusForSqlite.NOT_STARTED}, " \
                         f"{obclient_alive_num}), " \
                         f"({SubJobPriorityEnum.JOB_PRIORITY_2.value}, {SubJobStatusForSqlite.NOT_STARTED}, " \
                         f"{obclient_alive_num});"
    try:
        conn = sqlite3.connect(sqlite_file_name)
        cursor = conn.cursor()
        cursor.execute(sql_str_create_table)
        cursor.execute(sql_str_init_table)
        conn.commit()
    except Exception as exception:
        log.error(f"initialize sqlite file failed:{exception}")
        conn.rollback()
        return False
    finally:
        conn.close()
    log.info("init_sqlite_file success")
    return True


def wait_or_lock_sqlite(sqlite_file_name, timeout, sub_job_priority: int):
    sql_str_query_status = f"select status from obclient_status where sub_job_priority={sub_job_priority};"
    log.info(f"enter wait_or_lock_sqlite, sqlite_file_name is {sqlite_file_name}")
    try:
        conn = sqlite3.connect(sqlite_file_name, timeout=timeout, isolation_level='EXCLUSIVE')
        cursor = conn.cursor()
        cursor.execute("begin exclusive transaction;")
        log.info("this node lock succeed")
        cursor.execute(sql_str_query_status)
        result_fetch = cursor.fetchall()
        result = int(result_fetch[0][0])
        log.info(f'status query result is {result}')
    except Exception as err:
        log.error(f"execute sqlite error: {err}")
        return False
    if result == SubJobStatusForSqlite.NOT_STARTED.value or result == SubJobStatusForSqlite.RETRY.value:
        sql_str = f'UPDATE obclient_status SET status={SubJobStatusForSqlite.DOING.value} ' \
                  f'WHERE sub_job_priority={sub_job_priority};'
        cursor.execute(sql_str)
        conn.commit()
        log.info("execute lock sql")
        return conn
    elif result == SubJobStatusForSqlite.SUCCESS.value or result == SubJobStatusForSqlite.FAILED.value:
        conn.close()
        log.info("already succeeded or failed, sqlite connection close ")
        return result
    else:
        conn.close()
        return result


def update_sqlite_sub_job_status(conn, updated_status, priority):
    log.info("update sqlite status")
    retry_time = 0
    if updated_status == SubJobStatusForSqlite.RETRY.value:
        retry_time = 1
    sql_str = f'UPDATE obclient_status SET status={updated_status}, max_retry_time=max_retry_time-{retry_time}' \
              f' WHERE sub_job_priority={priority};'
    sql_str_query_retry_time = f'SELECT max_retry_time from obclient_status where sub_job_priority={priority}'
    try:
        cursor = conn.cursor()
        cursor.execute(sql_str)
        conn.commit()
        log.info("update to success")
        cursor.execute(sql_str_query_retry_time)
        retry_time_now = int(cursor.fetchall()[0][0])
        log.info(f"retry_time_now is {retry_time_now}")
        if not retry_time_now:
            return False
    except Exception as exception:
        log.error(f'fail to update sqlite status: {exception}')
        return False
    finally:
        conn.close()
        log.info("sqlite is closed here")
    return True


def get_agent_id():
    """
    功能描述：获取Agent ID
    参数：无
    返回值：Agent ID
    """
    agent_file_path = ParamConstant.HOST_SN_FILE_PATH
    if not agent_file_path or not os.path.exists(agent_file_path):
        return ""
    with open(agent_file_path, "r", encoding="utf-8") as f:
        text = f.read()
    return text.strip("\n")


def exec_sql_cmd(ip, port, user, pwd, sql_str):
    try:
        conn = pymysql.connect(host=ip, port=port, user=user, passwd=pwd)
        cursor = conn.cursor()
        cursor.execute(sql_str)
        result = cursor.fetchall()
        conn.close()
    except Exception as except_str:
        log.error(f"Connect observer service failed: {except_str}")
        return False, "raise except"
    return True, result


def check_special_characters(param):
    """
    校验参数（字符）的合法性
    """
    if not param:
        return True
    if not isinstance(param, str):
        log.info(f"The param is not str. The type of the parameter is {type(param)}.")
        return False
    if check_command_injection(param):
        log.error("The param has special characters.")
        return False
    return True


def str_to_float(origin_str):
    try:
        value = float(origin_str)
    except Exception:
        return 0
    return value


def check_mount(mount_point):
    """
    带有超时的校验是否有挂载，默认返回True
    :param mount_point:挂载点
    :return: bool
    """
    try:
        ismount = ismount_with_timeout(mount_point)
        log.info(f"The path {mount_point} is a mount point: {ismount}")
    except Exception as e:
        log.error(e, exc_info=True)
        ismount = True
    return ismount
