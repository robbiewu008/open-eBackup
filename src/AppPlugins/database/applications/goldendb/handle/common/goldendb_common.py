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
import json
import os
import pwd
import random
import re
import shutil
import subprocess
import time
from base64 import b64decode
from datetime import datetime

import psutil

from goldendb.logger import log
from common.common import check_command_injection_exclude_quote, check_path_legal, execute_cmd, read_tmp_json_file, \
    output_execution_result_ex, output_result_file, report_job_details
from common.common_models import SubJobDetails, LogDetail, ActionResult, SubJobModel
from common.const import EnumPathType, ParamConstant, SubJobStatusEnum, ReportDBLabel, DBLogLevel, ExecuteResultEnum, \
    RpcParamKey, CMDResult, CMDResultInt
from common.exception.common_exception import ErrCodeException
from common.file_common import exec_lchown_dir_recursively, check_file_or_dir, change_path_permission, get_user_info, \
    exec_lchown
from common.job_const import JobNameConst
from common.const import RepositoryDataTypeEnum
from common.util.check_user_utils import check_os_user
from common.util.exec_utils import exec_append_newline_file, exec_overwrite_file, exec_umount_cmd, exec_mount_cmd, \
    exec_mkdir_cmd, check_path_valid, ExecFuncParam, su_exec_cmd_list, exec_cp_dir_no_user, read_lines_cmd, \
    su_exec_rm_cmd, exec_cat_cmd
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure
from goldendb.handle.common.const import GoldenDBPath, MountBindPath, GetIPConstant, Report, ErrorCode, RoleIniName, \
    ManagerPriority, GoldenDBJsonConst, GoldenDBNodeType, RoleBackupDir, RoleIniSection, ActTxInfoConsts, \
    FormatCapacity, ErrCodeMessage, ErrPattern, BackupResultMessage, RestoreResultMessage, DbtoolTaskStatus, \
    FileInfoDict
from goldendb.schemas.glodendb_schemas import TaskInfo, StatusInfo

RECOGNIZED_DBTOOL_ERRORS = [ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL, ErrorCode.ERR_BKP_CHECK, ErrorCode.ERROR_LOGIN_INFO]
LOG_DETAIL_SUCCESS = [ReportDBLabel.SUB_JOB_SUCCESS, ReportDBLabel.COPY_VERIFICATION_SUCCESS,
                      ReportDBLabel.PRE_REQUISIT_SUCCESS, ReportDBLabel.BACKUP_SUB_JOB_SUCCESS]


def get_user_home(user_name):
    try:
        user_info = pwd.getpwnam(user_name)
        log.info(f"Get root path {user_info.pw_dir} for {user_name} success.")
        return user_info.pw_dir
    except Exception as err:
        log.error(f"User '{user_name}' not found, err: {err}.")
        return ""


def write_progress_file(message: str, file_name: str):
    log.info(f'Write message into progress file: {message}')
    exec_append_newline_file(file_name, message)


def write_progress_file_ex(message: str, file_name: str):
    # 先将进度文件清空在写
    content = f"{message}\n"
    exec_overwrite_file(file_name, content, json_flag=False)


def report_job_details_file_ex(job_id: str, sub_job_details: dict):
    """
    通过rpc工具主动上报任务详情，当输入文件存在时，覆盖该文件。
    """
    # 尝试执行命令
    try:
        result_info = exec_rc_tool_cmd(job_id, "ReportJobDetails", sub_job_details)
    except Exception as err:
        log.error(f"Invoke rpc_tool interface exception, err: {err}.")
        return False
    if not result_info:
        return False
    ret_code = result_info.get("code", -1)
    if ret_code != CMDResultInt.SUCCESS.value:
        log.error(f"Invoke rpc_tool interface failed, result code: {ret_code}.")
        return False
    return True


def clear_file(path):
    ret = check_path_legal(path, GoldenDBPath.GoldenDB_FILESYSTEM_MOUNT_PATH)
    if not ret:
        return
    if os.path.isfile(path):
        os.remove(path)


def exec_rc_tool_cmd(unique_id, interface_name, param_dict):
    """
    执行rc_tool命令
    @@param cmd: 需要执行的命令
    @@param in_param: 需要写入输入文件的命令参数
    @@param unique_id: 输入输出文件唯一标识
    @@return result:bool 命令执行结果
    @@return output:string 命令输出
    """
    input_file_path = os.path.join(RpcParamKey.PARAM_FILE_PATH, RpcParamKey.INPUT_FILE_PREFFIX + unique_id)
    output_file_path = os.path.join(RpcParamKey.RESULT_PATH, RpcParamKey.OUTPUT_FILE_PREFFIX + unique_id)
    ret = exec_overwrite_file(input_file_path, param_dict)
    if not ret:
        log.error(f"write to {input_file_path} failed")
        su_exec_rm_cmd(input_file_path)
        return {}

    cmd = f"sh {RpcParamKey.RPC_TOOL} {interface_name} {input_file_path} {output_file_path}"
    try:
        ret, std_out, std_err = execute_cmd(cmd)
    except Exception as err:
        raise err
    finally:
        # 执行命令后删除输入文件
        su_exec_rm_cmd(input_file_path)

    if ret != CMDResult.SUCCESS.value:
        return {}

    # 读取文件成功后删除文件
    try:
        with open(output_file_path, "r", encoding='utf-8') as tmp:
            result = json.load(tmp)
    except Exception as err:
        raise err
    finally:
        su_exec_rm_cmd(output_file_path)

    return result


def extract_ip():
    """
    获取当前主机所有ip
    :return: list，主机ip
    """
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


def umount_bind_backup_paths(job_id):
    path = os.path.join(ParamConstant.RESULT_PATH, f"goldendb_mount_list_{job_id}")
    try:
        ret, mount_list = read_lines_cmd(path)
        if ret:
            umount_bind_path_list(mount_list)
        else:
            log.error(f"read mount_list failed.path={path}, job_id={job_id}")
    finally:
        su_exec_rm_cmd(path)


def umount_bind_path_list(paths):
    if not paths:
        return
    for path in paths:
        umount_bind_path(path)


def umount_bind_path(des_area):
    try:
        return_code, std_out, std_err = exec_umount_cmd(des_area, '-l')
    except ErrCodeException as ex:
        log.warning(f"Failed to exec umount bind path: {ex}")
        return False

    if return_code != CMDResult.SUCCESS.value:
        log.warning(f"Failed to exec umount bind path: {des_area}! std_err: {std_err}")
        return False
    log.info(f"Succeed to exec umount bind path {des_area}")
    return True


def mount_bind_path(src_area, des_area, job_id):
    umount_bind_path(des_area)
    if not os.path.isdir(des_area) and not os.path.exists(des_area):
        ret = exec_mkdir_cmd(des_area, mode=0x700)
        if not ret:
            return False
    try:
        return_code, std_out, std_err = exec_mount_cmd(src_area, des_area)
    except ErrCodeException as ex:
        log.error(f"Failed to exec mount bind path err: {ex}!")
        return False

    if return_code != CMDResult.SUCCESS.value:
        log.error(f"Failed to exec mount bind path err: {std_err}!src path={src_area}, dest path={des_area}")
        return False
    log.info(f"Succeed to exec mount bind path. src path={src_area}, dest path={des_area}")

    exec_append_newline_file(os.path.join(ParamConstant.RESULT_PATH, f"goldendb_mount_list_{job_id}"), des_area)

    return True


def mount_bind_backup_path(data_area, meta_area, job_id):
    if not mount_bind_path(data_area, MountBindPath.DATA_FILE_PATH, job_id):
        log.error("Mount bind data path failed")
        return False
    if meta_area:
        if not mount_bind_path(meta_area, MountBindPath.META_FILE_PATH, job_id):
            log.error("Mount bind meta path failed")
            return False
    log.debug("Mount bind backup path success")
    return True


def exec_find_backup_dir(role_name, ini_name, dir_name):
    if check_command_injection_exclude_quote(role_name):
        log.error("command injection detected in role_name!")
        return ""
    if check_command_injection_exclude_quote(ini_name):
        log.error("command injection detected in ini_name!")
        return ""
    if check_command_injection_exclude_quote(dir_name):
        log.error("command injection detected in dir_name!")
        return ""
    find_dir_cmd = f'su - {role_name} -c "cat ~/etc/{ini_name} | grep {dir_name}"'

    return_code, backup_dir = subprocess.getstatusoutput(find_dir_cmd)

    if return_code != CMDResultInt.SUCCESS.value:
        log.error(f"Get source backup dir failed! return_code:{return_code}, backup_dir:{backup_dir}")
        return ""
    backup_dir = backup_dir.split("=")[1].strip()
    return backup_dir


def get_data_node_backup_dir(encode_str):
    """
    功能描述：从base64编码的字符串中解码出backup根目录
    参数：
    @encode_str：base64编码的字符传
    返回值：backup根目录
    """
    decode_byte = b64decode(encode_str)
    decode_str = decode_byte.decode('utf-8', errors='ignore')
    dir_re = r'backup_rootdir\s*=(.*?)\s*$'
    backup_root_dir = re.findall(dir_re, decode_str)
    if backup_root_dir:
        if isinstance(backup_root_dir[0], str):
            log.info(f"Get backup_root_dir.")
            return backup_root_dir[0].strip()
    return ''


def get_backup_path(role_name, role_type, file_content, json_const):
    source_backup_dir = get_data_node_backup_dir(
        file_content["job"][f"{json_const}"]["extendInfo"]["local_ini_cnf"])
    source_backup_dir = get_local_ini_path(role_name, role_type, source_backup_dir)
    return source_backup_dir


def get_local_ini_path(role_name, role_type, source_backup_dir):
    if check_command_injection_exclude_quote(role_name) or not check_os_user(role_name):
        log.error("command injection in role_name! The role_name(%s) is invalid.", role_name)
        return ''
    role_home = get_user_home(role_name)
    if source_backup_dir == "":
        # 上传配置路径为空，取当前用户home路径下的backup_root（insight界面可下载该模板）
        source_backup_dir = os.path.join(role_home, "backup_root")
    elif source_backup_dir.startswith("$HOME/"):
        backup_root = source_backup_dir.split("$HOME/")[1]
        source_backup_dir = os.path.join(role_home, backup_root)
    return source_backup_dir


def cp_active_folder(src_path, des_path, os_user):
    """
    将src_path中的活跃事务文件夹复制到des_path中
    :param src_path:
    :param des_path:
    :param os_user: os_user
    :return:
    """
    bkp_active_tx_info_path = os.path.join(src_path, 'Active_TX_Info')
    ret = exec_cp_dir_no_user(bkp_active_tx_info_path, des_path, is_check_white_list=False, is_overwrite=True)
    if not ret:
        log.error(f'execute copy cmd failed.')
        return False

    group_name, _ = get_user_info(os_user)
    return mkdir_chmod_chown_dir_recursively(os.path.join(des_path, 'Active_TX_Info'), 0o770, os_user, group_name)


def cp_result_info(cluster_id, bkp_file, src_path, des_path):
    """
    将src_path中，cluster_id对应的备份结果文件复制到des_path中
    :param cluster_id:
    :param bkp_file:
    :param src_path:
    :param des_path:
    :return:
    """
    src_bkp_file_path = os.path.join(src_path, f'DBCluster_{cluster_id}', bkp_file)
    des_bkp_folder_path = os.path.join(des_path, f'DBCluster_{cluster_id}')

    if not check_path_valid(src_bkp_file_path, is_check_white_list=False) or not check_path_valid(des_bkp_folder_path):
        return False
    path_type = check_file_or_dir(src_bkp_file_path)
    if path_type not in (EnumPathType.DIR_TYPE, EnumPathType.FILE_TYPE):
        log.error(f"Src path is invalid type: {path_type} can not copy ")
        return False

    cp_cmd = f'cp -a {src_bkp_file_path} {des_bkp_folder_path}'
    return_code, out_info, err_info = execute_cmd(cp_cmd)
    if return_code != CMDResult.SUCCESS.value:
        log.error(f'execute copy cmd failed, message: {out_info}, err: {err_info}')
        return False
    return True


def get_copy_info_param(meta_path):
    """
    根据meta_path，获取copy_info中的cluster_id，resultinfo_name，task_id，返回字典
    :param meta_path:
    :return:
    """
    meta_file = os.path.join(meta_path, "copy_info")
    if not os.path.exists(meta_file):
        log.error("Meta file not exist!")
        return ''
    try:
        process = subprocess.run(["/bin/cat", meta_file], timeout=5, shell=False, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, encoding="utf-8")
    except subprocess.TimeoutExpired as ex:
        raise Exception("Timeout") from ex
    data = process.stdout
    try:
        param_dict = json.loads(data)
    except Exception as err:
        log.err(f"get copy_info failed: {err}")
        return ''
    return param_dict


def get_bkp_type_from_result_info(result_info_path):
    """
    读取result_info_path，获取生成该文件的备份任务对应的备份类型
    :param result_info_path:
    :return:
    """
    if not os.path.exists(result_info_path):
        log.error("result info file not exist!")
        return ''
    try:
        process = subprocess.run(["/bin/cat", result_info_path], timeout=5, shell=False, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, encoding="utf-8")
    except subprocess.TimeoutExpired as ex:
        raise Exception("Timeout") from ex
    data = process.stdout
    try:
        bkp_type = [_ for _ in data.split(" ") if _][18]
    except Exception as err:
        log.err(f"get backup type failed: {err}")
        return ''
    return bkp_type


def get_task_path_from_result_info(result_info_path, cluster_id_path):
    """
    读取result_info_path，获取文件系统中，生成该文件的备份任务对应的task path
    :param result_info_path:
    :param cluster_id_path:
    :return:
    """
    ret, data = exec_cat_cmd(result_info_path)
    task_id = ''
    if ret:
        try:
            xbstream_path = [_ for _ in data.split(" ") if _ and "_INCREMENTAL_" in _][0]
            task_id = xbstream_path.split("DATA_BACKUP/")[1].split("/")[0]
            log.info(f"get task id from result file content, task id is:{task_id}")
        except Exception as err:
            log.err(f"get backup type failed: {err}")
            return False, ""
    task_path = os.path.join(cluster_id_path, 'DATA_BACKUP', task_id)
    if os.path.exists(task_path):
        log.info(f"Will remove task_path: {task_path}.")
        return True, task_path
    else:
        log.error(f"task_path: {task_path}, not exists.")
        return False, ""


def get_bkp_result_names_in_cluster_id_dir(backup_path, file_content):
    """
    获取cluster id文件夹中的所有备份结果文件的文件名
    :param backup_path:
    :param file_content:
    :return:
    """
    goldendb_structure = get_goldendb_structure(file_content)
    cluster_id = goldendb_structure.cluster_id
    dbcluster_path = os.path.join(backup_path, f'DBCluster_{cluster_id}')
    if os.path.exists(dbcluster_path):
        pre_bkp_result_names = [
            f
            for f in os.listdir(dbcluster_path)
            if os.path.isfile(os.path.join(dbcluster_path, f)) and f.startswith(f'{cluster_id}_backup')
        ]
    else:
        return []
    return pre_bkp_result_names


def verify_path_trustlist(path):
    """
    判断输入的path是存在目录逃逸风险，有返回False, 无返回True
    :param path:
    :return:
    """
    try:
        real_path = os.path.realpath(path)
    except Exception as error:
        return False
    if check_command_injection_exclude_quote(real_path):
        return False
    path_trustlist = [
        RpcParamKey.RPC_TOOL,
        RpcParamKey.PARAM_FILE_PATH,
        RpcParamKey.RESULT_PATH,
        MountBindPath.ROACH_META_FILE_PATH,
        MountBindPath.DATA_FILE_PATH,
        MountBindPath.META_FILE_PATH,
        MountBindPath.DB_BIN_PATH,
        GoldenDBPath.GoldenDB_FILESYSTEM_MOUNT_PATH,
        GoldenDBPath.GoldenDB_LINK_PATH
    ]
    for tmp_path in path_trustlist:
        if real_path.startswith(tmp_path):
            return True
    return False


def get_repository_path(file_content, repository_type):
    repositories = file_content.get("job", {}).get("repositories", [])
    repositories_path = ""
    for repository in repositories:
        if repository['repositoryType'] == repository_type:
            index = random.randint(0, len(repository["path"]) - 1)
            if repository_type != RepositoryDataTypeEnum.DATA_REPOSITORY:
                index = 0
            repositories_path = repository["path"][index]
            log.info(f"repository_type is {repository_type}, index is {index}, "
                     f"repository_path is {repositories_path}")
            break
    return repositories_path


def su_exec_cmd(user, cmd, param_list=None):
    """
    指定用户执行命令通用方法
    :param user: 用户
    :param cmd： 命令
    :param param_list： 校验列表
    :return:
    """
    if param_list is None:
        param_list = [[]]
    exec_param = ExecFuncParam(user, [cmd], param_list, chk_exe_owner=False)
    ret, out = su_exec_cmd_list(exec_param)
    return ret == CMDResult.SUCCESS.value, out


def exec_chmod_dir_recursively(input_path: str, mode):
    """
    递归修改目录所属用户和用户组
    :param mode: chmod参数
    :param input_path: 待修改目录
    """
    path_type = check_file_or_dir(input_path)
    if path_type == EnumPathType.INVALID_TYPE:
        log.error(f"input path: {input_path} is invalid can not exec chmod dir recursively")
        return False
    change_path_permission(input_path, mode=mode)
    for root, dirs, files in os.walk(input_path):
        for tmp_dir in dirs:
            tmp_dir_path = os.path.join(root, tmp_dir)
            change_path_permission(tmp_dir_path, mode=mode)
        for tmp_file in files:
            if tmp_file != ".snapshot":
                tmp_file = os.path.join(root, tmp_file)
                change_path_permission(tmp_file, mode=mode)
    return True


def mkdir_chmod_chown_dir_recursively(input_path: str, mode, owner, group, pre_dir=False):
    """
    当path不存在，创建path；当pre_dir为True, 从前一级路径修改属主和权限
    """
    if not os.path.exists(input_path):
        if not exec_mkdir_cmd(input_path, is_check_white_list=False):
            log.error(f'execute mkdir cmd failed')
            return False
    target_path = input_path
    if pre_dir:
        target_path = os.path.dirname(input_path)
    chmod_ret = exec_chmod_dir_recursively(target_path, mode=mode)
    chown_ret = exec_lchown_dir_recursively(target_path, owner, group)
    return chmod_ret and chown_ret


def check_repository_path(path_list):
    """
    检查挂载地址是否可以访问，如果不行，则换个挂载地址
    """
    repositories_path = ""
    for path in path_list:
        if can_connect(path):
            repositories_path = path
            break
    log.info(f"Exec check_repository_path success: {repositories_path}.")
    return repositories_path


def can_connect(path):
    """
    检查挂载是否可用
    """
    try:
        process = subprocess.Popen(["/bin/ls", path], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        process.wait(timeout=3)
        ret_code = process.poll()
        if ret_code == CMDResultInt.SUCCESS.value:
            log.info(f"can connect to path: {path}.")
            return True
        else:
            log.error(f"can't connect to path: {path}.")
            return False
    except Exception as e:
        log.error(f"can't connect path:{path}")
        return False


def mkdir_chmod_chown_dir(input_path: str, mode, owner, group):
    if not os.path.exists(input_path):
        if not exec_mkdir_cmd(input_path, is_check_white_list=False):
            log.error(f'execute mkdir cmd failed')
            return False
    chmod_ret = change_path_permission(input_path, mode=mode)
    chown_ret = exec_lchown(input_path, owner, group)
    return chmod_ret and chown_ret


def format_capacity(size):
    if FormatCapacity.MB_SIZE < size < FormatCapacity.GB_SIZE:
        size /= FormatCapacity.MB_SIZE
        return str('%.2f' % size) + "MB"
    elif FormatCapacity.GB_SIZE < size < FormatCapacity.TB_SIZE:
        size /= FormatCapacity.GB_SIZE
        return str('%.2f' % size) + "GB"
    elif FormatCapacity.TB_SIZE < size < FormatCapacity.PB_SIZE:
        size /= FormatCapacity.TB_SIZE
        return str('%.2f' % size) + "TB"
    elif size > FormatCapacity.PB_SIZE:
        size /= FormatCapacity.PB_SIZE
        return str('%.2f' % size) + "PB"
    else:
        return str(size) + "KB"


def count_files(path):
    count = 0
    for _, _, files in os.walk(path):
        count += len(files)
    return count


def get_agent_uuids(file_content):
    agent_infos = file_content.get("job", {}).get('extendInfo', {}).get('agents', [])
    # 备份时集群信息在agents字段中，恢复时的集群信息在targetEnv字段中
    agent_infos = agent_infos if agent_infos else file_content.get("job", {}).get('targetEnv', {}).get('nodes', [])
    agent_uuids = set()
    for agent_info in agent_infos:
        agent_uuids.add(agent_info['id'])
    return list(agent_uuids)


def get_all_data_nodes(cluster_structure):
    log.info("Get all data nodes.")
    data_nodes = []
    for group in cluster_structure.data_nodes.values():
        for _, values in group.items():
            for value in values:
                data_nodes.append(value)
    return data_nodes


def get_id_status_via_priority(status_file, priority, log_msg):
    """
    功能描述：根据管理节点优先级，从任务状态文件中获取对应的uuid和任务状态

    参数：
    status_file (str): 管理节点任务状态文件的路径
    priority (int): 管理节点的优先级
    log_msg (str): 日志信息

    返回值：
    如果成功获取到uuid和任务状态，返回一个元组，包含uuid和任务状态
    如果获取失败，返回一个元组，包含空字符串和SubJobStatusEnum.FAILED.value
    """
    results = read_tmp_json_file(status_file)
    if not results:
        log.error(f'{log_msg}')
        return "", SubJobStatusEnum.FAILED.value
    for agent_id, value in results.items():
        cur_priority = value.get("priority")
        if priority == cur_priority:
            log.info(f'{log_msg} success, agent_id: {agent_id}, priority: {priority}, status: {value.get("status")}.')
            return agent_id, value.get("status")
    log.error(f'{log_msg}, get_id_status_via_priority failed.')
    return "", SubJobStatusEnum.FAILED.value


def check_task_on_all_managers(agent_id, status_file, log_msg, task_infos: TaskInfo):
    """
    功能描述：检查所有管理节点的运行结果，无需考虑优先级，有一个节点运行成功即返回成功，所有管理节点失败，返回失败，状态文件无写操作。
    """
    pid, job_id, sub_job_id, task_type = task_infos.pid, task_infos.job_id, task_infos.sub_job_id, task_infos.task_type
    log.info(f"{log_msg}, {agent_id} exec {task_type} failed, check result on all managers.")
    start_time = time.time()  # 记录开始时间
    while True:
        status_dict = read_tmp_json_file(status_file)
        task_failed = all(
            [value.get('status', SubJobStatusEnum.RUNNING.value) == SubJobStatusEnum.FAILED.value for
             _, value in status_dict.items()])
        task_success = any(
            [value.get('status', SubJobStatusEnum.RUNNING.value) == SubJobStatusEnum.COMPLETED.value for
             _, value in status_dict.items()])
        if task_success:
            log.info(f"{log_msg}, success to exec {task_type}.")
            return True
        if task_failed:
            log.error(f"{log_msg}, failed to exec {task_type} on all managers.")
            return False
        log.info(f"{log_msg}, wait {task_type} result on all managers.")
        time.sleep(Report.STS_CHECK_INTERVAL)
        if time.time() - start_time > Report.TIME_OUT:  # 超时时间为1200小时，单位为秒
            log.error(f"{log_msg}, check {task_type} status on all managers timeout.")
            return False  # 跳出循环


def exec_task_on_all_managers(func, status_file, log_msg, task_infos: TaskInfo, task_params=None):
    """
    功能描述：根据状态文件中的优先级，在各管理节点中运行任务，并更新状态文件。
    """
    pid, job_id, sub_job_id, json_param_object, task_type, log_comm = (
        task_infos.pid, task_infos.job_id, task_infos.sub_job_id,
        task_infos.json_param_object, task_infos.task_type, task_infos.log_comm
    )
    log.info(f"{log_msg}, exec {task_type} on all managers, {log_comm}.")
    results = read_tmp_json_file(status_file)
    if not results:
        log.error(f'{log_msg} failed to get {task_type} status file, {log_comm}.')
        return False
    job_infos = json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                        "").split(" ")
    if len(job_infos) != 4:
        log.error(f'{log_msg} failed to get job info.')
        return False
    cur_agent_id = job_infos[3]
    cur_priority = results.get(cur_agent_id, {}).get('priority')
    if not cur_priority:
        log.error(f'{log_msg} failed to get restore priority for node: {cur_agent_id}.')
        return False
    if cur_priority == ManagerPriority.priority:
        # 当前节点优先级最高，直接执行恢复
        log.info(f'{log_msg} on {cur_agent_id} with highest priority: {cur_priority}.')
        return func(cur_agent_id, status_file, task_params) if task_type == JobNameConst.BACKUP else func(cur_agent_id,
                                                                                                          status_file)
    # 取当前节点前一级的优先级， 检查该优先级对应节点的状态
    pre_priority = cur_priority - 1
    start_time = time.time()
    log.info(f"{log_msg}, check {task_type} on agent with priority {pre_priority}, {log_comm}, from {start_time}.")
    while True:
        pre_id, pre_status = get_id_status_via_priority(status_file, pre_priority,
                                                        f"{log_msg}, get id and status via priority")
        log.info(f'{log_msg}, wait {task_type} result on {pre_id} with priority: {pre_priority}, {log_comm}.')
        if pre_status != SubJobStatusEnum.RUNNING.value:
            log.info(f'{log_msg} finished on {pre_id} with priority: {pre_priority}, {log_comm}.')
            break
        # 等待上一优先级节点的运行结果
        time.sleep(Report.STS_CHECK_INTERVAL)
        if time.time() - start_time > Report.TIME_OUT:  # 超时时间为1200小时，单位为秒
            log.error(f'{log_msg} timeout on {pre_id} with priority: {pre_priority}, {log_comm}.')
            break  # 跳出循环

    new_results = read_tmp_json_file(status_file)
    if pre_status == SubJobStatusEnum.COMPLETED.value:
        # 上一优先级节点任务成功，返回True
        log.info(f'{log_msg} success on {pre_id} with priority: {pre_priority}, {log_comm}.')
        new_results.update({cur_agent_id: {'priority': cur_priority, 'status': SubJobStatusEnum.COMPLETED.value,
                                           'message': '', 'log_detail': 0}})
        output_execution_result_ex(status_file, new_results)
        return True
    else:
        # 上一优先级节点任务失败，本节点开始执行
        log.error(f'{log_msg} failed on {pre_id}, start {task_type} on local {cur_agent_id}, {log_comm}.')
        return func(cur_agent_id, status_file, task_params) if task_type == JobNameConst.BACKUP else func(cur_agent_id,
                                                                                                          status_file)


def get_value_from_ini(path, session, option):
    """
    根据session和option解析配置文件中的信息
    :param path: 配置文件路径
    :param session: 待解析参数所属段
    :param option: 待解析参数所属选项
    :return: str: 字段
    """
    mysql_conf = configparser.ConfigParser(allow_no_value=True, strict=False)
    try:
        mysql_conf.read(path)
    except configparser.Error:
        pass
    try:
        result = mysql_conf.get(session, option)
    except Exception as err:
        log.error(f"Failed to parse ini file {path}, {err}")
        return ""
    return result


def get_etc_ini_path(role_name, file_type, backup_path, job_id):
    """
    根据副本文件类型获取备份根目录。

    参数:
    role_name (str): 节点类型，可以是ZX_MANAGER_NODE、DATA_NODE或GTM_NODE。
    file_type (str): gtm，active（6.1.02版本）的文件路径从ini配置文件中获取。
    backup_path (str): 数据节点，管理节点的数据文件，binlog和metadata， active_info(除6.1.02版本)从PM下发的备份路径中获取。
    job_id (str): 任务ID。

    返回:
    backup_path (str): 备份根目录。

    异常:
    如果节点类型无效，则记录错误日志并返回默认备份路径。
    """
    # 记录信息，获取备份根目录
    log.info(f"Get backup root directory in ini for {file_type}, job_id: {job_id}.")
    role_home = get_user_home(role_name)
    file_infos = FileInfoDict.file_info_dict.get(file_type, {})
    ini_name = file_infos.get("ini_name", "")
    section = file_infos.get("section", "")
    field = file_infos.get("field", "")
    ini_path = os.path.join(role_home, "etc", ini_name)
    prod_root = get_value_from_ini(ini_path, section, field)
    if prod_root.strip():
        # 如果备份根目录不为空，记录信息并更新备份路径
        backup_path = prod_root.strip()
        log.info(f"Update bkp root for {file_type}, {role_name}: {backup_path}, job_id: {job_id}.")
    return backup_path


def get_agent_err_code_from_sts_file(agent_id, sts_file, task_type):
    """
    从sts文件中获取指定agent的错误详情

    参数:
    agent_id (str): agent的唯一标识符
    sts_file (str): 状态文件的路径
    task_type (str): 任务类型（备份/恢复）

    返回值:
    LogDetail对象, 包含三个关键字段, log_info: str, 标签; log_detail_param: list, 错误参数; log_detail: int, 错误码;
    """
    # 从sts文件中获取agent的错误详情
    results = read_tmp_json_file(sts_file)
    log_detail_param = results.get(agent_id, {}).get('log_detail_param')
    # 错误码默认使用ErrorCode.ERROR_INTERNAL
    log_detail = results.get(agent_id, {}).get('log_detail', ErrorCode.ERROR_INTERNAL)
    # 标签默认使用ReportDBLabel.SUB_JOB_FALIED
    log_info = results.get(agent_id, {}).get('log_info', ReportDBLabel.SUB_JOB_FALIED)
    log.error(f"{task_type} failed on {agent_id}, code: {log_detail}, param: {log_detail_param}, label: {log_info}.")
    return LogDetail(logInfo=log_info, logDetail=log_detail, logDetailParam=log_detail_param)


def get_recognized_err_from_sts_file(loc_agent_id, sts_file, task_type):
    """
    根据状态文件，返回报错详情，优先处理已识别出的报错，防止前端报错信息被覆盖

    参数:
    agent_id (str): agent的唯一标识符
    sts_file (str): 状态文件的路径
    task_type (str): 任务类型（备份/恢复）

    返回值:
    LogDetail对象, 包含三个关键字段, log_info: str, 标签; log_detail_param: list, 错误参数; log_detail: int, 错误码;
    """
    # 根据状态文件，返回报错详情，优先处理已识别出的报错，防止前端报错信息被通用报错信息覆盖
    log.error(f"{task_type} failed on {loc_agent_id}, start to get log details.")
    loc_log_detail_model = get_agent_err_code_from_sts_file(loc_agent_id, sts_file, task_type)
    loc_log_detail = loc_log_detail_model.log_detail
    results = read_tmp_json_file(sts_file)
    for agent_id, record in results.items():
        # agent记录的报错为未识别的通用报错，同时找到有已识别的报错时，更新当前agent记录的报错信息
        if (record.get('log_detail') in [ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL, ErrorCode.ERR_BKP_CHECK]
                and loc_log_detail not in [ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL, ErrorCode.ERR_BKP_CHECK]):
            log_detail = record.get('log_detail')
            log_detail_param = record.get('log_detail_param', [task_type, f"{task_type} failed, check dbtool log."])
            log_info = record.get('log_info', ReportDBLabel.SUB_JOB_FALIED)
            log.error(f"Update {task_type} err info on {loc_agent_id} with dbtool err from {agent_id}.")
            return LogDetail(logInfo=log_info, logDetail=log_detail, logDetailParam=log_detail_param)
    log.error(f"{task_type} failed with LogDetail: {loc_log_detail_model}.")
    return loc_log_detail_model


def update_agent_sts(agent_id, sts_file, sts_info: StatusInfo):
    """
    更新指定agent_id的状态信息

    参数:
    agent_id (str): 需要更新状态的agent的uuid
    sts_file (str): 存储状态信息的临时json文件
    sts_info (StatusInfo): 管理节点任务状态文件中，待更新的信息
    """
    log.info(f"Start to update {agent_id}'s record.")
    results = read_tmp_json_file(sts_file)
    # 获取指定uuid的任务状态记录
    record = results.get(agent_id, {})
    # 更新该uuid的任务状态记录
    record.update(
        {'status': sts_info.status, 'log_detail_param': sts_info.log_detail_param, 'log_detail': sts_info.log_detail,
         'log_info': sts_info.log_info})
    log.info(f"Record of {agent_id} is updated, {record}.")
    # 更新结果文件
    results.update({agent_id: record})
    output_execution_result_ex(sts_file, results)


def update_agent_sts_general_after_exec(agent_id, sts_file, task_type, exec_result, log_comm):
    """
    根据子任务的执行结果更新状态文件。有报错时，只处理未识别的报错，防止漏更新。能识别的报错已更新至状态文件。

    参数:
    agent_id (str): 代理ID
    sts_file (str): 状态文件路径
    task_type (str): 任务类型 （Backup/Restore）
    exec_result (bool): 执行结果，True表示成功，False表示失败
    log_comm (str): 常用日志信息，例如job_id, subjob_id, req_id
    """
    if exec_result:
        # 子任务成功，状态完成，无错误码，无报错参数
        log.info(f'{task_type} success on {agent_id}, {log_comm}.')
        sts_info = StatusInfo(status=SubJobStatusEnum.COMPLETED.value, logInfo=ReportDBLabel.SUB_JOB_SUCCESS)
        update_agent_sts(agent_id, sts_file, sts_info)
    else:
        # 子任务失败，由于已识别的错误已经在执行子任务的过程中更新至状态文件，需要处理未识别的报错
        log.error(f'{task_type} failed on {agent_id}, {log_comm}.')
        results = read_tmp_json_file(sts_file)
        record = results.get(agent_id, {})
        log_detail = record.get('log_detail', ErrorCode.ERROR_INTERNAL)
        # 通过错误码判断该代理对应的报错信息是否为已识别的报错
        if log_detail not in RECOGNIZED_DBTOOL_ERRORS:
            if task_type == JobNameConst.RESTORE:
                log_info = ReportDBLabel.RESTORE_SUB_FAILED
            else:
                log_info = ReportDBLabel.BACKUP_SUB_FAILED
            log.error(f'Update {task_type} unrecognized error on {agent_id}, {log_comm}.')
            # 子任务失败，状态失败，默认错误码为ErrorCode.ERROR_INTERNAL，无报错参数
            sts_info = StatusInfo(status=SubJobStatusEnum.FAILED.value, logDetail=ErrorCode.ERROR_INTERNAL,
                                  logInfo=log_info)
            update_agent_sts(agent_id, sts_file, sts_info)


def report_action_result(req_id, job_id, sub_job_id, result, message):
    """
    上报数据库环境异常，根据报错信息，通过ActionResult结构体，生成输出文件，完成被动上报。

    参数:
    req_id (str): 请求ID
    job_id (str): 任务ID
    sub_job_id (str): 子任务ID
    result (bool): 执行结果，True表示成功，False表示失败
    message (str): 执行结果的消息

    异常描述:
    输出文件已存在，output_result_file会抛出异常。
    """
    if result:
        log.info(f"{message}, req_id: {req_id}, job_id: {job_id} ,sub_job_id: {sub_job_id}")
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
    else:
        log.error(f"{message}, req_id: {req_id}, job_id: {job_id} ,sub_job_id: {sub_job_id}")
        response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                bodyErr=ErrorCode.ERR_ENVIRONMENT,
                                message=message)
    output_result_file(req_id, response.dict(by_alias=True))


def report_subjob_via_rpc(req_id, sub_job_model: SubJobModel, key_log_details: LogDetail):
    """
    上报子任务（含前置任务）完成状态，根据日志详情，通过LogDetail，SubJobDetails结构体，更新输入文件，使用rpc工具，完成主动上报。

    参数:
    req_id (str): 请求ID
    sub_job_model (SubJobModel对象): job_id: str, 任务ID; sub_job_id: str, 子任务ID;
    key_log_details (LogDetail对象): log_info: str, 标签; log_detail_param: list, 错误参数; log_detail: int, 错误码;

    异常描述:
    输入文件（通过req_id生成）已存在，report_job_details会抛出异常。
    """
    job_id, sub_id = sub_job_model.job_id, sub_job_model.sub_job_id
    # 标签为子任务执行成功，副本校验成功，执行前置任务成功，子任务备份成功时，结果为成功
    result = True if key_log_details.log_info in LOG_DETAIL_SUCCESS else False
    if result:
        log.info(f"Task success, LogDetail: {key_log_details}, req_id: {req_id}, job_id: {job_id}, sub_job: {sub_id}.")
        log_detail_model = LogDetail(logInfo=key_log_details.log_info, logInfoParam=[sub_id],
                                     logLevel=DBLogLevel.INFO.value)
        sub_job_details = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail_model],
                                        taskStatus=SubJobStatusEnum.COMPLETED.value)
    else:
        log.error(f"Task failed, LogDetail: {key_log_details}, req_id: {req_id}, job_id: {job_id}, sub_job: {sub_id}.")
        # 标签为空，默认使用子任务失败标签
        log_info = ReportDBLabel.SUB_JOB_FALIED if not key_log_details.log_info else key_log_details.log_info
        # logDetailParam类型不为list，默认设置为None，防止上报出错造成卡死
        log_detail_param = None if not isinstance(key_log_details.log_detail_param,
                                                  list) else key_log_details.log_detail_param
        log_detail_model = LogDetail(logInfo=log_info, logInfoParam=[sub_id], logLevel=DBLogLevel.ERROR,
                                     logDetail=key_log_details.log_detail, logDetailParam=log_detail_param)
        sub_job_details = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail_model],
                                        taskStatus=SubJobStatusEnum.FAILED.value)
    report_job_details(req_id, sub_job_details)


def check_exec_on_cm(role_name, job_id):
    # 判断当前goldendb的dbtool备份恢复任务在cm中还是mds中运行
    backup_cmd = f'su - {role_name} -c "dbtool -mds -backup"'
    return_code, return_info, err_info = execute_cmd(backup_cmd)
    log.info(f"Get return_code: {return_code}, return_info: {return_info}, err_info: {err_info}, job id: {job_id}.")
    if ErrCodeMessage.DBTOOL_MDS_BACKUP_CAN_NOT_USE in return_info:
        log.info(f"Will exec job id: {job_id} on cm.")
        return True
    else:
        log.info(f"Will exec job id: {job_id} on mds.")
        return False


def check_dbtool_result_when_success(out_info):
    dbtool_task_id = ""
    success_flag = False
    for out_info_line in out_info.split("\n"):
        if "Successful response:" in out_info_line:
            success_flag = True
            continue
        if success_flag:
            dbtool_task_id = out_info_line.strip()
            log.info(f"Exec dbtool success, dbtool_task_id: {dbtool_task_id}.")
            return True, dbtool_task_id
    return success_flag, dbtool_task_id


def check_dbtool_start(req_id, job_id, sub_id, copy_feature, action):
    log.info(f"Get {action} copy feature {copy_feature}, req_id:{req_id}, job_id: {job_id}, sub_id:{sub_id}.")
    dbtool_task_id = ""
    log_detail = LogDetail()
    if copy_feature[0] != CMDResult.SUCCESS.value:
        if "Failure response:" in copy_feature[1]:
            log.error(f"Find failure response in dbtool {action}, req_id:{req_id}, job_id: {job_id}, sub_id:{sub_id}.")
            message = copy_feature[1].split("Failure response:", 1)[1].replace("\\r\\n", " ").rstrip(
                "'").strip()
            log.error(f"message: {message}, req_id:{req_id}, job_id: {job_id}, sub_id:{sub_id}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                   logDetailParam=[action, message])
        else:
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=0)
        log.error(f"Exec {action} failed with {log_detail}, req_id:{req_id}, job_id: {job_id}, sub_id: {sub_id}.")
        return False, dbtool_task_id, log_detail
    else:
        if "Successful response:" in copy_feature[1]:
            log.info(f"Find success response in dbtool {action}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
            success_flag, dbtool_task_id = check_dbtool_result_when_success(copy_feature[1])
    log.info(f"{action} finished: {success_flag}, dbtool_task_id: {dbtool_task_id}, job_id: {job_id}.")
    return success_flag, dbtool_task_id, log_detail


def check_cm_dbtool_sts(copy_feature, action, job_id, sub_id):
    log.info(f"Get {action} copy feature: {copy_feature}, job_id: {job_id}, sub_id: {sub_id}.")
    bkp_status = DbtoolTaskStatus.RUNNING
    if copy_feature[0] != CMDResult.SUCCESS.value:
        if "Failure response:" in copy_feature[1]:
            message = copy_feature[1].split("Failure response:", 1)[1].replace("\\r\\n", " ").rstrip(
                "'").strip()
            log.error(f"message: {message}, job_id: {job_id}, sub_id:{sub_id}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                   logDetailParam=[action, message])
        else:
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=0)
        return False, DbtoolTaskStatus.FAILED, log_detail
    else:
        result, bkp_status, log_detail = check_cm_dbtool_out_str(bkp_status, copy_feature, action, job_id, sub_id)
    return result, bkp_status, log_detail


def check_cm_dbtool_out_str(dbtool_status, copy_feature, action, job_id, sub_id):
    log.info(f"Start check_cm_dbtool_out_str for {action}, job_id: {job_id}, sub_id: {sub_id}.")
    log_detail = LogDetail()
    if action == JobNameConst.BACKUP:
        dbtool_is_going = BackupResultMessage.BACKUP_IS_GOING
        dbtool_is_done = BackupResultMessage.BACKUP_IS_DONE
    else:
        dbtool_is_going = RestoreResultMessage.RESTORE_IS_GOING
        dbtool_is_done = RestoreResultMessage.RESTORE_IS_DONE
    for out_info_line in copy_feature[1].split("\n"):
        if "ResultDesc:" in out_info_line:
            result_desc = out_info_line.split(":", 1)[1].strip()
            if dbtool_is_going in result_desc:
                dbtool_status = DbtoolTaskStatus.RUNNING
            elif dbtool_is_done in result_desc:
                dbtool_status = DbtoolTaskStatus.COMPLETED
            else:
                log.error(f"{action} failed: {result_desc}, job_id: {job_id}, sub_id: {sub_id}.")
                dbtool_status = DbtoolTaskStatus.FAILED
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                       logLevel=DBLogLevel.ERROR.value,
                                       logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                       logDetailParam=[action, result_desc])
                return False, dbtool_status, log_detail
            log.info(f"{action} status: {dbtool_status}, des: {result_desc}, job_id: {job_id}, sub_id: {sub_id}.")
            return True, dbtool_status, log_detail
    log.error(f"Not find ResultDesc in {action} result, job_id: {job_id}, sub_id: {sub_id}.")
    log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                           logLevel=DBLogLevel.ERROR.value,
                           logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                           logDetailParam=[JobNameConst.BACKUP, f"Not find ResultDesc in {action} result"])
    return False, dbtool_status, log_detail


def get_dbtool_cm_result(action, role_name, dbtool_start_results, job_id, sub_id):
    result, dbtool_task_id, log_detail = dbtool_start_results
    log.info(f"Check {action} result: {result}, {action} id {dbtool_task_id}, job id: {job_id}.")
    if not result:
        log.error(f"Check {action} result fail, job id: {job_id}.")
        return False, log_detail
    dbtool_task_type = "-query-backup-task" if action == JobNameConst.BACKUP else "-query-restore-task"
    # 轮询备份结果，直到任务成功或失败
    while True:
        query_cmd = f'su - {role_name} -c "dbtool -cm {dbtool_task_type} -taskid={dbtool_task_id}"'
        log.info(f"Get {action} query cmd: {query_cmd}, job id: {job_id}.")
        return_code, return_info, err_info = execute_cmd(query_cmd)
        log.info(f"Get {action} return code {return_code}, return_info {return_info}, err_info: {err_info}")
        result, bkp_sts, log_detail = check_cm_dbtool_sts((return_code, return_info), action, job_id, sub_id)
        log.info(f"{action} status: {bkp_sts}, result: {result}, job id: {job_id}.")
        if not result or bkp_sts == DbtoolTaskStatus.FAILED:
            log.error(f"{action} failed, log_detail: {log_detail}, job id: {job_id}.")
            return False, log_detail
        if result and bkp_sts == DbtoolTaskStatus.COMPLETED:
            break
        time.sleep(Report.REPORT_INTERVAL / 2)  # 15秒检查一次
    log.info(f"{action} success, result: {result}, job id: {job_id}.")
    # 检查正确，无错误码，无错误参数，返回成功标签
    return True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)


def get_err_log_detail_from_echo(err_message, task_type):
    """
    根据错误信息和任务类型，获取错误日志的详细信息

    参数:
    err_message: str, cmd回显的错误信息
    task_type: str, 任务类型（备份/恢复）

    返回:
    LogDetail对象，关键字段, log_info: str, 标签; log_detail_param: list, 错误参数; log_detail: int, 错误码;
    """
    log.info(f"{task_type} failed, start to get log detail based on cmd echo.")
    log_info = ReportDBLabel.SUB_JOB_FALIED
    log_detail_param = None
    if ErrPattern.Auth_Check_Fail in err_message:
        log_detail = ErrorCode.ERROR_LOGIN_INFO
    else:
        log_detail = ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL
        log_detail_param = [task_type, err_message]
    return LogDetail(logInfo=log_info, logDetail=log_detail, logDetailParam=log_detail_param)


def shutil_copy_tree(src_dir, dest_dir, msg):
    log.info(f"Start {msg}.")
    if not os.path.exists(dest_dir):
        log.info(f"{msg}, {dest_dir} not exists.")
        exec_mkdir_cmd(dest_dir)
    if os.path.exists(src_dir):
        try:
            shutil.copytree(src_dir, dest_dir, dirs_exist_ok=True)
        except Exception as e:
            log.error(f"{msg} failed: {e}", exc_info=True)
            return False
    log.info(f"{msg} success.")
    return True


def shutil_copy_file(src_file, dest_file, msg):
    log.info(f"Start {msg}.")
    try:
        shutil.copy(src_file, dest_file)
    except FileNotFoundError as no_found_err:
        log.error(f"{msg} failed: {no_found_err}.", exc_info=True)
        return False
    except PermissionError as permission_err:
        log.error(permission_err)
        log.error(f"{msg} failed: {permission_err}.", exc_info=True)
        return False
    except Exception as e:
        log.error(f"{msg} failed: {e}.", exc_info=True)
        return False
    log.info(f"{msg} success.")
    return True


def get_active_tx_info_files(base_path):
    matching_files = []
    pattern = r"DBCluster_\d+_Active_TX_info\.\d{14}(\.current)?"
    # 遍历指定路径及其子目录
    for root, _, files in os.walk(base_path):
        for file in files:
            if re.match(pattern, file):
                matching_files.append(os.path.join(root, file))
    return matching_files


def is_valid_time(time_str, time_format='%Y-%m-%d %H:%M:%S'):
    """
    判断字符串是否符合指定的时间格式
    :param time_str: 需要判断的字符串
    :param time_format: 时间格式，默认为 '%Y-%m-%d %H:%M:%S'（yyyy-mm-dd HH:MM:SS）
    :return: 如果符合时间格式返回 True，否则返回 False
    """
    try:
        datetime.strptime(time_str, time_format)
    except ValueError as exp:
        log.error(f"{time_str} is invalid: {exp}.", exc_info=True)
        return False
    return True


def get_gtid_timestamps(file_path):
    active_tx_info = ""
    with open(file_path, 'r') as f:
        active_tx_info = f.read()
    active_tx_records = [item.strip() for item in active_tx_info.split('\n') if item.strip()]
    act_gtid_timestamps = []
    for active_tx_record in active_tx_records:
        # 获取活跃事务信息
        if active_tx_record[0] == ActTxInfoConsts.active_tx_info:
            act_tx_infos = active_tx_record.split()
            if len(act_tx_infos) < ActTxInfoConsts.min_act_tx_info_len:
                log.error(f"Active_Tx_Info record is invalid: {active_tx_record}.")
                continue
            consistent_time = act_tx_infos[ActTxInfoConsts.consistent_date_idx:ActTxInfoConsts.consistent_time_idx + 1]
            act_tx_consistent_time = ' '.join(consistent_time)
            if is_valid_time(act_tx_consistent_time):
                log.info(f"Get active_tx consistent time success, {act_tx_consistent_time}.")
                # 将字符串转换为datetime对象
                dt_object = datetime.strptime(act_tx_consistent_time, '%Y-%m-%d %H:%M:%S')
                # 将datetime对象转换为时间戳
                timestamp = dt_object.timestamp()
                act_gtid_timestamps.append(int(timestamp))
            else:
                log.error(f"Get invalid active_tx consistent time: {act_tx_consistent_time} from {active_tx_record}.")
                continue
    act_gtid_timestamps = list(set(act_gtid_timestamps))
    act_gtid_timestamps.sort()
    log.info(f"Get active gtid collection timestamps success, total number: {len(act_gtid_timestamps)}.")
    return act_gtid_timestamps


def rm_redundant_incr_result_info(cid_path, result_info, pre_bkp_files):
    """
    判断输入的文件是否为冗余文件，并删除。
    如果该文件备份前已存在于生产端，且该文件为增量备份任务生成，需要在文件系统中，删除该文件。

    参数:
    cid_path: 文件系统中的集群ID路径
    result_info: 文件系统中的备份结果文件
    pri_bkp_files: 本次备份前，生产环境中的文件列表

    返回值:
    如果删除成功，返回True，否则返回False

    异常描述:
    如果无法备份任务文件夹，或者删除冗余增量备份任务文件夹或结果文件失败，将记录错误日志并返回False
    """
    # 判断该文件是否为本次备份前存在的文件
    if result_info in pre_bkp_files:
        # 获取该备份结果信息在文件系统中的全路径：数据仓/DBCluster_{cid}/{cid}_backup_resultsinfo.{tid}
        result_info_path = os.path.join(cid_path, result_info)
        # 获取当前文件的备份类型
        bkp_file_type = get_bkp_type_from_result_info(result_info_path)
        if bkp_file_type == 'INCREMENTAL':
            # 读文件系统中的备份结果文件，获取对应的增量备份任务（时间戳）路径：数据仓/DBCluster_{cid}/DATA_BACKUP/{tid}
            ret, task_path = get_task_path_from_result_info(result_info_path, cid_path)
            if not ret:
                log.error(f'Backup task folders {task_path} not exists.')
            # 在文件系统中，删除冗余增量备份task id（时间戳）文件夹
            if not su_exec_rm_cmd(task_path):
                log.error(f'Remove redundant incremental backup task folders {task_path} failed.')
            # 在文件系统中，删除冗余增量备份结果文件
            if not su_exec_rm_cmd(result_info_path):
                log.error(f'Remove redundant incremental backup info files {result_info_path} failed.')
