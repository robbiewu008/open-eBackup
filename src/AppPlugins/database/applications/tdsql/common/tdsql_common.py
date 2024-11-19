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
import locale
import os
import platform
import re
import shlex
import shutil
import socket
import stat
import subprocess
import time
import pwd
import psutil

from common.common import execute_cmd, invoke_rpc_tool_interface
from common.const import CMDResult, IPConstant, SysData, RepositoryDataTypeEnum, RpcParamKey, PathConstant
from dws.commons.common import get_uid_gid
from tdsql.common.const import MountType, ArchiveType
from tdsql.handle.common.const import ErrorCode, TDSQLSubType
from tdsql.handle.common.tdsql_exception import ErrCodeException
from tdsql.logger import log


def report_job_details(job_id: str, sub_job_details: dict):
    try:
        cur_time = str(int((time.time())))
        result_info = invoke_rpc_tool_interface(job_id + cur_time, "ReportJobDetails", sub_job_details)
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


def output_execution_result_ex(file_path, payload):
    # 先将文件清空再写
    json_str = json.dumps(payload)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_path, flags, modes), 'w') as file_stream:
        file_stream.truncate(0)
        file_stream.write(json_str)


def write_file(file_name, message):
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'w') as file_stream:
        file_stream.truncate(0)
        file_stream.write(message)


def get_std_in_variable(str_env_variable: str):
    env_variable = ''
    input_dict = json.loads(SysData.SYS_STDIN)
    if input_dict.get(str_env_variable):
        env_variable = input_dict.get(str_env_variable)
    return env_variable


def set_restore_name():
    local_time = time.strftime("%Y%m%d_%H%M%S", time.localtime())
    restore_name = f'Restorename_{local_time}'
    return restore_name


def set_backup_name():
    local_time = time.strftime("%Y%m%d_%H%M%S", time.localtime())
    restore_name = f'Backupname_{local_time}'
    return restore_name


def get_hostname():
    return socket.gethostname()


def change_file_permission(path, permission_code):
    root_uid, root_gid = get_uid_gid("root")
    rdadmin_uid, rdadmin_gid = get_uid_gid("rdadmin")
    execute_cmd(f"chmod {permission_code} {path}")
    execute_cmd(f"chown {root_uid}:{rdadmin_gid} {path}")


def read_file(path):
    with open(path, "r", encoding='utf-8') as tmp:
        result = json.load(tmp)
    log.debug(f"Read progress end")
    return result


def extract_ip():
    """
    获取当前主机所有ip
    :return: list，主机ip
    """
    log.info(f"Start getting all local ips ...")
    local_ips = []
    ip_dict = {}
    try:
        ip_dict = psutil.net_if_addrs()
    except Exception as err:
        log.error(f"Get ip address err: {err}.")
        return local_ips
    for _, info_list in ip_dict.items():
        for i in info_list:
            if i[0] == 2 and i[1] != IPConstant.LOCAL_HOST:
                local_ips.append(i[1])
    log.info(f"Get all local ips success.")
    return local_ips


def delete_files_except(filename, directory):
    for root_path, dirs, files in os.walk(directory):
        for file in files:
            if file != filename and os.path.isfile(os.path.join(root_path, file)):
                os.remove(os.path.join(root_path, file))
        for dir_in_path in dirs:
            shutil.rmtree(os.path.join(root_path, dir_in_path), ignore_errors=True)


def check_ip(ip):
    patterm = re.compile('^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$')
    if patterm.match(ip):
        return True
    else:
        return False


def check_port(port):
    try:
        if int(port) not in range(0, 65536):
            raise ErrCodeException(ErrorCode.ERROR_PARAM, f"port {port} is not expected")
    except Exception as ex:
        log.error(f"when checking port {port}, exception {ex} occurs")
        raise ErrCodeException(ErrorCode.ERROR_PARAM, f"port {port} is not expected").format(ex)


def get_agent_uuids(file_content):
    agent_infos = file_content.get("job", {}).get('extendInfo', {}).get('agents', [])
    agent_uuids = set()
    for agent_info in agent_infos:
        agent_uuids.add(agent_info['id'])
    return list(agent_uuids)


def get_nodes(file_content):
    protect_object = file_content.get("job", {}).get("protectObject", {})
    sub_type = protect_object.get("subType")
    log.info(f"tdsql backup get_nodes subType: {sub_type}.")
    if sub_type == TDSQLSubType.SUBTYPE_CLUSTER_INSTANCE:
        cluster_info_str = protect_object.get(
            "extendInfo", {}).get("clusterInstanceInfo")
        cluster_info = json.loads(cluster_info_str)
        nodes = cluster_info.get("groups", [])[0].get("dataNodes")
    else:
        cluster_group_str = protect_object.get(
            "extendInfo", {}).get("clusterGroupInfo")
        cluster_group_info = json.loads(cluster_group_str)
        nodes = cluster_group_info.get("group").get("dataNodes")
    return nodes


def umount_bind_path(des_area, mount_type=''):
    if mount_type == MountType.FUSE:
        umount_bind_cmd = f"{PathConstant.FILE_CLIENT_PATH} --remove --mount_point={des_area}"
    else:
        umount_bind_cmd = f"umount -l {des_area}"
    return_code, std_out, std_err = execute_cmd(umount_bind_cmd)
    if return_code != CMDResult.SUCCESS:
        log.error(f"{umount_bind_cmd} failed, std_out {std_out}, std_err {std_err}")
        return False
    log.info(f"Succeed to exec umount bind path {des_area}")
    return True


def mount_bind_path(src_area, des_area, mount_type='', osad_params=None):
    umount_bind_path(des_area, mount_type)
    if mount_type == MountType.FUSE:
        osad_ip_list, osad_auth_port, osad_server_port, job_id = osad_params
        mount_bind_cmd = f"{PathConstant.FILE_CLIENT_PATH} --add --mount_point={des_area} " \
                         f"--source_id={job_id} --osad_ip_list={osad_ip_list} " \
                         f"--osad_auth_port={osad_auth_port}  --osad_server_port={osad_server_port}"
    else:
        mount_bind_cmd = f"mount --bind {src_area} {des_area}"
    return_code, std_out, std_err = execute_cmd(mount_bind_cmd)
    if return_code != CMDResult.SUCCESS:
        log.error(f"{mount_bind_cmd} failed, std_out{std_out}, err: {std_err}!")
        return False
    log.info("Succeed to exec mount bind path")
    return True


def chown_path_owner_to_tdsql(backup_path):
    stat_info = os.stat(backup_path)
    user = pwd.getpwuid(stat_info.st_uid)[0]
    if user == "tdsql":
        log.info(f"chown_backup_path_owner {backup_path} already tdsql")
        return
    os.lchown(backup_path, pwd.getpwnam("tdsql").pw_uid, pwd.getpwnam("tdsql").pw_gid)
    log.info(f"chown_backup_path_owner to tdsql")
    return


def chown_owner_to_tdsql(dir_path):
    stat_info = os.stat(dir_path)
    user = pwd.getpwuid(stat_info.st_uid)[0]
    if user == "tdsql":
        log.info(f"chown_owner_to_tdsql {dir_path} already tdsql")
        return
    chown_cmd = f"chown -R tdsql: {dir_path}"
    execute_cmd(chown_cmd)
    log.info(f"chown_owner_to_tdsql to tdsql")
    return


def execute_cmd_list(cmd_list, locale_encoding=False, timeout=None):
    """
    执行命令列表，前一条命令的输出是后一条命令的输入
    :param cmd_list: 命令列表
    :param locale_encoding: 是否使用本地编码
    :param timeout: 执行命令超时
    :return: (字符串格式返回码, 标准输出, 标准错误)
    """
    encoding = "utf-8"
    if platform.system().lower() == "windows" or locale_encoding:
        encoding = locale.getdefaultlocale()[1]
    ret_code = 1
    std_out = None
    std_err = None
    for tmp_cmd in cmd_list:
        process = subprocess.Popen(shlex.split(tmp_cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   encoding=encoding)
        try:
            std_out, std_err = process.communicate(input=std_out, timeout=timeout)
        except TimeoutError as err:
            process.kill()
            ret_code = 1
            std_out = ""
            std_err = str(err)
            break
        else:
            ret_code = process.returncode
    return str(ret_code), std_out, std_err


def get_log_uri(copies):
    restore_type = copies[len(copies) - 1]['type']
    data_uri = get_repository_path(copies[len(copies) - 1], RepositoryDataTypeEnum.DATA_REPOSITORY)
    id_list = []
    log_uri = ''
    if restore_type == 'log':
        # 从后往前找到第一个不是日志的副本，然后记下id
        restore_copy_id = None
        for copy in copies[::-1]:
            restore_type = copy['type']
            if restore_type != 'log':
                restore_copy_id = copy['id']
                data_uri = get_repository_path(copy, RepositoryDataTypeEnum.DATA_REPOSITORY)
                break
        log_uri = get_repository_path(copies[len(copies) - 1], RepositoryDataTypeEnum.LOG_REPOSITORY)
        log_uri_strs = log_uri.split('/')
        log_uri = log_uri.replace(f'/{log_uri_strs[len(log_uri_strs) - 1]}', '')
        id_list = get_id_list(log_uri, restore_copy_id)
        log.info(f'get_log_uri, id_list is {id_list}')
    elif restore_type in ArchiveType.archive_array:
        # 日志恢复的最后一个副本是日志副本，倒数第二个是数据副本
        copy = copies[-2]
        restore_copy_id = copy['id']
        data_uri = get_repository_path(copy, RepositoryDataTypeEnum.DATA_REPOSITORY)
        log_uri = get_repository_path(copies[len(copies) - 1], RepositoryDataTypeEnum.LOG_REPOSITORY)
        log_uri_strs = log_uri.split('/')
        log_uri = log_uri.replace(f'/{log_uri_strs[len(log_uri_strs) - 1]}', '')
        id_list = get_id_list(log_uri, restore_copy_id)
    return id_list, log_uri, data_uri


def get_id_list(log_path_parent_dir, restore_copy_id):
    restore_copy_id_file = restore_copy_id + ".meta"
    dot_meta_path = os.path.join(log_path_parent_dir, restore_copy_id_file)
    id_list = []
    if not os.path.exists(dot_meta_path) or not dot_meta_path:
        return id_list
    with open(dot_meta_path, 'r', encoding='utf-8') as file_read:
        for line in file_read.readlines():
            key_value = line.strip('\n').split(";")
            key = key_value[0].strip()
            id_list.append(key)
    return id_list


def get_repository_path(copy, repository_type):
    repositories = copy.get("repositories", [])
    repositories_path = ""
    for repository in repositories:
        if repository['repositoryType'] == repository_type and repository.__contains__('path'):
            repositories_path = repository["path"][0]
            break
    return repositories_path


def get_last_backup_time_and_id(last_copy_info):
    copy_type = last_copy_info.get("type", "")
    log.info(f"get_last_any_copy_type copy_type is {copy_type}")
    if not copy_type:
        return {}, {}
    extend_info = last_copy_info.get("extendInfo", {})
    if copy_type == RpcParamKey.LOG_COPY:
        timestamp = extend_info.get("endTime", 0)
        last_copy_id = extend_info.get("associatedCopies", [])
    else:
        timestamp = extend_info.get("backup_time", 0)
        last_copy_id = extend_info.get("copy_id", "")
    log.info(f'timestamp, last_copy_id info:{timestamp, last_copy_id}')
    return timestamp, last_copy_id


def get_remote_host_ips(repositories):
    remote_hosts = [repository.get('remoteHost', []) for repository in repositories]
    ips = []
    for items in remote_hosts:
        for item in items:
            ip = item.get('ip', '')
            ips.append(ip)
    result = ','.join(set(ips))
    return result
