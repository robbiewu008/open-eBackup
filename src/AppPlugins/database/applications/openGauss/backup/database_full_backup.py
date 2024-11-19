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
import re
import sys
import time

from common.common import change_dir_permission, write_content_to_file, convert_timestamp_to_time
from common.common_models import LogDetail
from common.const import SubJobStatusEnum
from common.file_common import change_path_permission
from common.util.exec_utils import exec_mkdir_cmd
from openGauss.backup.backup_base import BackupBase
from openGauss.common.common import str_to_float, get_ids_by_name, execute_cmd_by_user, safe_remove_path, str_to_int, \
    get_value_from_dict, get_env_value, is_cmdb_distribute
from openGauss.common.const import Tool, ProtectObject, \
    ResultCode, ProgressPercentage, CopyDirectory, MetaDataKey, ProtectSubObject, SUCCESS_RET, DatabaseToolLog, \
    BackupStatus, AuthKey, ParamKey


class DatabaseFullBackup(BackupBase):
    def __init__(self, pid, job_id, param_json):
        super(DatabaseFullBackup, self).__init__(pid, job_id, param_json)
        self.init_environment_info()
        self._tool_log_dict = {
            DatabaseToolLog.CONNECT_FAILED: ['connection to database \"{}\" failed', u'连接数据库 \"{}\" 失败'],
            DatabaseToolLog.BACKUP_SUCCESS: ['dump database {} successfully', u'转储数据库{}成功'],
            DatabaseToolLog.PROGRESS_PARTTERN: [r": \[(.*?)%\]", r": 【(.*?)%】"]
        }

    def init_environment_info(self):
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type:
            self._backup_tool = Tool.GS_DUMP
        elif is_cmdb_distribute(self._deploy_type, self._database_type):
            self._backup_tool = Tool.GS_DUMP
        elif ProtectObject.CMDB in self._database_type:
            self._backup_tool = Tool.CM_DUMP
        else:
            self._backup_tool = Tool.VB_DUMP

    def present_copy_info(self):
        self.log.info(f'Get present copy info. job id: {self._job_id}')
        copy_id = f'{self._object_name}.sql'
        copy_file = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._job_id, copy_id)
        self.log.info(f'start check database back path {copy_file}. job id: {self._job_id}')
        if not os.path.isfile(copy_file):
            self.log.error(f'Copy file not exists. job id: {self._job_id}')
            return False, "", 0
        self.log.info(f'Get present copy info success. job id: {self._job_id}')
        return True, copy_id, "FULL"

    def check_backup_type(self):
        self.log.info(f"The backup type does not need to be checked for full backup. job id: {self._job_id}")
        return SUCCESS_RET

    def get_copy_time(self):
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            self.log.info("Start to get database full copy time")
            return True, convert_timestamp_to_time(time.time())
        self.log.info(f'Get copy time. job id: {self._job_id}')
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if not os.path.isfile(progress_file):
            self.log.error(f"Progress file: {progress_file} not exist. job id: {self._job_id}")
            return False, ""
        with open(progress_file, "r", encoding='UTF-8') as f:
            data = f.read()
        for info in data.split("\n"):
            if not self.is_success_log_line(info):
                continue
            copy_time = info[info.rfind("[") + 1: info.rfind("]")]
            self.log.info(f"Succeed to get copy time. job id: {self._job_id}")
            return True, copy_time
        self.log.error(f"The backup is not complete. job id: {self._job_id}")
        return False, ""

    def is_success_log_line(self, log_line):
        for backup_success in self._tool_log_dict.get(DatabaseToolLog.BACKUP_SUCCESS):
            if backup_success.format(self._object_name) in log_line:
                return True
        return False

    def backup(self):
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            try:
                return self.cmdb_backup()
            except Exception as err:
                self.log.info(f"Get cmdb databse backup failed {err}")
                self.update_progress(BackupStatus.FAILED)
                return False
        self.log.info(f"Execute database full backup. job id: {self._job_id}")
        if not self.pre_backup():
            self.log.error(f"Execute prepare backup faild. job id: {self._job_id}")
            return False
        database_copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY)
        if not self.mkdir_by_user(database_copy_dir):
            self.log.error(f"Make database copy dir failed! job id: {self._job_id}")
            return False
        copy_dir = os.path.join(database_copy_dir, self._job_id)
        if not self.mkdir_by_user(copy_dir):
            self.log.error(f"Make copy dir failed! job id: {self._job_id}")
            return False
        copy_id = f'{self._object_name}.sql'
        copy_file = os.path.join(copy_dir, copy_id)
        progress_file = os.path.join(self._cache_dir, self._job_id)
        backup_cmd = f'{self._backup_tool} -f {copy_file} -p {self._port} {self._object_name} -F p &> {progress_file}'
        return_code, _, _ = execute_cmd_by_user(self._user_name, self._env_file, backup_cmd)
        self.duplicate_check()
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"Execute database full backup failed. job id: {self._job_id}")
            return False
        self.log.info(f"Execute database full backup success. job id: {self._job_id}")
        return True

    def cmdb_backup(self):
        self.update_progress(BackupStatus.RUNNING)
        self.log.info(f"Execute database full backup. job id: {self._job_id}")
        if not self.pre_backup():
            self.log.error(f"Execute prepare backup faild. job id: {self._job_id}")
            return False
        database_copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY)
        if not self.mkdir_by_user(database_copy_dir):
            self.log.error(f"Make database copy dir failed! job id: {self._job_id}")
            return False
        copy_dir = os.path.join(database_copy_dir, self._job_id)
        if not self.mkdir_by_user(copy_dir):
            self.log.error(f"Make copy dir failed! job id: {self._job_id}")
            return False
        self.log.info(f"Get copy dir {copy_dir}")
        copy_id = f'{self._object_name}.sql'
        copy_file = os.path.join(copy_dir, copy_id)
        _, protect_env_extend_info = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                                         ParamKey.EXTEND_INFO)
        ret, dcs_user = get_value_from_dict(protect_env_extend_info, ParamKey.DCS_USER)
        dcs_pass = get_env_value(f"{AuthKey.PROTECT_ENV_DCS}{self._pid}")
        backup_cmd = f'{self._backup_tool} {self._object_name} -f {copy_file} -U {dcs_user} -W {dcs_pass}'
        self.log.info(f"Get backup tool {self._backup_tool}, name: {self._object_name}, copy file {copy_file}")
        return_code, out, err = execute_cmd_by_user(self._user_name, self._env_file, backup_cmd)
        self.log.info(f"Get dump result code {return_code}, result {out}, err: {err}")
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"Execute database full backup failed. job id: {self._job_id}")
            self.update_progress(BackupStatus.FAILED)
            return False
        self.log.info(f"Execute database full backup success. job id: {self._job_id}")
        self.update_progress(BackupStatus.COMPLETED)
        return True

    def update_progress(self, status):
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.isfile(progress_file):
            safe_remove_path(progress_file)
        content = f"INFO: Progress: (0/100). Process file"
        if status == BackupStatus.COMPLETED:
            content = content + f"INFO: Backup completed"
        if status == BackupStatus.FAILED:
            content = content + f"INFO: Backup failed"
        write_content_to_file(progress_file, content)

    def post_backup(self):
        return self.clean_cache_file()

    def stop_backup(self):
        self.log.info(f"Execute stop backup job. job id: {self._job_id}")
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._job_id)
        return self.kill_backup_tool_process(copy_dir)

    def backup_progress(self):
        self.log.info(f"Get backup progress. job id: {self._job_id}")
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            return self.get_cmdb_backup_progress()
        sub_task_id = '0'
        if len(sys.argv) > 4:
            sub_task_id = sys.argv[4]
        progress_file = os.path.join(self._cache_dir, self._job_id)
        error_detail = LogDetail(logInfo="opengauss_plugin_database_backup_subjob_failed_label",
                                 logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()), logLevel=3)
        if not os.path.isfile(progress_file):
            self.log.error(f'Get progress file failed. job id: {self._job_id}')
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, error_detail
        with open(progress_file, "r", encoding='UTF-8') as f:
            data = f.read()
        for connect_failed in self._tool_log_dict.get(DatabaseToolLog.CONNECT_FAILED):
            if connect_failed.format(self._object_name) in data:
                self.log.error(f'Connect database failed. job id: {self._job_id}')
                return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, error_detail
        for backup_success in self._tool_log_dict.get(DatabaseToolLog.BACKUP_SUCCESS):
            if backup_success.format(self._object_name) in data:
                self.log.info(f"The backup is complete. job id: {self._job_id}")
                success_detail = LogDetail(logInfo="opengauss_plugin_database_backup_subjob_success_label",
                                           logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()), logLevel=1)
                return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.COMPLETED.value, success_detail
        progress_list = []
        for progress_parttern in self._tool_log_dict.get(DatabaseToolLog.PROGRESS_PARTTERN):
            progress_list = re.findall(progress_parttern, data)
            if progress_list:
                break
        running_detail = LogDetail(logInfo="opengauss_plugin_execute_database_backup_subjob_label",
                                   logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()), logLevel=1)
        if not progress_list:
            self.log.error(f'Backing Up Just Started! job id: {self._job_id}')
            return ProgressPercentage.START_PROGRESS.value, SubJobStatusEnum.RUNNING.value, running_detail
        self.log.info(f"Get Backup progress success. job id: {self._job_id}")
        return int(str_to_float(progress_list[-1])), SubJobStatusEnum.RUNNING.value, running_detail

    def get_backup_data_info(self, data, parttern):
        progress_info_list = re.findall(parttern, data)
        if not progress_info_list:
            self.log.error(f"No data has been backed up. job id: {self._job_id}")
            return 0, 0
        last_info = progress_info_list[-1]
        info_list = last_info.split("/")
        if len(info_list) < 2:
            self.log.error(f"Bad progress info. job id: {self._job_id}")
            return 0, 0
        return str_to_int(info_list[0], 10), str_to_int(info_list[1], 10)

    def get_copy_meta_data(self, copy_time):
        self.log.info(f"Get copy meta data!. job id: {self._job_id}")
        ret, copy = self.get_base_meta_data()
        if not ret:
            self.log.error(f"Get base meta failed!. job id: {self._job_id}")
            copy = {}
            return copy
        protect_obj = copy.setdefault(MetaDataKey.PROTECT_OBJECT, {})
        protect_obj[MetaDataKey.SUB_TYPE] = ProtectSubObject.DATABASE
        self.log.info(f"Get copy meta data success!. job id: {self._job_id}")
        return copy

    def mkdir_by_user(self, path):
        if os.path.isdir(path):
            return True
        try:
            exec_mkdir_cmd(path, is_check_white_list=False)
        except Exception:
            return False
        user_id, group_id = get_ids_by_name(self._user_name)
        change_dir_permission(path, user_id, group_id)
        return True

    def replica_repository_chown(self):
        database_copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY)
        if not os.path.exists(database_copy_dir):
            self.log.warning(f"Copy dir not exists. job id: {self._job_id}")
            return
        copy_dir = os.path.join(database_copy_dir, self._job_id)
        copy_id = f'{self._object_name}.sql'
        copy_file = os.path.join(copy_dir, copy_id)
        progress_file = os.path.join(self._cache_dir, self._job_id)
        path_list = [database_copy_dir, copy_dir, copy_file, progress_file]
        for path in path_list:
            if not os.path.exists(path):
                continue
            change_path_permission(path)

    def duplicate_check(self):
        ret, copy_id, copy_mode = self.present_copy_info()
        if not ret:
            self.log.error(f"Get copy info failed. job id: {self._job_id}")
            return False
        progress, status, progress_detail = self.backup_progress()
        if status == SubJobStatusEnum.COMPLETED.value:
            self.log.debug(f"duplicate check ok. job id: {self._job_id}")
            return True
        self.log.info(f"Database full backup failed, now deleting Residual Data. job id: {self._job_id}")
        database_copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY)
        copy_dir = os.path.join(database_copy_dir, self._job_id)
        if os.path.exists(copy_dir):
            safe_remove_path(copy_dir)
        if os.path.exists(database_copy_dir) and not os.listdir(database_copy_dir):
            safe_remove_path(database_copy_dir)
        return False

    def get_backup_path(self):
        return os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._job_id)

    def get_cmdb_backup_path(self):
        instance_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._job_id)
        self.log.info(f"Get instance backup dir {instance_dir}")
        return instance_dir