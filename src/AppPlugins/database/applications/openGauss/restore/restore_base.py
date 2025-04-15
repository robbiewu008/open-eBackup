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

import abc
import os

from openGauss.common.const import logger, MetaDataKey, ParamKey, AuthKey, ProtectObject, Tool
from common.const import RepositoryDataTypeEnum
from common.file_common import delete_file_or_dir_specified_user
from openGauss.backup.resource_info import ResourceInfo
from openGauss.common.common import get_value_from_dict, get_repository_dir, \
    execute_cmd_by_user, check_path, safe_get_environ, check_injection_char, is_cmdb_distribute


class RestoreBase(metaclass=abc.ABCMeta):
    log = logger

    def __init__(self, pid, job_id, param):
        self._pid = pid
        self._job_id = job_id
        self._user_name = safe_get_environ(f"{AuthKey.TARGET_ENV}{self._pid}")
        self._backup_dir = ""
        self._cache_dir = ""
        self._resource_info = None
        self._port = ""
        self._source_cmd = ""
        self._user_group = ""
        self._copy_id = ""
        self._copy_index_id = ""
        self._target_name = ""
        self._copy_version = ""
        self._env_file = ""
        self._sql_tool = ""
        self._guc_tool = ""
        self._database_type = ""
        self._deploy_type = ""
        self.init_environment(param)
        self._param = param

    def get_copy_repository_dir(self, copy, repository_type):
        """
        获取副本仓
        :return:
              path: 仓绝对路径
        """
        self.log.info(f'Get repository by repository type. job id: {self._job_id}')
        ret, repositories = get_value_from_dict(copy, ParamKey.REPOSITORIES)
        if not ret or len(repositories) == 0:
            self.log.error(f'Get repositories failed. job id: {self._job_id}')
            return ""
        return get_repository_dir(repositories, repository_type)

    def init_environment(self, param):
        """
        初始化环境信息
        :return:
        """
        _, env_file = get_value_from_dict(param, ParamKey.JOB, ParamKey.TARGET_ENV, ParamKey.EXTEND_INFO,
                                            ParamKey.ENV_FILE)
        self.init_resource_obj(env_file)
        self._port = self._resource_info.get_instance_port()
        _, copyies = get_value_from_dict(param, ParamKey.JOB, ParamKey.COPYIES)
        self.init_copy_info(copyies)
        _, new_target_name = get_value_from_dict(param, ParamKey.JOB, ParamKey.EXTEND_INFO, ParamKey.NEW_NAME)
        if isinstance(new_target_name, str) and check_injection_char(new_target_name):
            self._target_name = new_target_name
        _, self._database_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.TARGET_ENV, ParamKey.EXTEND_INFO,
                                                     ParamKey.CLUSTER_VERSION)
        _, self._deploy_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.TARGET_ENV, ParamKey.EXTEND_INFO,
                                                     ParamKey.DEPLOY_TYPE)
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type or \
                ProtectObject.CMDB in self._database_type:
            self._sql_tool = Tool.GSQL
        else:
            self._sql_tool = Tool.VSQL

    def init_resource_obj(self, env_file):
        if isinstance(env_file, str) and os.path.isfile(env_file) and check_path(env_file):
            self._resource_info = ResourceInfo(self._user_name, env_file)
            self._env_file = env_file
        else:
            self._resource_info = ResourceInfo(self._user_name)

    def init_copy_info(self, copyies):
        if len(copyies) < 1:
            return
        copy = copyies[0]
        self._backup_dir = self.get_copy_repository_dir(copy, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._cache_dir = self.get_copy_repository_dir(copy, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        _, restore_type = get_value_from_dict(copy, MetaDataKey.TYPE)
        if restore_type == "tapeArchive" or restore_type == "s3Archive":
            _, extend_info = get_value_from_dict(copy, ParamKey.EXTEND_INFO, ParamKey.EXTEND_INFO)
        else:
            _, extend_info = get_value_from_dict(copy, ParamKey.EXTEND_INFO)
        _, self._copy_id = get_value_from_dict(extend_info, MetaDataKey.COPY_ID)
        _, self._copy_index_id = get_value_from_dict(extend_info, MetaDataKey.BACKUP_INDEX_ID)
        _, self._copy_version = get_value_from_dict(extend_info, MetaDataKey.PROTECT_OBJECT,
                                                    MetaDataKey.TYPE)
        _, old_target_name = get_value_from_dict(extend_info, MetaDataKey.PROTECT_OBJECT,
                                                 MetaDataKey.PROTECT_NAME)
        if isinstance(old_target_name, str) and check_injection_char(old_target_name):
            self._target_name = old_target_name

    def clean_cache_file(self):
        """
        清理临时文件
        :return:成功返回True， 失败返回False
        """
        tmp_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.exists("tmp_file"):
            if not delete_file_or_dir_specified_user(self._user_name, tmp_file):
                return False
        speed_file = os.path.join(self._cache_dir, f"{self._job_id}_speed_record")
        if os.path.exists(speed_file):
            delete_file_or_dir_specified_user(self._user_name, speed_file)
        return True

    def exec_sql_cmd(self, cmd):
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            cmd = f'{self._sql_tool} -c \"{cmd}\" postgres -p {self._resource_info.get_local_cn_port()}'
        else:
            cmd = f'{self._sql_tool} -c \"{cmd}\" postgres -p {self._port}'
        return execute_cmd_by_user(self._user_name, self._env_file, cmd)

    @abc.abstractmethod
    def restore_prerequisite(self, param):
        """
        恢复前置
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def restore(self):
        """
        恢复
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def restore_post(self):
        """
        恢复后置
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def restore_progress(self):
        """
        恢复进度
        :return:
               progress: 进度
               status: 状态
        """
        pass
