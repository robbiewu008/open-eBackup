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
import time

from common.common_models import LogDetail
from common.common import write_content_to_file, read_tmp_json_file
from common.const import SubJobStatusEnum, DBLogLevel, BackupTypeEnum
from common.file_common import change_path_permission
from common.util.backup import backup_files, query_progress
from openGauss.backup.backup_base import BackupBase
from openGauss.common.common import get_value_from_dict, execute_cmd_by_user, check_injection_char, str_to_int, \
    safe_remove_path, get_backup_info_file, get_last_backup_stop_time, is_wal_file
from openGauss.common.const import Tool, ProtectObject, ResultCode, CopyDirectory, CopyInfoKey, MetaDataKey, \
    ProgressPercentage, TableSpace, ParamKey, GsprobackupParam, BackupStatus
from openGauss.common.error_code import OpenGaussErrorCode


class InstanceBackup(BackupBase):
    def __init__(self, pid, job_id, param_json):
        super(InstanceBackup, self).__init__(pid, job_id, param_json)
        self._channel_number = GsprobackupParam.DEFAULT_PARALLEL
        self._tool_err_logs = ["WARNING: Backup {} is running, setting its status to ERROR", "ERROR: Backup {} failed",
                               "ERROR: "]
        self.init_environment_info()
        self.init_channel_number(param_json)
        _, self._backup_type = get_value_from_dict(param_json, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
        self._stop_time = ""

    def init_channel_number(self, param):
        _, channel_number = get_value_from_dict(param, ParamKey.JOB, ParamKey.EXTEND_INFO, ParamKey.CHANNEL_NUMBER)
        if isinstance(channel_number, str):
            channel_number = str_to_int(channel_number, 10)
        if not isinstance(channel_number, int) or channel_number < 1:
            self.log.info(f"No parallel number is specified or the parallel number is improper."
                          f" The default parallel number will be used for backup. job id: {self._job_id}")
            return
        self._channel_number = channel_number

    def init_environment_info(self):
        if ProtectObject.OPENGAUSS in self._database_type or ProtectObject.MOGDB in self._database_type:
            self._backup_tool = Tool.GS_PROBACKUP
        elif ProtectObject.CMDB in self._database_type:
            self._backup_tool = Tool.CM_PROBACKUP
        else:
            self._backup_tool = Tool.VB_PROBACKUP

    def present_copy_info(self):
        self.log.info(f"Get present copy info. job id: {self._job_id}")
        copy_id = ""
        copy_mode = ""
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if not os.path.isfile(progress_file):
            self.log.error(f'Progress file not exist! job id: {self._job_id}')
            return False, "", ""
        with open(progress_file, "r", encoding='UTF-8') as file_descriptor:
            data = file_descriptor.read()
        if not data:
            self.log.error(f'Progress file is empty! job id: {self._job_id}')
            return False, "", ""
        backup_info_list = data[:data.find('\n')].split(",")
        for info in backup_info_list:
            copy_id_prefix = " backup ID: "
            copy_mode_prefix = " backup mode: "
            if info.startswith(copy_id_prefix):
                copy_id = info[len(copy_id_prefix):]
            if info.startswith(copy_mode_prefix):
                copy_mode = info[len(copy_mode_prefix):]
        if not check_injection_char(copy_id) or not copy_mode:
            self.log.error(f"Bad copy id or copy_mode not exist. job id: {self._job_id}")
            return False, copy_id, copy_mode
        self.log.info(f"Get present copy info success. job id: {self._job_id} copy_id: {copy_id} copy_mode:{copy_mode}")
        return True, copy_id, copy_mode

    def get_copy_time(self):
        self.log.info(f"Get copy time. job id: {self._job_id}")
        call_ret, copy_id, copy_mode = self.present_copy_info()
        if not call_ret:
            self.log.error(f"Get present copy id and copy mode failed. job id: {self._job_id}")
            return False, ""
        copy_info = self.get_copy_info_by_copy_id(copy_id)
        if not copy_info:
            self.log.error(f"Get present copy info failed. job id: {self._job_id}")
            return False, ""
        ret, backups = get_value_from_dict(copy_info, CopyInfoKey.BACKUPS)
        if not ret or len(backups) < 1:
            self.log.error(f"Bad std_out. job id: {self._job_id}")
            return False, ""
        ret, recovery_time = get_value_from_dict(backups[0], CopyInfoKey.RECOVERY_TIME)
        if not ret:
            self.log.error(f"End time not exist in std_out. job id: {self._job_id}")
            return False, ""
        copy_time = recovery_time[:len(recovery_time) - len(CopyInfoKey.UTC_TIME_SUFFIX)]
        self.log.info(f"Succeed to get copy time. job id: {self._job_id}")
        return True, copy_time

    def get_copy_info_by_copy_id(self, copy_id):
        self.log.info(f"Get copy info by copy id. job id: {self._job_id}")
        empty_info = {}
        if not check_injection_char(copy_id):
            self.log.error(f"Bad copy id. job id: {self._job_id}")
            return empty_info
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        cmd = f'{self._backup_tool} show -B {copy_dir} --instance {CopyInfoKey.BACKUP_INSTANCE} -i {copy_id}' \
              f' --format=json'
        return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, cmd)
        if return_code != ResultCode.SUCCESS or "OK" not in std_out:
            self.log.error(f"Failed to get copy info by copy id, std_err: {std_err}. job id: {self._job_id}")
            return empty_info
        copy_info_list = json.loads(std_out)
        if len(copy_info_list) < 1:
            self.log.error(f"Bad copy info list. job id: {self._job_id}")
            return empty_info
        self.log.info(f"Get copy info by copy id. job id: {self._job_id}")
        return copy_info_list[0]

    def post_backup(self):
        return self.clean_cache_file()

    def stop_backup(self):
        self.log.info(f"Execute stop backup job. job id: {self._job_id}")
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        return self.kill_backup_tool_process(copy_dir)

    def get_enable_cbm_tracking_status(self):
        cmd = "show enable_cbm_tracking;"
        ret, std_out, std_err = self.exec_sql_cmd(cmd)
        if ret != ResultCode.SUCCESS:
            return ""
        tracking_info_list = std_out.split("\n")
        if len(tracking_info_list) < 3:
            return ""
        return tracking_info_list[2].strip()

    def get_pg_probackup_conf_data(self):
        conf_file = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY,
                                 CopyInfoKey.BACKUPS, CopyInfoKey.BACKUP_INSTANCE, MetaDataKey.PG_PROBACKUP_CONF)
        if not os.path.isfile(conf_file):
            return ""
        with open(conf_file, "r", encoding='UTF-8') as f:
            conf_data = f.read()
        return conf_data

    def backup_progress(self):
        """
        解析实例备份进度
        :return:
        """
        empty_detail = {}
        sub_task_id = sys.argv[4] if len(sys.argv) > 4 else 0
        backup_failed_detail = LogDetail(logInfo="opengauss_plugin_database_backup_subjob_failed_label",
                                         logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()),
                                         logLevel=DBLogLevel.ERROR.value)
        ret, copy_id, copy_mode = self.present_copy_info()
        if not ret:
            self.log.error(f"Get present backup info failed. job id: {self._job_id}")
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, backup_failed_detail
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if not os.path.isfile(progress_file):
            self.log.error(f'progress file not exist! job id: {self._job_id}')
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, backup_failed_detail
        with open(progress_file, "r", encoding='UTF-8') as f:
            data = f.read()
        if "WARNING:  Session unused timeout.\nFATAL:  terminating connection due to administrator command" in data:
            error_detail = LogDetail(logDetail=OpenGaussErrorCode.ERR_SESSION_TIMEOUT, logTimestamp=int(time.time()),
                                     logLevel=DBLogLevel.ERROR.value)
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, error_detail
        for err_log in self._tool_err_logs:
            if err_log.format(copy_id) in data:
                self.log.error(f"Execute backup failed. job id: {self._job_id}")
                return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, backup_failed_detail
        if "INFO: Progress" not in data:
            return ProgressPercentage.START_PROGRESS.value, SubJobStatusEnum.RUNNING.value, empty_detail
        present_data, total_data = self.get_backup_data_info(data, r"INFO: Progress: \((.*?)\). Process file")
        if f"INFO: Backup {copy_id} completed" in data:
            complete_detail = LogDetail(logInfo="opengauss_plugin_backup_subjob_success_label",
                                        logInfoParam=[f"{sub_task_id}", f"{total_data}"],
                                        logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
            self.log.info(f"The backup is complete. job id: {self._job_id}")
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.COMPLETED.value, complete_detail
        try:
            progress = int((present_data / total_data) * ProgressPercentage.COMPLETE_PROGRESS.value)
        except Exception:
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.FAILED.value, empty_detail
        if "Validate file" in data:
            present_data, total_data = self.get_backup_data_info(data, r"INFO: Progress: \((.*?)\). Validate file")
            running_detail = LogDetail(logInfo="opengauss_plugin_execute_validate_backup_subjob_label",
                                       logInfoParam=[f"{sub_task_id}", f"{total_data}", f"{present_data}"],
                                       logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
        elif "Syncing backup files to disk" in data:
            running_detail = LogDetail(logInfo="opengauss_plugin_execute_sync_backup_subjob_label",
                                       logInfoParam=[f"{sub_task_id}"], logTimestamp=int(time.time()),
                                       logLevel=DBLogLevel.INFO.value)
        else:
            running_detail = LogDetail(logInfo="opengauss_plugin_execute_backup_subjob_label",
                                       logInfoParam=[f"{sub_task_id}", f"{total_data}", f"{present_data}"],
                                       logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
        return progress, SubJobStatusEnum.RUNNING.value, running_detail

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

    def get_external_dirs_option(self):
        space_root_path = os.path.join(self._data_dir, TableSpace.TABLESPACE_PARENT_DIR)
        if not os.path.isdir(space_root_path) or len(os.listdir(space_root_path)) < 1:
            return ""
        dir_list = []
        for cur_dir in os.listdir(space_root_path):
            dir_list.append(os.path.realpath(os.path.join(space_root_path, cur_dir)))
        sparator = " "
        return "--external-dirs " + sparator.join(dir_list)

    def exec_instance_backup(self, backup_mode):
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        if not os.path.isdir(copy_dir):
            self.log.error(f'Copy repository not exist! job id: {self._job_id}')
            return False
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.islink(progress_file):
            self.log.error(f'Progress file is link! job id: {self._job_id}')
            return False
        backup_cmd = f'{self._backup_tool} backup -B {copy_dir} -D {self._data_dir} --instance ' \
                     f'{CopyInfoKey.BACKUP_INSTANCE} {self.get_external_dirs_option()} -b {backup_mode}' \
                     f' -j {self._channel_number} --progress -d postgres -p {self._port} &> {progress_file}'
        return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, backup_cmd)
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"Execute backup failed. job id: {self._job_id}")
            self.print_gs_probackup_log()
            self.exec_sql_cmd("select pg_stop_backup();")
        return self.duplicate_check()

    def print_gs_probackup_log(self):
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.islink(progress_file):
            self.log.error(f'Progress file is link! job id: {self._job_id}')
            return
        with open(progress_file, "r", encoding='UTF-8') as file_descriptor:
            data = file_descriptor.read()
        if not data:
            self.log.error(f'Progress file is empty! job id: {self._job_id}')
            return
        self.log.info(data)

    def replica_repository_chown(self):
        self.log.debug(f"Backup instance chown. job id: {self._job_id}")
        path_list = []
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        if not os.path.exists(copy_dir):
            self.log.warning(f"Copy dir not exists. job id: {self._job_id}")
            return
        path_list.append(copy_dir)
        wal_dir = os.path.join(copy_dir, CopyInfoKey.WAl)
        path_list.append(wal_dir)
        for dirpath, dirnames, filenames in os.walk(wal_dir):
            for filename in filenames:
                path_list.append(os.path.join(dirpath, filename))
            for dirname in dirnames:
                path_list.append(os.path.join(dirpath, dirname))
        backups_dir = os.path.join(copy_dir, CopyInfoKey.BACKUPS)
        path_list.append(backups_dir)
        backup_instance_dir = os.path.join(backups_dir, CopyInfoKey.BACKUP_INSTANCE)
        path_list.append(backup_instance_dir)
        name_list = os.listdir(backup_instance_dir) if os.path.isdir(backup_instance_dir) else []
        for cur_name in name_list:
            abs_path = os.path.abspath(os.path.join(backup_instance_dir, cur_name))
            path_list.append(abs_path)
            if os.path.isdir(abs_path):
                path_list.extend(self.get_copy_conf_files(abs_path))
        path_list.append(os.path.join(self._cache_dir, self._job_id))
        for path in path_list:
            if not os.path.exists(path):
                continue
            change_path_permission(path, self._user_name)

    def get_copy_conf_files(self, duplicate_dir):
        file_list = []
        conf_list = ["backup_content.control", "backup.control"]
        for conf_name in conf_list:
            abs_path = os.path.abspath(os.path.join(duplicate_dir, conf_name))
            if not os.path.isfile(abs_path) or os.path.islink(abs_path):
                self.log.warning(f"Bad conf file. job id: {self._job_id}")
                continue
            file_list.append(abs_path)
        return file_list

    def duplicate_check(self):
        call_ret, copy_id, copy_mode = self.present_copy_info()
        if not call_ret:
            self.log.error(f"Get present copy info failed. job id: {self._job_id}")
            return False
        copy_info = self.get_copy_info_by_copy_id(copy_id)
        duplicate_status = copy_info.get(CopyInfoKey.BACKUPS, [{}])[0].get(CopyInfoKey.STATUS)
        if duplicate_status == "OK":
            return True
        self.log.info(f"Duplicate status is abnormal, Deleting Residual Data. job id: {self._job_id}")
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        delete_copy_cmd = f'{self._backup_tool} delete -B {copy_dir} --instance={CopyInfoKey.BACKUP_INSTANCE} -i' \
                          f' {copy_id}'
        delete_ret_code, _, delete_std_err = execute_cmd_by_user(self._user_name, self._env_file, delete_copy_cmd)
        if delete_ret_code != ResultCode.SUCCESS:
            self.log.error(f"Delete copy failed, std_err: {delete_std_err}. job id: {self._job_id}")
            return False
        check_cmd = f'{self._backup_tool} show -B {copy_dir} --instance={CopyInfoKey.BACKUP_INSTANCE} --format=json'
        check_ret_code, check_std_out, check_std_err = execute_cmd_by_user(self._user_name, self._env_file, check_cmd)
        if check_ret_code != ResultCode.SUCCESS:
            self.log.error(f"Clean copy residual data failed, std_err: {check_std_err}. job id: {self._job_id}")
            return False
        empty_instance = [{"instance": "backup_instance", "backups": []}]
        out_list = json.loads(check_std_out)
        if empty_instance == out_list:
            self.log.debug("Clean copy residual data. job id: {self._job_id}")
            safe_remove_path(copy_dir)
        self.log.debug(f"Delete Residual Data finish. job id: {self._job_id}")
        return False

    def get_backup_path(self):
        ret, backup_index_id, _ = self.present_copy_info()
        if ret and backup_index_id:
            return os.path.join(
                self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY,
                CopyInfoKey.BACKUPS, CopyInfoKey.BACKUP_INSTANCE, backup_index_id
            )
        return ""

    # 检查是否开启归档
    def query_archive_mode(self):
        sql_cmd = "show archive_mode;"
        ret, cont, std_err = self.exec_sql_cmd(sql_cmd)
        if ret != ResultCode.SUCCESS:
            return False
        archive_mode_list = cont.split("\n")
        self.log.info(f"show archive_mode return {archive_mode_list}, {std_err}")
        if len(archive_mode_list) < 3:
            return False
        return archive_mode_list[2].strip() != "off"

    # 检查是否配置归档文件夹
    def query_archive_dir(self):
        sql_cmd = "show archive_command;"
        ret, archive_info, std_err = self.exec_sql_cmd(sql_cmd)
        if ret != ResultCode.SUCCESS:
            return ""
        try:
            archive_dir = archive_info.split()[archive_info.split().index("cp") + 2]
        except Exception as err_exception:
            self.log.error(f"Query archive dir failed, exception: {err_exception}")
            return ""
        archive_dir = archive_dir.strip('"%f')
        self.log.info(f"query archive dir: {archive_dir}")
        if not os.path.isdir(archive_dir):
            return ""
        return archive_dir

    # 解析归档文件夹下的备份文件
    def decode_wal_backup_file(self, backup_file):
        self.log.info(f"Ready to decode file: {backup_file}")
        with open(backup_file, "r") as f:
            lines = f.readlines()
        start_wal = None
        stop_wal = None
        for info in lines:
            if "START WAL" in info:
                # START WAL LOCATION: 0/8000028 (file 000000010000000000000008)
                start_wal_info = info.split()[-1]
                # 000000010000000000000008)
                start_wal = start_wal_info[:-1]
            if "STOP WAL" in info:
                # STOP WAL LOCATION: 0/B0000F0 (file 00000001000000000000000B)
                stop_wal_info = info.split()[-1]
                # 000000010000000000000008)
                stop_wal = stop_wal_info[:-1]
        self.log.info(f"backup file is decoded start wal: {start_wal}, stop wal: {stop_wal}")
        return start_wal, stop_wal

    # 备份本次备份产生的日志
    def query_wal_file_list(self, wal_dir):
        # 查找备份文件
        file_list = []
        backup_file = get_backup_info_file(wal_dir)
        if not backup_file:
            self.log.error(f"Failed to get backup info file, job id: {self._job_id}.")
            return file_list
        # 解析上一次备份的备份文件
        start_wal, stop_wal = self.decode_wal_backup_file(backup_file)
        file_list = os.listdir(wal_dir)
        file_list = sorted(file_list, key=lambda x: os.path.getmtime(os.path.join(wal_dir, x)))
        if not (start_wal in file_list and stop_wal in file_list):
            self.log.info(f"Start wal:{start_wal} or stop wal:{stop_wal} not in archive dir!")
            return []
        start = file_list.index(start_wal)
        # 日志备份的时候，先去找上次记录下来的上次的备份结束位置，如果能找到，就用上次的结束位置+1。
        # 如果找不到，再用当前备份任务产生的开始位置。最后新增或者更新这个结束位置。
        if start_wal in file_list:
            start = file_list.index(start_wal)
        end = file_list.index(stop_wal) + 1
        backup_wal_list = file_list[start:end]
        self.log.info(f"Query wal file list: {backup_wal_list}")
        return backup_wal_list

    # 获取批量备份文件任务的状态
    def get_backup_status(self, job_id):
        backup_status = False
        while True:
            time.sleep(10)
            status, progress, data_size = query_progress(job_id)
            self.log.info(f"Get backup progress: status:{status}, progress:{progress}, "
                        f"data_size:{data_size}")
            if status == BackupStatus.COMPLETED:
                self.log.info(f"Backup completed, jobId: {self._job_id}.")
                backup_status = True
                break
            elif status == BackupStatus.RUNNING:
                continue
            elif status == BackupStatus.FAILED:
                self.log.error(f"Backup failed, jobId: {self._job_id}.")
                backup_status = False
                break
            else:
                self.log.error(f"Backup failed, status error jobId: {self._job_id}.")
                backup_status = False
                break
        return backup_status

    # 批量备份日志
    def backup_file_list(self, files, target):
        if not files or not target:
            self.log.error(f"Param error, job id: {self._job_id}.")
            return False
        res = backup_files(self._job_id, files, target, write_meta=True)
        if not res:
            self.log.error(f"Failed to start backup, jobId: {self._job_id}.")
            return False
        return self.get_backup_status(self._job_id)

    # 备份本次备份过程中产生的日志吻技安
    def backup_wal_files(self):
        if self._backup_type != BackupTypeEnum.LOG_BACKUP:
            self.log.info(f"Backup type is not log, {self._backup_type}")
            return False
        archive_dir = self.query_archive_dir()
        self.log.info(f"archive dir is {archive_dir}")
        if not archive_dir:
            self.log.error(f"Failed to query archive dir, job id: {self._job_id}")
            return False
        file_list = self.query_wal_file_list(archive_dir)
        if not file_list:
            self.log.error(f"Failed to get wal file list, job id: {self._job_id}")
            return False

        target = self._log_dir
        try:
            files = [os.path.join(archive_dir, file) for file in file_list]
            result = self.backup_file_list(files, target)
            if not result:
                self.log.error(f"Failed to wal file: {len(files)}, job id: {self._job_id}.")
                return False
            self.log.info(f"Succeed to backup wal file: {len(files)}, job id: {self._job_id}.")
            self.log.info(f"Backup wal file list: {files}")
        except Exception as e:
            self.log.error(f"Failed to backup wal file, err: {e}")
        return True

    # 解析查询出的全备增备信息
    def parse_backup_infos(self):
        copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY)
        if not os.path.isdir(copy_dir):
            self.log.error(f'Copy repository not exist! job id: {self._job_id}')
            return []
        check_cmd = f'{self._backup_tool} show -B {copy_dir} --instance={CopyInfoKey.BACKUP_INSTANCE} --format=json'
        return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, check_cmd)
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"get backup infos failed, std_err: {std_err}. job id: {self._job_id}")
            return []
        empty_instance = [{"instance": "backup_instance", "backups": []}]
        out_list = json.loads(std_out)
        if empty_instance == out_list:
            self.log.debug("parse backup infos: empty backups. job id: {self._job_id}")
            safe_remove_path(copy_dir)
        backups = out_list[0]["backups"]
        self.log.info(f"backup info in backups: {backups}")
        return out_list[0]["backups"]

    # 根据lsn获取对应的wal文件
    def get_start_file_name(self, start_file):
        sql_cmd = f"select pg_xlogfile_name(\'\\\'{start_file}\\\'\');"
        # 解析xlog文件
        ret, std_out, std_err = self.exec_sql_cmd(sql_cmd)
        if ret != ResultCode.SUCCESS:
            self.log.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}")
            return ""
        start_wal_list = std_out.split("\n")
        if len(start_wal_list) < 3:
            return ""
        return start_wal_list[2].strip()

    # 调用api手动启动备份
    def select_start_backup(self):
        sql_cmd = f"select pg_start_backup('\\'{self._job_id}\\'', false, true);"
        ret, std_out, std_err = self.exec_sql_cmd(sql_cmd)
        if ret != ResultCode.SUCCESS:
            self.log.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}")
            return ""
        start_list = std_out.split("\n")
        if len(start_list) < 3:
            return ""
        return start_list[2].strip()

    # 获取备份结束时间
    def get_backup_time(self):
        archive_dir = self.query_archive_dir()
        if not archive_dir:
            return ""
        self.log.info("Try to find the latest backup.")
        last_backup_file = get_backup_info_file(archive_dir)
        if not last_backup_file:
            self.log.error(f"Failed to get last backup file, job id: {self._job_id}.")
            return ""
        last_backup_file_path = os.path.join(archive_dir, last_backup_file)
        _, self._stop_time = get_last_backup_stop_time(last_backup_file_path)
        return self._stop_time