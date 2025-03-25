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
import time

import pymysql

from common.cleaner import clear
from common.common import check_path_legal, check_sql_cmd_param, check_command_injection_exclude_quote
from common.common import execute_cmd, output_execution_result_ex
from common.const import CMDResult, SysData, ParamConstant, ExecuteResultEnum
from common.job_const import JobNameConst
from common.util.check_utils import is_ip_address, is_port
from common.util.checkout_user_utils import get_path_owner
from tidb.common.const import TiDBSubType, RpcParamKey, TidbPath, ClusterRequiredHost, TiDBDataBaseFilter, \
    TiDBResourceKeyName, TiDBConst, MysqlTimeOut, ErrorCode
from tidb.logger import log


class ErrCodeException(Exception):
    def __init__(self, err_code, *parameters, message: str = None):
        super(ErrCodeException, self).__init__(message)
        self._error_code = err_code
        self._parameters = parameters
        self._message = message

    @property
    def error_code(self):
        return self._error_code

    @property
    def error_message_json(self):
        return json.dumps({"errorCode": self._error_code, "parameters": self._parameters,
                           "errorMessage": self._message})

    @property
    def error_message_str(self):
        return self._message


class TiDBStructure:
    # TiDB集群结构体类
    def __init__(self):
        self.tidb_nodes = []
        self.tikv_nodes = []
        self.tiflash_nodes = []
        self.pd_nodes = []

    def add_tidb_nodes(self, node_info):
        self.tidb_nodes.append(node_info)

    def add_tikv_nodes(self, node_info):
        self.tikv_nodes.append(node_info)

    def add_tiflash_nodes(self, node_info):
        self.tiflash_nodes.append(node_info)

    def add_pd_nodes(self, node_info):
        self.pd_nodes.append(node_info)


def get_env_variable(str_env_variable: str):
    input_str = json.loads(SysData.SYS_STDIN)
    env_variable = input_str.get(str_env_variable, '')
    return env_variable


def exec_mysql_sql(task_type, pid, sql_list, host, port):
    if task_type == JobNameConst.BACKUP:
        user = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHKEY_BKP + pid)
        pwd = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHPWD_BKP + pid)
    elif task_type == JobNameConst.RESTORE:
        user = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHKEY_RST + pid)
        pwd = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHPWD_RST + pid)
    elif task_type == JobNameConst.RESOURCE:
        user = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHKEY + pid)
        pwd = get_env_variable(TiDBResourceKeyName.APPLICATION_AUTH_AUTHPWD + pid)
    else:
        log.error(f"task_type is invalid")
        return False, ()
    if not user:
        log.error(f"user_name is null")
        return False, ()
    if not pwd:
        log.error(f"password is null")
        return False, ()
    try:
        results = run_mysql_query(host, port, pwd, sql_list, user)
    except Exception as err:
        log.error(f"execute sql {sql_list} failed: {err}!")
        return False, ()
    finally:
        clear(pwd)
    return True, results


def run_mysql_query(host, port, pwd, sql_list, user):
    mysql_conn = pymysql.connect(user=user, password=pwd, host=host, port=port,
                                 connect_timeout=MysqlTimeOut.MYSQL_TIME_OUT)
    cursor = mysql_conn.cursor()
    for sql_str in sql_list:
        cursor.execute(sql_str)
    results = cursor.fetchall()
    mysql_conn.close()
    return results


def exec_rc_tool_cmd(unique_id, interface_name, param_dict):
    """
    执行rc_tool命令
    @@param cmd: 需要执行的命令
    @@param in_param: 需要写入输入文件的命令参数
    @@param unique_id: 输入输出文件唯一标识
    @@return result:bool 命令执行结果
    @@return output:string 命令输出
    """

    def clear_file(path):
        ret = check_path_legal(path, TidbPath.TIDB_FILESYSTEM_MOUNT_PATH)
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


def get_db_total_size(pid, cluster_name, tiup_path, task_type):
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    list_cmd = [
        f"select sum(data_length + index_length)  from information_schema.tables "
        f"where table_schema not in ('{TiDBDataBaseFilter.MYSQL}','{TiDBDataBaseFilter.METRICS_SCHEMA}',"
        f"'{TiDBDataBaseFilter.INFORMATION_SCHEMA}','{TiDBDataBaseFilter.PERFORMANCE_SCHEMA}');"
    ]
    try:
        ret, output = exec_mysql_sql(task_type, pid, list_cmd, host, port)
    except Exception as err:
        ret = False
        log.error(f"Get total database size failed!: {err}!")
    if not ret:
        log.error("Get total database size failed!")
        return ""
    total_size = output[0][0]
    if not total_size:
        return "0"
    else:
        return total_size


def get_db(pid, cluster_name, tiup_path, task_type):
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    list_cmd = [f"show databases;"]
    try:
        ret, output = exec_mysql_sql(task_type, pid, list_cmd, host, port)
    except Exception as err:
        ret = False
        log.error(f"Get database list failed!: {err}!")
    if not ret:
        log.error("Get database list failed!")
        return []
    db_list = []
    db_filter = {
        TiDBDataBaseFilter.MYSQL, TiDBDataBaseFilter.METRICS_SCHEMA, TiDBDataBaseFilter.INFORMATION_SCHEMA,
        TiDBDataBaseFilter.PERFORMANCE_SCHEMA
    }
    for item in output:
        if not str(item[0]) in db_filter:
            db_list.append(str(item[0]))
    log.info(f"Success to execute get_db command. pid:{pid}, database:{db_list}")
    return db_list


def get_db_size(pid, cluster_name, tiup_path, task_type):
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    list_cmd = [
        "select table_schema, size from (select table_schema, sum(data_length + index_length) as size "
        f"from information_schema.TABLES  group by table_schema) as dt;"
    ]
    try:
        ret, output = exec_mysql_sql(task_type, pid, list_cmd, host, port)
    except Exception as err:
        ret = False
        log.error(f"Get database size failed!: {err}!")
    if not ret:
        log.error("Get database size failed!")
        return []
    db_size_list = []
    db_filter = {
        TiDBDataBaseFilter.MYSQL, TiDBDataBaseFilter.METRICS_SCHEMA, TiDBDataBaseFilter.INFORMATION_SCHEMA,
        TiDBDataBaseFilter.PERFORMANCE_SCHEMA
    }
    for item in output:
        if not str(item[0]) in db_filter:
            db_size_list.append((str(item[0]), str(item[1])))
    log.info(f"Success to execute get_db_size command. pid:{pid}, database size:{db_size_list}")
    return db_size_list


def get_table(pid, cluster_name, tiup_path, db_name, task_type):
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    log.info(f"Db name:{db_name}.")
    if not check_params_valid(db_name):
        log.error(f"The db_name {db_name} verification fails")
        return []
    use_db_cmd = [f"use {db_name};", "show tables;"]
    try:
        ret, output = exec_mysql_sql(task_type, pid, use_db_cmd, host, port)
    except Exception as err:
        ret = False
        log.error(f"Get table list failed!: {err}!")
    if not ret:
        log.error(f"Use database {db_name} failed!")
        return []
    table_list = []
    for item in output:
        table_list.append(str(item[0]))
    log.info(f"Success to execute get_table command. pid:{pid}, table_list:{table_list}")
    return table_list


def get_tab_size(pid, cluster_name, tiup_path, db, task_type):
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    list_cmd = [
        "select table_name, (data_length + index_length) as size "
        f"from information_schema.TABLES where table_schema='{db}'"
    ]
    try:
        ret, output = exec_mysql_sql(task_type, pid, list_cmd, host, port)
    except Exception as err:
        ret = False
        log.error(f"Get table size failed!: {err}!")
    if not ret:
        log.error("Get table size failed!")
        return []
    tab_size_list = []
    for item in output:
        tab_size_list.append((str(item[0]), str(item[1])))
    log.info(f"Success to execute get_tab_size command. pid:{pid}, table size:{tab_size_list}")
    return tab_size_list


def get_tidb_node(cluster_info_str, tidb_role):
    host = ''
    port = 0
    tidb_id = ''
    for cluster_node in json.loads(cluster_info_str):
        role = cluster_node.get("role", '')
        if role == tidb_role:
            tidb_id = cluster_node.get("id", '')
            break
    if tidb_id:
        tidb_host_port = tidb_id.split(":")
        host = tidb_host_port[0]
        port = int(tidb_host_port[1])
    return host, port


def get_tidb_structure(cluster_info):
    # 从解析出的配置文件中获取TiDB集群结构信息
    cluster_info_list = json.loads(cluster_info)

    cluster_structure = TiDBStructure()
    for node_info in cluster_info_list:
        if node_info['role'] == 'tidb':
            cluster_structure.add_tidb_nodes(node_info)

        if node_info['role'] == 'tikv':
            cluster_structure.add_tikv_nodes(node_info)

        if node_info['role'] == 'pd':
            cluster_structure.add_pd_nodes(node_info)

        if node_info['role'] == 'tiflash':
            cluster_structure.add_tiflash_nodes(node_info)

    log.info("get the tidb structure success")
    return cluster_structure


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


def output_result_file(pid, result_info):
    # 先清空，再写结果文档
    if not pid or not result_info:
        raise Exception("Param pid or result info is none")
    output_file_name = "{}{}".format("result", pid)
    output_file_path = os.path.join(ParamConstant.RESULT_PATH, output_file_name)
    output_execution_result_ex(output_file_path, result_info)
    if not os.path.exists(output_file_path):
        raise Exception(f"Result file: {output_file_path} can not create")


def write_error_to_result_file(pid, err_info, sub_type, report_dic):
    param = {
        "type": TiDBSubType.TYPE, "subType": sub_type,
        "extendInfo": report_dic
    }
    item = [param]
    result_param = {"exception": err_info, "resourceList": item}
    output_result_file(pid, result_param)


def get_backup_tso_validate(tiup_path, path):
    if not check_paths_valid(path):
        log.error(f"path {path} is invalid")
        return '', '', ''
    br_cmd = f'{tiup_path} br validate decode --field="end-version" -s {path}'
    cmd = f"su - {get_path_owner(tiup_path)} -c '{br_cmd}'"
    ret, std_out, std_err = execute_cmd(cmd)
    if int(ret) != int(CMDResult.SUCCESS):
        log.error(f'execute get backup_tso cmd failed, message: {std_out}, err: {std_err}')
        return '', br_cmd, std_err
    return std_out.strip(), '', ''


def get_br_version(pid, tiup_path):
    log.info(f"Start to get br version {pid}")
    cmd_get_br_version = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} br --version'"
    br_version = ""
    ret_code, std_out, std_err = execute_cmd(cmd_get_br_version)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"Failed get br version: {std_err}.")
        return ""
    br_info = std_out.split("\n")
    for item in br_info:
        if TiDBConst.BR_VERSION in item:
            br_version = item.split(":")[1].strip()
    if not br_version:
        log.error("Failed get br version!")
    return br_version


def get_err_msg(err_msgs):
    err_msg_ls = err_msgs.split('\n')
    err_msg = ''
    for msg in err_msg_ls:
        if 'Error: ' in msg:
            err_msg = msg
            break
    if not err_msg:
        err_msg = ''.join(err_msgs.split('\n')[2:])
    return err_msg


def parse_tso_to_time(tiup_path, pd_id, ts):
    br_version = get_br_version(pd_id, tiup_path)
    if check_command_injection_exclude_quote(pd_id) or not check_params_valid(br_version, ts):
        log.error(f"br_version: {br_version} or pd_id: {pd_id} or ts: {ts} is invalid")
        return '', '', ''
    ctl_cmd = f'{tiup_path} ctl:{br_version} pd -u http://{pd_id} tso {ts}'
    cmd = f"su - {get_path_owner(tiup_path)} -c '{ctl_cmd}'"
    ret_code, std_out, std_err = execute_cmd(cmd)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"Failed parse tso: {std_err}.")
        return '', ctl_cmd, std_err
    log_info = std_out.split("\n")
    ts_time = ""
    for item in log_info:
        if TiDBConst.SYSTEM in item:
            ts_time = item[item.index("system") + 7:].strip()
    if not ts_time:
        log.error("Failed parse tso.")
    return ts_time, '', ''


def convert_tso_time_to_ts(time_str):
    if len(time_str) > 29:
        tz = time_str[-3:]
        time_array = time.strptime(time_str, f"%Y-%m-%d %H:%M:%S.%f %z {tz}")
    else:
        time_array = time.strptime(time_str, f"%Y-%m-%d %H:%M:%S.%f %z")
    return int(time.mktime(time_array))


def get_log_status(pd_host, tiup_path):
    # 从集群获取日志备份状态
    # pd_host格式: "ip:port"
    cmd_get_log_path = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} br log status --pd {pd_host}'"
    log_status = ""
    ret_code, std_out, std_err = execute_cmd(cmd_get_log_path)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"Failed get log status: {std_err}.")
        return ""
    log_info = std_out.split("\n")
    for item in log_info:
        if TiDBConst.LOG_STATUS in item:
            log_status = item.split(":")[1]
    if not log_status:
        log.error("Failed get log status.")
        return ""
    log_status_string = "".join(list(item for item in log_status if item.isalpha()))
    return log_status_string


def get_log_path(pd_host, tiup_path):
    # 从集群获取本地日志备份路径
    # pd_host格式: "ip:port"
    cmd_get_log_path = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} br log status --pd {pd_host}'"
    log_path = ""
    ret_code, std_out, std_err = execute_cmd(cmd_get_log_path)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"Failed get log path: {std_err}.")
        return ""
    log_info = std_out.split("\n")
    for item in log_info:
        if TiDBConst.LOG_PATH in item:
            log_path = item.split(":")[2][2:]
    if not log_path:
        log.error("Failed get log path.")
        return ""
    return log_path


def get_log_stat_info(pd_id, tiup_path, pattern):
    log_status_cmd = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} br log status --pd {pd_id}'"
    status, stdout, stderr = execute_cmd(log_status_cmd)
    if int(status) != int(CMDResult.SUCCESS):
        log.error(f"fail to check log backup meta: {stderr}.")
        return ""
    log_info = stdout.split("\n")
    result = str()
    for item in log_info:
        if pattern in item:
            idx = item.index(pattern + ":")
            idx_start = idx + len(pattern) + 1
            result = item[idx_start:].strip()
    if not result:
        log.error(f"Failed get {pattern} in log status.")
    if pattern == TiDBConst.LOG_STATUS:
        result = result[1:].strip()
    if pattern == TiDBConst.CHECKPOINT:
        result = result.split(";")[0].strip()
    return result


def drop_db(pid, cluster_name, tiup_path, task_type, db):
    if not check_params_valid(pid, cluster_name):
        log.error(f"The pid {pid} or cluster_name {cluster_name} verification fails")
        return False, ErrorCode.ERR_INPUT_STRING
    tidb_id = get_status_up_role_one_host(cluster_name, tiup_path, ClusterRequiredHost.TIDB)
    if not tidb_id:
        log.error("Get tidb host failed!")
        return False, ErrorCode.CHECK_PD_TIDB_FAILED
    host = tidb_id.split(":")[0]
    port = int(tidb_id.split(":")[1])
    drop_db_cmd = [f"DROP DATABASE IF EXISTS {db};"]
    ret, output = exec_mysql_sql(task_type, pid, drop_db_cmd, host, port)
    if not ret:
        log.error("Drop temporary db failed!")
        return False, ErrorCode.BKP_DB_TAB_NOT_EXIST
    return True, ExecuteResultEnum.SUCCESS.value


def get_available_filename(file_parent_path, file_name):
    n = [0]
    log.info('get available file name.')

    def check_name(file_name):
        file_name_new = file_name
        file_path = os.path.join(file_parent_path, file_name_new)
        if os.path.isfile(file_path):
            file_name_new = file_name[:file_name.rfind('.')] + '_' + str(n[0]) + file_name[file_name.rfind('.'):]
            n[0] += 1
        file_path_new = os.path.join(file_parent_path, file_name_new)
        if os.path.isfile(file_path_new):
            file_name_new = check_name(file_name)
        return file_name_new

    return_name = check_name(file_name)
    log.info(f'get available file name success: {return_name};')
    return return_name


def create_file_append(file_path, payload):
    file_parent_path = os.path.split(file_path)[0]
    file_name = os.path.split(file_path)[1]
    file_name_post = get_available_filename(file_parent_path, file_name)
    file_path_post = os.path.join(file_parent_path, file_name_post)
    output_execution_result_ex(file_path_post, payload)


def get_uid(root, file, head):
    uid = ""
    if file.startswith(head):
        file_path = os.path.join(root, file)
        try:
            with open(file_path, "r", encoding='utf-8') as f:
                result = json.loads(f.readlines()[0])
        except Exception as err:
            log.error(f"fail to get uid: {err}")
            return ""
        uid = result.get("uid", "")
    return uid


def get_uids(path, head):
    uids = set()
    for root, _, files in os.walk(path):
        for file in files:
            uid = get_uid(root, file, head)
            if uid:
                uids.add(uid)
    return list(uids)


def get_cluster_user(cluster_name, tiup_path):
    if not check_params_valid(cluster_name):
        log.error(f"The cluster_name {cluster_name} verification fails")
        return ""
    if not check_paths_valid(tiup_path):
        log.error(f"The tiup_path {tiup_path} verification fails")
        return ""
    cmd_list_cluster = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} cluster display {cluster_name}'"
    ret_code, std_out, std_err = execute_cmd(cmd_list_cluster)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"Get cluster user for cluster {cluster_name}  failed.")
        log.error(f"Error: {std_err}")
        return ""
    hosts_info = std_out.split("\n")
    for info in hosts_info:
        if TiDBConst.CLUSTER_USER in info:
            user_info = info.split(":")
            user = user_info[1].strip()
            log.info(f"Succeed get cluster {cluster_name} user: {user}")
            return user
    log.error(f"Failed get cluster user for cluster {cluster_name}.")
    return ""


def get_cluster_hosts(cluster_name, tiup_path):
    if not check_params_valid(cluster_name):
        log.error(f"The cluster_name {cluster_name} verification fails")
        return ""
    if not check_paths_valid(tiup_path):
        log.error(f"The tiup_path {tiup_path} verification fails")
        return ""
    cmd_list_cluster = f"su - {get_path_owner(tiup_path)} -c '{tiup_path} cluster display {cluster_name}'"
    ret_code, std_out, std_err = execute_cmd(cmd_list_cluster)
    if ret_code == CMDResult.FAILED.value:
        log.error(f"List hosts info for cluster {cluster_name}  failed.")
        log.error(f"Error: {std_err}")
        return ""

    hosts_info = std_out.split("\n")
    strip_line = 0
    if not hosts_info[-1]:
        strip_line += 1
    hosts_count_str = hosts_info[- strip_line - 1]
    if not hosts_count_str:
        log.error("Failed get hosts_count_str!")
        return ""
    hosts_count = int(hosts_count_str.split()[-1])
    hosts = hosts_info[-hosts_count - strip_line - 1: -strip_line - 1]
    return hosts


def get_status_up_role_one_host(cluster_name, tiup_path, role):
    # 实时从集群获取指定role且在线的一台pd/tidb主机
    if role not in ["pd", "tidb"]:
        return ""
    hosts = get_cluster_hosts(cluster_name, tiup_path)
    if not hosts:
        return ""
    required_host_type = role
    for item in hosts:
        host = item.split()
        if host and host[1] in required_host_type:
            host_status = host[5].split("|")
            if "Up" in host_status:
                host = host[0]
                break
    ip, port = host.split(":")
    if is_ip_address(ip) and is_port(port):
        return host
    log.info(f"Failed to get status up {role} host.")
    return ""


def get_status_up_role_all_hosts(cluster_name, tiup_path, role):
    # 实时从集群获取指定role：tikv和tiflash
    # 获取所有在线主机，若存在不在线主机，返回"down"
    if role not in ["tikv", "tiflash"]:
        return ""
    hosts = get_cluster_hosts(cluster_name, tiup_path)
    if not hosts:
        return ""
    required_host_type = role
    required_host_list = []
    for item in hosts:
        host = item.split()
        if host and host[1] in required_host_type:
            host_status = host[5].split("|")
            if "Up" not in host_status:
                log.error(f"{required_host_type} host down!")
                return "down"
            else:
                required_host_list.append(host[0])
    required_host = ",".join(required_host_list)
    return required_host


def check_roles_up(cluster_name, tiup_path, roles):
    # 检查指定role列表的主机状态
    # pd、tidb要求有一个在线，tikv、tiflash要求所有在线
    # tiflash 可能不存在，其他role查不到则认为down
    for role in ["pd", "tidb"]:
        if role in roles:
            ret = get_status_up_role_one_host(cluster_name, tiup_path, role)
            if not ret:
                return {"down": role}
    for role in ["tikv", "tiflash"]:
        if role in roles:
            ret = get_status_up_role_all_hosts(cluster_name, tiup_path, role)
            if not ret and role == "tikv":
                return {"down": role}
            if ret == "down":
                return {"down": role}
    return {"up": roles}


def get_agent_uuids(file_content):
    agent_infos = file_content.get("job", {}).get('extendInfo', {}).get('agents', [])
    agent_uuids = set()
    for agent_info in agent_infos:
        agent_uuids.add(agent_info['id'])
    return list(agent_uuids)


def check_params_valid(*params):
    for param in params:
        if param and not check_sql_cmd_param(param):
            log.error(f"The param {param} verification fails")
            return False
    return True


def check_paths_valid(*paths):
    """
    校验文件路径，不能包含特殊字符
    :param: 文件路径
    :return: True，合法文件；False，非法文件
    """
    for path in paths:
        if not path:
            log.error(f"The path: {path} is empty")
            return False
        if not os.path.isfile(path) and not os.path.isdir(path):
            log.error(f"The path: {path} is not file or dir")
            return False
        if not check_abs_path(path):
            return False
        if not check_path_validity(path):
            log.error(f"The path: {path} is invalid")
            return False
    return True


def check_path_validity(path):
    # 不能为空，不能含有?*|;&$><`"\\!等字符
    regex = r'^[^?*|;&$><`"\\!]+$'
    if not re.match(regex, path):
        log.error(f"Path: {path} is invalid")
        return False
    return True


def check_abs_path(path):
    """
    将路径转为绝对路径，并校验是否和原路径一致,防止目录穿越
    :path : 需要检验的目录
    :return: bool
    """
    abs_path = os.path.realpath(path)
    if path.endswith("/"):
        path = path[:-1]
    if abs_path == path:
        return True
    return False


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


def query_local_business_ip(nodes):
    """
    功能描述：查询当前Agent节点的业务ip
    参数：
    @nodes： 节点列表
    返回值：节点索引号, 节点
    """
    local_agent_id = get_agent_id()
    for _, v in enumerate(nodes):
        if local_agent_id == v['id']:
            return v['endpoint']
    return None
