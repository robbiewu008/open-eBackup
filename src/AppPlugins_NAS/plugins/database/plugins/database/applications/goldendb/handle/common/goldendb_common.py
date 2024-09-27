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
import subprocess
from base64 import b64decode

import psutil

from goldendb.logger import log
from common.common import check_command_injection_exclude_quote, check_path_legal, execute_cmd
from common.const import EnumPathType, ParamConstant
from common.exception.common_exception import ErrCodeException
from common.file_common import exec_lchown_dir_recursively, check_file_or_dir, change_path_permission, get_user_info, \
    exec_lchown
from common.util.check_user_utils import check_os_user
from common.util.exec_utils import exec_append_newline_file, exec_overwrite_file, exec_umount_cmd, exec_mount_cmd, \
    exec_mkdir_cmd, check_path_valid, ExecFuncParam, su_exec_cmd_list, exec_cp_dir_no_user, read_lines_cmd, \
    su_exec_rm_cmd, exec_cat_cmd
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure
from goldendb.handle.common.const import RpcParamKey, CMDResult, GoldenDBPath, MountBindPath, GetIPConstant, \
    FormatCapacity


def write_progress_file(message: str, file_name: str):
    log.info(f'Write message into progress file: {message}')
    exec_append_newline_file(file_name, message)


def write_progress_file_ex(message: str, file_name: str):
    # 先将进度文件清空在写
    content = f"{message}\n"
    exec_overwrite_file(file_name, content, json_flag=False)


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

    if ret != CMDResult.SUCCESS:
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


def umount_bind_path_list(paths: list[str]):
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

    if return_code != CMDResult.SUCCESS:
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

    if return_code != CMDResult.SUCCESS:
        log.error(f"Failed to exec mount bind path err: {std_err}!src path={src_area}, dest path={des_area}")
        return False
    log.info(f"Succeed to exec mount bind path. src path={src_area}, dest path={des_area}")

    exec_append_newline_file(os.path.join(ParamConstant.RESULT_PATH, f"goldendb_mount_list_{job_id}"), des_area)

    return True


def mount_bind_backup_path(data_area, meta_area, job_id):
    if not mount_bind_path(data_area, os.path.join(MountBindPath.DATA_FILE_PATH, job_id), job_id):
        log.error("Mount bind data path failed")
        return False
    if meta_area:
        if not mount_bind_path(meta_area, os.path.join(MountBindPath.META_FILE_PATH, job_id), job_id):
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

    if return_code != int(CMDResult.SUCCESS):
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
    decode_str = str(b64decode(encode_str))
    dir_re = r'backup_rootdir =(.*?)\\n'
    backup_root_dir = re.findall(dir_re, decode_str)
    if backup_root_dir:
        return backup_root_dir[0]
    else:
        return ''


def get_backup_path(role_name, role_type, file_content, json_const):
    source_backup_dir = get_data_node_backup_dir(
        file_content["job"][f"{json_const}"]["extendInfo"]["local_ini_cnf"])
    source_backup_dir = get_local_ini_path(role_name, source_backup_dir, "backup_root")
    log.debug(f"Get source {role_type} backup dir success.")
    return source_backup_dir


def get_local_ini_path(role_name, source_backup_dir, default_root):
    if check_command_injection_exclude_quote(role_name) or not check_os_user(role_name):
        log.error("command injection in role_name! The role_name(%s) is invalid.", role_name)
        return ''
    find_home_cmd = f'su - {role_name} -c "pwd"'
    return_code, role_name_home = subprocess.getstatusoutput(find_home_cmd)
    if source_backup_dir == "":
        source_backup_dir = os.path.join(role_name_home, default_root)
    elif source_backup_dir.startswith("$HOME/"):
        backup_root = source_backup_dir.split("$HOME/")[1]
        source_backup_dir = os.path.join(role_name_home, backup_root)
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

    if not check_path_valid(src_bkp_file_path) or not check_path_valid(des_bkp_folder_path):
        return False
    path_type = check_file_or_dir(src_bkp_file_path)
    if path_type not in (EnumPathType.DIR_TYPE, EnumPathType.FILE_TYPE):
        log.error(f"Src path is invalid type: {path_type} can not copy ")
        return False

    cp_cmd = f'cp -a {src_bkp_file_path} {des_bkp_folder_path}'
    return_code, out_info, err_info = execute_cmd(cp_cmd)
    if int(return_code) != int(CMDResult.SUCCESS):
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
    读取result_info_path，获取生成该文件的备份任务对应的task path
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
    log.info(f"remove task_path:{task_path}")
    if os.path.exists(task_path):
        return True, task_path
    else:
        return False, ""


def get_bkp_files_in_cluster_id_folder(backup_path, file_content):
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
        pre_bkp_files = [
            f
            for f in os.listdir(dbcluster_path)
            if os.path.isfile(os.path.join(dbcluster_path, f)) and f.startswith(f'{cluster_id}_backup')
        ]
    else:
        return []
    return pre_bkp_files


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
        RpcParamKey.DB_BIN_PATH,
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
            path_list = repository.get("path", [])
            repositories_path = check_repository_path(path_list)
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
    return ret == CMDResult.SUCCESS, out


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
            tmp_file = os.path.join(root, tmp_file)
            change_path_permission(tmp_file, mode=mode)
    return True


def mkdir_chmod_chown_dir_recursively(input_path: str, mode, owner, group, pre_dir=False):
    if not os.path.exists(input_path):
        if not exec_mkdir_cmd(input_path, is_check_white_list=False):
            log.error(f'execute mkdir cmd failed')
            return False
    if pre_dir:
        target_path = os.path.dirname(input_path)
    else:
        target_path = input_path
    chmod_ret = exec_chmod_dir_recursively(target_path, mode=mode)
    chown_ret = exec_lchown_dir_recursively(target_path, owner, group)
    return chmod_ret and chown_ret


def check_repository_path(path_list: list[str]):
    """
    检查挂载地址是否可以访问，如果不行，则换个挂载地址
    """
    repositories_path = ""
    for path in path_list:
        if can_connect(path):
            repositories_path = path
            break
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
        if ret_code == 0:
            return True
        else:
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
    return str(count)
