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
import platform
import re
import shutil
import signal
import socket
import stat
import time
import uuid

from common.const import SubJobStatusEnum, ParamConstant, RepositoryDataTypeEnum
from common.common import execute_cmd, output_execution_result_ex
from common.util.check_user_utils import check_os_user
from common.util.check_utils import check_repo_path
from common.util.exec_utils import exec_overwrite_file, check_path_valid, exec_append_file, exec_append_newline_file, \
    exec_cat_cmd, su_exec_rm_cmd
from openGauss.common.error_code import NormalErr
from openGauss.common.const import logger, RpcParamKey, ResultCode, ProgressInfo, WhitePath, OpenGaussDeployType, \
    ProtectObject
from openGauss.common.const import ParamKey

if platform.system().lower() == "linux":
    import pwd
    import grp

INJECTION_CHAR_LIST = (
    "|", ";", "&", "$", "<", ">", "`", "\\", "'", "\"", "{", "}", "(", ")",
    "[", "]", "~", "*", "?", " ", "!", "\n"
)


def path_check(path):
    return os.path.exists(path) and not os.path.islink(path)


def check_user_name(user_name):
    try:
        uid, _ = get_ids_by_name(user_name)
    except Exception as e_obj:
        return False
    return True


def check_file_owner(path, owner):
    """
    检测文件是否属于属主
    :param path:
    :param owner:
    :return:
    """
    try:
        f_uid = os.lstat(path).st_uid
    except Exception as e_obj:
        logger.error(f"Get path owner info failed, exception: {e_obj}")
        return False
    try:
        o_uid, _ = get_ids_by_name(owner)
    except Exception as e_obj:
        logger.error("Get user info failed, not found user with err %s", str(e_obj))
        return False
    return f_uid == o_uid


def get_base_dir(path):
    if not path_check(path):
        return ""
    base_dir = os.path.dirname(path)
    return base_dir


def join_path(base_path, *paths):
    return os.path.join(base_path, *paths)


def get_hostname():
    return socket.gethostname()


def get_hostname_by_addr(addr):
    try:
        hostname, *_ = socket.gethostbyaddr(addr)
    except ConnectionError:
        hostname = "localhost"
    except socket.herror:
        hostname = "localhost"
    return hostname


def get_host_ip(hostname):
    try:
        host_ip = socket.gethostbyname(hostname)
    except Exception:
        host_ip = ""
    return host_ip


def set_uuid(*args):
    string = "".join([str(arg) for arg in args])
    return str(uuid.uuid5(uuid.NAMESPACE_X500, string))


def get_repository_dir(repositories, repository_type):
    find_type = repository_type
    if repository_type == RepositoryDataTypeEnum.LOG_META_REPOSITORY.value:
        find_type = RepositoryDataTypeEnum.META_REPOSITORY.value
    for repository_info in repositories:
        ret, present_repository_type = get_value_from_dict(repository_info, ParamKey.REPOSITORY_TYPE)
        if present_repository_type != find_type:
            continue
        ret, path_list = get_value_from_dict(repository_info, ParamKey.PATH)
        if not ret or len(path_list) <= 0:
            return ""
        repository_dir = path_list[0]
        if repository_type == RepositoryDataTypeEnum.LOG_META_REPOSITORY.value and "log" not in repository_dir:
            continue
        if os.path.isdir(repository_dir) and check_path(repository_dir) \
                and repository_dir.startswith(WhitePath.MOUNT_PATH):
            return repository_dir
    return ""


def copy_sub_dir_backup_name(src_dir, dest_dir, backup_name, start_time=0):
    logger.info(f"Start copy packing info, src dir {src_dir}, dest dir {dest_dir}, backup name {backup_name}")
    backup_path = os.path.join("backups", "panwei", backup_name)
    wal_path = os.path.join("wal", "panwei")

    # 创建目标目录
    os.makedirs(dest_dir, exist_ok=True)

    # 拷贝日志起始时间
    copy_start_time = int(time.time()) - (ParamKey.DAY * ParamKey.HOUR * ParamKey.MINUTE)
    if start_time != 0:
        copy_start_time = start_time - (ParamKey.HALF_HOUR * ParamKey.MINUTE)
    logger.info(f"Copy start time: {copy_start_time}")

    # 遍历源目录下的所有数据库组件目录
    for db_component in os.listdir(src_dir):
        src_backup_path = os.path.join(src_dir, db_component, backup_path)
        dest_backup_path = os.path.join(dest_dir, db_component, backup_path)
        # 检查源备份路径是否存在
        if os.path.isdir(src_backup_path):
            # 创建目标备份路径的父目录
            os.makedirs(os.path.dirname(dest_backup_path), exist_ok=True)
            # 复制备份文件夹
            copy_backup_file(dest_backup_path, src_backup_path)
        else:
            logger.info(f"Sub dir {src_backup_path} not exist")

        # 拷贝WAL文件
        src_wal_path = os.path.join(src_dir, db_component, wal_path)
        dest_wal_path = os.path.join(dest_dir, db_component, wal_path)
        if os.path.isdir(src_wal_path):
            os.makedirs(dest_wal_path, exist_ok=True)
            deal_log_files(copy_start_time, dest_wal_path, src_wal_path)
        else:
            logger.info(f"Wal dir {src_wal_path } not exist")
    logger.info(f"Copy packing info success")


def copy_backup_file(dest_backup_path, src_backup_path):
    try:
        shutil.copytree(src_backup_path, dest_backup_path)
        logger.info(f"Copy dir success, src path {src_backup_path}, dest path {dest_backup_path}")
    except FileExistsError as err:
        logger.error(f"Copy dir failed, src path {src_backup_path}, dest path {dest_backup_path}, {err}")


def deal_log_files(copy_start_time, dest_wal_path, src_wal_path):
    for log_file in os.listdir(src_wal_path):
        src_log_file_path = os.path.join(src_wal_path, log_file)
        if os.path.isfile(src_log_file_path) and os.stat(src_log_file_path).st_mtime > copy_start_time:
            dest_log_file_path = os.path.join(dest_wal_path, log_file)
            shutil.copy2(src_log_file_path, dest_log_file_path)


def merge_backups(src_dirs, dest_dir, cluster_name: str = "panwei"):
    """
    将多个源目录合并到目标目录.

    Args:
        src_dirs: 源目录路径列表.
        dest_dir: 目标目录路径.
        cluster_name: 实例名
    """
    logger.info(f"Start to copy src dirs {src_dirs} to dest dir {dest_dir}")
    backup_path = os.path.join("backups", cluster_name)
    wal_path = os.path.join("wal", cluster_name)
    for src_dir in src_dirs:
        if not os.path.isdir(src_dir):
            logger.info(f"Merge packing dir not exist {src_dir}")
            continue

        # 遍历源目录下的所有数据库归档文件
        copy_sub_dir(backup_path, dest_dir, src_dir, wal_path)
    logger.info(f"Merge packing info success")


def copy_sub_dir(backup_path, dest_dir, src_dir, wal_path):
    for db_component in os.listdir(src_dir):
        src_backup_path = os.path.join(src_dir, db_component, backup_path)
        dest_backup_path = os.path.join(dest_dir, db_component, backup_path)
        # 检查源备份路径是否存在
        if os.path.isdir(src_backup_path):
            # 创建目标备份路径的父目录
            os.makedirs(os.path.dirname(dest_backup_path), exist_ok=True)
            # 复制备份文件夹
            try:
                copy_first_level(src_backup_path, dest_backup_path)
                logger.info(f"Copy backup dir success, src path {src_backup_path}, dest path {dest_backup_path}")
            except Exception as err:
                logger.error(f"Copy backup dir failed, src path {src_backup_path}, dest path {dest_backup_path}, {err}")
        else:
            logger.info(f"Backup sub dir {src_backup_path} not exist")

        # 拷贝WAL文件
        src_wal_path = os.path.join(src_dir, db_component, wal_path)
        dest_wal_path = os.path.join(dest_dir, db_component, wal_path)
        if os.path.isdir(src_wal_path):
            os.makedirs(dest_wal_path, exist_ok=True)
            # 复制备份文件夹
            try:
                copy_first_level(src_wal_path, dest_wal_path)
                logger.info(f"Copy wal dir success, src path {src_wal_path}, dest path {dest_wal_path}")
            except Exception as err:
                logger.error(f"Copy wal dir failed, src path {src_wal_path}, dest path {dest_wal_path}, {err}")
        else:
            logger.info(f"Wal dir {src_wal_path} not exist")


def copy_first_level(source_dir, destination_dir):
    """
    复制源目录第一层的文件和文件夹到目标目录，保留目标目录原有文件和文件夹。

    Args:
        source_dir: 源目录路径。
        destination_dir: 目标目录路径。
    """
    if not os.path.exists(source_dir):
        logger.error(f"Source dir {source_dir} not exist")
        return

    if not os.path.exists(destination_dir):
        logger.error(f"Target dir {destination_dir} not exist")
        return

    for item in os.listdir(source_dir):
        source_path = os.path.join(source_dir, item)
        destination_path = os.path.join(destination_dir, item)

        if os.path.isfile(source_path):
            if not os.path.exists(destination_path):
                shutil.copy2(source_path, destination_path)
                logger.info(f"Copy success, source file: {source_path}, target file: {destination_path}")
            else:
                logger.info(f"target file exist: {destination_path}, skip")
        elif os.path.isdir(source_path):
            if not os.path.exists(destination_path):
                shutil.copytree(source_path, destination_path)
                logger.info(f"Copy success, source dir: {source_path}, target dir: {destination_path}")
            else:
                logger.info(f"target file exist: {destination_path}, skip")


def get_value_from_dict(param, *keys):
    ret = True
    value = param
    for key in keys:
        if not isinstance(value, dict):
            ret = False
            break
        value = value.get(key)
    if value is None:
        ret = False
    return ret, value


def get_ids_by_name(user_name):
    user_id = pwd.getpwnam(user_name).pw_uid
    group_id = pwd.getpwnam(user_name).pw_gid
    return user_id, group_id


def get_dbuser_gname(user_name):
    group_id = pwd.getpwnam(user_name).pw_gid
    group = grp.getgrgid(group_id).gr_name
    return group


def check_injection_char(check_value):
    """
    function: check suspicious injection value
    input : check_value
    output: NA
    """
    if not check_value.strip():
        return False
    return not any(rac in check_value for rac in INJECTION_CHAR_LIST)


def str_to_int(origin_str, number_system):
    try:
        value = int(origin_str, number_system)
    except Exception:
        return 0
    return value


def str_to_float(origin_str):
    try:
        value = float(origin_str)
    except Exception:
        return 0
    return value


def write_progress_file(message: str, file_name: str):
    if os.path.islink(file_name):
        logger.error(f"Link file:{file_name},stop writing.")
        return

    exec_append_newline_file(file_name, message)


def read_progress(progress_file: str):
    """
    解析恢复进度
    :return:
    """
    logger.info('Query restore progress!')
    progress = 0
    status = SubJobStatusEnum.RUNNING.value
    if not progress_file.startswith(WhitePath.MOUNT_PATH):
        logger.error(f"Progress file: {progress_file} not exist")
        status = SubJobStatusEnum.FAILED.value
        return progress, status
    ret, data = exec_cat_cmd(progress_file, encoding='UTF-8')
    if not ret:
        logger.error(f"Execute cat command failed! File path: {progress_file}.")
        status = SubJobStatusEnum.FAILED.value
        return progress, status
    if ProgressInfo.FAILED in data:
        progress = 0
        status = SubJobStatusEnum.FAILED.value
        if not su_exec_rm_cmd(progress_file):
            logger.error(f"Execute remove file command failed! File path: {progress_file}.")
        return progress, status
    if ProgressInfo.SUCCEED in data:
        progress = 100
        status = SubJobStatusEnum.COMPLETED.value
        if not su_exec_rm_cmd(progress_file):
            logger.error(f"Execute remove file command failed! File path: {progress_file}.")
        return progress, status
    if "Progress" not in data:
        return progress, status
    progress = 5
    status = SubJobStatusEnum.RUNNING.value
    progress_info = data.split("\n")
    progress_info.reverse()
    for info in progress_info:
        if 'Progress' in info:
            present_file = int(re.findall("\d+", info)[0])
            total_file = int(re.findall("\d+", info)[1])
            try:
                progress = int((present_file + 0.5) / total_file * 100)
                break
            except Exception:
                logger.error("Read progress err!")
    return progress, status


def write_time_file(file_name: str):
    message = str(int((time.time())))
    if os.path.islink(file_name):
        logger.error(f"Link file:{file_name},stop writing.")
        return
    exec_append_file(file_name, message)


def query_speed(restore_path: str, time_file: str):
    """
    解析恢复进度
    :return:
    """
    logger.info('Query restore progress!')
    size = 0
    speed = 0
    if os.path.islink(time_file):
        logger.error(f"Link file:{time_file},stop writing.")
        return size, speed
    if not os.path.exists(time_file):
        logger.error(f"Time file: {time_file} not exist")
        return size, speed
    logger.info('Time path exist')
    with open(time_file, "r", encoding='UTF-8') as f:
        start_time = f.read().strip()
    size = safe_get_dir_size(restore_path)
    new_time = int((time.time()))
    try:
        speed = int(size / (new_time - int(start_time)))
    except Exception:
        logger.error("Error while calculating speed! time difference is 0!")
        return 0, 0
    return size, speed


def record_err_code(errcode: int, file_name: str):
    logger.info(f'Write errcode {errcode}')
    if os.path.islink(file_name):
        logger.error(f"Link file:{file_name},stop writing.")
        return
    exec_append_file(file_name, str(errcode))


def query_err_code(code_file: str):
    """
    解析错误码
    :return:
    """
    logger.info('Query err code!')
    code = NormalErr.WAITING
    logger.info(f'{code_file}')
    if os.path.islink(code_file):
        logger.error(f"Link file:{code_file},stop writing.")
        return code
    if not os.path.exists(code_file):
        logger.info(f"Code file: {code_file} not exist, maybe running, please wait.")
        return code
    if not code_file.startswith(WhitePath.MOUNT_PATH):
        logger.error('Code path not in mount path')
        return code
    with open(code_file, "r", encoding='UTF-8') as code_file_obj:
        pre_job_err = code_file_obj.read().strip()
    if not su_exec_rm_cmd(code_file):
        logger.error(f"Execute remove file command failed! File path: {code_file}.")
        return code
    try:
        int_pre_job_err = int(pre_job_err)
    except Exception:
        logger.error("Covert data type fail")
        return code
    return int_pre_job_err


def get_previous_copy_info(protect_obj, job_id):
    if not os.path.isdir(ParamConstant.PARAM_FILE_PATH) or not os.path.isdir(ParamConstant.RESULT_PATH):
        return {}
    input_file_name = RpcParamKey.INPUT_FILE_PREFFIX + job_id
    input_file_path = os.path.join(ParamConstant.PARAM_FILE_PATH, input_file_name)
    input_dict = {
        RpcParamKey.APPLICATION: protect_obj,
        RpcParamKey.TYPES: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY], RpcParamKey.COPY_ID: "",
        ParamKey.JOB_ID: job_id
    }
    exec_overwrite_file(input_file_path, input_dict)
    output_file_name = RpcParamKey.OUTPUT_FILE_PREFFIX + job_id
    output_file_path = os.path.join(ParamConstant.RESULT_PATH, output_file_name)
    cmd = f"sh {RpcParamKey.RPC_TOOL} {RpcParamKey.QUERY_PREVIOUS_CPOY} {input_file_path} {output_file_path}"
    ret, std_out, std_err = execute_cmd(cmd)
    if os.path.isfile(input_file_path):
        if not su_exec_rm_cmd(input_file_path, check_white_black_list_flag=False):
            logger.error(f"Execute remove file command failed! File path: {input_file_path}.")
            return {}
    if ret != ResultCode.SUCCESS or not os.path.isfile(output_file_path):
        return {}
    ret, output_data = exec_cat_cmd(output_file_path, encoding='UTF-8')
    if not su_exec_rm_cmd(output_file_path, check_white_black_list_flag=False):
        logger.error(f"Execute remove file command failed! File path: {output_file_path}.")
        return {}
    try:
        copy_info = json.loads(output_data)
    except Exception:
        return {}
    return copy_info


def get_dir_size(dir_path):
    present_size = 0
    for root, _, files in os.walk(dir_path):
        present_size += sum([os.path.getsize(os.path.join(root, name)) for name in files])
    return int(present_size / 1024)


def safe_get_dir_size(dir_path):
    if not os.path.isdir(dir_path):
        return 0
    signal.signal(signal.SIGALRM, lambda signum, frame: "TimeoutError")
    signal.alarm(20)
    try:
        dir_size = get_dir_size(dir_path)
    except Exception as ret:
        logger.exception("Err:", ret)
        return 0
    return dir_size


def execute_cmd_by_user(user_name, env_file, cmd):
    if not isinstance(user_name, str) or not check_injection_char(user_name) or not check_os_user(user_name):
        return ResultCode.FAILED, "", "bad user name"
    source_cmd = ""
    if os.path.isfile(env_file) and check_path(env_file) and check_file_owner(env_file, user_name):
        source_cmd = f"source {env_file} &&"
    partition_cmd = f"{source_cmd} {cmd}"
    if not partition_cmd.strip():
        return ResultCode.FAILED, "", "empty cmd"
    complete_cmd = f"su - {user_name} -c '{partition_cmd}'"
    logger.info(f"execute_cmd_by_user: {complete_cmd}")
    ret, stdout, stderr = execute_cmd(complete_cmd)
    logger.info(f"ret: {ret}, stdout: {stdout}, stderr: {stderr}")
    return ret, stdout, stderr


def get_now_time():
    return int(time.time())


def check_path(path):
    return isinstance(path, str) and os.path.exists(path) and os.path.realpath(path) == os.path.abspath(path)


STDIN_VALUE = ""


def safe_get_environ(key):
    global STDIN_VALUE
    if not STDIN_VALUE:
        STDIN_VALUE = input()
    try:
        value_dict = json.loads(STDIN_VALUE)
    except Exception:
        logger.exception("Standard input can not json load.")
        return ""
    value = value_dict.get(key)
    return value if isinstance(value, str) and check_injection_char(value) else ""


def safe_check_injection_char(*check_values):
    # 只检查字符串中是否存在命令注入，对其他数据类型则不做处理
    for value in check_values:
        if not isinstance(value, str):
            continue
        if not check_injection_char(value):
            return False
    return True


def get_env_value(key):
    global STDIN_VALUE
    if not STDIN_VALUE:
        STDIN_VALUE = input()
    try:
        value_dict = json.loads(STDIN_VALUE)
    except Exception:
        logger.exception("Standard input can not json load.")
        return ""
    return value_dict.get(key)


def repositories_check(repositories, *repository_types):
    # 只校验挂载仓路径是否存在软链接
    if not isinstance(repositories, list) or len(repositories) == 0:
        return True
    for repository_type in repository_types:
        path = get_repository_dir(repositories, repository_type)
        if not path:
            logger.warning(f"Path not exists. repository type: {repository_type}")
            continue
        if not check_path(path) or not check_repo_path(path):
            logger.error(f"Bad path. repository type: {repository_type}")
            return False
    return True


def safe_remove_path(path: str):
    if os.path.islink(path):
        return False
    if os.path.isdir(path):
        if not su_exec_rm_cmd(path, check_white_black_list_flag=False):
            logger.error(f"Execute remove file command failed! File path: {path}.")
            return False
    elif os.path.isfile(path):
        if not su_exec_rm_cmd(path, check_white_black_list_flag=False):
            logger.error(f"Execute remove file command failed! File path: {path}.")
            return False
    return not os.path.exists(path)


def is_cmdb_distribute(deploy_type, database_type):
    if deploy_type == OpenGaussDeployType.DISTRIBUTED and ProtectObject.CMDB in database_type:
        logger.info("Cur job is CMDB distributed")
        return True
    else:
        logger.info("Cur job is not CMDB distributed")
        return False


def is_wal_file(file_name):
    """
    是否WAL文件名称
    WAL segment file文件名称为24个字符，由3部分组成，每个部分是8个字符，每个字符是一个16进制值（即0~F）
    第1部分是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF
    第2部分是逻辑文件ID，取值范围是0x00000000 -> 0xFFFFFFFF
    第3部分是物理文件ID，取值范围是0x00000000 -> 0x000000FF
    """
    if not file_name:
        return False
    file_name = str(file_name)
    if not re.match(r"^[0-9A-F]{24}$", file_name):
        return False
    first_name = file_name[:8]
    second_name = file_name[8:16]
    third_name = file_name[-8:]
    if (0x00000000 <= int(first_name, 16) <= 0xFFFFFFFF) \
            and (0x00000000 <= int(second_name, 16) <= 0xFFFFFFFF) \
            and (0x00000000 <= int(third_name, 16) <= 0x000000FF):
        return True
    return False


def is_backup_wal_file(wal_file):
    if not re.match(r"^[0-9A-F]{24}\.[0-9A-F]{8}\.backup$", wal_file):
        return False
    timelines = wal_file.split(".")
    if not is_wal_file(timelines[0]):
        return False
    return True


def get_backup_info_file(wal_dir, index=-1):
    wal_files = os.listdir(wal_dir)
    file_list = []
    if index < -len(wal_files) or index >= len(wal_files):
        return ""
    for wal_file in wal_files:
        if is_backup_wal_file(wal_file):
            file_list.append(os.path.join(wal_dir, wal_file))
    file_list = sorted(file_list, key=lambda x: os.path.getmtime(x))
    backup_file = file_list[index]
    logger.info(f"Get backup info file:{backup_file}")
    return backup_file


def get_last_backup_stop_time(backup_file):
    if not os.path.exists(backup_file) or not os.path.isfile(backup_file):
        return "", ""
    with open(backup_file, "r") as file:
        lines = file.readlines()
    start_time = ""
    stop_time = ""
    for info in lines:
        if "START TIME:" in info:
            start_time = f"{info.split()[2]} {info.split()[3]}"
        if "STOP TIME:" in info:
            stop_time = f"{info.split()[2]} {info.split()[3]}"
            break
    return start_time, stop_time
