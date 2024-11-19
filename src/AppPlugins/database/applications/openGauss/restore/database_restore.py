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

from common.common import execute_cmd, is_clone_file_system
from common.common_models import LogDetail
from common.const import SubJobStatusEnum
from common.file_common import exec_lchown, exec_lchown_dir_recursively
from openGauss.common.common import get_dbuser_gname, \
    write_progress_file, get_now_time, check_injection_char, execute_cmd_by_user, is_cmdb_distribute
from openGauss.common.const import ResultCode, CopyDirectory, ProtectObject
from openGauss.common.error_code import OpenGaussErrorCode
from openGauss.restore.restore_base import RestoreBase


class DatabaseRestore(RestoreBase):
    def __init__(self, pid, job_id, param):
        super(DatabaseRestore, self).__init__(pid, job_id, param)

    def check_db_info(self):
        source_db_version = self._resource_info.get_db_version()
        copy_version = self._copy_version
        if ProtectObject.CMDB in source_db_version:
            if self.check_connection():
                return True, ResultCode.SUCCESS
            else:
                return False, OpenGaussErrorCode.CHECK_CLUSTER_FAILED
        if copy_version != source_db_version:
            self.log.error(f'The current database version {source_db_version} does not match the replica version '
                           f'{copy_version}. job id: {self._job_id}')
            return False, OpenGaussErrorCode.ERROR_DIFFERENT_VERSION
        return True, ResultCode.SUCCESS

    def check_connection(self):
        check_cmd = "\\l"
        ret, std_out, std_err = self.exec_sql_cmd(check_cmd)
        if ret != ResultCode.SUCCESS or f"failed to connect" in std_out:
            self.log.info(f"Check Connection failed, out: {std_out}, err: {std_err}")
            return False
        self.log.info(f"Check Connection success")
        return True

    def restore_prerequisite(self, param):
        if self._copy_version != self._resource_info.get_db_version():
            self.log.error(f'The current database version does not match the replica version. job id: {self._job_id}')
            return False
        if is_clone_file_system(param):
            if not self.chang_dir_permission_recursive():
                self.log.error(f'Chang backup dir permission failed. job id: {self._job_id}')
                return False
        copy_file = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._copy_id, self._copy_index_id)
        self.log.info(f'Database restore prerequisite finish. job id: {self._job_id}')
        return os.path.isfile(copy_file)

    def chang_dir_permission_recursive(self):
        database_copy_dir = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY)
        if not os.path.isdir(database_copy_dir):
            return False
        group_name = get_dbuser_gname(self._user_name)
        if not exec_lchown_dir_recursively(database_copy_dir, self._user_name, group_name):
            self.log.error(f"Change owner for {database_copy_dir} failed.")
            return False
        return True

    def restore(self):
        self.log.info(f"Start database restore. job_id: {self._job_id}")
        copy_file = os.path.join(self._backup_dir, CopyDirectory.DATABASE_DIRECTORY, self._copy_id, self._copy_index_id)
        tmp_database = self.create_database(self._target_name)
        if not tmp_database:
            self.log.error(f"Create database failed. job_id: {self._job_id}")
            return False
        progress_file = os.path.join(self._cache_dir, self._job_id)
        # 写入开始时间和临时数据库名
        speed_file = os.path.join(self._cache_dir, f"{self._job_id}_speed_record")
        restore_start_time = get_now_time()
        progress_line = f"{restore_start_time}\n{tmp_database} "
        write_progress_file(progress_line, speed_file)
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            restore_cmd = f'{self._sql_tool} {tmp_database} -r -f {copy_file} &>> {progress_file}'
        else:
            restore_cmd = f'{self._sql_tool} -p {self._port} {tmp_database} -r -f {copy_file} &>> {progress_file}'
        ret, out, std_err = execute_cmd_by_user(self._user_name, self._env_file, restore_cmd)

        # 写入完成时间
        restore_end_time = get_now_time()
        write_progress_file(f"{restore_end_time}\n{self._target_name}", speed_file)
        if ret != ResultCode.SUCCESS:
            self.drop_database(tmp_database)
            self.log.error(f"Execute database restore cmd failed, std_err: {std_err}. job_id: {self._job_id}")
            return False
        if tmp_database != self._target_name:
            ret = self.drop_database(self._target_name)
            if ret:
                ret = self.database_rename(tmp_database, self._target_name)
            if not ret:
                self.log.error(f"Drop database or database rename failed. job_id: {self._job_id}")
                failed_message = f"echo \"RESTORE FAILED\" > {progress_file}"
                execute_cmd_by_user(self._user_name, self._env_file, failed_message)
                self.drop_database(tmp_database)
                return False
        success_message = f"echo \"JOB SUCCESS\" > {progress_file}"
        execute_cmd_by_user(self._user_name, self._env_file, success_message)
        self.log.info(f"Restore database success. job_id: {self._job_id}")
        return True

    def restore_post(self):
        self.log.info(f"Database restore post. job_id: {self._job_id}")
        return self.clean_cache_file()

    def restore_progress(self):
        self.log.info(f"Database restore progress. job_id: {self._job_id}")
        progress_file = os.path.join(self._cache_dir, self._job_id)
        speed_file = os.path.join(self._cache_dir, f"{self._job_id}_speed_record")
        speed = 0
        progress = 0
        sub_task_id = sys.argv[4] if len(sys.argv) > 4 else 0
        error_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label",
                                 logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()), logLevel=3)
        if not os.path.isfile(progress_file):
            self.log.error(f"Get database restore progress failed. job_id: {self._job_id}")
            ret = (speed, progress, SubJobStatusEnum.FAILED.value, error_detail)
            return ret
        with open(progress_file, "r", encoding='UTF-8') as f:
            data = f.read()
        if not data or "sql: FATAL:" in data or "RESTORE FAILED" in data:
            self.log.error(f"Restore job failed. job_id: {self._job_id}")
            failed_speed = 0
            failed_progress = 100
            ret = (failed_speed, failed_progress, SubJobStatusEnum.FAILED.value, error_detail)
            return ret
        speed = self.get_restore_speed(speed_file)
        self.log.info(f"Database restore progress success. job_id: {self._job_id}")
        if "JOB SUCCESS" not in data:
            progress = 50
            status = SubJobStatusEnum.RUNNING.value
        else:
            progress = 100
            status = SubJobStatusEnum.COMPLETED.value
        ret = (speed, progress, status, {})
        return ret

    def create_database(self, database_name):
        if not check_injection_char(self._pid):
            self.log.error("Invalid database name to create database.")
            return ""
        new_database = f"temp{self._pid}"
        check_cmd = "\\l"
        ret, std_out, std_err = self.exec_sql_cmd(check_cmd)
        if ret != ResultCode.SUCCESS or f" {database_name} " not in std_out:
            new_database = database_name
        if not check_injection_char(new_database):
            self.log.error("Invalid database name to create.")
            return ""
        create_cmd = f"create database \\\"{new_database}\\\";"
        ret, std_out, std_err = self.exec_sql_cmd(create_cmd)
        if ret != ResultCode.SUCCESS or "CREATE DATABASE" not in std_out:
            return ""
        return new_database

    def drop_database(self, database_name):
        if not check_injection_char(database_name):
            self.log.error(f"Invalid database name: {database_name}, check it ")
            return False
        break_link_cmd = f"clean connection to all force for database \\\"{database_name}\\\""
        ret, std_out, std_err = self.exec_sql_cmd(break_link_cmd)
        if ret != ResultCode.SUCCESS or "CLEAN CONNECTION" not in std_out:
            return False
        drop_cmd = f"drop database \\\"{database_name}\\\";"
        ret, std_out, std_err = self.exec_sql_cmd(drop_cmd)
        return ret == ResultCode.SUCCESS and "DROP DATABASE" in std_out

    def database_rename(self, old_name, new_name):
        if not check_injection_char(old_name) or not check_injection_char(new_name):
            self.log.error(f"Invalid database name. check it ")
            return False
        rename_cmd = f"alter database \\\"{old_name}\\\" rename to \\\"{new_name}\\\";"
        ret, std_out, std_err = self.exec_sql_cmd(rename_cmd)
        return ret == ResultCode.SUCCESS and "ALTER DATABASE" in std_out

    def get_restore_speed(self, speed_file):
        speed = 0
        end_time = get_now_time()
        if not os.path.exists(speed_file):
            self.log.error("Not find speed record file")
            return speed
        with open(speed_file, "r") as fp:
            data = fp.read()
        speed_data = [item.strip() for item in data.split("\n") if item]
        if len(speed_data) < 2:
            return 0
        start_time, target_db = int(speed_data[0]), speed_data[1]
        if len(speed_data) == 4:
            end_time = int(speed_data[2])
            target_db = speed_data[3]

        if not check_injection_char(target_db):
            self.log.error("Invalid db name while get restore speed")
            return 0
        cmd = f"select pg_database_size('\\'{target_db}\\'') as size;"
        ret, stdout, _ = self.exec_sql_cmd(cmd)
        if ret != "0":
            self.log.error("Get db size failed with ret:%s" % ret)
            return 0
        std = re.search(r"\d+", stdout)
        if not std:
            self.log.error("Get db size failed, may it not found the db.")
            return 0
        db_size = std.group(0)
        size = int(db_size) / 1024 if db_size else 0
        diff_time = end_time - start_time
        if diff_time < 1:
            diff_time = 1
        speed = size / diff_time
        return speed
