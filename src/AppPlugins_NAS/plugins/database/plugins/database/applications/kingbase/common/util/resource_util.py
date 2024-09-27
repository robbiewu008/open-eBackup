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
import platform
import re
import socket

import psutil

from common.common import check_command_injection, execute_cmd
from common.const import CMDResult, IPConstant
from common.logger import Logger
from common.util import check_utils
from common.util.check_utils import check_file_path, is_domain
from kingbase.common.const import ConfigKeyStatus, KbConst
from kingbase.common.error_code import ErrorCode
from oceanbase.common.const import CMDResult

if platform.system() == "Linux":
    import pwd

LOGGER = Logger().get_logger("kingbase.log")


def get_local_ips():
    """
    获取当前主机所有ip
    :return: list，主机ip
    """
    LOGGER.info(f"Start getting all local ips ...")
    local_ips = []
    ip_dict = {}
    try:
        ip_dict = psutil.net_if_addrs()
    except Exception as err:
        LOGGER.error(f"Get ip address err: {err}.")
    for _, info_list in ip_dict.items():
        for i in info_list:
            if i[0] == KbConst.ADDRESS_FAMILY_AF_INET and i[1] != IPConstant.LOCAL_HOST:
                local_ips.append(i[1])
    LOGGER.info(f"Get all local ips: {local_ips} success.")
    return local_ips


def get_version(pid, client_path, os_username):
    """获取数据库版本

    :param pid: 任务pid
    :param client_path: 数据库客户端
    :param os_username: 操作系统用户名
    :return: 结果状态码, 数据库版本
    """
    cmd = f"su - {os_username} -c \'{client_path} --version\'"
    return_code, std_out, err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS.value:
        LOGGER.error(f"Get kingbase version error! pid: {pid}, error: {err}")
        return ErrorCode.GET_VERSION_FAILED, ""
    version = std_out.strip().split(' ')[-1]
    LOGGER.info(f"Succeed to get kingbase version: {version}")
    return ErrorCode.SUCCESS, version


def domain_2_ip(domain):
    if is_domain(domain):
        domain = socket.gethostbyname(domain)
    LOGGER.info(f"Success convert domain to {domain}!")
    return domain


def is_wal_file(file_name):
    """
    是否WAL文件名称
    WAL segment file文件名称为24个字符，由3部分组成，每个部分是8个字符，每个字符是一个16进制值（即0~F）
    第1部分是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF
    第2部分是逻辑文件ID，取值范围是0x00000000 -> 0xFFFFFFFF
    第3部分是物理文件ID，取值范围是0x00000000 -> 0x000000FF
    sys_rman归档日志为24个字符后加-和40个字符
    """
    if not file_name:
        return False
    file_name = str(file_name)
    if not re.match(r"^[0-9A-F]{24}-[A-Za-z0-9]{40}(\.gz)?$", file_name):
        return False
    first_name = file_name[:8]
    second_name = file_name[8:16]
    third_name = file_name[16:24]
    if (0x00000000 <= int(first_name, 16) <= 0xFFFFFFFF) \
            and (0x00000000 <= int(second_name, 16) <= 0xFFFFFFFF) \
            and (0x00000000 <= int(third_name, 16) <= 0x000000FF):
        return True
    return False


def check_special_character(check_strings):
    for check_string in check_strings:
        if check_command_injection(check_string):
            LOGGER.error(f"String contains special characters, check string: {check_string}.")
            raise Exception(f"String contains special characters.")
    LOGGER.info(f"Check special character complete.")


def check_black_list(check_paths):
    for check_path in check_paths:
        if re.match(KbConst.PATH_BLACK_LIST, check_path):
            LOGGER.error(f"Path: {check_path} is in the black list.")
            raise Exception(f"Path: {check_path} is in the black list.")
    LOGGER.info('Check black list complete.')


def check_white_list(check_paths):
    for check_path in check_paths:
        if not re.match(KbConst.PATH_WHITE_LIST, check_path):
            LOGGER.error(f"Path: {check_path} is not in the white list.")
            raise Exception(f"Path: {check_path} is not in the white list.")
    LOGGER.debug('Check white list complete.')


def check_is_path_exists(check_path):
    if not os.path.exists(check_path):
        LOGGER.error(f"Path: {check_path} not exist.")
        raise Exception(f"Path: {check_path} not exist.")


def convert_path_to_realpath(input_path):
    if input_path:
        return os.path.realpath(input_path)
    return input_path


def check_file_path(file_path):
    if not file_path or not os.path.isfile(file_path):
        LOGGER.error("The file is not a file.")
        raise Exception("The file is not a file")
    check_black_list([file_path])


def check_dir_path(dir_path):
    if not dir_path or not os.path.isdir(dir_path):
        LOGGER.error(f"The directory is not a directory.")
        raise Exception("The directory is not a directory")
    check_black_list([dir_path])


def get_conf_item_status(cfg_file, start_str: str):
    LOGGER.info(f"Try to check if item is in config file, start of item: {start_str}.")
    status = ConfigKeyStatus.NOT_EXIST
    if not start_str:
        LOGGER.warning("Check if item is in config file, input item info is empty.")
        return status
    check_file_path(cfg_file)
    with open(cfg_file, 'r') as tmp_file:
        lines = tmp_file.readlines()
    start_str = start_str.lstrip("#")
    for i in lines:
        if str(i).strip().startswith(f"#{start_str}"):
            status = ConfigKeyStatus.ANNOTATED
            break
        elif str(i).strip().startswith(start_str):
            status = ConfigKeyStatus.CONFIGURED
            break
    LOGGER.info(f"Check if item is in config file success, status: {status}.")
    return status


def check_service_ip(service_ip):
    if not check_utils.is_ip_address(service_ip):
        LOGGER.error(f"The service ip is invalid.")
        return False
    local_ips = get_local_ips()
    if service_ip not in local_ips:
        LOGGER.error(f"The service ip is not the IP address of the agent host.")
        return False
    return True


def check_os_name(os_username, file_path):
    if os_username == "root":
        LOGGER.info("The os username can not be root")
        return False
    try:
        pwd.getpwnam(os_username)
    except KeyError:
        LOGGER.error(f"kingbase os_username: {os_username} is not exist")
        return False
    stat_info = os.stat(file_path)
    uid = stat_info.st_uid
    user = pwd.getpwuid(uid)[0]
    if user != os_username:
        LOGGER.error(f"Os user name and file path is not matching! os_username: {os_username}, file_path:{file_path}")
        return False
    LOGGER.info(f"Success to check os name! os_username: {os_username}")
    return True


def is_backup_wal_file(wal_file):
    if not re.match(r"^[0-9A-F]{24}\.[0-9A-F]{8}\.backup$", wal_file):
        return False
    return True


def check_path_islink(input_path):
    """检查路径是否软连接"""
    if not os.path.exists(input_path):
        LOGGER.warning("The path does not exist when checking if the path is link.")
        return
    if os.path.islink(input_path):
        LOGGER.error(f"Input path: {input_path} is link.")
        raise Exception("Input path is link")


def get_tmp_node_ip(node):
    node_ip = node.get("extendInfo", {}).get("subNetFixedIp", "")
    if node_ip:
        return node_ip
    return node.get("endpoint", "")


def get_sys_rman_configuration_item(install_path, job_id, configuration="_repo_path"):
    """
    读取sys_rman.conf中配置项_repo_path
    :return: _repo_path
    """
    # 获取sys_backup.conf路径
    sys_backup_conf_path = os.path.join(install_path, KbConst.BIN_DIR_NAME, KbConst.SYS_BACKUP_CONF_FILE_NAME)
    check_file_path(sys_backup_conf_path)
    # 读取sys_rman.conf中配置项_repo_path
    try:
        with open(sys_backup_conf_path, "r") as f:
            lines = f.readlines()
    except Exception as err:
        LOGGER.error(f"Fail to read sys_rman.conf for {err}, self._job_id:  {job_id}.")
    result = ''
    for line_str in lines:
        if line_str.startswith(configuration):
            result = line_str.split("=")[1].strip().strip('"')
            break
    if not result:
        LOGGER.error(f"Fail to get {configuration}, jobId: {job_id}.")
    return result


def create_soft_link(src_path, dst_path, job_id=""):
    if not os.path.exists(src_path):
        LOGGER.error(f"The source path:{src_path} does not exist.")
        return False
    if os.path.exists(dst_path) and os.path.islink(dst_path) and os.path.realpath(dst_path) == src_path:
        LOGGER.info("Soft link already exists!")
        return True
    else:
        ret, output, err = execute_cmd(f"rm -rf {dst_path}")
        if ret != '0' or "errors" in err:
            LOGGER.error(
                f"Failed to rm -rf {dst_path},err：{err}, jobId: {job_id}.")
            return False
    try:
        os.symlink(src_path, dst_path)
    except Exception as exception_info:
        LOGGER.error(f"Failed to create the soft link.err: {exception_info}, job id: {job_id}")
        return False
    return True


def delete_soft_link(src_path, job_id=""):
    ret, output, err = execute_cmd(f"rm -rf {src_path}")
    if ret != '0' or "errors" in err:
        LOGGER.error(
            f"Failed to rm -rf {src_path},err：{err}, jobId: {job_id}.")
        return False
    return True


def get_parallel_process(job_dict):
    extend_info = job_dict.get("extendInfo", {})
    if extend_info:
        parallel_process = extend_info.get("parallel_process")
        if parallel_process:
            parallel_process = int(parallel_process)
            if parallel_process > KbConst.MAX_PARALLEL_PROCESS or parallel_process < KbConst.MIN_PARALLEL_PROCESS:
                LOGGER.warn(f"The parameter parallel_process is not within the range of 1-999, it will be treated as 1")
                return 1
            else:
                return parallel_process
        else:
            LOGGER.warn(f"No parameter parallel_process received; it will be treated as 1.")
    return 1


def get_db_version_id_and_system_id(archive_info_file_path, job_id=""):
    """
    archive.info文件获取db_version, db_id组成日志目录名，获取db_system_id
    """
    try:
        with open(archive_info_file_path, "r") as f:
            lines = f.readlines()
    except Exception as ex:
        raise Exception(f"Fail to read archive.info, self._job_id:{job_id}.") from ex

    db_id, db_version, db_system_id = '', '', ''
    for line_str in lines:
        if line_str.startswith("db-id"):
            db_id = line_str.split("=")[1].strip()
            continue
        if line_str.startswith("db-system-id"):
            db_system_id = line_str.split("=")[1].strip()
            continue
        elif line_str.startswith("db-version"):
            db_version = line_str.split("=")[1].strip().strip('"')
    if not db_id or not db_version or not db_system_id:
        raise Exception(f"Fail to get db-version or db-id or db_system_id, jobId: {job_id}.")
    else:
        LOGGER.info(f"Successfully get {db_version}-{db_id},{db_system_id}")
        return f"{db_version}-{db_id}", db_system_id


def extract_ip():
    """
    获取当前主机所有ip
    :return: list，主机ip
    """
    LOGGER.info(f"Start getting all local ips ...")
    local_ips = []
    try:
        ip_dict = psutil.net_if_addrs()
    except Exception as err:
        LOGGER.error(f"Get ip address err: {err}.")
        return local_ips
    for _, info_list in ip_dict.items():
        for i in info_list:
            if i[0] == 2 and i[1] != IPConstant.LOCAL_HOST:
                local_ips.append(i[1])
    LOGGER.info(f"Get all local ips success.")
    return local_ips


def get_current_repo_host(sys_rman_conf_path, job_id=""):
    """
    读取repo仓/sys_rman.conf文件获取repo1-host
    """
    repo_host = ""
    if not os.path.exists(sys_rman_conf_path):
        return repo_host
    try:
        with open(sys_rman_conf_path, "r") as f:
            lines = f.readlines()
    except Exception as err:
        LOGGER.error(f"Fail to read sys_rman.conf for {err}, jobId: {job_id}.")
        return repo_host
    for line_str in lines:
        if line_str.startswith("repo1-host"):
            repo_ip = line_str.split("=")[1].strip()
            return repo_ip
    return repo_host
