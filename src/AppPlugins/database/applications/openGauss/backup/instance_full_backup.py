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

from common.const import BackupTypeEnum
from openGauss.backup.instance_backup import InstanceBackup
from openGauss.common.const import ResultCode, CopyDirectory, CopyInfoKey, MetaDataKey, \
    ProtectSubObject, SUCCESS_RET, BackupStatus
from openGauss.common.common import execute_cmd_by_user, safe_remove_path, is_cmdb_distribute
from common.common import convert_time_to_timestamp


class InstanceFullBackup(InstanceBackup):
    def __init__(self, pid, job_id, param_json):
        super(InstanceFullBackup, self).__init__(pid, job_id, param_json)

    def check_backup_type(self):
        self.log.info(f"The backup type does not need to be checked for full backup. job id: {self._job_id}")
        return SUCCESS_RET

    def check_backup_instance(self):
        self.log.info(f"Check backup instance. job id: {self._job_id}")
        if not self.pre_backup():
            self.log.error(f"Backup prepare failed. job id: {self._job_id}")
            return False
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        check_cmd = f'{self._backup_tool} show -B {copy_dir} --instance {CopyInfoKey.BACKUP_INSTANCE}'
        return_code, _, std_err = execute_cmd_by_user(self._user_name, self._env_file, check_cmd)
        if return_code == ResultCode.SUCCESS:
            self.log.info(f"Check backup instance success, std err: {std_err}. job id: {self._job_id}")
            return True
        if os.path.isdir(copy_dir):
            safe_remove_path(copy_dir)
        init_dir_cmd = f'{self._backup_tool} init -B {copy_dir}'
        return_code, _, std_err = execute_cmd_by_user(self._user_name, self._env_file, init_dir_cmd)
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"Init dir failed, std err: {std_err}. job id: {self._job_id}")
            return False
        add_cmd = f'{self._backup_tool} add-instance -B {copy_dir} -D {self._data_dir}' \
                  f' --instance={CopyInfoKey.BACKUP_INSTANCE} -d postgres -p {self._port}'
        return_code, _, std_err = execute_cmd_by_user(self._user_name, self._env_file, add_cmd)
        return return_code == ResultCode.SUCCESS

    def backup(self):
        self.log.info(f"Execute instance full backup. job id: {self._job_id}. deploy type {self._deploy_type}."
                      f"database type {self._database_type}")
        # CMDB分布式场景
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            try:
                return self.exec_cmdb_instance_backup(BackupTypeEnum.FULL_BACKUP)
            except Exception as err:
                self.log.error(f'exec cmdb failed, error: {err}')
                self.update_progress(BackupStatus.FAILED)
                return False
        if not self.check_backup_instance():
            self.log.error(f'check backup instance failed! job id: {self._job_id}')
            return False
        return self.exec_instance_backup("full")

    def get_copy_meta_data(self, copy_time):
        self.log.info(f"Get copy meta data!. job id: {self._job_id}")
        ret, copy = self.get_base_meta_data()
        if not ret:
            self.log.error(f"Get base meta failed!. job id: {self._job_id}")
            copy = {}
            return copy
        protect_obj = copy.setdefault(MetaDataKey.PROTECT_OBJECT, {})
        protect_obj[MetaDataKey.SUB_TYPE] = ProtectSubObject.INSTANCE
        copy[MetaDataKey.PG_PROBACKUP_CONF] = self.get_pg_probackup_conf_data()
        copy[MetaDataKey.ENABLE_CBM_TRACKING] = self.get_enable_cbm_tracking_status()
        copy[MetaDataKey.BACKUP_TIME] = convert_time_to_timestamp(copy_time)
        self.log.info(f"Get copy meta data success!. job id: {self._job_id}")
        return copy