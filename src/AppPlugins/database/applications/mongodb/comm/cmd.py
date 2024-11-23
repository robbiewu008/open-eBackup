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

import sys
from contextlib import contextmanager
from functools import wraps
from inspect import signature

import pwd

from common.cleaner import clear
from common.common import check_sql_cmd_param, execute_cmd, check_command_injection, execute_cmd_with_expect
from common.const import SysData, AuthType
from common.exception.common_exception import ErrCodeException

from common.file_common import change_path_permission

from common.util.check_user_utils import check_path_owner

from common.util.check_utils import check_path_in_white_list
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd, ExecFuncParam, \
    su_exec_cmd_list, su_exec_cat_cmd
from common.util.validators import ValidatorEnum
from mongodb import LOGGER
from mongodb.comm.const import EnvName, CMDResult, ParamField
from mongodb.comm.const import MongoTool, DefaultValue
from mongodb.comm.utils import get_stdin_field


def register_check(*check_args, **check_kwargs):
    """
    注册校验规则函数
    check_args: tuple[check_func]
    check_kwargs: {check_func_name:check_func}
    :return: wrapper
    e.g:  register_check(check_func1,path_check=path_check_func,cmd_check=cmd_check_func)
    """

    def decorate(func):
        sig = signature(func)
        bound_types = sig.bind_partial(*check_args, **check_kwargs).arguments

        @wraps(func)
        def wrapper(*args, **kwargs):
            bound_val = sig.bind(*args, **kwargs).arguments
            for name, val in bound_val.items():
                if name in bound_types:
                    if val is None:
                        del kwargs[name]
                    else:
                        kwargs[name] = (bound_types[name], val)
            return func(*args, **kwargs)

        return wrapper

    return decorate


class Checker:
    @staticmethod
    def sql_check(sql_arg):
        if not check_sql_cmd_param(sql_arg):
            raise ErrCodeException(err_code=0, message="Invalid sql cmd params")

    @staticmethod
    def path_check(path_arg):
        if not check_path_in_white_list(path_arg):
            raise ErrCodeException(err_code=0, message="Invalid path params")

    @staticmethod
    def cmd_check(cmd_arg):
        if check_command_injection(cmd_arg):
            raise ErrCodeException(err_code=0, message="Invalid cmd params")

    @classmethod
    @register_check(
        path_args=path_check,
        sql_args=sql_check,
        cmd_args=cmd_check
    )
    def check_args(cls, path_args=None, cmd_args=None, sql_args=None):

        def decorate(func):
            sig = signature(func)
            chk_args = [arg for arg in (path_args, cmd_args, sql_args) if arg is not None]
            # 获取待检验参数名与校验函数映射关系
            unpacked_args = {
                arg_name: check_func
                for (check_func, check_args) in chk_args
                for arg_name in check_args
            }

            @wraps(func)
            def wrapper(*args, **kwargs):
                bound_val = sig.bind(*args, **kwargs).arguments
                for name, val in bound_val.items():
                    if name in unpacked_args:
                        check_func = unpacked_args.get(name)
                        if isinstance(check_func, staticmethod):
                            exe_func = check_func.__func__
                        elif isinstance(check_func, classmethod):
                            exe_func = check_func.__func__
                        else:
                            exe_func = check_func
                        if not callable(exe_func):
                            raise ErrCodeException(
                                err_code=0,  # 错误码待替换
                                message="Unexpected function to register on check wrapper."
                            )
                        if val:
                            exe_func(val)
                return func(*args, **kwargs)

            return wrapper

        return decorate


class Cmd:
    """
        参数检验与安全校验后的公共命令类，
        sys.stdin输入流统一处理入口
    """

    def __init__(self, pid=None):
        self._base_cmd = "su - {} -c \"{}\""
        self._timeout = 30
        self.pid = pid
        self._init_stdin()

    @staticmethod
    def get_mount_info():
        filename = "/etc/fstab"
        # 获取文件的元数据信息
        stat_info = os.stat(filename)
        # 获取属主用户的用户名
        owner_username = pwd.getpwuid(stat_info.st_uid).pw_name
        return su_exec_cat_cmd(filename, owner_username)

    @staticmethod
    def get_bin_path(path):
        cmd = f"which {path}"
        return_code, out_info, err_info = execute_cmd(cmd)
        ret = (return_code == CMDResult.SUCCESS.value)
        if not ret:
            LOGGER.error(f"Execute cmd wrong, return_code: {return_code}, out_info: {out_info}, "
                         f"err_info: {err_info}")
            return_code, whoami, err_info = execute_cmd("whoami")
            if return_code != CMDResult.SUCCESS.value:
                LOGGER.error(f"Execute cmd wrong, return_code: {return_code}, out_info: {whoami}, "
                             f"err_info: {err_info}")
                return path
            cmd = cmd_format("su - {} -c 'which {}'", whoami.strip(), path)
            return_code, out_info, err_info = execute_cmd(cmd)
            if return_code != CMDResult.SUCCESS.value:
                LOGGER.error(f"Execute cmd wrong, return_code: {return_code}, out_info: {out_info}, "
                             f"err_info: {err_info}")
                return path
        return out_info

    @staticmethod
    def _init_stdin():
        """ 获取只需用的字段信息，并清除其余字段敏感信息"""
        input_string = sys.stdin.readline()
        input_dict = json.loads(input_string)
        clear(input_string)
        if not input_dict:
            return
        SysData.SYS_STDIN = json.dumps(input_dict)
        del input_dict

    @staticmethod
    def _execute_cmd(cmd):
        return_code, out_info, err_info = execute_cmd(cmd)
        ret = (return_code == CMDResult.SUCCESS.value)
        if not ret:
            LOGGER.error(f"Execute cmd wrong, return_code: {return_code}, out_info: {out_info}, err_info: {err_info}")
        res_cont = out_info if ret else err_info
        return ret, res_cont

    def get_db_user(self, node_id=""):
        db_user_key = self.get_db_key(node_id)
        return get_stdin_field(db_user_key)

    def execute_cmd_by_os_user(self, cmd, path):
        user, path = self.get_os_user(path)
        return_code, out_info, err_info = execute_cmd(self._base_cmd.format(user, cmd))
        ret = (return_code == CMDResult.SUCCESS.value)
        if not ret:
            LOGGER.error(f"Execute cmd wrong, return_code: {return_code}, out_info: {out_info}, err_info: {err_info}")
        res_cont = out_info if ret else err_info
        return ret, res_cont

    def get_os_user(self, path):
        if ParamField.FORWARD_SLASH.value not in path:
            path = self.get_bin_path(path).replace("\n", "")
        stat_info = os.stat(path)
        uid = stat_info.st_uid
        user = pwd.getpwuid(uid)[0]
        return user, path

    def get_db_key(self, node_id=""):
        return "_".join((EnvName.DB_USER_NAME.format(node_id), self.pid))

    def get_db_user_auth_type_key(self, node_id=""):
        return "_".join((EnvName.DB_AUTH_TYPE.format(node_id), self.pid))

    def get_db_pwd_key(self, node_id=""):
        return "_".join((EnvName.DB_PASSWORD.format(node_id), self.pid))

    def get_db_restore_job_auth_type_key(self, node_id=""):
        return "_".join((EnvName.CUSTOM_SETTINGS.format(node_id), self.pid))

    def get_db_pwd(self, node_id=""):
        key = self.get_db_pwd_key(node_id)
        return get_stdin_field(key)

    def get_db_user_auth_type(self, node_id=""):
        key = self.get_db_user_auth_type_key(node_id)
        return get_stdin_field(key)

    def get_db_restore_job_auth_type(self, node_id=""):
        key = self.get_db_restore_job_auth_type_key(node_id)
        return get_stdin_field(key)

    @Checker.check_args(
        cmd_args=["user_name", "des_path"],
        path_args=["des_path"]
    )
    def delete_specified_user(self, user_name, des_path):
        """
        指定用户删除文件夹下指定用户文件或文件夹
        :param user_name: 用户名
        :param des_path: 指定路径
        :return: Bool
        """
        return su_exec_rm_cmd(des_path, user_name)

    @Checker.check_args(
        cmd_args=["os_user", "path"],
        path_args=["path"]
    )
    def mk_user_dir(self, path, os_user):
        """
        创建指定用户属主的目录
        :param path: 目录路径
        :param os_user: 属主
        :return:
        """
        return exec_mkdir_cmd(path, os_user)

    @Checker.check_args(
        cmd_args=["db_path"],
        path_args=["db_path"]
    )
    def get_lvm_info(self, db_path):
        df_cmd = f"df -Th {db_path}"
        return self._execute_cmd(df_cmd)

    def show_lv_info(self):
        lvs = "lvs -o name,vg_name,path,size --units m"
        ret, lv_info = self._execute_cmd(lvs)
        report_list = []
        if ret and len(lv_info) > 1:
            split_lines = lv_info.splitlines()
            lv_list = []
            for row in split_lines[1:]:
                row = row.strip()
                columns = re.split(r' +', row)
                lv_list.append({
                    "lv_name": columns[0].strip(),
                    "vg_name": columns[1].strip(),
                    "lv_path": columns[2].strip(),
                    "lv_size": columns[3].strip()
                })
            report_list.append({"lv": lv_list})
        result = {"report": report_list}
        return ret, result

    def show_vg_info(self):
        lvs = "vgs -o name,free --units m"
        ret, vg_info = self._execute_cmd(lvs)
        report_list = []
        if ret and len(vg_info) > 1:
            split_lines = vg_info.splitlines()
            vg_list = []
            for row in split_lines[1:]:
                row = row.strip()
                columns = re.split(r' +', row)
                vg_list.append({
                    "vg_name": columns[0].strip(),
                    "vg_free": columns[1].strip()
                })
            report_list.append({"vg": vg_list})
        result = {"report": report_list}
        return ret, result

    @Checker.check_args(
        cmd_args=["size", "name", "lvm_name"]
    )
    def create_snap_shot(self, size, name, lvm_name):
        lvcreatecmd = "lvcreate --size {} --snap --name {} {}"
        fmt_lvcreatecmd = cmd_format(lvcreatecmd, size, name, lvm_name)
        return self._execute_cmd(fmt_lvcreatecmd)

    @Checker.check_args(
        cmd_args=["snap_full_path", "destination"],
        path_args=["destination"]
    )
    def mount(self, snap_full_path, destination):
        # 此处mount的是设备文件，此处不使用公共方法，不校验是否为目录
        mount = f"mount {snap_full_path} {destination}"
        return self._execute_cmd(mount)

    @Checker.check_args(
        cmd_args=["snap_full_path", "destination"],
        path_args=["destination"]
    )
    def mount_xfs(self, snap_full_path, destination):
        mount = f"mount -o nouuid {snap_full_path} {destination}"
        return self._execute_cmd(mount)

    @Checker.check_args(
        cmd_args=["snap_full_path"],
        path_args=["snap_full_path"]
    )
    def umount(self, snap_full_path):
        """

        :param snap_full_path: 快照全路径
        :return:
        """
        # 此处mount的是设备文件，此处不使用公共方法，不校验是否为目录
        mount = f"umount {snap_full_path}"
        return self._execute_cmd(mount)

    @Checker.check_args(
        cmd_args=["snap_full_path"]
    )
    def lvremove(self, snap_full_path):
        lvremove = f"lvremove -f {snap_full_path}"
        return self._execute_cmd(lvremove)

    @Checker.check_args(
        cmd_args=["mongod_bin_dir", "conf_file_path"],
        path_args=["mongod_bin_dir", "conf_file_path"]
    )
    def start_up_instance(self, mongod_bin_dir, conf_file_path, user):
        param = ExecFuncParam(os_user=user, cmd_list=['{mongod_bin_dir} -f {conf_file_path}'],
                              fmt_params_list=[[("mongod_bin_dir", mongod_bin_dir, ValidatorEnum.CHAR_CHK_COMMON),
                                                ("conf_file_path", conf_file_path, ValidatorEnum.PATH_CHK_FILE)]],
                              chk_exe_owner=False)
        result, error_param = su_exec_cmd_list(param)
        return (result == CMDResult.SUCCESS.value), error_param

    @Checker.check_args(
        cmd_args=["mongod_bin_dir"]
    )
    def check_instance_user_exist(self, mongod_bin_dir, user):
        param = ExecFuncParam(os_user=user, cmd_list=['{mongod_bin_dir} --version'],
                              fmt_params_list=[[("mongod_bin_dir", mongod_bin_dir, ValidatorEnum.CHAR_CHK_COMMON)]],
                              chk_exe_owner=False)
        result, error_param = su_exec_cmd_list(param)
        return (result == CMDResult.SUCCESS.value), error_param

    @Checker.check_args(
        cmd_args=["conf_file_path"],
        path_args=["conf_file_path"]
    )
    def chown_conf_path_permissions(self, conf_file_path, user):
        return change_path_permission(conf_file_path, user)

    @Checker.check_args(
        cmd_args=["mongos_bin_dir", "conf_file_path"],
        path_args=["mongos_bin_dir", "conf_file_path"]
    )
    def start_up_mongos_instance(self, mongos_bin_dir, conf_file_path):
        start_up_instance_cmd = "%s -f %s" % (mongos_bin_dir, conf_file_path)
        return self.execute_cmd_by_os_user(start_up_instance_cmd, mongos_bin_dir)

    @Checker.check_args(
        cmd_args=["mongo_tool_type"],
        path_args=["mongo_tool_type"]
    )
    def check_mongo_tool(self, mongo_tool_type):
        mongo_check_cmd = "%s --help" % mongo_tool_type
        LOGGER.debug(f"Check mongo tool: {mongo_check_cmd}")
        return self.execute_cmd_by_os_user(mongo_check_cmd, mongo_tool_type)

    @Checker.check_args(
        cmd_args=["mongo_tool_type"],
        path_args=["mongo_tool_type"]
    )
    def check_mongo_version(self, mongo_tool_type):
        mongo_check_cmd = "%s --version" % mongo_tool_type
        LOGGER.debug(f"Check mongo version: {mongo_check_cmd}")
        return self.execute_cmd_by_os_user(mongo_check_cmd, mongo_tool_type)

    @Checker.check_args(
        cmd_args=["mongorestore_bin_dir", "host_url", "log_copy_path"],
        path_args=["mongorestore_bin_dir", "log_copy_path"]
    )
    def oplog_restore_copy(self, mongorestore_bin_dir, host_url, log_copy_path):
        oplog_restore_copy_cmd = "%s -h %s --oplogReplay %s" % (mongorestore_bin_dir, host_url, log_copy_path)
        return self.execute_cmd_by_os_user(oplog_restore_copy_cmd, mongorestore_bin_dir)

    @Checker.check_args(
        cmd_args=["host_url", "last_copie_end_lsn", "last_log_copies_data_path"],
        path_args=["last_log_copies_data_path"]
    )
    def oplog_restore_timestamp(self, mongorestore_bin_dir, host_url, last_copie_end_lsn, last_log_copies_data_path):
        oplog_restore_timestamp_cmd = "%s -h  %s --oplogReplay  --oplogLimit %s %s " % (
            mongorestore_bin_dir, host_url, last_copie_end_lsn, last_log_copies_data_path)
        return self.execute_cmd_by_os_user(oplog_restore_timestamp_cmd, mongorestore_bin_dir)

    @Checker.check_args(
        cmd_args=["dst", "node_id", "uri", "mongo_bin_dir"],
        path_args=["dst", "mongo_bin_dir"]
    )
    def dump(self, query, mongo_bin_dir, uri, dst, node_id):
        auth_type = self.get_db_user_auth_type()
        mongo_dump = MongoTool.MONGODUMP.value
        if mongo_bin_dir:
            mongo_dump = mongo_bin_dir + ParamField.FORWARD_SLASH + MongoTool.MONGODUMP.value
        auth = ""
        if auth_type != str(AuthType.APP_PASSWORD.value):
            dump_cmd = f"{mongo_dump} -h {uri} {auth} -d {DefaultValue.LOCAL_DB.value}" \
                       f" -c {DefaultValue.OPLOG_COLLECTION.value}" \
                       f" --query='{query}' -o {dst}"
            return self.execute_cmd_by_os_user(dump_cmd, mongo_dump)

        username = self.get_db_user(node_id)
        auth = f"--authenticationDatabase admin -u {username}"
        db_key = self.get_db_pwd_key(node_id)
        dump_cmd = f"{mongo_dump} -h {uri} {auth} -d {DefaultValue.LOCAL_DB.value}" \
                   f" -c {DefaultValue.OPLOG_COLLECTION.value}" \
                   f" --query='{query}' -o {dst}"
        user, path = self.get_os_user(mongo_dump)
        dump_cmd = self._base_cmd.format(user, dump_cmd)
        return self._execute_cmd_with_expect(dump_cmd, db_key)

    def check_mongo_user_and_path(self, path):
        os_user, envpath = self.get_os_user(path)
        if not check_path_owner(envpath, [os_user]):
            LOGGER.error(f"The owner of path: {envpath} is not {os_user}.")
            return False
        return True

    @contextmanager
    def _safe_get_key(self, env_key):
        yield env_key
        clear(env_key)

    def shutdown_instance(self, mongod_bin_dir, user, db_path):
        shutdown_cmd = "su - %s -c '%s --shutdown --dbpath %s'" % (user, mongod_bin_dir, db_path)
        LOGGER.info(f'Shut down the instance: {shutdown_cmd}')
        return self._execute_cmd(shutdown_cmd)

    def _execute_cmd_with_expect(self, cmd, env_key, time_out: bool = False):
        """

        :param cmd:
        :param time_out：是否支持超时
        :return:
        """
        if time_out:
            timeout = self._timeout
        else:
            timeout = None
        with self._safe_get_key(get_stdin_field(env_key)) as key:
            return_code, std_out, std_err = execute_cmd_with_expect(cmd, key, timeout)
        ret = (return_code == 0)
        res_cont = std_out if ret else std_err
        if not ret:
            LOGGER.debug(f"execute with expect: {ret}, result: {res_cont}")
        return ret, res_cont
