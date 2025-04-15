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
import json
from common.const import DeployType, BackupTypeEnum
from common.common import write_content_to_file, read_tmp_json_file, convert_time_to_timestamp
from openGauss.backup.instance_backup import InstanceBackup
from openGauss.common.const import Status, ResultCode, SyncMode, \
    MetaDataKey, ProtectSubObject, CopyInfoKey, SUCCESS_RET, MAX_FILE_NUMBER_OF_LOG_BACKUP, BackupStatus
from openGauss.common.error_code import BodyErr
from openGauss.common.common import get_value_from_dict, str_to_int, get_previous_copy_info, execute_cmd_by_user, \
    is_wal_file, get_backup_info_file, get_last_backup_stop_time, safe_remove_path, is_cmdb_distribute


class InstancePitrBackup(InstanceBackup):
    def __init__(self, pid, job_id, param_json):
        super(InstancePitrBackup, self).__init__(pid, job_id, param_json)
        self._parent_id_list = []
        self._start_time = ""
        self._stop_time = None

    # 日志备份需要检查是否已经存在全量备份，并检查归档模式是否开启。在增量备份的基础上增加归档模式开启检查
    def check_backup_type(self):
        self.log.info(f"Check backup type. job id: {self._job_id}")
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            archive_mode = self.query_archive_mode()
            if not archive_mode:
                self.log.error(f'Archive mode is off. job id: {self._job_id}')
                return BodyErr.ERROR_ARCHIVE_MODE
            return SUCCESS_RET
        endpoint = self._resource_info.get_local_endpoint()
        if self._resource_info.get_node_status(endpoint) != Status.NORMAL:
            self.log.error(f"Node status is abnormal job id: {self._job_id}")
            return BodyErr.ERR_PLUGIN_CANNOT_BACKUP
        # 获取前一次备份信息时日志备份信息也要获取
        previous_copy = get_previous_copy_info(self._protect_obj, self._job_id)
        ret, last_copy_id = get_value_from_dict(previous_copy, MetaDataKey.EXTEND_INFO, MetaDataKey.BACKUP_INDEX_ID)
        if not ret or not isinstance(last_copy_id, str) or not self.get_copy_info_by_copy_id(last_copy_id):
            self.log.error(f"Check exist complete copy failed. job id: {self._job_id}")
            return BodyErr.ERR_LOG_TO_FULL

        archive_mode = self.query_archive_mode()
        if not archive_mode:
            self.log.error(f'Archive mode is off. job id: {self._job_id}')
            return BodyErr.ERROR_ARCHIVE_MODE
        # 检查归档位置已设置且路径可用
        archive_dir = self.query_archive_dir()
        if not archive_dir:
            self.log.error(f'Archive dir is off. job id: {self._job_id}')
            return BodyErr.ERROR_ARCHIVE_DIR

        self.log.info(f"Check backup type success. job id: {self._job_id}")

        return SUCCESS_RET

    def backup(self):
        """
        执行备份任务
        :return:
        """
        self.log.info(f"Execute instance log backup. job id: {self._job_id}. deploy type {self._deploy_type}."
                      f"database type {self._database_type}")
        # CMDB分布式场景
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            try:
                return self.exec_cmdb_instance_backup(BackupTypeEnum.LOG_BACKUP.value)
            except Exception as err:
                self.log.error(f'exec cmdb failed, error: {err}')
                self.update_progress(BackupStatus.FAILED)
                return False
        if not self.pre_backup():
            self.log.error(f"Backup prepose failed. job id: {self._job_id}")
            return False
        return self.exec_instance_pitr_backup()

    def extend_parent_id_list(self, last_log_stop_time):
        backups = self.parse_backup_infos()
        if not backups:
            return
        all_full_backups = [x for x in reversed(backups) if x["status"] == "OK" and x["backup-mode"] == "FULL"]
        self._parent_id_list.extend(
            [x["id"] for x in all_full_backups if x["recovery-time"] > last_log_stop_time])

    # 首次日志备份 备份首次全备到本次日备的所有日志
    def first_log_backup(self, wal_dir, start_file):
        stop_wal_file = self.get_start_file_name(start_file)
        self.log.info(f"first_log_backup start file name: {stop_wal_file}")
        if stop_wal_file:
            wal_file_time_id = stop_wal_file[:8]
            # 前8位是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF，备份和当前数据timeline一致的wal
            file_list = list(filter(lambda x: is_wal_file(x) and x[:8] == wal_file_time_id,
                                    os.listdir(wal_dir)))
            #  只备份在首次日志备份开始时间生成之前的日志，转换为16进制比较时间大小，越大的说明生成时间越靠后
            wal_file_int = int(stop_wal_file, 16)
            files = [file for file in file_list if int(file, 16) <= wal_file_int]
        else:
            files = [file for file in os.listdir(wal_dir) if is_wal_file(file)]
            self.log.info("The first backup file is not exist, ready to backup all wal files.")
            return files

        if len(files) == 0:
            return files

        backups = self.parse_backup_infos()
        self.log.info("Find the earliest full backup with archived logs")
        for backup in reversed(backups):
            # 寻找最早且日志已归档的全量备份
            if backup["backup-mode"] == "FULL" and backup["status"] == "OK":
                start_wal_file = self.get_start_file_name(backup["stop-lsn"])
                # 全备存在日志无归档则跳过此备份
                if start_wal_file not in files:
                    continue
                last_wal_file_time_id = int(start_wal_file, 16)
                # 筛选日志文件生成晚于首次全备
                wal_files = [file for file in files if int(file, 16) > last_wal_file_time_id]
                self.log.info(f"Filter file list number :{len(wal_files)} backup: {backup}")
                self._parent_id_list.append(backup["id"])
                recovery_time = backup["recovery-time"]
                self.extend_parent_id_list(recovery_time)
                recovery_time = recovery_time[:len(recovery_time) - len(CopyInfoKey.UTC_TIME_SUFFIX)]
                self._start_time = recovery_time
                self.log.info(f"start time：{self._start_time}")
                return wal_files
        self.log.info(f"Filter file list number :{len(files)}")
        return files

    def rsync_archive_logs(self, wal_dir):
        endpoint = self._resource_info.get_local_endpoint()
        nodeips = self._resource_info.get_cluster_nodes()
        for nodeip in nodeips:
            if nodeip != endpoint:
                check_cmd = f'rsync -av --ignore-existing -e \"ssh\" {self._user_name}@{nodeip}:{wal_dir} {wal_dir}'
                self.log.info(f"Try to rsync logs, local ip: {endpoint}, source ip: {nodeip}")
                return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, check_cmd)
                if return_code != ResultCode.SUCCESS:
                    self.log.error(f"Fail to rsync log, std_err: {std_err}")
                else:
                    self.log.info(f"Success to rsync.")

    def backup_log_wals(self, wal_dir, last_wal, start_file):
        # 检查.backup文件在当前节点的归档目录中是否存在
        if self._resource_info.get_deploy_type() == DeployType.CLUSTER_TYPE and last_wal not in os.listdir(wal_dir):
            # 非单机时尝试与其他节点同步日志
            self.rsync_archive_logs(wal_dir)
            if last_wal not in os.listdir(wal_dir):
                self.log.warn("Can't find target backup file after rsync, try first log backup")
                return self.first_log_backup(wal_dir, start_file)

        # 备份两次日志备份之间的日志
        stop_wal_file = self.get_start_file_name(start_file)
        self.log.info(f"This backup first wal: {start_file}")
        if stop_wal_file:
            wal_file_time_id = stop_wal_file[:8]
            # 前8位是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF，备份和当前数据timeline一致的wal
            file_list = list(filter(lambda x: is_wal_file(x) and x[:8] == wal_file_time_id,
                                    os.listdir(wal_dir)))
            #  只备份在首次日志备份开始时间生成之前的日志，转换为16进制比较时间大小，越大的说明生成时间越靠后
            wal_file_int = int(stop_wal_file, 16)
            files = [file for file in file_list if int(file, 16) <= wal_file_int]
        else:
            files = [file for file in os.listdir(wal_dir) if is_wal_file(file)]
            self.log.info("The first backup file is not exist, ready to backup all wal files.")
            return files

        last_wal_start_file, last_wal_stop_file = self.decode_wal_backup_file(last_wal)

        if len(files) == 0 or last_wal_stop_file not in files or last_wal_start_file not in files:
            self.log.info(f"The last wal {last_wal_start_file} or {last_wal_stop_file} not in {files}")
            return files

        last_wal_file_time_id = int(last_wal_stop_file, 16)
        # 筛选晚于前一次日志备份的日志文件
        wal_files = [file for file in files if int(file, 16) > last_wal_file_time_id]
        self.log.info(f"Filter file list number :{len(wal_files)}")
        return wal_files

    def check_backups_before_last_log(self, last_log_stop_time):
        # 检查最近一次日备之前是否存在其他备份
        backups = self.parse_backup_infos()
        # 比较时间最早的备份与上一次日志备份结束时间
        for backup in reversed(backups):
            if backup["status"] == "OK" and backup["end-time"] < last_log_stop_time:
                return True
        return False

    # 最近一次备份的恢复时间
    def latest_backup_recovery_time(self):
        backups = self.parse_backup_infos()
        if not backups:
            return ''
        return backups[0]["recovery-time"]

    # 查询上一次备份到本次日志备份的日志文件
    def query_log_file_list(self, wal_dir, start_file):
        self.log.info("Backup wal logs from last backup")
        # 将前一次日志备份的元数据保存在meta目录下的LastBackupCopy.info
        copy_meta_file = os.path.join(self._meta_dir, "LastBackupCopy.info")
        # 首次日志备份 查找已归档日志的全量备份日志
        if not os.path.exists(copy_meta_file):
            self.log.info("No LastBackupCopy exists, start first log backup")
            return self.first_log_backup(wal_dir, start_file)

        copy_meta_info = read_tmp_json_file(copy_meta_file)
        self.log.info(f"query_log_file_list copy_meta_info: {copy_meta_info}")
        last_log_stop_time = copy_meta_info.get("stop_time")
        last_wal = copy_meta_info.get("wal_file")
        # 检查前一次日志备份的时间是否早于最近非日志备份时间
        latest_backup_recovery_time = self.latest_backup_recovery_time()
        # 最近日志备份时间早于最近非日志备份时间 检查前一次日志备份依赖的备份是否存在
        self.log.info(
            f"Check if latest log backup time({last_log_stop_time}) earlier than "
            f"latest other backup({latest_backup_recovery_time})")
        if last_log_stop_time < latest_backup_recovery_time:
            # 检查前一次日志备份之前的备份是否存在 不存在则当作第一次日志备份
            if not self.check_backups_before_last_log(last_log_stop_time):
                self.log.info("Former log backup been deleted")
                return self.first_log_backup(wal_dir, start_file)
        # 备份两次日志备份之间的日志
        self._parent_id_list.append(copy_meta_info.get("copy_id"))
        self.extend_parent_id_list(last_log_stop_time)
        self._start_time = last_log_stop_time
        self.log.info("Try to backup logs between two log backup")
        return self.backup_log_wals(wal_dir, last_wal, start_file)

    # 备份前一次备份结束到当前备份的日志文件
    def backup_data(self, start_file):
        if not os.path.isdir(self._log_dir):
            return False
        archive_dir = self.query_archive_dir()
        file_list = self.query_log_file_list(archive_dir, start_file)
        if len(file_list) == 0:
            self.log.info("No new logs need to be backed up.")
            return True
        files = [os.path.realpath(os.path.join(archive_dir, file)) for file in file_list]
        # 一次性备份多个日志文件
        for i in range(0, len(files), MAX_FILE_NUMBER_OF_LOG_BACKUP):
            result = self.backup_file_list(files[i:i + MAX_FILE_NUMBER_OF_LOG_BACKUP], self._log_dir)
            if not result:
                self.log.error(f"Failed to backup wal file: {len(files)}, job id: {self._job_id}.")
                return False
        return True

    # 保存备份信息
    def save_backup_info(self):
        archive_dir = self.query_archive_dir()
        self.log.info("First log backup save backup time.")
        last_backup_file = get_backup_info_file(archive_dir)
        if not last_backup_file:
            self.log.error(f"First log backup ,failed to get last backup file, job id: {self._job_id}.")
            return False
        last_backup_file_path = os.path.join(archive_dir, last_backup_file)
        start_time, stop_time = get_last_backup_stop_time(last_backup_file_path)
        if not self._start_time:
            self._start_time = start_time
        copy_meta_info = {
            "copy_id": f"log_{self._job_id}", "stop_time": stop_time, "start_time": self._start_time,
            "wal_file": last_backup_file_path, "parent_backup": self._parent_id_list
        }
        # 将备份信息存入LastBackupCopy.info
        file_name = os.path.join(self._meta_dir, "LastBackupCopy.info")
        if os.path.exists(file_name) and os.path.islink(file_name):
            self.log.error(f"The path[{file_name}] is invalid, job id: {self._job_id}.")
            return False
        if os.path.exists(file_name):
            safe_remove_path(file_name)
        self.log.info(f"LastBackupCopy.info: {file_name}, copy_meta_info: {copy_meta_info}")
        write_content_to_file(file_name, json.dumps(copy_meta_info))
        self.log.info(f"Success to get and save last backup stop time, stop_time:{stop_time}.")
        self.update_progress(BackupStatus.COMPLETED)
        return True

    def exec_instance_pitr_backup(self):
        # 开始备份
        self.update_progress(BackupStatus.RUNNING)
        start_file = self.select_start_backup()
        if not start_file:
            self.log.error(f"Execute pg_backup_start failed. job id: {self._job_id}")
            self.update_progress(BackupStatus.FAILED)
            return False

        # 备份数据文件
        try:
            result = self.backup_data(start_file)
            if not result:
                self.log.error("Failed to backup wal files for log backup")
        except Exception as ex:
            self.log.error(f"Back up data Failed! Exception is {ex}")
            self.exec_sql_cmd("select pg_stop_backup();")
            self.update_progress(BackupStatus.FAILED)
            raise ex
        # 停止备份
        ret, std_out, std_err = self.exec_sql_cmd("select pg_stop_backup();")
        if not ret:
            self.log.error(f"Execute pg_stop_backup failed. job id: {self._job_id}")
            self.update_progress(BackupStatus.FAILED)
            return False

        # 备份备份过程中产生的wal文件
        result = self.backup_wal_files()
        if not result:
            self.log.error(f"Failed to backup wal files. job id: {self._job_id}")
            self.update_progress(BackupStatus.FAILED)
            return False

        # 保存LastBackupInfo
        result = self.save_backup_info()
        if not result:
            self.log.error(f"Failed to save backup info. job id: {self._job_id}")
            return False
        return True

    def present_copy_info(self):
        return True, f"log_{self._job_id}", "LOG"

    def update_progress(self, status):
        copy_id = f"log_{self._job_id}"
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.isfile(progress_file):
            safe_remove_path(progress_file)
        if status == BackupStatus.FAILED:
            content = f"ERR: Backup failed."
            return
        content = f"INFO: Progress: (0/100). Process file"
        if status == BackupStatus.COMPLETED:
            content = content + f"\nINFO: Backup {copy_id} completed"
        write_content_to_file(progress_file, content)

    def get_copy_meta_data(self, copy_time):
        # 组装备份副本信息
        self.log.info(f"Get copy meta data!. job id: {self._job_id}")
        ret, copy = self.get_base_meta_data()
        if not ret:
            self.log.error(f"Get base meta failed!. job id: {self._job_id}")
            return copy
        protect_obj = copy.setdefault(MetaDataKey.PROTECT_OBJECT, {})
        protect_obj[MetaDataKey.SUB_TYPE] = ProtectSubObject.INSTANCE
        copy[MetaDataKey.PG_PROBACKUP_CONF] = self.get_pg_probackup_conf_data()
        copy[MetaDataKey.PARENT_COPY_ID] = self._parent_id_list
        copy[MetaDataKey.ENABLE_CBM_TRACKING] = self.get_enable_cbm_tracking_status()
        copy[MetaDataKey.BEGIN_TIME] = convert_time_to_timestamp(self._start_time)
        copy[MetaDataKey.END_TIME] = convert_time_to_timestamp(self._stop_time)
        copy[MetaDataKey.BACKUP_TIME] = convert_time_to_timestamp(copy_time)
        self.log.info(
            f"Get copy meta data!. job id: {self._job_id}, start time: {self._start_time}, "
            f"stop time: {self._stop_time}")
        return copy

    # 取得备份副本时间
    def get_copy_time(self):
        file_name = os.path.join(self._meta_dir, "LastBackupCopy.info")
        if os.path.exists(file_name) and os.path.islink(file_name):
            self.log.error(f"The path[{file_name}] is invalid, job id: {self._job_id}.")
            return False, ""
        copy_meta_info = read_tmp_json_file(file_name)
        self.log.info(f"Enter get_copy_time. LastBackupCopy.info: {file_name}, copy_meta_info: {copy_meta_info}")
        self._stop_time = copy_meta_info.get("stop_time")
        self._start_time = copy_meta_info.get("start_time")
        self._parent_id_list = copy_meta_info.get("parent_backup")
        return True, self._stop_time
