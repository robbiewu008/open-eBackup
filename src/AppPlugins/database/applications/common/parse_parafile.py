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
from typing import List

from common.const import RepositoryDataTypeEnum, ParamConstant, SysData
from common.common import check_path_legal, check_command_injection, exter_attack
from validation.validator import ParamValidator

if platform.system().lower() != "windows":
    import pwd


class ParamFileUtil:
    @staticmethod
    @exter_attack
    def parse_param_file(pid, remove_after_read: bool = True):
        """
        This method is deprecated. Use parse_param_file_and_valid() instead.
        解析参数文件
        :return:
        """
        # 读取文件超时
        file_name = "{}{}".format("param", pid)
        file_path = os.path.join(ParamConstant.PARAM_FILE_PATH, file_name)
        if not os.path.exists(file_path):
            raise Exception(f"File:{file_path} not exist")
        with open(file_path, 'r') as f_read:
            param_dict = json.load(f_read)

        if remove_after_read:
            os.remove(file_path)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_file_and_valid(pid, paths: List[str]):
        """
        解析参数文件, 并校验参数
        :return:
        """
        param_dict = ParamFileUtil.parse_param_file(pid)

        ParamValidator.valid(param_dict, paths)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_file_and_valid_by_schema(pid, path: str):
        """
        解析参数文件, 并校验参数
        :return:
        """
        param_dict = ParamFileUtil.parse_param_file(pid)

        ParamValidator.valid_data(param_dict, path)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_file_and_valid(pid, paths: List[str]):
        """
        解析参数文件, 并校验参数
        :return:
        """
        param_dict = ParamFileUtil.parse_param_file(pid)

        ParamValidator.valid(param_dict, paths)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_windows_file(pid, remove_after_read: bool = True):
        """
        This method is deprecated. Use parse_param_windows_file_and_valid() instead.
        解析参数文件
        :return:
        """
        # 读取文件超时
        file_name = "{}{}".format("param", pid)
        file_path = f"{ParamConstant.WINDOWS_PARAM_FILE_PATH}/{file_name}"

        if not os.path.exists(file_path):
            raise Exception(f"File:{file_path} not exist")
        with open(file_path, 'r') as f:
            param_dict = json.load(f)

        if remove_after_read:
            os.remove(file_path)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_windows_file_and_valid(pid, paths: List[str]):
        """
        解析参数文件, 并校验参数
        :return:
        """
        param_dict = ParamFileUtil.parse_param_windows_file(pid)
        ParamValidator.valid(param_dict, paths)
        return param_dict

    @staticmethod
    @exter_attack
    def parse_param_windows_file_and_valid_by_schema(pid, path: str):
        """
        解析参数文件, 并校验参数
        :return:
        """
        param_dict = ParamFileUtil.parse_param_windows_file(pid)

        ParamValidator.valid_data(param_dict, path)
        return param_dict

    @staticmethod
    def parse_backup_type(job_param: dict):
        """
        解析备份类型
        :param job_param:
        :return:
        """
        return job_param.get("backupType")

    @staticmethod
    def parse_backup_path(repositories: list):
        """
        解析数据仓信息
        :param repositories:
        :return:
        """
        backup_dirs = dict()
        for repository in repositories:
            repository_type = repository.get("repositoryType")
            if repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                backup_dirs["data_repository"] = repository.get("path")
            elif repository_type == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                backup_dirs["cache_repository"] = repository.get("path")
            elif repository_type == RepositoryDataTypeEnum.LOG_REPOSITORY:
                backup_dirs["log_repository"] = repository.get("path")
            elif repository_type == RepositoryDataTypeEnum.META_REPOSITORY:
                backup_dirs["meta_repository"] = repository.get("path")
        return backup_dirs

    @staticmethod
    def parse_copy_id(copy_info: list):
        """
        解析copy_id
        :param copy_info:
        :return:
        """
        copy_id = None
        for copy in copy_info:
            if copy.get("id"):
                copy_id = copy.get("id")
        return copy_id

    @staticmethod
    def parse_database_type(job_param: dict):
        """
        解析数据库类型
        :param job_param:
        :return:
        """
        return job_param.get("job", {}).get("protectObject", {}).get("subType", "")

    @staticmethod
    def get_rep_info(job_param: dict, rep_type):
        for rep in job_param.get("repositories", []):
            if rep.get("repositoryType") == rep_type:
                return rep
        raise Exception("Param of reps err")


class CopyParamParseUtil:
    @staticmethod
    @exter_attack
    def get_copies(param):
        return param.get("job", {}).get("copies", [])

    @staticmethod
    @exter_attack
    def get_backup_type(param):
        return param.get("extendInfo", {}).get("backupType", 0)

    @staticmethod
    @exter_attack
    def get_copy_type(param):
        return param.get("type", "")

    @staticmethod
    @exter_attack
    def get_copy_id(param):
        copy_id = param.get("id", "")
        if check_command_injection(copy_id):
            raise Exception(f"Param of copy_id invalid.{copy_id}")
        return copy_id

    @staticmethod
    @exter_attack
    def get_cache_path(param):
        for rep in param.get("repositories", []):
            if rep.get("repositoryType") == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                if len(rep.get("path")) > 0 and check_path_legal(rep.get("path")[0], "/mnt/databackup/"):
                    return rep.get("path")[0]
        raise Exception('Param of cache_path err')

    @staticmethod
    @exter_attack
    def get_meta_path(param):
        for rep in param.get("repositories", []):
            if rep.get("repositoryType") == RepositoryDataTypeEnum.META_REPOSITORY:
                if len(rep.get("path")) > 0 and check_path_legal(rep.get("path")[0], "/mnt/databackup/"):
                    return rep.get("path")[0]
        raise Exception('Param of meta_path err')

    @staticmethod
    @exter_attack
    def get_data_reps(param):
        data_reps = []
        for rep in param.get("repositories", []):
            if rep.get("repositoryType") == RepositoryDataTypeEnum.DATA_REPOSITORY:
                if len(rep.get("path")) > 0 and check_path_legal(rep.get("path")[0], "/mnt/databackup/"):
                    data_reps.append(rep)

        if len(data_reps) == 0:
            raise Exception('Param of data_reps err')
        return data_reps

    @staticmethod
    @exter_attack
    def get_log_reps(param):
        log_reps = []
        for rep in param.get("repositories", []):
            if rep.get("repositoryType") == RepositoryDataTypeEnum.LOG_REPOSITORY:
                if len(rep.get("path")) > 0 and check_path_legal(rep.get("path")[0], "/mnt/databackup/"):
                    log_reps.append(rep)

        if len(log_reps) == 0:
            raise Exception('Param of log_reps err')
        return log_reps


def get_user_name(str_name: str):
    user_name = ''
    input_str = json.loads(SysData.SYS_STDIN)
    if input_str.get(str_name):
        user_name = input_str.get(str_name)
        try:
            pwd.getpwnam(user_name)
        except Exception:
            return ""
        if user_name == "root":
            return ""
        return user_name.strip()
    return ""


def get_env_variable(str_env_variable: str):
    env_variable = ''
    input_dict = json.loads(SysData.SYS_STDIN)
    if input_dict.get(str_env_variable):
        env_variable = input_dict.get(str_env_variable)
    return env_variable


def add_env_param(key: str, value: str):
    input_dict = json.loads(SysData.SYS_STDIN)
    input_dict[key] = value
    env_dict_str = json.dumps(input_dict)
    SysData.SYS_STDIN = env_dict_str
