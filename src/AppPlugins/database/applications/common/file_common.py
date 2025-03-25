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
import shutil
from shutil import copy
from stat import S_ISDIR, S_ISLNK, S_ISREG

from typing import List

from common.common import execute_cmd, retry_when_exception
from common.const import EnumPathType, CMDResult

if platform.system().lower() == "windows":
    from common.logger_wins import Logger
else:
    from common.logger import Logger
    import grp
    import pwd
log = Logger().get_logger()


def check_path_validity(path):
    # 不能为空，不能含有?*|;&$><`"\\!.等字符
    regex = r'^[^?*|;&$><`"\\!.]+$'
    if not re.match(regex, path):
        log.error(f"Path: {path} is invalid")
        return False
    return True


def check_file_or_dir(path):
    """
    检测路径类型，文件 文件夹 符号链接
    :param path:
    :return:
    """
    try:
        status = os.lstat(path).st_mode
    except Exception as e:
        log.error(f"Get path info failed, exception: {e}")
        return EnumPathType.INVALID_TYPE
    if S_ISLNK(status):
        return EnumPathType.LINK_TYPE
    elif S_ISDIR(status):
        return EnumPathType.DIR_TYPE
    elif S_ISREG(status):
        return EnumPathType.FILE_TYPE
    log.error(f"Path not a valid type: {status}")
    return EnumPathType.INVALID_TYPE


def get_user_info(user_name):
    """
    获取用户信息，不存在报错
    :param user_name:
    :return: group_name user_info
    """
    try:
        user_info = pwd.getpwnam(str(user_name))
    except Exception:
        log.error("Get user info failed, not found user")
        return "", ""
    group_id = user_info.pw_gid
    user_group = grp.getgrgid(group_id).gr_name
    return user_group, user_info


def get_group_info(group_name):
    """
    获取用户组信息，不存在报错
    :param group_name:
    :return: group_name user_info
    """
    try:
        group_info = grp.getgrnam(str(group_name))
    except Exception:
        log.error("Get user info failed, not found user")
        return ""
    return group_info


def delete_file_or_dir_specified_user(user_name, des_path):
    """
    删除指定用户的文件或者文件夹
    :param user_name: 用户名
    :param des_path: 指定路径
    :return:
    """
    path_type = check_file_or_dir(des_path)
    if path_type in (EnumPathType.INVALID_TYPE, EnumPathType.LINK_TYPE):
        log.error(f"Des path not file or dir type: {path_type}, delete failed")
        return False
    _, user_info = get_user_info(user_name)
    if not user_info:
        log.error("Get user info failed")
        return False
    cmd = f"su - {user_name} -c 'rm -rf {des_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS or std_err:
        log.error(f"Fail to delete file or dir, err: {std_err}")
        return False
    return True


def copy_user_file_to_dest_path(src_path, desc_path, user_name=None):
    """
    复制文件到指定路径 默认指定路径存在文件会覆盖
    :param src_path: 源文件路径只支持文件
    :param desc_path: 目标路径 文件夹或这文件均可以
    :param user_name: 指定用户拷贝
    :return: bool
    """
    path_type = check_file_or_dir(src_path)
    if path_type != EnumPathType.FILE_TYPE:
        log.error(f"Src path is invalid type: {path_type} can not copy ")
        return False
    path_type = check_file_or_dir(desc_path)
    if path_type == EnumPathType.LINK_TYPE:
        log.error(f"Des path is link type can not copy")
        return False
    if not user_name:
        log.info("Use default user copy file")
        try:
            copy(src_path, desc_path)
        except Exception as e:
            log.error(f"Default Copy file to des path failed, exception: {e}")
            return False
        return True
    _, user_info = get_user_info(user_name)
    if not user_info:
        log.error(f"User is invalid can not copy")
        return False
    cmd = f"su - {user_name} -c 'cp -rp {src_path} {desc_path}'"
    return_code, _, std_err = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        log.error(f"Use user copy file failed src path: {src_path} des path: {desc_path}, error: {std_err}")
        return False
    return True


def change_path_permission(des_path: str, user_name: str = None, mode=None):
    """
    修改路径权限包括文件和文件夹，不存在或者符号链接报错,
    :param des_path:
    :param user_name:
    :param mode: chmod参数
    :return:
    """
    path_type = check_file_or_dir(des_path)
    if path_type in (EnumPathType.INVALID_TYPE, EnumPathType.LINK_TYPE):
        log.error(f"Des path: {des_path} is invalid can not change permission")
        return False
    if user_name:
        user_group, user_info = get_user_info(user_name)
        if not user_group:
            log.error("User is invalid can not change permission")
            return False
        stat_info = os.stat(des_path)
        if stat_info.st_uid != user_info.pw_uid or stat_info.st_gid != user_info.pw_gid:
            try:
                os.lchown(des_path, user_info.pw_uid, user_info.pw_gid)
            except Exception as e:
                log.error(f"Change dest path owner failed, exception: {e}")
                return False
    if mode:
        try:
            os.chmod(des_path, mode)
        except Exception as e:
            log.error(f"Change dest path permission failed, exception: {e}")
            return False
    return True


def exec_lchown(input_path: str, owner: str, group: str):
    """
    修改目录所属用户和用户组
    :param input_path: 待修改目录
    :param owner: 属主
    :param group: 属组
    """
    path_type = check_file_or_dir(input_path)
    if path_type in (EnumPathType.INVALID_TYPE, EnumPathType.LINK_TYPE):
        log.error(f"input path: {input_path} is invalid can not exec lchown")
        return False
    _, owner_info = get_user_info(owner)
    group_info = get_group_info(group)
    if not owner_info:
        log.error("owner_user is invalid can not exec lchown")
        return False
    if not group_info:
        log.error("group_user is invalid can not exec lchown")
        return False
    stat_info = os.stat(input_path)
    if stat_info.st_uid != owner_info.pw_uid or stat_info.st_gid != group_info.gr_gid:
        os.lchown(input_path, owner_info.pw_uid, group_info.gr_gid)
    return True


def exec_lchown_dir_recursively(input_path: str, owner: str, group: str):
    """
    递归修改目录所属用户和用户组
    :param input_path: 待修改目录
    :param owner: 属主
    :param group: 属组
    """
    path_type = check_file_or_dir(input_path)
    if path_type == EnumPathType.INVALID_TYPE:
        log.error(f"input path: {input_path} is invalid can not exec lchown dir recursively")
        return False
    _, owner_info = get_user_info(owner)
    group_info = get_group_info(group)
    if not owner_info:
        log.error("owner_user is invalid can not exec lchown dir recursively")
        return False
    if not group_info:
        log.error("group_user is invalid can not exec lchown dir recursively")
        return False
    os.lchown(input_path, owner_info.pw_uid, group_info.gr_gid)
    for root, dirs, files in os.walk(input_path):
        for tmp_dir in dirs:
            tmp_dir_path = os.path.join(root, tmp_dir)
            os.lchown(tmp_dir_path, owner_info.pw_uid, group_info.gr_gid)
        for tmp_file in files:
            tmp_file = os.path.join(root, tmp_file)
            os.lchown(tmp_file, owner_info.pw_uid, group_info.gr_gid)
    return True


def exec_lchown_dirs_recursively(input_path_list: List[str], owner: str, group: str):
    """
    递归修改多个目录所属用户和用户组
    :param input_path_list: 待修改目录集
    :param owner: 属主
    :param group: 属组
    """
    for dir_path in input_path_list:
        if not exec_lchown_dir_recursively(dir_path, owner, group):
            return False
    return True


@retry_when_exception(retry_times=3, delay=3)
def delete_file(file_path):
    if os.path.exists(file_path):
        os.remove(file_path)


@retry_when_exception(retry_times=3, delay=3)
def delete_path(path):
    if not os.path.exists(path):
        log.info(f"{path} not exists")
        return
    if os.path.isdir(path):
        shutil.rmtree(path)
    elif os.path.isfile(path):
        os.remove(path)
    elif os.path.islink(path):
        os.remove(path)


@retry_when_exception(retry_times=3, delay=3)
def create_dir_recursive(file_path):
    os.makedirs(file_path, exist_ok=True)


@retry_when_exception(retry_times=3, delay=3)
def check_file_exist(file_path):
    return os.path.exists(file_path)


@retry_when_exception(retry_times=3, delay=1)
def change_owner_by_name(file_path: str, owner: str, group: str):
    uid = pwd.getpwnam(owner).pw_uid
    gid = grp.getgrnam(group).gr_gid
    os.lchown(file_path, uid, gid)


@retry_when_exception(retry_times=3, delay=1)
def change_mod(path, mode):
    os.chmod(path, mode)


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
    try:
        change_path_permission(input_path, mode=mode)
    except Exception as e:
        log.error(f"Change input_path owner failed, exception: {e}")
        return False
    for root, dirs, files in os.walk(input_path):
        for tmp_dir in dirs:
            tmp_dir_path = os.path.join(root, tmp_dir)
            try:
                change_path_permission(tmp_dir_path, mode=mode)
            except Exception as e:
                log.error(f"Change tmp_dir_path owner failed, exception: {e}")
                continue
        for tmp_file in files:
            tmp_file = os.path.join(root, tmp_file)
            try:
                change_path_permission(tmp_file, mode=mode)
            except Exception as e:
                log.error(f"Change tmp_file owner failed, exception: {e}")
                continue
    return True
