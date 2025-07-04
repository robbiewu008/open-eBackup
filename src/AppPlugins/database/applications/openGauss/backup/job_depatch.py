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
import sys
import time
import json
import time

from common.common import (execute_cmd, convert_time_to_timestamp, exter_attack, get_previous_copy_info,
                           output_result_file)
from common.common_models import ActionResult, SubJobDetails, RepositoryPath, ScanRepositories
from common.const import ExecuteResultEnum, BackupTypeEnum, SubJobStatusEnum, DeployType, RepositoryDataTypeEnum, \
    RpcParamKey, RepositoryNameEnum
from common.util.exec_utils import exec_mkdir_cmd
from common.util.scanner_utils import scan_dir_size
from openGauss.backup.database_full_backup import DatabaseFullBackup
from openGauss.backup.instance_full_backup import InstanceFullBackup
from openGauss.backup.instance_increment_backup import InstanceIncrementBackup
from openGauss.backup.instance_pitr_backup import InstancePitrBackup
from openGauss.backup.resource_info import ResourceInfo
from openGauss.common.common import get_value_from_dict, get_dbuser_gname, check_path, \
    safe_get_environ, is_cmdb_distribute, get_repository_dir, is_wal_file
from openGauss.common.common_models import JobPermission
from openGauss.common.const import ParamKey, Status, ProtectSubObject, JobType, ProgressPercentage, NodeRole, logger, \
    AuthKey, SyncMode, SUCCESS_RET, CopyDirectory, CopyInfoKey, MetaDataKey, FunctionResult, ProtectObject, \
    OpenGaussDeployType, OpenGaussSubJobName, BackupStatus
from openGauss.common.error_code import BodyErr, OpenGaussErrorCode


class JobDepatch:
    log = logger

    def __init__(self, pid, job_id, param):
        self._pid = pid
        self._job_id = job_id
        self._param = param
        self._database_type = ""
        self._deploy_type = ""
        self._user_name = ""
        self._resource_obj = None
        self.init_environment()
        self.depatch = {
            JobType.QUERY_JOB_PERMISSION: self.query_job_permission,
            JobType.ALLOW_BACKUP_IN_LOCAL_NODE: self.allow_backup_in_local_node,
            JobType.CHECK_BACKUP_TYPE: self.check_backup_type,
            JobType.PREREQUISITE: self.prerequisite_backup,
            JobType.BACKUPGENSUB: self.backup_gen_sub_job,
            JobType.BACKUP: self.backup,
            JobType.QUERY_BACKUP_COPY: self.query_backup_copy,
            JobType.QUERY_SCAN_REOSITORIES: self.query_scan_repositories,
            JobType.POST: self.post_backup,
            JobType.ASYNC_ABORT: self.stop_backup,
            JobType.PAUSE: self.stop_backup,
            JobType.PREREQUISITE_PROGRESS: self.prerequistie_progress,
            JobType.BACKUP_PROGRESS: self.backup_progress,
            JobType.POST_PROGRESS: self.post_progress
        }

    @staticmethod
    def find_log_repo(log_repository, repositories):
        for repository_info in repositories:
            ret, present_repository_type = get_value_from_dict(repository_info, ParamKey.REPOSITORY_TYPE)
            if present_repository_type == RepositoryDataTypeEnum.LOG_REPOSITORY:
                log_repository = repository_info
                break
        return log_repository

    def init_environment(self):
        self._user_name = safe_get_environ(f"{AuthKey.APP_ENV}{self._pid}")
        if not self._user_name:
            self._user_name = safe_get_environ(f"{AuthKey.PROBECT_ENV}{self._pid}")
        ret, env_file = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                            ParamKey.ENV_FILE)
        if isinstance(env_file, str) and os.path.isfile(env_file) and check_path(env_file):
            self._resource_obj = ResourceInfo(self._user_name, env_file)
        else:
            self._resource_obj = ResourceInfo(self._user_name)
        _, self._database_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                                     ParamKey.EXTEND_INFO,
                                                     ParamKey.CLUSTER_VERSION)
        _, self._deploy_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                                   ParamKey.EXTEND_INFO,
                                                   ParamKey.DEPLOY_TYPE)

    @exter_attack
    def query_job_permission(self):
        """
        查询任务权限
        :return:
        """
        self.log.info(f'Execute query job permission job. job id: {self._job_id}')
        group_name = get_dbuser_gname(self._user_name)
        if not group_name:
            self.log.error(f"Get user name or group name failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Get user name faild")
            return False, output.dict(by_alias=True)
        output = JobPermission(user=self._user_name, group=group_name, fileMode="0755")
        self.log.info(f"Execute query job permission job success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    def create_backup_object(self):
        self.log.info(f"Create create backup object. job id: {self._job_id}")
        ret, backup_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
        if not ret:
            self.log.error(f"Get backup type failed. job id: {self._job_id}")
            return ""
        ret, object_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.SUB_TYPE)
        if not ret:
            self.log.error(f"Get object type failed. job id: {self._job_id}")
            return ""
        if backup_type == BackupTypeEnum.FULL_BACKUP:
            if object_type == ProtectSubObject.INSTANCE:
                backup_object = InstanceFullBackup(self._pid, self._job_id, self._param)
            else:
                backup_object = DatabaseFullBackup(self._pid, self._job_id, self._param)
        elif backup_type == BackupTypeEnum.LOG_BACKUP:
            backup_object = InstancePitrBackup(self._pid, self._job_id, self._param)
        else:
            backup_object = InstanceIncrementBackup(self._pid, self._job_id, self._param)
        self.log.info(f"Create create backup object success. job id: {self._job_id}")
        return backup_object

    @exter_attack
    def allow_backup_in_local_node(self):
        """
        检查本节点是否支持备份
        :return:
        """
        self.log.info(f"Execute allow backup in local node job. job id: {self._job_id}")
        _, object_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.SUB_TYPE)
        endpoint = self._resource_obj.get_local_endpoint()
        allow_node_status = self._resource_obj.get_node_status(endpoint)
        if allow_node_status and allow_node_status != Status.NORMAL:
            self.log.error(f"Not normal. Execute allow backup in local node job failed. job id: {self._job_id}.")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  bodyErr=OpenGaussErrorCode.ERR_DATABASE_STATUS,
                                  message="The current can not exec backup job")
            return False, output.dict(by_alias=True)
        if object_type == ProtectSubObject.DATABASE:
            # 集群 数据库 只在主节点
            # CMDB 分布式 是 sharding
            # 单机是 single ，主备是 cluster
            if self._resource_obj.get_deploy_type() == DeployType.CLUSTER_TYPE \
                    and self._resource_obj.get_node_role(endpoint) != NodeRole.PRIMARY:
                self.log.error(f"Database backup must in primary node. job id: {self._job_id}")
                output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                      message="The current can not exec backup job")
                return False, output.dict(by_alias=True)
            if self._resource_obj.get_deploy_type() == DeployType.SHARDING_TYPE:
                self.log.info("Start CMDB dist database allow_backup_in_local_node")
                if not self._resource_obj.get_local_cn_port():
                    self.log.error(f"Database backup must in node with cn. job id: {self._job_id}")
                    output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR)
                    return False, output.dict(by_alias=True)
            self.log.info(f"Execute allow backup in local node job success. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.SUCCESS)
            return True, output.dict(by_alias=True)
        ret = self.local_node_allow_instance_backup_cluster(endpoint)
        if ret == FunctionResult.SUCCESS:
            self.log.info(f"Execute allow backup in local node job success. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.SUCCESS)
            return True, output.dict(by_alias=True)
        self.log.error(f"Local node not allow instance backup. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="The current can not exec backup job")
        if ret == FunctionResult.TRACKING_ERR:
            output.body_err = OpenGaussErrorCode.ERR_ENABLE_CBM_TRACKING
        elif ret == FunctionResult.ARCHIVEMODE_ERR:
            output.body_err = OpenGaussErrorCode.ERROR_ARCHIVE_MODE
        elif ret == FunctionResult.ARCHIVEDIR_ERR:
            output.body_err = OpenGaussErrorCode.ERROR_ARCHIVE_DIR
        else:
            output.body_err = OpenGaussErrorCode.NO_ERR
        return False, output.dict(by_alias=True)

    def local_node_allow_log_backup_cluster(self, backup_obj, backup_type):
        # 分布式cmdb仅要求归档开启
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            return FunctionResult.SUCCESS if backup_obj.query_archive_mode() else FunctionResult.ARCHIVEMODE_ERR
        if backup_type != BackupTypeEnum.LOG_BACKUP:
            # 全备增备开启归档没配归档文件夹报错
            if backup_obj.query_archive_mode() and not backup_obj.query_archive_dir():
                self.log.error(f"Full or increment backup job start archive with no archive dir. "
                               f"job id: {self._job_id}")
                return FunctionResult.ARCHIVEDIR_ERR
            return FunctionResult.SUCCESS
        self.log.info(f"Try to check archive info. job id: {self._job_id}")
        if not backup_obj.query_archive_mode():
            self.log.error(f"Archive mode need to set on. job id: {self._job_id}")
            return FunctionResult.ARCHIVEMODE_ERR
        archive_dir = backup_obj.query_archive_dir()
        if not archive_dir:
            self.log.error(f"Archive dir need to set properly. job id: {self._job_id}")
            return FunctionResult.ARCHIVEDIR_ERR
        files = [file for file in os.listdir(archive_dir) if is_wal_file(file)]
        if len(files) == 0:
            self.log.error(f"There are no WAL log files in the archive directory, indicating it is an error directory. "
                           f"job id: {self._job_id}")
            return FunctionResult.ARCHIVEDIR_ERR
        self.log.info(f"It's log backup, try to check if the node is primary. job id: {self._job_id}")
        endpoint = self._resource_obj.get_local_endpoint()
        if self._resource_obj.get_node_role(endpoint) != NodeRole.PRIMARY:
            self.log.err(f"{endpoint} is standby node, not allowed to do log backup. job id: {self._job_id}")
            return FunctionResult.FAILED
        return FunctionResult.SUCCESS

    def local_node_allow_instance_backup_cluster(self, present_endpoint):
        self.log.info(f"Local node allow instance backup cluster. job id: {self._job_id}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            return FunctionResult.FAILED
        present_tracking = backup_obj.get_enable_cbm_tracking_status()
        if "on" != present_tracking:
            self.log.error(f"The value of present enable_cbm_tracking is not on. job id: {self._job_id}")
            return FunctionResult.TRACKING_ERR
        _, backup_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.JOB_PARAM,
                                             ParamKey.BACKUP_TYPE)
        result = self.local_node_allow_log_backup_cluster(backup_obj, backup_type)
        if result != FunctionResult.SUCCESS:
            self.log.error(f"The log backup not satisfied. job id: {self._job_id}")
            return result
        replication_mode = self._resource_obj.get_sync_state()
        # 此处的意义是什么呢。提前判断成功
        if backup_type == BackupTypeEnum.FULL_BACKUP or replication_mode == SyncMode.SYNC:
            self.log.info(f"Full backup or replication mode is sync, any node can backup. job id: {self._job_id}")
            return FunctionResult.SUCCESS
        _, protect_object = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT)
        previous_copy = get_previous_copy_info(protect_object, [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
                                               self._job_id)
        if not previous_copy:
            self.log.info(f"First incremental backup. job id: {self._job_id}")
            return FunctionResult.SUCCESS
        _, previous_tracking = get_value_from_dict(previous_copy, ParamKey.EXTEND_INFO, MetaDataKey.ENABLE_CBM_TRACKING)
        if previous_tracking != "on":
            self.log.error(f"The value of previous enable_cbm_tracking is not on. job id: {self._job_id}")
            return FunctionResult.TRACKING_ERR
        self.log.info(f"Local node allow instance increment backup. job id: {self._job_id}")
        # cmdb分布式不需要保证前一次备份在本节点
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            self.log.info(f"Local node is cmdb distributed, allow different from last. job id: {self._job_id}")
            return FunctionResult.SUCCESS
        # 日志备份不需要保证前一次备份在本节点
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            return FunctionResult.SUCCESS
        _, previous_endpoint = get_value_from_dict(previous_copy, ParamKey.EXTEND_INFO, ParamKey.ENDPOINT)
        if previous_endpoint != present_endpoint and \
                self._resource_obj.get_node_status(previous_endpoint) == Status.NORMAL:
            self.log.error(f"Local node is not previous backup node,"
                           f" previous backup node status is normal. job id: {self._job_id}")
            return FunctionResult.FAILED
        return FunctionResult.SUCCESS

    # 这个函数没有调用
    def check_cluster_status(self, replication_mode):
        _, database_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                               ParamKey.CLUSTER_VERSION)
        if ProtectObject.VASTBASE in database_type:
            self.log.debug(f"Database type if vastbase.  job id: {self._job_id}")
            return True
        if replication_mode != SyncMode.ASYNC and Status.DEGRADED == self._resource_obj.get_cluster_status():
            self.log.error(f"If the cluster data replication mode is not asynchronous,"
                           f" instances cannot be backed up when the cluster is degraded.")
            return False
        return True

    def get_backup_copy_size(self, data_path):
        ret, size = scan_dir_size(self._job_id, data_path)
        return 0 if not ret else int(size)

    @exter_attack
    def check_backup_type(self):
        """
        检查备份类型
        :return:
        """
        self.log.info(f"Execute check backup type job. job id: {self._job_id}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Check backup type failed")
            return False, output.dict(by_alias=True)
        error_code = backup_obj.check_backup_type()
        # 剩下部分只是处理 返回值
        if error_code == SUCCESS_RET:
            output = ActionResult(code=ExecuteResultEnum.SUCCESS)
            self.log.info(f"Execute check backup type job success. job id: {self._job_id}")
            return True, output.dict(by_alias=True)
        if error_code == BodyErr.ERR_INC_TO_FULL:
            self.log.info(f"Need increment backup to full backup, err_code: {error_code}. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=error_code,
                                  message="Need increment backup to full backup")
            return True, output.dict(by_alias=True)
        if error_code == BodyErr.ERROR_ARCHIVE_MODE:
            self.log.error(f"Query archive mode off. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=error_code, message="Archive mode off")
            return False, output.dict(by_alias=True)
        if error_code == BodyErr.ERROR_ARCHIVE_DIR:
            self.log.error(f"Query archive dir failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=error_code,
                                  message="Archive dir not exists")
            return False, output.dict(by_alias=True)

        self.log.info(f"Cannot execute the backup type job, err_code: {error_code}. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=error_code,
                              message="Cannot execute the backup type job")
        return False, output.dict(by_alias=True)

    @exter_attack
    def prerequisite_backup(self):
        """
        执行备份前置
        :return:
        """
        self.log.info(f"No operation is performed on the previous task. job id: {self._job_id}")
        # cmdb分布式备份任务，备份前记录仓库大小，用于备份完成后计算备份文件大小
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            backup_obj = self.create_backup_object()
            backup_obj.save_start_back_size()
        else:
            backup_obj = self.create_backup_object()
            backup_obj.check_archive_mode()
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        return True, output.dict(by_alias=True)

    @exter_attack
    def backup_gen_sub_job(self):
        """
        执行备份分发子任务
        :return:
        """
        self.log.info(f"Execute gen sub job. job id: {self._job_id}, {self._param}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute exec backup failed")
            return False, output.dict(by_alias=True)
        sub_job_array = backup_obj.gen_sub_job(is_cmdb_distribute(self._deploy_type, self._database_type))
        logger.info(f"Gen sub job succeed.")
        return True, sub_job_array

    @exter_attack
    def backup(self):
        """
        执行备份
        :return:
        """
        self.log.info(f"Execute exec backup. job id: {self._job_id}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute exec backup failed")
            return False, output.dict(by_alias=True)
        ret, job_name = get_value_from_dict(self._param, "subJob", "jobName")
        logger.warn(f"get sub job name: {job_name}")
        if job_name == OpenGaussSubJobName.EMPTY_SUB_JOB:
            output = ActionResult(code=ExecuteResultEnum.SUCCESS, message="Execute backup success")
            return True, output.dict(by_alias=True)
        # 备份
        result = backup_obj.backup()
        if not result:
            self.log.error(f"Execute exec backup failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute exec backup failed")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute backup success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS, message="Execute backup success")
        return True, output.dict(by_alias=True)

    @exter_attack
    def query_backup_copy(self):
        # 上报副本信息
        self.log.info(f"Query backup copy. job id: {self._job_id}")
        copy_info = {}
        try:
            copy_info = self.assembly_copy_info()
        except Exception as err:
            self.log.error(f"Get assembly failed, err: {err}")
        self.log.info(f"Get copy info {copy_info}")
        if not copy_info:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Query backup copy failed")
            return False, output.dict(by_alias=True)
        self.log.info(f"Query backup copy success. job id: {self._job_id}. copy_info: {copy_info}")
        return True, copy_info

    @exter_attack
    def query_scan_repositories(self):
        if ProtectObject.MOGDB in self._database_type:
            # E6000适配
            self.log.info(f"Query scan repositories, job_id: {self._job_id}.")
            _, backup_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
            _, repositories = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.REPOSITORIES)
            data_area = get_repository_dir(repositories, RepositoryDataTypeEnum.DATA_REPOSITORY)
            meta_area = get_repository_dir(repositories, RepositoryDataTypeEnum.META_REPOSITORY)
            log_area = get_repository_dir(repositories, RepositoryDataTypeEnum.LOG_REPOSITORY)
            if backup_type == BackupTypeEnum.LOG_BACKUP.value:
                # log仓的meta区 /Database_{resource_id}_LogRepository_su{num}/{ip}/meta/{job_id}
                meta_copy_path = os.path.join(os.path.dirname(log_area), RepositoryNameEnum.META, self._job_id)
                # log仓的data区 /Database_{resource_id}_LogRepository_su{num}/{ip}/{job_id}
                data_path = log_area
                # /Database_{resource_id}_LogRepository_su{num}/{ip}
                save_path = os.path.dirname(log_area)
            else:
                # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
                meta_copy_path = meta_area
                # data/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context/{ip}
                data_path = data_area
                # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
                save_path = meta_area
            if not os.path.exists(meta_copy_path):
                exec_mkdir_cmd(meta_copy_path, mode=0x777)
            log_meta_copy_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.META_REPOSITORY.value,
                                                scanPath=meta_copy_path)
            log_data_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.LOG_REPOSITORY.value,
                                           scanPath=data_path)
            scan_repos = ScanRepositories(scanRepoList=[log_data_repo, log_meta_copy_repo], savePath=save_path)
            output_result_file(self._pid, scan_repos.dict(by_alias=True))
            self.log.info(f"Query scan repos success, return result {scan_repos}, job id: {self._job_id}")
            return True
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute exec backup failed")
            return False, output.dict(by_alias=True)
        try:
            ret, scan_repos = backup_obj.query_scan_repositories()
        except Exception as err:
            self.log.error(f"Create backup object failed. job id: {self._job_id}, err: {err}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute exec backup failed")
            return False, output.dict(by_alias=True)
        return True, scan_repos.dict(by_alias=True)

    def assembly_copy_info(self):
        empty_info = {}
        ret, copy = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.COPY)
        if not ret or len(copy) < 1:
            self.log.error(f"Parse copy failed. job id: {self._job_id}")
            return empty_info
        backup_copy = copy[0]
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            return empty_info
        ret, copy_time = backup_obj.get_copy_time()
        if not ret:
            self.log.error(f"Get copy time failed. job id: {self._job_id}")
            return empty_info
        self.log.info(f"Get copy time success. copy time: {copy_time}")
        meta_data = backup_obj.get_copy_meta_data(copy_time)
        if not meta_data:
            self.log.error(f"Get copy meta data failed. job id: {self._job_id}")
            return empty_info
        self.set_metadata_copy_id(meta_data)
        backup_copy[ParamKey.EXTEND_INFO] = meta_data
        timestamp = convert_time_to_timestamp(copy_time)
        backup_copy[ParamKey.TIMESTAMP] = timestamp
        if backup_copy[ParamKey.EXTEND_INFO][MetaDataKey.BACKUP_TYPE] == "LOG":
            log_repository = {}
            ret, repositories = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.REPOSITORIES)
            log_repository = self.find_log_repo(log_repository, repositories)
            if not log_repository:
                self.log.error(f"Get data repository failed. job id: {self._job_id}")
                return empty_info
            self.check_log_repository(log_repository)
            backup_copy[ParamKey.REPOSITORIES] = [log_repository]
            return backup_copy
        data_repository = {}
        ret, repositories = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.REPOSITORIES)
        for repository_info in repositories:
            ret, present_repository_type = get_value_from_dict(repository_info, ParamKey.REPOSITORY_TYPE)
            if present_repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                data_repository = repository_info
                break
        if not data_repository:
            self.log.error(f"Get data repository failed. job id: {self._job_id}")
            return empty_info
        self.check_repository(data_repository, meta_data)
        backup_copy[ParamKey.REPOSITORIES] = [data_repository]
        logger.info(f"backup_copy: {backup_copy}")
        return backup_copy

    def set_metadata_copy_id(self, meta_data):
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            ret, backup_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
            if backup_type == BackupTypeEnum.FULL_BACKUP:
                base_copy_id = self._job_id
            else:
                ret, protect_object = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT)
                previous_full_copy = get_previous_copy_info(protect_object, [RpcParamKey.FULL_COPY], self._job_id)
                base_copy_id = previous_full_copy.get("id", "")
            meta_data[MetaDataKey.BASE_COPY_ID] = base_copy_id

    def check_log_repository(self, log_repository):
        _, remote_path = get_value_from_dict(log_repository, ParamKey.REMOTE_PATH)
        log_repository[ParamKey.REMOTE_PATH] = os.path.join(remote_path, self._job_id)

    def check_repository(self, data_repository, meta_data):
        _, object_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.SUB_TYPE)
        _, remote_path = get_value_from_dict(data_repository, ParamKey.REMOTE_PATH)
        _, backup_index_id = get_value_from_dict(meta_data, MetaDataKey.BACKUP_INDEX_ID)
        if object_type == ProtectSubObject.INSTANCE:
            if is_cmdb_distribute(self._deploy_type, self._database_type):
                data_repository[ParamKey.REMOTE_PATH] = os.path.join(remote_path, CopyDirectory.INSTANCE_DIRECTORY,
                                                                     self._job_id)
            else:
                data_repository[ParamKey.REMOTE_PATH] = os.path.join(remote_path, CopyDirectory.INSTANCE_DIRECTORY,
                                                                     CopyInfoKey.BACKUPS, CopyInfoKey.BACKUP_INSTANCE,
                                                                     backup_index_id)
        else:
            data_repository[ParamKey.REMOTE_PATH] = \
                os.path.join(remote_path, CopyDirectory.DATABASE_DIRECTORY, self._job_id)

    @exter_attack
    def post_backup(self):
        """
        执行备份后置
        :return:
        """
        self.log.info(f"Execute post job. job id: {self._job_id}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute post job failed")
            return False, output.dict(by_alias=True)
        if not backup_obj.post_backup():
            self.log.error(f"Execute post job failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="execute post backup job failed")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute post job success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS, message="execute post backup job success")
        return True, output.dict(by_alias=True)

    @exter_attack
    def stop_backup(self):
        """
        停止备份
        :return:
        """
        self.log.info(f"Start to stop backup task. job id: {self._job_id}")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Execute stop job failed")
            return False, output.dict(by_alias=True)
        if not backup_obj.stop_backup():
            self.log.error(f"Execute stop job failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="execute stop backup job failed")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute stop job success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="execute stop backup job success")
        return True, output.dict(by_alias=True)

    @exter_attack
    def backup_progress(self):
        """
        查询备份进度
        :return:
        """
        self.log.info("Start to check backup progress task")
        backup_obj = self.create_backup_object()
        if not backup_obj:
            self.log.error(f"Create backup object failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Query backup progress failed")
            return False, output.dict(by_alias=True)
        progress, status, progress_detail = backup_obj.backup_progress()
        self.log.info(f'Get progress: {progress}, status: {status}, progress_detail: {progress_detail}')
        sub_task_id = '0'
        if len(sys.argv) > 4:
            sub_task_id = sys.argv[4]
        if progress_detail:
            output = SubJobDetails(taskId=self._job_id, subTaskId=sub_task_id, taskStatus=status, progress=progress,
                                   logDetail=[progress_detail])
        else:
            output = SubJobDetails(taskId=self._job_id, subTaskId=sub_task_id, taskStatus=status, progress=progress)
        # 分布式cmdb场景
        _, database_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                               ParamKey.EXTEND_INFO,
                                               ParamKey.CLUSTER_VERSION)
        _, deploy_type = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                             ParamKey.EXTEND_INFO,
                                             ParamKey.DEPLOY_TYPE)
        self.log.info(f'Get progress deploy type: {deploy_type} database type: {database_type} progress: {output}')
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            data_path = backup_obj.get_cmdb_backup_path()
            if status == SubJobStatusEnum.COMPLETED.value and data_path:
                output.data_size = backup_obj.get_backup_size_diff()
                self.log.info(f"Get output datasize {output}")
            return True, output.dict(by_alias=True)
        data_path = backup_obj.get_backup_path()
        if status == SubJobStatusEnum.COMPLETED.value and data_path:
            output.data_size = self.get_backup_copy_size(data_path)
        self.log.info(f"Get progress success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    @exter_attack
    def prerequistie_progress(self):
        self.log.info(f"Get progress. job id: {self._job_id}")
        output = SubJobDetails(taskId=self._job_id, subTaskId="", taskStatus=SubJobStatusEnum.COMPLETED.value,
                               progress=ProgressPercentage.COMPLETE_PROGRESS.value)
        self.log.info(f"Get progress success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    @exter_attack
    def post_progress(self):
        self.log.info(f"Get post progress. job id: {self._job_id}")
        if len(sys.argv) < 5:
            self.log.error(f"Bad argv. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="execute backup progress job failed")
            return False, output.dict(by_alias=True)
        sub_task_id = sys.argv[4]
        output = SubJobDetails(taskId=self._job_id, subTaskId=sub_task_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                               progress=ProgressPercentage.COMPLETE_PROGRESS.value)
        self.log.info(f"Get progress success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)
