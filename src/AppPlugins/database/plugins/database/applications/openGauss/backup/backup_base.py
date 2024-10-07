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
import psutil

from common.file_common import change_path_permission, get_user_info
from openGauss.common.const import logger, MetaDataKey, ParamKey, AuthKey, ProtectObject, Tool
from openGauss.common.common import get_repository_dir, get_value_from_dict, \
    execute_cmd_by_user, check_path, safe_get_environ, check_injection_char, safe_remove_path
from openGauss.backup.resource_info import ResourceInfo
from common.const import RepositoryDataTypeEnum


class BackupBase(metaclass=abc.ABCMeta):
    log = logger

    def __init__(self, pid, job_id, param_json):
        self._pid = pid
        self._job_id = job_id
        self._user_name = safe_get_environ(f"{AuthKey.PROBECT_ENV}{self._pid}")
        self._backup_tool = ""
        self._source_cmd = ""
        self._resource_info = None
        self._database_type = ""
        self._backup_dir = ""
        self._cache_dir = ""
        self._log_dir = ""
        self._object_name = ""
        self._protect_obj = {}
        self._data_dir = ""
        self._port = ""
        self._env_file = ""
        self._sql_tool = ""
        self.init_resource(param_json)
        self.replica_repository_chown()

    def init_resource(self, param):
        ret, env_file = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                            ParamKey.ENV_FILE)
        if isinstance(env_file, str) and os.path.isfile(env_file) and check_path(env_file):
            self._resource_info = ResourceInfo(self._user_name, env_file)
            self._env_file = env_file
        else:
            self._resource_info = ResourceInfo(self._user_name)
        _, self._database_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                                     ParamKey.CLUSTER_VERSION)
        # 日志备份的仓库目录为LOG_REPOSITORY
        self._backup_dir = self.get_repository_dir(param, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._cache_dir = self.get_repository_dir(param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self._meta_dir = self.get_repository_dir(param, RepositoryDataTypeEnum.META_REPOSITORY)
        self._log_dir = self.get_repository_dir(param, RepositoryDataTypeEnum.LOG_REPOSITORY)
        _, object_name = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.NAME)
        if isinstance(object_name, str) and check_injection_char(object_name):
            self._object_name = object_name
        _, self._protect_obj = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_OBJECT)
        self._data_dir = self._resource_info.get_instance_data_path()
        self._port = self._resource_info.get_instance_port()
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type or \
                ProtectObject.CMDB in self._database_type:
            self._sql_tool = Tool.GSQL
        else:
            self._sql_tool = Tool.VSQL

    def get_repository_dir(self, param, repository_type):
        self.log.info(f'Get repository by repository type. job id: {self._job_id}')
        ret, repositories = get_value_from_dict(param, ParamKey.JOB, ParamKey.REPOSITORIES)
        empty_path = ""
        if not ret or len(repositories) == 0:
            self.log.error(f'Get repositories failed. job id: {self._job_id}')
            return empty_path
        repository_dir = get_repository_dir(repositories, repository_type)
        if not os.path.isdir(repository_dir) or not change_path_permission(repository_dir, self._user_name):
            return ""
        self.log.info(f"Get repository dir: {repository_dir}")
        if repository_type != RepositoryDataTypeEnum.CACHE_REPOSITORY:
            return repository_dir
        parent_dir = os.path.abspath(os.path.join(repository_dir, os.pardir))
        if os.path.islink(parent_dir):
            return empty_path

        _, user_info = get_user_info(self._user_name)
        try:
            change_path_permission(parent_dir, self._user_name)
        except Exception as err_exception:
            self.log.error(f"Change dest path owner failed, exception: {err_exception}")
            return empty_path
        return repository_dir

    def pre_backup(self):
        """
        备份前检查副本仓和cache仓
        :return:成功返回True， 失败返回False
        """
        self.log.info(f'Start to exec pre task. job id: {self._job_id}')
        # 检查数据目录
        return os.path.isdir(self._backup_dir) and os.path.isdir(self._cache_dir) and os.path.isdir(self._meta_dir)

    def clean_cache_file(self):
        """
        清理临时文件
        :return:成功返回True， 失败返回False
        """
        tmp_file = os.path.join(self._cache_dir, self._job_id)
        return safe_remove_path(tmp_file)

    def get_base_meta_data(self):
        """
        获取基本源数据
        :return:
               ret:成功为True， 失败为False
               copy: 副本基本元数据，字典类型
        """
        self.log.info(f"Get copy base meta data!. job id: {self._job_id}")
        ret, copy_time = self.get_copy_time()
        if not ret:
            self.log.error(f"Get copy time failed!. job id: {self._job_id}")
            return False, []
        ret, backup_key, backup_mode = self.present_copy_info()
        if not ret:
            self.log.error(f"Get backup key and  backup mode failed!. job id: {self._job_id}")
            return False, []
        cluster_nodes = list(set(self._resource_info.get_cluster_nodes()))
        if not cluster_nodes:
            self.log.error(f"Get cluster nodes failed!. job id: {self._job_id}")
            return False, []
        endpoint = self._resource_info.get_local_endpoint()
        if not endpoint:
            self.log.error(f"Get endpoint failed!. job id: {self._job_id}")
            return False, []
        copy = {MetaDataKey.COPY_ID: self._job_id, MetaDataKey.COPY_TIME: copy_time, MetaDataKey.ENDPOINT: endpoint,
                MetaDataKey.BACKUP_INDEX_ID: backup_key, MetaDataKey.BACKUP_TYPE: backup_mode,
                MetaDataKey.CLUSTER: {MetaDataKey.UUID: "", MetaDataKey.NODES: cluster_nodes},
                MetaDataKey.PROTECT_OBJECT: {MetaDataKey.TYPE: self._database_type, MetaDataKey.ID: "",
                                             MetaDataKey.SUB_TYPE: "", MetaDataKey.SUB_ID: self._data_dir,
                                             MetaDataKey.PROTECT_NAME: self._object_name, MetaDataKey.EXTEND_INFO: ""},
                MetaDataKey.PARENT_COPY_ID: [], MetaDataKey.ENABLE_CBM_TRACKING: "",
                MetaDataKey.PG_PROBACKUP_CONF: "", MetaDataKey.USER_NAME: self._user_name}
        self.log.info(f"Get copy base meta data success!. job id: {self._job_id}")
        return True, copy

    def kill_backup_tool_process(self, copy_dir):
        """
        杀死工具进程
        :return:成功返回True， 失败返回False
        """
        self.log.info(f"Kill backup tool process. job id: {self._job_id}")
        pidlist = psutil.pids()
        tool_process = None
        for pid in pidlist:
            try:
                process = psutil.Process(pid)
            except Exception as err:
                self.log.warn(f"Get process err: {err}.")
                continue
            try:
                cmd = process.cmdline()
            except Exception as err:
                self.log.warn(f"Get cmdline err: {err}.")
                continue
            if self._backup_tool in cmd and copy_dir in cmd:
                tool_process = process
                break
        if tool_process is None:
            self.log.debug(f"Tool progress not exist. job id: {self._job_id}")
            return True
        try:
            tool_process.kill()
        except Exception:
            self.log.exception(f"Kill tool process failed. job id: {self._job_id}")
            return False
        self.log.info(f"Kill backup tool process success. job id: {self._job_id}")
        return True

    def exec_sql_cmd(self, cmd):
        cmd = f'{self._sql_tool} -c \"{cmd}\" postgres -p {self._port}'
        return execute_cmd_by_user(self._user_name, self._env_file, cmd)

    @abc.abstractmethod
    def init_environment_info(self):
        """
        初始化环境信息
        :return:无
        """
        pass

    @abc.abstractmethod
    def present_copy_info(self):
        """
        获取当前副本信息
        :return:
               ret: 执行结果，成功返回True， 失败返回False
               copy_id: 备份工具生成的副本id
               copy_mode: 备份模式
        """
        pass

    @abc.abstractmethod
    def check_backup_type(self):
        """
        检查备份类型
        :return:
               ret: 执行结果，成功返回True， 失败返回False
               error_code: 错误码
        """
        pass

    @abc.abstractmethod
    def get_copy_time(self):
        """
        获取副本时间
        :return:
               ret: 执行结果，成功返回True， 失败返回False
               copy_id: 副本时间
        """
        pass

    @abc.abstractmethod
    def backup(self):
        """
        备份
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def post_backup(self):
        """
        备份后置
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def stop_backup(self):
        """
        停止备份
        :return:成功返回True， 失败返回False
        """
        pass

    @abc.abstractmethod
    def backup_progress(self):
        """
        备份进度
        :return:
               ret: 执行结果，成功返回True， 失败返回False
               progress: 备份进度
               status: 任务状态
        """
        pass

    @abc.abstractmethod
    def get_copy_meta_data(self, copy_time):
        """
        获取副本源数据
        :return:
               copy: 副本元数据，字典类型
        """
        pass

    @abc.abstractmethod
    def replica_repository_chown(self):
        """
        副本仓权限修改
        :return:
        """
        pass

    @abc.abstractmethod
    def duplicate_check(self):
        """
        副本状态检查
        :return:
        """
        pass
