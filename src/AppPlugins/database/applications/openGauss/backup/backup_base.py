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
import sys
import time

import psutil

from common.common import output_result_file, write_content_to_file, read_result_file
from common.common_models import SubJobModel, LogDetail
from common.file_common import change_path_permission, get_user_info
from common.util.scanner_utils import scan_dir_size
from openGauss.common.const import logger, MetaDataKey, ParamKey, AuthKey, ProtectObject, Tool, OpenGaussSubJobName, \
    SubJobPolicy, OpenGaussDeployType, ProtectSubObject, ProgressPercentage, BackupFileCount, CopyDirectory
from openGauss.common.common import get_repository_dir, get_value_from_dict, \
    execute_cmd_by_user, check_path, safe_get_environ, check_injection_char, safe_remove_path, is_cmdb_distribute
from openGauss.backup.resource_info import ResourceInfo
from common.const import RepositoryDataTypeEnum, SubJobPriorityEnum, SubJobTypeEnum, BackupTypeEnum, DBLogLevel, \
    SubJobStatusEnum


class BackupBase(metaclass=abc.ABCMeta):
    log = logger

    def __init__(self, pid, job_id, param_json):
        self._deploy_type = None
        self._pid = pid
        self._job_id = job_id
        self._user_name = safe_get_environ(f"{AuthKey.PROBECT_ENV}{self._pid}")
        self._backup_tool = ""
        self._base_id = self._job_id
        self._backup_type = ""
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
        self._param = param_json
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
        _, self._deploy_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                                     ParamKey.DEPLOY_TYPE)
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
        _, self._backup_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type or \
                ProtectObject.CMDB in self._database_type:
            self._sql_tool = Tool.GSQL
        else:
            self._sql_tool = Tool.VSQL

    def save_start_back_size(self):
        cmdb_backup_path = self.get_cmdb_backup_path()
        data_size = 0
        if os.path.exists(cmdb_backup_path):
            ret, data_size = scan_dir_size(self._job_id, cmdb_backup_path)
        logger.info(f"Get data size {data_size}")
        tmp_file = os.path.join(self._cache_dir, "dataSize")
        if os.path.exists(tmp_file):
            os.remove(tmp_file)
        write_content_to_file(tmp_file, str(data_size))
        self.log.info(f"Write backup copy size: {data_size} to file: {tmp_file}")

    def get_start_backup_size(self):
        start_size = 0
        tmp_file = os.path.join(self._cache_dir, "dataSize")
        if os.path.exists(tmp_file):
            start_size = read_result_file(tmp_file)
        self.log.info(f"Read backup copy size: {start_size} from: {tmp_file}")
        return start_size

    def get_backup_size_diff(self):
        start_size = int(self.get_start_backup_size())
        cmdb_backup_path = self.get_cmdb_backup_path()
        end_size = 0
        if os.path.exists(cmdb_backup_path):
            ret, end_size = scan_dir_size(self._job_id, cmdb_backup_path)
        self.log.info(f"Get start size {start_size}, end size {end_size}")
        if start_size >= end_size:
            return 0
        else:
            return end_size - start_size

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

    def gen_sub_job(self):
        """
        执行分发子任务
        """
        self.log.info(f"Start to gen sub task. job param: {self._param}")
        sub_job_array = []
        # 子任务1：执行备份
        sub_job = self.build_sub_job(OpenGaussSubJobName.SUB_EXEC, SubJobPriorityEnum.JOB_PRIORITY_1,
                                     SubJobPolicy.ANY_NODE.value)
        sub_job_array.append(sub_job)

        # 子任务2：上报备份副本
        sub_job = self.build_sub_job(OpenGaussSubJobName.QUERY_COPY, SubJobPriorityEnum.JOB_PRIORITY_2,
                                     SubJobPolicy.ANY_NODE.value)
        sub_job_array.append(sub_job)

        self.log.info(f"Backup sub task split succeeded. sub_job_array: {sub_job_array}")
        output_result_file(self._pid, sub_job_array)
        return True

    def build_sub_job(self, job_name, job_priority, job_policy, job_info=None):
        return SubJobModel(
            jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, jobInfo=job_info,
            jobName=job_name, jobPriority=job_priority, policy=job_policy).dict(by_alias=True)

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
        self.log.info(f"Get copy time {copy_time}")
        ret, object_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.SUB_TYPE)
        self.log.info(f"Get object type {object_type}")
        if is_cmdb_distribute(self._deploy_type, self._database_type) and object_type == ProtectSubObject.INSTANCE:
            backup_key = BackupTypeEnum.FULL_BACKUP.value
            backup_mode = BackupTypeEnum.FULL_BACKUP.value
        else:
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
        copy = {
            MetaDataKey.COPY_ID: self._job_id, MetaDataKey.COPY_TIME: copy_time, MetaDataKey.ENDPOINT: endpoint,
            MetaDataKey.BACKUP_INDEX_ID: backup_key, MetaDataKey.BACKUP_TYPE: backup_mode,
            MetaDataKey.CLUSTER: {MetaDataKey.UUID: "", MetaDataKey.NODES: cluster_nodes},
            MetaDataKey.PROTECT_OBJECT: {
                MetaDataKey.TYPE: self._database_type, MetaDataKey.ID: "",
                MetaDataKey.SUB_TYPE: "", MetaDataKey.SUB_ID: self._data_dir,
                MetaDataKey.PROTECT_NAME: self._object_name, MetaDataKey.EXTEND_INFO: ""
            },
            MetaDataKey.PARENT_COPY_ID: [], MetaDataKey.ENABLE_CBM_TRACKING: "",
            MetaDataKey.PG_PROBACKUP_CONF: "", MetaDataKey.USER_NAME: self._user_name
        }
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

    def get_cmdb_backup_progress(self):
        """
        解析实例备份进度
        :return:
        """
        self.log.info("Start to get cmdb backup progress")
        sub_task_id = sys.argv[4] if len(sys.argv) > 4 else 0
        progress_file = os.path.join(self._cache_dir, self._job_id)
        backup_failed_detail = LogDetail(logInfo="opengauss_plugin_database_backup_subjob_failed_label",
                                         logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()),
                                         logLevel=DBLogLevel.ERROR.value)
        if not os.path.isfile(progress_file):
            self.log.error(f'progress file not exist! job id: {self._job_id}')
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, backup_failed_detail
        with open(progress_file, "r", encoding='UTF-8') as f:
            data = f.read()
        self.log.info(f"Get Progress: {data} from {progress_file}")
        if "completed" in data:
            complete_detail = LogDetail(logInfo="opengauss_plugin_backup_subjob_success_label",
                                        logInfoParam=[f"{sub_task_id}", f"{BackupFileCount.ONE_FILE.value}"],
                                        logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
            self.log.info(f"The backup is complete. job id: {self._job_id}")

            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.COMPLETED.value, complete_detail
        if "failed" in data:
            self.log.info(f"The backup is failed. job id: {self._job_id}")
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, backup_failed_detail
        running_detail = LogDetail(logInfo="opengauss_plugin_execute_backup_subjob_label",
                                   logInfoParam=[f"{sub_task_id}", f"{BackupFileCount.ONE_FILE.value}",
                                                 f"{BackupFileCount.ZERO_FILE.value}"],
                                   logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
        return ProgressPercentage.START_PROGRESS.value, SubJobStatusEnum.RUNNING.value, running_detail

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
    def get_cmdb_backup_path(self):
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
