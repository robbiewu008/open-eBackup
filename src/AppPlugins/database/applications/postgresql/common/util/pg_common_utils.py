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

import base64
import datetime
import functools
import json
import math
import os
import platform
import re
import shlex
import shutil
import socket
import stat
import subprocess
import threading
import time

import psutil
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

from common import common as pg_util_common
from common.common import check_command_injection, invoke_rpc_tool_interface, is_clone_file_system
from common.const import CMDResult
from common.file_common import change_path_permission
from common.logger import Logger
from common.number_const import NumberConst
from common.util import check_utils as pg_check_utils, check_user_utils
from common.util.backup import query_progress, backup, backup_files
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import check_path_valid, exec_overwrite_file, exec_append_file, exec_cp_cmd
from postgresql.common.const import PgConst, CmdRetCode, ConfigKeyStatus, BackupStatus
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import RestoreProgress
from postgresql.common.pg_exception import ErrCodeException

if platform.system() == "Linux":
    import grp
    import pwd

LOGGER = Logger().get_logger("postgresql.log")
GLOBAL_PG_PROCESS_FILE_LOCK = threading.Lock()
GLOBAL_PG_RES_SPEED_FILE_LOCK = threading.Lock()
# 版本和recovery.conf.sample文件目录的映射
PG_VER_SAMPLE_CFG_DIR_MAP = {
    "9.0": "recovery_conf_9_0",
    "9.1": "recovery_conf_9_1",
    "9.2": "recovery_conf_9_1",
    "9.3": "recovery_conf_9_1",
    "9.4": "recovery_conf_9_4",
    "9.5": "recovery_conf_9_5",
    "9.6": "recovery_conf_9_5",
    "10": "recovery_conf_10",
    "11": "recovery_conf_10"
}


def check_transaction_decorator(func):
    @functools.wraps(func)
    def inner(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except Exception:
            LOGGER.exception(f"Check transactions after recovery target time failed.")
            return False

    return inner


class PostgreCommonUtils:
    @staticmethod
    def check_file_path(file_path):
        if not file_path or not os.path.isfile(file_path):
            LOGGER.error(f"The file is not a file.")
            raise Exception("The file is not a file")
        if not PostgreCommonUtils.check_path_characters_and_black_list(file_path):
            LOGGER.error(f"Check the path!")
            raise Exception("Check the path!")

    @staticmethod
    def check_dir_path(dir_path):
        if not dir_path or not os.path.isdir(dir_path):
            LOGGER.error(f"The directory is not a directory.")
            raise Exception("The directory is not a directory")
        if not PostgreCommonUtils.check_path_characters_and_black_list(dir_path):
            LOGGER.error(f"Check the path!")
            raise Exception("Check the path!")

    @staticmethod
    def check_path_exist(check_path):
        if not check_path or not os.path.exists(check_path):
            LOGGER.error(f"The path doest not exist.")
            raise Exception("The path doest not exist")
        if not PostgreCommonUtils.check_path_characters_and_black_list(check_path):
            LOGGER.error(f"Check the path!")
            raise Exception("Check the path!")

    @staticmethod
    def check_path(check_path):
        if not check_path or not os.path.exists(check_path):
            LOGGER.error(f"The path doest not exist.")
            return False
        if not PostgreCommonUtils.check_path_characters_and_black_list(check_path):
            LOGGER.error(f"Check the path!")
            return False
        return True

    @staticmethod
    def check_path_islink(input_path):
        """检查路径是否软连接"""
        if not os.path.exists(input_path):
            LOGGER.warning("The path does not exist when checking if the path is link.")
            return
        if os.path.islink(input_path):
            LOGGER.error(f"Input path: {input_path} is link.")
            raise Exception("Input path is link")

    @staticmethod
    def convert_timestamp_to_datetime(s_timestamp):
        try:
            s_timestamp = int(s_timestamp)
        except ValueError as ex:
            LOGGER.exception(f"Timestamp parameter: {s_timestamp} is invalid when converting.")
            raise Exception("Timestamp parameter is invalid") from ex
        return datetime.datetime.fromtimestamp(s_timestamp).strftime(PgConst.RECOVERY_TARGET_TIME_FORMATTER)

    @staticmethod
    def is_db_running(pg_system_user: str, db_install_path: str, db_data_path: str) -> bool:
        LOGGER.info("Start checking database is running ...")
        PostgreCommonUtils.check_dir_path(db_install_path)
        pg_cfg_file = os.path.realpath(os.path.join(db_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        if any([not os.path.isdir(db_data_path), not os.path.exists(pg_cfg_file)]):
            LOGGER.warning(f"Database data path: {db_data_path} doest not exist or is corrupted when checking status.")
            return False
        if not PostgreCommonUtils.check_os_user(pg_system_user, db_install_path)[0]:
            return False
        pg_ctl_path = os.path.realpath(os.path.join(db_install_path, "bin", "pg_ctl"))
        PostgreCommonUtils.check_file_path(pg_ctl_path)
        PostgreCommonUtils.check_path_islink(pg_ctl_path)
        check_user_utils.check_path_owner(pg_ctl_path, [pg_system_user])
        PostgreCommonUtils.check_path_islink(db_data_path)
        check_user_utils.check_path_owner(db_data_path, [pg_system_user])
        check_db_status_cmd = cmd_format("su - {} -c '{} -D {} status'", pg_system_user, pg_ctl_path, db_data_path)
        return_code, std_out, std_err = pg_util_common.execute_cmd(check_db_status_cmd)
        if return_code == CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.info("Database is running.")
            return True
        LOGGER.info(f"Execute check database status command, return code: {return_code}, "
                    f"out: {std_out}, err: {std_err}.")
        LOGGER.info(f"Database is not running.")
        return False

    @staticmethod
    def start_postgresql_database(pg_system_user: str, db_install_path: str, db_data_path: str):
        LOGGER.info("Try to start database ...")
        PostgreCommonUtils.check_dir_path(db_install_path)
        PostgreCommonUtils.check_dir_path(db_data_path)
        pg_ctl_path = os.path.realpath(os.path.join(db_install_path, "bin", "pg_ctl"))
        tmp_logfile = os.path.realpath(os.path.join(db_data_path, "pg_start.log"))
        PostgreCommonUtils.delete_path(tmp_logfile)
        enable_root = PostgreCommonUtils.get_root_switch()
        if not PostgreCommonUtils.check_os_user(pg_system_user, db_install_path, enable_root)[0]:
            LOGGER.error("Execute start database command failed,because os username is not exist.")
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Os username is not exist.")
        if not enable_root:
            if not check_user_utils.check_path_owner(pg_ctl_path, [pg_system_user]):
                raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Check path owner failed.")
        if not check_user_utils.check_path_owner(db_data_path, [pg_system_user]):
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Check path owner failed.")
        # 启动时等待直到启动成功
        start_db_cmd = cmd_format("su - {} -c '{} -D {} -w -t {} -l {} start'", pg_system_user, pg_ctl_path,
                                  db_data_path, PgConst.CHECK_POINT_TIME_OUT, tmp_logfile)
        LOGGER.info("Executing start database command: %s.", start_db_cmd)
        return_code, std_out, std_err = pg_util_common.execute_cmd(start_db_cmd)
        LOGGER.info(f"Execute start database command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute start database command failed.")
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Execute start database command failed")
        LOGGER.info("Start database success.")
        return True

    @staticmethod
    def start_pgpool(install_path: str):
        LOGGER.info("Try to start pgpool ...")
        PostgreCommonUtils.check_dir_path(install_path)
        pgpool_file = os.path.realpath(os.path.join(install_path, "bin", "pgpool"))
        # 获取pgpool拥有者
        pgpool_file_owner = PostgreCommonUtils.get_path_owner(pgpool_file)
        pgpool_log_file = os.path.realpath(os.path.join(install_path, f"pgpool.log"))
        PostgreCommonUtils.delete_path(pgpool_log_file)
        PostgreCommonUtils.check_file_path(pgpool_file)
        PostgreCommonUtils.check_path_islink(pgpool_file)
        start_pgpool_cmd = cmd_format("su - {} -c '{} -n -D > /dev/null 2>&1 &'", pgpool_file_owner, pgpool_file)
        LOGGER.debug(f"Start pgpool command: {start_pgpool_cmd}.")
        return_code, std_out, std_err = pg_util_common.execute_cmd(start_pgpool_cmd)
        LOGGER.info(f"Execute start pgpool command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute start pgpool command failed.")
            raise ErrCodeException(ErrorCode.EXEC_START_PGPOOL_CMD_FAILED,
                                   message="Execute start pgpool command failed")
        LOGGER.info("Start pgpool success.")
        return True

    @staticmethod
    def stop_pgpool(install_path: str):
        LOGGER.info("Try to stop pgpool ...")
        PostgreCommonUtils.check_dir_path(install_path)
        pgpool_file = os.path.realpath(os.path.join(install_path, "bin", "pgpool"))
        if not check_path_valid(pgpool_file, False):
            LOGGER.error("The touch file_path is invalid: %s.", pgpool_file)
            return False
        # 获取pgpool拥有者
        pgpool_file_owner = PostgreCommonUtils.get_path_owner(pgpool_file)
        if pgpool_file_owner != PgConst.OS_USER_ROOT:
            start_pgpool_cmd = f"su - {pgpool_file_owner} -c '{pgpool_file} -m fast stop'"
        else:
            start_pgpool_cmd = f"{pgpool_file} -m fast stop"
        PostgreCommonUtils.check_file_path(pgpool_file)
        PostgreCommonUtils.check_path_islink(pgpool_file)
        LOGGER.debug(f"Stop pgpool command: {start_pgpool_cmd}.")
        return_code, std_out, std_err = pg_util_common.execute_cmd(start_pgpool_cmd)
        LOGGER.info(f"Execute stop pgpool command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute stop pgpool command failed.")
            raise Exception("Execute stop pgpool command failed")
        LOGGER.info("Stop pgpool success.")
        return True

    @staticmethod
    def is_db_version_matched(src_version: str, target_version: str) -> bool:
        src_ver_splits, target_ver_splits = src_version.split('.'), target_version.split('.')
        if any([len(src_ver_splits) < 2, len(target_ver_splits) < 2]):
            LOGGER.error(f"Source version: {src_version} or target version: {target_version} is invalid.")
            raise Exception("Source version or target version is invalid")
        return src_version.strip() == target_version.strip()

    @staticmethod
    def is_path_space_enough(check_path: str, needed_space: int) -> bool:
        PostgreCommonUtils.check_dir_path(check_path)
        stat_ret = os.statvfs(check_path)
        # 单位：MB
        m_free_space = math.floor(stat_ret.f_bavail * stat_ret.f_frsize / 1024 / 1024)
        LOGGER.info(f"Path: {check_path} free space is {m_free_space} MB, needed space is {needed_space} MB.")
        return m_free_space > needed_space

    @staticmethod
    def get_path_uid_and_gid(check_path):
        """
        获取路径的用户ID和用户组ID
        """
        PostgreCommonUtils.check_path_exist(check_path)
        return os.stat(check_path).st_uid, os.stat(check_path).st_gid

    @staticmethod
    def get_path_owner(check_path):
        """
        获取路径用户
        """
        PostgreCommonUtils.check_path_exist(check_path)
        return pwd.getpwuid(os.stat(check_path).st_uid).pw_name

    @staticmethod
    def get_path_group(check_path):
        """
        获取路径用户组
        """
        PostgreCommonUtils.check_path_exist(check_path)
        return grp.getgrgid(os.stat(check_path).st_gid).gr_name

    @staticmethod
    def get_uid_gid_by_os_user(os_user):
        """
        根据系统用户名获取用户ID和用户组ID
        """
        try:
            user_info = pwd.getpwnam(os_user)
        except KeyError as ex:
            LOGGER.error(f"Current system does not have user: {os_user}.")
            raise Exception(f"Current system does not have user: {os_user}") from ex
        return user_info.pw_uid, user_info.pw_gid

    @staticmethod
    def is_dir_readable_and_writable_for_input_user(check_dir, input_user) -> bool:
        PostgreCommonUtils.check_dir_path(check_dir)
        uid, gid = PostgreCommonUtils.get_uid_gid_by_os_user(input_user)
        s = os.stat(check_dir)
        mode = s[stat.ST_MODE]
        return all([
            (s[stat.ST_UID] == uid and (mode & stat.S_IRUSR > 0))
            or (s[stat.ST_GID] == gid and (mode & stat.S_IRGRP > 0))
            or (mode & stat.S_IROTH > 0),
            (s[stat.ST_UID] == uid and (mode & stat.S_IWUSR > 0))
            or (s[stat.ST_GID] == gid and (mode & stat.S_IWGRP > 0))
            or (mode & stat.S_IWOTH > 0)
        ])

    @staticmethod
    def get_archive_path_offline(db_data_path):
        archive_path = ""
        pg_conf_file_path = os.path.realpath(os.path.join(db_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        PostgreCommonUtils.check_file_path(pg_conf_file_path)
        with open(pg_conf_file_path, "r", encoding="utf-8") as fp:
            lines = fp.readlines()
            for i in lines:
                if not i.strip() or i.strip().startswith("#"):
                    continue
                if all([i.strip().startswith("archive_command"), "=" in i, "%p" in i, "/%f" in i]):
                    tmp_splits = i.split("%p")
                    archive_path = tmp_splits[1].split("/%f")[0].strip().strip("\'\"") if len(tmp_splits) > 1 else ""
                    break
        return archive_path.strip().strip("\'\"")

    @staticmethod
    def delete_path(del_path):
        del_path = os.path.realpath(del_path)
        LOGGER.info(f"Start deleting path: {del_path}")
        if not os.path.exists(del_path):
            LOGGER.warning(f"The path: {del_path} does not exist when trying to delete it.")
            return
        if not check_path_valid(del_path, False):
            LOGGER.warning(f"The path: {del_path} not allowed to be deleted.")
            return
        if os.path.isfile(del_path) or os.path.islink(del_path):
            # 文件不存在会报错
            os.remove(del_path)
            LOGGER.info(f"Delete file: {del_path} success.")
        else:
            # 目录不存在会报错
            shutil.rmtree(del_path)
            LOGGER.info(f"Delete directory: {del_path} success.")

    @staticmethod
    def clear_dir_when_exist(clear_path):
        LOGGER.info(f"Start clearing directory: {clear_path}.")
        if not os.path.isdir(clear_path):
            LOGGER.warning(f"The path: {clear_path} does not exist when trying to clear it.")
            return
        pg_util_common.clean_dir_not_walk_link(clear_path)
        LOGGER.info(f"Cleared directory: {clear_path} successfully.")

    @staticmethod
    def preserve_copy_file(src_file_path, tgt_dir, tgt_file_name=""):
        PostgreCommonUtils.check_file_path(src_file_path)
        PostgreCommonUtils.check_dir_path(tgt_dir)
        tgt_file_path = os.path.realpath(os.path.join(tgt_dir, tgt_file_name))
        copy_file_cmd = f'/bin/cp -fa {src_file_path} {tgt_file_path}'
        LOGGER.info(f"Execute copy file command: {copy_file_cmd}.")
        return_code, std_out, std_err = pg_util_common.execute_cmd(copy_file_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Copy file: {src_file_path} to {tgt_dir} failed, return code: {return_code}, "
                         f"out: {std_out}, err: {std_err}.")
            raise Exception("Copy file failed")
        LOGGER.info(f"Copy file: {src_file_path} to {tgt_dir} success.")

    @staticmethod
    def copy_data(os_user: str, src_path: str, target_path: str, wildcard="."):
        LOGGER.info(f"Start copying data: {src_path} to path: {target_path}")
        PostgreCommonUtils.check_path_exist(src_path)
        PostgreCommonUtils.check_path_exist(target_path)
        src_path = src_path if src_path.endswith("/") else f"{src_path}/"
        LOGGER.info(f"Execute copy data command.")
        ret = exec_cp_cmd(f"{src_path}{wildcard}", target_path, os_user, "-rf", is_check_white_list=False)
        if not ret:
            LOGGER.error(f"Copy data: {src_path} to path: {target_path} failed.")
            raise Exception("Copy source data to target path failed")
        LOGGER.info(f"Copy data: {src_path} to path: {target_path} success.")

    @staticmethod
    def copy_directory(src_path: str, target_path: str, wildcard=".", job_id=""):
        LOGGER.info(f"Start copying dir: {src_path} to path: {target_path}")
        PostgreCommonUtils.check_path_exist(src_path)
        PostgreCommonUtils.check_path_exist(target_path)
        src_path = src_path if src_path.endswith("/") else f"{src_path}/"
        source = src_path
        LOGGER.info(f"Copying dir: {source} to path: {target_path}")
        res = backup(job_id, source, target_path)
        if not res:
            LOGGER.error(f"Failed to start backup, jobId: {job_id}.")
            return False
        return PostgreCommonUtils.get_restore_status(job_id)

    @staticmethod
    def copy_files(os_user: str, src_path: str, target_path: str, wildcard=".", job_id=""):
        LOGGER.info(f"Start copying file: {src_path} to path: {target_path}")
        PostgreCommonUtils.check_path_exist(target_path)
        if os.path.isdir(src_path):
            src_path = f"{src_path}" if src_path.endswith("/") else f"{src_path}/"
        res = backup_files(job_id, [src_path], target_path, write_meta=True)
        if not res:
            LOGGER.error(f"Failed to start backup, jobId: {job_id}.")
            return False
        return PostgreCommonUtils.get_restore_status(job_id)

    @staticmethod
    def get_restore_status(job_id):
        restore_status = False
        while True:
            time.sleep(10)
            status, progress, data_size = query_progress(job_id)
            LOGGER.info(f"Get restore result: status:{status}, progress:{progress}, data_size:{data_size}")
            if status == BackupStatus.COMPLETED:
                LOGGER.info(f"Restore completed, jobId: {job_id}.")
                restore_status = True
                break
            elif status == BackupStatus.RUNNING:
                continue
            elif status == BackupStatus.FAILED:
                LOGGER.error(f"Restore failed, jobId: {job_id}.")
                restore_status = False
                break
            else:
                LOGGER.error(f"Backup failed, status error jobId: {job_id}.")
                restore_status = False
                break
        return restore_status

    @staticmethod
    def check_exist_process_by_reg(process_reg):
        LOGGER.info(f"Start checking if process exist ..., regex: {process_reg}")
        ps_cmd = f"ps -ef"
        return_code, std_out, std_err = pg_util_common.execute_cmd(ps_cmd)
        LOGGER.info(f"Execute view process command: {ps_cmd}, return code: {return_code}, "
                    f"out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"View process failed. Error Message: {std_err}.")
            raise Exception("View process failed")
        LOGGER.info(f"View process success.")
        split_num = -1
        pid_idx = 1
        cmd_idx = -1
        for idx, val in enumerate(std_out.split('\n')):
            if idx == 0:
                first_splits = val.strip().split()
                split_num = len(first_splits) - 1
                pid_idx = first_splits.index("PID")
                cmd_idx = first_splits.index("CMD")
                LOGGER.debug(f"Split num: {split_num}, pid index: {pid_idx}, cmd index: {cmd_idx}")
                continue
            each_splits = val.strip().split(None, split_num)
            if each_splits and re.match(process_reg, each_splits[cmd_idx].strip()):
                return True, each_splits[pid_idx].strip()
        return False, ""

    @staticmethod
    def kill_process(p_id):
        LOGGER.info(f"Start killing process: {p_id} ...")
        kill_process_cmd = f"kill -9 {p_id}"
        return_code, std_out, std_err = pg_util_common.execute_cmd(kill_process_cmd)
        LOGGER.info(f"Execute kill process command: {kill_process_cmd}, return code: {return_code}, "
                    f"out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Kill process: {p_id} failed. Error Message: {std_err}.")
            raise Exception("Kill process failed")
        LOGGER.info(f"Kill process: {p_id} success.")

    @staticmethod
    def configure_vip(if_up_cmd):
        LOGGER.info(f"Try to execute configure vip command: {if_up_cmd}.")
        if not if_up_cmd:
            LOGGER.warning("Configure vip command is empty.")
            return
        return_code, std_out, std_err = pg_util_common.execute_cmd(if_up_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute configure vip command failed, return code: {return_code}, out: {std_out}, "
                         f"err: {std_err}.")
            return
        LOGGER.info("Execute configure vip command success.")

    @staticmethod
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

    @staticmethod
    def manual_cleanup_archive_dir(tgt_archive_path):
        LOGGER.info(f"Try to cleanup archive directory: {tgt_archive_path} manually.")
        all_files = os.listdir(tgt_archive_path)
        for f_n in all_files:
            tmp_file_path = os.path.realpath(os.path.join(tgt_archive_path, f_n))
            if not check_path_valid(tmp_file_path, False):
                LOGGER.warn("path is invalid")
                continue
            if not os.path.isfile(tmp_file_path):
                continue
            # 清理WAL文件、.backup文件、.history文件
            if PostgreCommonUtils.is_wal_file(f_n) \
                    or re.match(r"^[0-9A-F]{24}\.[0-9A-F]{8}\.backup$", f_n) \
                    or re.match(r"^[0-9A-F]{8}\.history$", f_n):
                LOGGER.debug(f"Remove file: {f_n} from archive directory.")
                os.remove(tmp_file_path)
        LOGGER.info("Manually cleanup archive directory success.")

    @staticmethod
    def get_local_ips() -> list:
        """获取本机所有IP
        """
        LOGGER.info(f"Start getting all local ips ...")
        local_ips = []
        ip_dict = psutil.net_if_addrs()
        for _, v in ip_dict.items():
            for i in v:
                if i[0] == 2 and i[1] != '127.0.0.1':
                    local_ips.append(i[1])
        LOGGER.info(f"Get all local ips: {local_ips} success.")
        return local_ips

    @staticmethod
    def write_progress_info(cache_path, file_name, context: RestoreProgress):
        file_path = os.path.realpath(os.path.join(cache_path, str(file_name)))
        with GLOBAL_PG_PROCESS_FILE_LOCK:
            exec_overwrite_file(file_path, context.json(), json_flag=False)

    @staticmethod
    def read_process_info(cache_path, file_name):
        file_path = os.path.realpath(os.path.join(cache_path, file_name))
        if not os.path.isfile(file_path):
            return {}
        with GLOBAL_PG_PROCESS_FILE_LOCK:
            with open(file_path, 'r') as tmp_file:
                context = tmp_file.read()
                return json.loads(context) if context else {}

    @staticmethod
    def change_path_owner(param_dict, input_path, uid, gid, is_data_dir=True):
        LOGGER.info(f"Start changing owner of the path: {input_path}, uid: {uid}, gid: {gid}, is data: {is_data_dir}.")
        if not os.path.exists(input_path):
            LOGGER.warning(f"The input path: {input_path} does not exist.")
            return
        PostgreCommonUtils.check_path_islink(input_path)
        # 处理文件
        if os.path.isfile(input_path):
            os.lchown(input_path, uid, gid)
            LOGGER.info(f"Change owner of the file: {input_path} success, uid: {uid}, gid: {gid}.")
            return
        # 日志目录需要修改owner
        if not is_data_dir:
            LOGGER.info("Current input path is log dir, change its owner.")
            os.lchown(input_path, uid, gid)
        if not is_clone_file_system(param_dict):
            LOGGER.info("Current filesystem is not clone.")
            return
        # 处理目录
        for root, dirs, files in os.walk(input_path):
            for tmp_dir in dirs:
                if tmp_dir in PgConst.RESTORE_NOT_COPIED_DIRS:
                    continue
                os.lchown(os.path.join(root, tmp_dir), uid, gid)
            for tmp_file in files:
                os.lchown(os.path.join(root, tmp_file), uid, gid)
        LOGGER.info(f"Change owner of the path: {input_path} success, uid: {uid}, gid: {gid}.")

    @staticmethod
    def change_path_mode(input_path, mode, is_data_dir=True):
        LOGGER.info(f"Start changing mode of the path: {input_path}, mode: {mode}, is data: {is_data_dir}.")
        if not os.path.exists(input_path):
            LOGGER.warning(f"The input path: {input_path} does not exist.")
            return
        PostgreCommonUtils.check_path_islink(input_path)
        # 处理文件
        if os.path.isfile(input_path):
            change_path_permission(input_path, mode=mode)
            LOGGER.info(f"Change mode of the file: {input_path} success, mode: {mode}.")
            return
        # 处理目录下文件
        for root, _, files in os.walk(input_path):
            for tmp_file in files:
                tmp_file_path = os.path.join(root, tmp_file)
                # 传入不是data目录（日志目录），处理所有文件
                if not is_data_dir:
                    change_path_permission(tmp_file_path, mode=mode)
                    continue
                # 传入是data目录，只修改权限为”000“的文件
                if oct(os.stat(tmp_file_path).st_mode)[-3:] == '000':
                    change_path_permission(tmp_file_path, mode=mode)
        LOGGER.info(f"Change mode of the path: {input_path} success, mode: {mode}.")

    @staticmethod
    def check_port_is_listen(port):
        sock = None
        try:
            if not pg_check_utils.is_port(port):
                LOGGER.error(f"The port: {port} is invalid.")
                raise Exception("The port is invalid")
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            result = sock.connect_ex(('127.0.0.1', int(port)))
            if result != NumberConst.ZERO:
                LOGGER.info(f"Port {port} is not listen.")
                return False
            LOGGER.info(f"Port {port} is listen.")
            return True
        finally:
            if sock:
                sock.close()

    @staticmethod
    def get_wal_dir_name_by_version(version):
        wal_dir = "pg_wal"
        if int(version.split('.')[0]) <= PgConst.MAJOR_VERSION_NINE:
            wal_dir = "pg_xlog"
        return wal_dir

    @staticmethod
    def get_conf_item_status(cfg_file, start_str: str):
        LOGGER.info(f"Try to check if item is in config file, start of item: {start_str}.")
        status = ConfigKeyStatus.NOT_EXIST
        if not start_str:
            LOGGER.warning("Check if item is in config file, input item info is empty.")
            return status
        PostgreCommonUtils.check_file_path(cfg_file)
        with open(cfg_file, 'r') as tmp_file:
            lines = tmp_file.readlines()
        start_str = start_str.lstrip("#")
        LOGGER.info(f"start_str is, {start_str}.")
        for i in lines:
            if str(i).strip().startswith(f"#{start_str}"):
                status = ConfigKeyStatus.ANNOTATED
                break
            elif str(i).strip().startswith(start_str):
                LOGGER.info(f"match str i is, {str(i)}.")
                status = ConfigKeyStatus.CONFIGURED
                break
        LOGGER.info(f"Check if item is in config file success, status: {status}.")
        return status

    @staticmethod
    def check_special_characters(check):
        if check_command_injection(check):
            LOGGER.error(f"String contains special characters, check string:{check}!")
            return False
        return True

    @staticmethod
    def check_black_list(path):
        if re.search(PgConst.PATH_BLACK_LIST, path):
            LOGGER.error(f"Path in black list!")
            return False
        return True

    @staticmethod
    def check_path_characters_and_black_list(path):
        if check_command_injection(path):
            LOGGER.error(f"String contains special characters!")
            return False
        return True

    @staticmethod
    def check_path_in_white_list(path):
        try:
            real_path = os.path.realpath(path)
        except Exception:
            LOGGER.exception(f"The path is invalid!")
            return False, path
        if check_command_injection(real_path):
            return False, path
        if re.match(PgConst.DELETING_PATH_WHITELIST, real_path):
            return True, real_path
        return False, path

    @staticmethod
    def check_version_gt_nine_dot_four(pg_ver):
        """
        检查PostgreSQL版本号大于9.4
        """
        major_ver = int(pg_ver.split(".")[0])
        if major_ver > PgConst.MAJOR_VERSION_NINE:
            return True
        if major_ver < PgConst.MAJOR_VERSION_NINE:
            raise Exception("The postgresql version is not supported")
        minor_ver = int(pg_ver.split(".")[1])
        return minor_ver > PgConst.MINOR_VERSION_FOUR

    @staticmethod
    def get_local_recovery_conf_sample_by_ver(pg_ver):
        """
        获取本地的recovery.conf.sample文件路径
        """
        major_ver = int(pg_ver.split(".")[0])
        minor_ver = int(pg_ver.split(".")[1])
        if major_ver == PgConst.MAJOR_VERSION_NINE and PG_VER_SAMPLE_CFG_DIR_MAP.get(f"{major_ver}.{minor_ver}"):
            return os.path.join(
                PgConst.CURR_RECOVERY_CONF_SAMPLE_PATH, PG_VER_SAMPLE_CFG_DIR_MAP.get(f"{major_ver}.{minor_ver}"),
                PgConst.RECOVERY_CONF_SAMPLE_NAME)
        # 10、11版本
        elif major_ver in (PgConst.DATABASE_V10, PgConst.DATABASE_V11):
            return os.path.join(
                PgConst.CURR_RECOVERY_CONF_SAMPLE_PATH, PG_VER_SAMPLE_CFG_DIR_MAP.get(str(major_ver)),
                PgConst.RECOVERY_CONF_SAMPLE_NAME)
        else:
            LOGGER.error(f"The postgresql version: {pg_ver} is not supported when getting local sample file.")
            raise Exception("The postgresql version is not supported when getting local sample file")

    @staticmethod
    def exec_cmd_for_big_stdout(cmd, timeout=None):
        """针对大stdout场景执行cmd命令"""
        process = subprocess.Popen(shlex.split(cmd), stdin=None, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, encoding="utf-8")
        output, err = process.communicate(timeout=timeout)
        ret_code = process.poll()
        return str(ret_code), str(output), str(err)

    @staticmethod
    def filter_wal_files(input_path):
        all_files = os.listdir(input_path)
        wal_names = list()
        for i in all_files:
            if PostgreCommonUtils.is_wal_file(i):
                wal_names.append(i)
        return wal_names

    @staticmethod
    @check_transaction_decorator
    def check_transaction_after_target_time(os_user, tgt_install_path, log_path, tgt_time_str):
        exist_txn = False
        LOGGER.info(f"Checking transactions after recovery target time: {tgt_time_str}.")
        tgt_time = time.strptime(tgt_time_str, PgConst.RECOVERY_TARGET_TIME_FORMATTER)
        wal_dump_path = os.path.realpath(os.path.join(tgt_install_path, "bin", "pg_waldump"))
        PostgreCommonUtils.check_file_path(wal_dump_path)
        PostgreCommonUtils.check_path_islink(wal_dump_path)
        PostgreCommonUtils.check_dir_path(log_path)
        PostgreCommonUtils.check_path_islink(log_path)
        wal_names = PostgreCommonUtils.filter_wal_files(log_path)
        if not wal_names:
            LOGGER.warning("The log copy are empty when checking transaction.")
            return exist_txn
        enable_root = PostgreCommonUtils.get_root_switch()
        if not PostgreCommonUtils.check_os_user(os_user, wal_dump_path, enable_root)[0]:
            return False
        # WAL日志名称是16进制，转int型排序
        wal_names.sort(key=lambda x: int(x, 16))
        # 时间线为WAL名称的前8位
        curr_time_line = wal_names[-1][:8]
        # pg_waldump只支持同一时间线WAL日志分析
        same_timeline_wal_names = list(filter(lambda x: x[:8] == curr_time_line, wal_names))
        first_wal_p = os.path.realpath(os.path.join(log_path, same_timeline_wal_names[0]))
        last_wal_p = os.path.realpath(os.path.join(log_path, same_timeline_wal_names[-1]))
        wal_dump_cmd = cmd_format("su - {} -c '{} -b -r Transaction {} {} | tail -n 1'", os_user, wal_dump_path,
                                  first_wal_p, last_wal_p)
        LOGGER.info(f"Executing wal dump command: {wal_dump_cmd}.")
        # 执行pg_waldump命令，限制超时为1h=3600s
        return_code, std_out, std_err = PostgreCommonUtils.exec_cmd_for_big_stdout(wal_dump_cmd, timeout=3600)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute wal dump command failed, return code: {return_code}, "
                         f"out: {std_out}, err: {std_err}.")
            return exist_txn
        last_line = std_out.strip().split(os.linesep)[-1]
        LOGGER.info(f"Execute wal dump command success, last line: {last_line}.")
        wal_time_pattern = re.compile(PgConst.MATCH_WAL_TIME_REGEX)
        match_ret = wal_time_pattern.findall(last_line)
        if not match_ret:
            LOGGER.warning(f"The wal dump output last line does not contain time.")
            return exist_txn
        last_txn_time_str = match_ret[0]
        LOGGER.info(f"The last transaction time in the wal log is {last_txn_time_str}.")
        try:
            last_txn_time = time.strptime(last_txn_time_str, PgConst.RECOVERY_TARGET_TIME_FORMATTER)
        except (ValueError, TypeError):
            LOGGER.warning("Transition transaction time exception.")
            return exist_txn
        # 恢复时间大于WAL中最后完成事务时间
        if tgt_time > last_txn_time:
            LOGGER.warning(f"The transactions after recovery target time don't exist in the log copies.")
            return exist_txn
        exist_txn = True
        LOGGER.info(f"The transactions after recovery target time exist in the log copies.")
        return exist_txn

    @staticmethod
    def get_root_switch():
        current_dir = os.getcwd()
        conf_file = os.path.join(current_dir, 'applications', 'postgresql', 'conf', PgConst.OWNER_FILE_NAME)
        try:
            with open(conf_file, "r", encoding='utf-8') as f:
                result = json.loads(f.readlines()[1])
        except Exception as e:
            raise Exception("Read switch failed.") from e
        enable_root = result.get("enable_root", 0)
        return enable_root

    @staticmethod
    def check_os_user(os_username, file_path, enable_root=1):
        if os_username.strip() == "root":
            LOGGER.info("The os username can not be root")
            return False, ErrorCode.USER_IS_NOT_EXIST
        try:
            pwd.getpwnam(os_username)
        except KeyError:
            LOGGER.error(f"os_username: {os_username} is not exist")
            return False, ErrorCode.USER_IS_NOT_EXIST
        LOGGER.info(f"Begin to check os name! os_username: {os_username}")
        stat_info = os.stat(file_path)
        uid = stat_info.st_uid
        user = pwd.getpwuid(uid)[0]
        if not enable_root:
            if user != os_username:
                LOGGER.error(f"Os user name is incorrect! os_username: {os_username}")
                return False, ErrorCode.LOGIN_FAILED
        LOGGER.info(f"Success to check os name! os_username: {os_username}")
        return True, file_path

    @staticmethod
    def is_backup_wal_file(wal_file):
        if not re.match(r"^[0-9A-F]{24}\.[0-9A-F]{8}\.backup$", wal_file):
            return False
        timelines = wal_file.split(".")
        if not PostgreCommonUtils.is_wal_file(timelines[0]):
            return False
        return True

    @staticmethod
    def get_tmp_node_ip(obj):
        tmp_node_ip = obj.get("extendInfo", {}).get("subNetFixedIp", "")
        if tmp_node_ip:
            return tmp_node_ip
        return obj.get("endpoint", "")

    @staticmethod
    def get_tmp_node_ip_from_nodes(nodes, host_id):
        tmp_node_ip = ""
        for node in nodes:
            if host_id == node.get("id", ""):
                tmp_node_ip = PostgreCommonUtils.get_tmp_node_ip(node)
        return tmp_node_ip

    @staticmethod
    def write_content_to_file(file_path, content):
        exec_append_file(file_path, content)

    @staticmethod
    def get_patroni_config(host_ips, nodes):
        patroni_config = ''
        port = ''
        for node in nodes:
            node_extend_info = node.get("extendInfo", {})
            service_ip = node_extend_info.get('serviceIp', "")
            if service_ip in host_ips:
                patroni_config = node_extend_info.get('pgpoolClientPath', "")
                port = node_extend_info.get('instancePort', "")
                role = node_extend_info.get('role', "")
                break
        return patroni_config, port, role

    @staticmethod
    def check_db_user_valid(db_user):
        """
        校验数据库用户名、数据库流复制用户名
        """
        expression = "^[a-zA-Z_][a-zA-Z0-9_]{0,62}$"
        if PostgreCommonUtils.check_special_characters(db_user) and re.search(expression, db_user):
            return True
        return False

    @staticmethod
    def report_job_details(job_id: str, sub_job_details: dict):
        # 主动上报任务
        try:
            cur_time = str(int((time.time())))
            result_info = invoke_rpc_tool_interface(job_id + cur_time, "ReportJobDetails", sub_job_details)
        except Exception as err:
            LOGGER.error(f"Invoke rpc_tool interface exception, err: {err}.")
            return False
        if not result_info:
            return False
        ret_code = result_info.get("code", -1)
        if ret_code != int(CMDResult.SUCCESS):
            LOGGER.error(f"Invoke rpc_tool interface failed, result code: {ret_code}.")
            return False
        return True

    @staticmethod
    def aes_ecb_encrypt(key, plaintext):
        # 加密函数
        # 填充数据，确保长度符合AES要求
        padder = padding.PKCS7(algorithms.AES.block_size).padder()
        padded_data = padder.update(plaintext.encode()) + padder.finalize()
        # 初始化加密器
        backend = default_backend()
        cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=backend)
        encryptor = cipher.encryptor()
        # 加密数据
        ciphertext = encryptor.update(padded_data) + encryptor.finalize()
        return base64.b64encode(ciphertext).decode('utf-8')

    @staticmethod
    def to_db_text(data):
        # 复刻clup对密码的加密
        jd = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
        vu = "5k12IuKPrThB3t9LoYU8g*nW4pJGvi7eSs-yQNcaEHA6fDVjRdMlqzwbm+F0xCZXO"
        kne = {}
        for index, char in enumerate(vu):
            kne[jd[index]] = char
        tem = []
        data = PostgreCommonUtils.aes_ecb_encrypt(b'3743535544415441', data)
        for index, char in enumerate(data):
            if index < len(data):
                tem.append(kne.get(char))
        result = "".join(tem) + "A"
        return result
