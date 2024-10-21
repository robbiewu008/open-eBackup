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
import os
import re
import subprocess
from enum import Enum

from public_cbb.security.anonym_utils.anonymity import Anonymity
from public_cbb.log.logger import get_logger
from public_cbb.config.global_config import get_settings


log = get_logger()
MOUNT_POINT_IN_SHELL_LOG_START_TAG = "mount_point is: "
MOUNT_POINT_IN_SHELL_LOG_END_TAG = ". Line:"
OS_CONFIG_SC_PAGE_SIZE = 'SC_PAGE_SIZE'
OS_CHAR_BIT = 32
DEFAULT_STRING_LENGTH = 65536


class OperationClass:
    def __init__(self, operation, path):
        self.operation = operation
        self.path = path


class OperationEnum(Enum):
    MOUNT_SCRIPT = OperationClass("mount_oper", get_settings().MOUNT_OPER_PATH)

    def get_operation(self):
        return self.value.operation

    def get_path(self):
        return self.value.path


def shell_log_list_print(out):
    logs = out.decode("UTF-8").split("\n")
    last_error_desc = ''
    for single_log in logs:
        single_log = single_log.replace("\r", "")
        log_string = f"{single_log}"
        if not log_string:
            continue
        if ".sh.info" in single_log:
            log.info(f"{log_string}")
        elif ".sh.error" in single_log:
            log.error(f"{log_string}")
            last_error_desc = log_string
        elif ".sh.warning" in single_log:
            log.warning(f"{log_string}")
        elif ".sh.debug" in single_log:
            log.debug(f"{log_string}")
        else:
            log.info(f"{log_string}")
    return last_error_desc


def mount_nfs_by_root_account(mount_point, mount_src):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "mount_nfs", mount_point, mount_src),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def mount_cifs_by_root_account(mount_point, mount_src, user_name, passwd):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "mount_cifs", mount_point, mount_src,
                              user_name, passwd),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def mount_fuse_by_root_account(mount_point, source_id, osad_ip_list, osad_auth_port,
                               osad_server_port):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(),
                              "mount_fuse", mount_point, source_id,
                              osad_ip_list, osad_auth_port, osad_server_port
                              ),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def umount_by_root_account(mount_point):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "umount", mount_point),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def umount_fuse_by_root_account(mount_point):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(),
                              "umount_fuse", mount_point),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def delete_path_by_root_account(delete_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "rm", delete_path),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode


def read_lines_by_root_account(file_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "read", file_path),
                             shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True,
                             encoding='UTF-8')
    data_list = child.stdout.readlines()
    out, error = child.communicate()
    context_line = []
    for item in data_list:
        context_line.append(item.strip())
    return context_line, error, child.returncode


def over_write_by_root_account(file_path, data):
    log.debug(f'Will data size: {len(data)}')
    max_string_length = DEFAULT_STRING_LENGTH
    try:
        max_string_length = os.sysconf(OS_CONFIG_SC_PAGE_SIZE)
    except Exception as config_ex:
        log.warning(f'Fail to get config! exception={Anonymity.process(str(config_ex))}')
    max_string_length = DEFAULT_STRING_LENGTH if max_string_length == DEFAULT_STRING_LENGTH \
        else max_string_length * OS_CHAR_BIT - 1

    text_list = re.findall('.{' + str(max_string_length) + '}', data)
    text_list.append(data[(len(text_list) * max_string_length):])
    text_list_length = len(text_list)
    need_next = 'no_next' if text_list_length > 1 else 'next'
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "over_write", file_path,
                              text_list[0], need_next), shell=False, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    if error or child.returncode != 0:
        log.error(f'Write data failed index 0, error: {error}, code: {child.returncode}.')
        return out, error, child.returncode
    if text_list_length > 1:
        for index in range(1, text_list_length):
            need_next = 'next' if index == (text_list_length - 1) else 'no_next'
            child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "append_write", file_path,
                                      text_list[index], need_next), shell=False, stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE, encoding='UTF-8')
            out, error = child.communicate()
            if error or child.returncode != 0:
                log.error(f'Write data failed index {index}, error: {error}, code: {child.returncode}.')
                return out, error, child.returncode
    return out, '', 0


def append_write_by_root_account(file_path, data, need_next='next'):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "append_write", file_path, data,
                              need_next), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    return out, error, child.returncode


def create_path_by_root_account(dir_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "create_path", dir_path),
                             shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    return out, error, child.returncode


def check_file_exists_by_root_account(file_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "check_file", file_path),
                             shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    code = child.returncode
    if error or code != 0:
        log.error(f'Check file exists failed, out: {out}, error: {error}, code: {code}')
        return False
    return True


def check_is_directory_sudo(file_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "is_directory", file_path),
                             shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    code = child.returncode
    if error or code != 0:
        log.error(f'Check is directory failed, out: {out}, error: {error}, code: {code}')
        return False
    return True


def check_is_mount_sudo(file_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "is_mount", file_path),
                             shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='UTF-8')
    out, error = child.communicate()
    code = child.returncode
    if error or code != 0:
        log.error(f'Check is a mount point failed, out: {out}, error: {error}, code: {code}')
        return False
    return True


def clear_path_by_root_account(clear_path):
    child = subprocess.Popen(("sudo", OperationEnum.MOUNT_SCRIPT.get_path(), "clear_path", clear_path),
                             shell=False,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    out, error = child.communicate()
    return out, error, child.returncode
