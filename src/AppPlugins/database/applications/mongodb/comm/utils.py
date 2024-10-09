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
import stat
import time
from errno import EEXIST
from functools import partial
from itertools import chain
from json import JSONDecodeError

import pwd

from common.common import check_command_injection, retry_when_exception, \
    read_tmp_json_file, output_execution_result, execute_cmd, output_execution_result_ex
from common.const import RpcToolInterface, SysData, RpcParamKey, CMDResult
from common.util.exec_utils import exec_overwrite_file, su_exec_rm_cmd
from mongodb.comm.const import MongoRoles, ParamField


def valid_username(username: str):
    """
    检测用户名是否为空并判断是否有特殊字符
    :param : 用户名
    :return: bool
    """
    if not username:
        return True
    return check_command_injection(username)


def loads(json_str):
    try:
        json_obj = json.loads(json_str)
    except JSONDecodeError as e:
        raise ValueError from e
    return json_obj


def parse_custom_str_to_dict(custom_str) -> dict:
    """
    转换自定义参数为字典
    :param custom_str: "field:value,field1:value1,……”
    :return: {field:value,field1:value,……}
    """
    custom_list = [kv_str.split(":", 1) for kv_str in custom_str.split(",")]
    custom_dict = {kv_list[0]: kv_list[1] for kv_list in custom_list if len(kv_list) > 1}
    return custom_dict


def get_stdin_field(field):
    input_dict = json.loads(SysData.SYS_STDIN)
    val = input_dict.get(field)
    del input_dict
    return val


def translate_to_snake_name(camel_name: str) -> str:
    """
    转换命名风格：从驼峰格式 转换为蛇形
    :param camel_name: 驼峰字符串
    :return: 以"_"拼接的字符串
    """

    def latin(x):
        return f'_{chr(x + 32)}'

    trans_map = {chr(key): latin(key) for key in range(65, 91)}
    tran_map = camel_name.maketrans(trans_map)
    method = camel_name.translate(tran_map)
    return method.lstrip("_")


def bind_func_args(func, args: tuple = None, kwargs: dict = None):
    """
    为函数绑定参数
    :param func: 函数名
    :param args: 删除位置参数
    :param kwargs: 函数关键字参数
    :return: 绑定参数后的函数
    """
    partial_fun = partial(func)
    if args is not None:
        partial_fun = partial(partial_fun, *args)
    if kwargs is not None:
        partial_fun = partial(partial_fun, **kwargs)
    return partial_fun


def expansion(list_value):
    return chain(*list_value)


def rpc_tool_with_lock(job_id, interface, report_info):

    timestamp = time.time()
    unique_id = f"{job_id}_{timestamp}"
    input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id)
    output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
    exec_overwrite_file(input_file_path, report_info)

    cmd = f"sh {RpcParamKey.RPC_TOOL} {interface} {input_file_path} {output_file_path}"
    # 执行命令后不论结果都需要删除输入文件
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        su_exec_rm_cmd(input_file_path, check_white_black_list_flag=False)

    if ret != CMDResult.SUCCESS.value:
        err_info = f"Invoke rpc_tool script failed, std_err: {std_err}."
        raise Exception(err_info)

    # 不管读取文件是否成功都需要删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        su_exec_rm_cmd(input_file_path, check_white_black_list_flag=False)
    return result


@retry_when_exception()
def job_report(job_id, sub_job_details):
    return rpc_tool_with_lock(job_id, RpcToolInterface.REPORT_JOB_DETAIL, sub_job_details)


@retry_when_exception()
def copy_info_report(job_id, copy_info):
    return rpc_tool_with_lock(job_id, RpcToolInterface.REPORT_COPY_INFO, copy_info)


@retry_when_exception()
def get_previous_copy_info(job_id, protect_obj, backup_types, copy_id=""):
    query_info = {
        RpcParamKey.APPLICATION: protect_obj,
        RpcParamKey.TYPES: backup_types,
        RpcParamKey.COPY_ID: copy_id,
        RpcParamKey.JOB_ID: job_id
    }
    return rpc_tool_with_lock(job_id, RpcToolInterface.QUERY_PREVIOUS_COPY, query_info)


class FileLock:
    def __init__(self, file_name, timeout=10, delay=.05):
        self.is_locked = False
        self.lockfile = "%s.lock" % file_name
        self.file_name = file_name
        self.timeout = timeout
        self.delay = delay
        self.fd = None

    def __enter__(self):
        if not self.is_locked:
            self.acquire()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.is_locked:
            self.release()

    def __del__(self):
        self.release()

    def acquire(self):
        start_time = time.time()
        while True:
            try:
                # 独占式打开文件
                flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
                modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
                self.fd = os.open(self.lockfile, flags, modes)
                break
            except OSError as e:
                if e.errno != EEXIST:
                    raise
                if (time.time() - start_time) >= self.timeout:
                    raise Exception("Timeout occured.") from e
                time.sleep(self.delay)
        self.is_locked = True

    def release(self):
        """ Get rid of the lock by deleting the lockfile.
            When working in a `with` statement, this gets automatically
            called at the end.
        """
        # 关闭文件，删除文件
        if self.is_locked:
            os.close(self.fd)
            su_exec_rm_cmd(self.lockfile, check_white_black_list_flag=False)
            self.is_locked = False


def output_result_ignore_exists(file_path, payload):
    exec_overwrite_file(file_path, payload)


def read_file_with_lock(file_path):
    lock_file = ".".join((file_path, "lock"))
    with FileLock(lock_file):
        cont = read_tmp_json_file(file_path)
    return cont


def write_file_with_lock(file_path, payload):
    lock_file = ".".join((file_path, "lock"))
    with FileLock(lock_file):
        output_result_ignore_exists(file_path, payload)


def check_node_server_type(line_opts):
    if not line_opts.get(MongoRoles.PARSED, {}).get(MongoRoles.SHARDING):
        if line_opts.get(MongoRoles.PARSED, {}).get(MongoRoles.REPLICATION):
            return MongoRoles.REPLICATION
        return MongoRoles.SINGLE
    if line_opts.get(MongoRoles.PARSED, {MongoRoles.SHARDING: {}}).get(MongoRoles.SHARDING, {}).get(
            MongoRoles.CONFIG_DB):
        return MongoRoles.MONGOS
    if line_opts.get(MongoRoles.PARSED, {MongoRoles.SHARDING: {}}).get(MongoRoles.SHARDING, {}).get(
            MongoRoles.CLUSTER_ROLE):
        if MongoRoles.CONFIG_SVR == line_opts.get(MongoRoles.PARSED, {MongoRoles.SHARDING: {}}).get(
                MongoRoles.SHARDING, {}).get(MongoRoles.CLUSTER_ROLE):
            return MongoRoles.CONFIG
        elif MongoRoles.SHARDS_VR == line_opts.get(MongoRoles.PARSED, {MongoRoles.SHARDING: {}}).get(
                MongoRoles.SHARDING, {}).get(MongoRoles.CLUSTER_ROLE):
            return MongoRoles.SHARD
    return ""


def sort_nodes(nodes):
    rules = ["", "PRIMARY", "SECONDARY", "FAULT", "ARBITER"]

    return sorted(nodes, key=lambda x: rules.index(x.get("stateStr")))


def get_mkdir_user(db_path):
    stat_info = os.stat(db_path)
    uid = stat_info.st_uid
    return pwd.getpwuid(uid)[0]


def get_base_nodes_info(cluster_nodes, nodes):
    agent_ip_map = {}
    extend_info_map = {}
    for node in nodes:
        agent_ips = node.get(ParamField.EXTEND_INFO).get(ParamField.AGENT_IP_LIST, "")
        agent_id = node.get(ParamField.EXTEND_INFO).get(ParamField.AGENT_UID, "")
        node_agent_map = {node_ip: agent_id for node_ip in agent_ips.split(",") if node_ip not in agent_ip_map}
        agent_ip_map.update(node_agent_map)
        bin_path = node.get(ParamField.EXTEND_INFO.value, {}).get(ParamField.BIN_PATH.value, "")
        mongodump_bin_path = node.get(ParamField.EXTEND_INFO.value, {}).get(ParamField.MOBGODUMP_BIN_PATH.value, "")
        extend_info = {ParamField.BIN_PATH.value: bin_path, ParamField.MOBGODUMP_BIN_PATH.value: mongodump_bin_path}
        url_map = {node_ip: extend_info for node_ip in agent_ips.split(",") if node_ip not in extend_info_map}
        extend_info_map.update(url_map)
    node_info = {}
    for inst in cluster_nodes:
        inst_uri = inst.get(ParamField.AGENTURL, "")
        node_ip = ""
        if inst_uri:
            node_ip, _ = inst_uri.split(":")
        if node_ip in agent_ip_map:
            inst[ParamField.ID.value] = agent_ip_map.get(node_ip, "")
            inst[ParamField.EXTEND_INFO.value] = extend_info_map.get(node_ip, {})
            tmp = node_info.setdefault(node_ip, [])
            tmp.append(inst)
    del agent_ip_map
    del extend_info_map
    return node_info


def check_real_path(path):
    """
    校验文件路径是否是真实路径，覆盖相对路径、~与软链接等
    :param: 文件路径
    :return: True，真实路径；False，非真实路径
    """
    if path.endswith("/"):
        path = path[:-1]
    abs_path = os.path.realpath(path)
    if abs_path != path:
        return False
    return True
