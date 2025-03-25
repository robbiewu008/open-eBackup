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
import shutil
import sys
import time
from datetime import datetime
import subprocess
import shlex
from pathlib import Path

from common.common_models import LogDetail, RepositoryPath, ScanRepositories
from common.common import write_content_to_file, read_tmp_json_file, get_local_ips, output_execution_result_ex, \
    convert_timestamp_to_time, get_previous_copy_info, read_result_file, execute_cmd
from common.const import SubJobStatusEnum, DBLogLevel, BackupTypeEnum, RpcParamKey, RepositoryDataTypeEnum
from common.file_common import change_path_permission
from common.util.backup import backup_files, query_progress
from common.util.cmd_utils import cmd_format
from openGauss.backup.backup_base import BackupBase
from openGauss.common.common import get_value_from_dict, execute_cmd_by_user, check_injection_char, str_to_int, \
    safe_remove_path, get_backup_info_file, get_last_backup_stop_time, get_env_value, is_cmdb_distribute, \
    copy_sub_dir_backup_name
from openGauss.common.const import Tool, ProtectObject, ResultCode, CopyDirectory, CopyInfoKey, MetaDataKey, \
    ProgressPercentage, TableSpace, ParamKey, GsprobackupParam, BackupStatus, AuthKey, OpenGaussSubJobName
from openGauss.common.error_code import OpenGaussErrorCode


class InstanceBackup(BackupBase):
    def __init__(self, pid, job_id, param_json):
        super(InstanceBackup, self).__init__(pid, job_id, param_json)
        self._channel_number = GsprobackupParam.DEFAULT_PARALLEL
        self._tool_err_logs = [
            "WARNING: Backup {} is running, setting its status to ERROR", "ERROR: Backup {} failed",
            "ERROR: "
        ]
        self.init_environment_info()
        self.init_channel_number(param_json)
        _, self._backup_type = get_value_from_dict(param_json, ParamKey.JOB, ParamKey.JOB_PARAM, ParamKey.BACKUP_TYPE)
        self._stop_time = ""
        self._cluster_name = self._cluster_name()

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

        # 取基于的全备ID
        if self._backup_type != BackupTypeEnum.FULL_BACKUP.value:
            last_full_copy = get_previous_copy_info(self._protect_obj, [RpcParamKey.FULL_COPY], self._job_id)
            self._base_id = last_full_copy.get("id")
        else:
            self._base_id = self._job_id

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

    def query_scan_repositories(self):
        if self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            backup_path = self._log_dir
            backup_repo_path = RepositoryPath(repositoryType=RepositoryDataTypeEnum.LOG_REPOSITORY.value,
                                              scanPath=backup_path)
            meta_dir = self.get_repository_dir(self._param, RepositoryDataTypeEnum.LOG_META_REPOSITORY)
            save_path = os.path.dirname(self._log_dir)
        else:
            backup_path = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, self._job_id) \
                if is_cmdb_distribute(self._deploy_type, self._database_type) else self.get_backup_path()
            backup_repo_path = RepositoryPath(repositoryType=RepositoryDataTypeEnum.DATA_REPOSITORY.value,
                                              scanPath=backup_path)
            meta_dir = self._meta_dir
            save_path = meta_dir
        cur_meta_data = os.path.join(meta_dir, self._job_id)
        self.log.info(f"Query scan repos. get backup path: {backup_path}")
        if not os.path.exists(cur_meta_data):
            os.makedirs(cur_meta_data)
        self.log.info(f"Query scan repos. get meta path: {cur_meta_data}")
        meta_repo_path = RepositoryPath(repositoryType=RepositoryDataTypeEnum.META_REPOSITORY.value,
                                        scanPath=cur_meta_data)
        scan_repos = ScanRepositories(scanRepoList=[backup_repo_path, meta_repo_path], savePath=save_path)
        self.log.info(f"Query scan repos success. job id: {scan_repos.dict(by_alias=True)}")
        return True, scan_repos

    def get_copy_time(self):
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            # cmdb场景，副本时间从cmdb备份信息取
            cur_back_time = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, "cur_back_time")
            cur_copy_time = read_result_file(cur_back_time)
            str_copy_time = convert_timestamp_to_time(int(cur_copy_time))
            self.log.info(f"Success to get cur copy time {str_copy_time}.")
            return True, str_copy_time
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
        # 清理 cache 仓
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
        ret, job_name = get_value_from_dict(self._param, "subJob", "jobName")
        # 当子任务为EMPTY_SUB_JOB直接返回成功
        sub_task_id = sys.argv[4] if len(sys.argv) > 4 else 0
        if job_name == OpenGaussSubJobName.EMPTY_SUB_JOB:
            complete_detail = LogDetail(logInfo="opengauss_plugin_database_backup_subjob_success_label",
                                    logInfoParam=[f"{sub_task_id}"],
                                    logTimestamp=int(time.time()), logLevel=DBLogLevel.INFO.value)
            self.log.info(f"The empty backup is complete. job id: {self._job_id}")
            return ProgressPercentage.COMPLETE_PROGRESS.value, SubJobStatusEnum.COMPLETED.value, complete_detail

        # 分布式cmdb场景
        self.log.info(f'Get progress deploy type: {self._deploy_type} database type: {self._database_type}')
        if is_cmdb_distribute(self._deploy_type, self._database_type):
            return self.get_cmdb_backup_progress()
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
        running_detail = self.get_detail_infos(data, present_data, sub_task_id, total_data)
        return progress, SubJobStatusEnum.RUNNING.value, running_detail

    def get_detail_infos(self, data, present_data, sub_task_id, total_data):
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
        return running_detail

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
        # 备份表空间路径
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
            # 日志打印备份工具回显
            self.print_gs_probackup_log()
            self.exec_sql_cmd("select pg_stop_backup();")
            self.update_progress(BackupStatus.FAILED)
        # show 备份信息
        return self.duplicate_check()

    def update_progress(self, status):
        progress_file = os.path.join(self._cache_dir, self._job_id)
        if os.path.isfile(progress_file):
            safe_remove_path(progress_file)
        content = f"INFO: Progress: (100/100). Process file"
        if status == BackupStatus.COMPLETED:
            content = content + f"INFO: Backup completed"
        if status == BackupStatus.FAILED:
            content = content + f"INFO: Backup failed"
        write_content_to_file(progress_file, content)

    def _cluster_name(self) -> str:
        cmd = "echo $GS_CLUSTER_NAME"
        ret, stdout, stderr = execute_cmd_by_user(self._user_name, self._env_file, cmd)
        cluster_name = stdout.strip()
        self.log.warn(cluster_name)
        return cluster_name

    def _move_wal_files(self, monitor_info: dict, parent_dir: str) -> None:
        for gtm in monitor_info["gtm"]:
            src = Path(parent_dir, f'{gtm["name"]}_wal')
            dst = Path(parent_dir, "gtm", "wal", self._cluster_name)
            self.log.info(f"Found {len(os.listdir(src))} gtm wal files. {src}")
            [os.renames(w, dst / w.name) for w in src.iterdir()]
            self.log.info(f"Moved gtm wal files. {src} -> {dst}")
        for cn in monitor_info["coordinator"]:
            src = Path(parent_dir, f'{cn["name"]}_wal')
            dst = Path(parent_dir, "cn", "wal", self._cluster_name)
            self.log.info(f"Found {len(os.listdir(src))} coordinator wal files. {src}")
            [os.renames(w, dst / w.name) for w in src.iterdir()]
            self.log.info(f"Moved coordinator wal files. {src} -> {dst}")
        for k, v in monitor_info["datanode"].items():
            src = Path(parent_dir, f'{v[0]["name"]}_wal')
            dst = Path(parent_dir, k, "wal", self._cluster_name)
            self.log.info(f"Found {len(os.listdir(src))} datanode wal files. {src}")
            [os.renames(w, dst / w.name) for w in src.iterdir()]
            self.log.info(f"Moved datanode wal files. {src} -> {dst}")

    def _set_archive_command(self, monitor_info: dict, parent_dir: str, dcs: str) -> None:
        gt_cmds = [f"""ha_ctl set gtm {gtm["name"]} \
        -p archive_command="'cp %p {parent_dir}/{gtm["name"]}_wal/%f'" -l {dcs} -c {self._cluster_name}"""
                   for gtm in monitor_info["gtm"]]
        cn_cmds = [f"""ha_ctl set coordinator {cn["name"]} \
        -p archive_command="'cp %p {parent_dir}/{cn["name"]}_wal/%f'" -l {dcs} -c {self._cluster_name}"""
                   for cn in monitor_info["coordinator"]]
        dn_cmds = [f"""ha_ctl set datanode {v[0]["name"]} \
        -p archive_command="'cp %p {parent_dir}/{v[0]["name"]}_wal/%f'" -l {dcs} -c {self._cluster_name}"""
                   for k, v in monitor_info["datanode"].items()]
        dirs = ([f'{gtm["name"]}_wal' for gtm in monitor_info["gtm"]] +
                [f'{cn["name"]}_wal' for cn in monitor_info["coordinator"]] +
                [f'{v[0]["name"]}_wal' for k, v in monitor_info["datanode"].items()])
        [os.makedirs(os.path.join(parent_dir, d), mode=0o777, exist_ok=True) for d in dirs]
        su_cmd = "su - omm"
        ipt = "\n".join(gt_cmds + cn_cmds + dn_cmds)
        self.log.info(ipt)
        process = subprocess.run(shlex.split(su_cmd), input=ipt,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.log.info(f"_set_archive_command: {process.stdout, process.stderr}")

    def _monitor_info(self, dcs_address: str, dcs_port: str) -> dict:
        cmd = f"""ha_ctl monitor all -l http://{dcs_address}:{dcs_port} -c {self._cluster_name}"""
        self.log.info(cmd)
        ret, stdout, stderr = execute_cmd_by_user(self._user_name, self._env_file, cmd)
        self.log.warn(stdout)
        return json.loads(stdout)

    def _backup_yml(self, monitor_info: dict, parent_dir: str, ptrack: bool = False) -> str:
        a = f"""
gtm:
    backup_host: {monitor_info["gtm"][0]["host"]}
    backup_dir: {parent_dir}/gtm
    tbs_dir: {parent_dir}/gtm_tbs
{"    backup_type: PTRACK" * ptrack}
coordinator:
    backup_host: {monitor_info["coordinator"][0]["host"]}
    backup_dir: {parent_dir}/cn
    tbs_dir: {parent_dir}/cn_tbs
{"    backup_type: PTRACK" * ptrack}
datanode:"""
        bs = [f"""
    - {k}:
        backup_host: {v[0]["host"]}
        backup_dir: {parent_dir}/{k}
        tbs_dir: {parent_dir}/{k}_tbs
{"        backup_type: PTRACK" * ptrack}""" for k, v in monitor_info["datanode"].items()]
        yml = a + "".join(bs)
        return yml

    def _dcs_address(self) -> str:
        _, protect_env_extend_info = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                                         ParamKey.EXTEND_INFO)
        ret, dcs_address = get_value_from_dict(protect_env_extend_info, ParamKey.DCS_ADDRESS)
        return dcs_address

    def exec_cmdb_instance_backup(self, backup_mode):
        self.update_progress(BackupStatus.RUNNING)
        _, protect_env_extend_info = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.PROTECT_ENV,
                                                         ParamKey.EXTEND_INFO)
        ret, dcs_address = get_value_from_dict(protect_env_extend_info, ParamKey.DCS_ADDRESS)
        ret, dcs_port = get_value_from_dict(protect_env_extend_info, ParamKey.DCS_PORT)
        ret, dcs_user = get_value_from_dict(protect_env_extend_info, ParamKey.DCS_USER)
        dcs_pass = get_env_value(f"{AuthKey.PROTECT_ENV_DCS}{self._pid}")
        cur_copy_dir = self.set_backup_copy_dir(backup_mode)
        # 全备基于此次备份，其他基于上次全备
        base_copy_id = self._job_id
        if backup_mode != BackupTypeEnum.FULL_BACKUP.value:
            base_copy_id = get_previous_copy_info(self._protect_obj, [RpcParamKey.FULL_COPY], self._job_id).get("id")
            if base_copy_id is None:
                self.log.error(f"Can not find last full copy, cur backup type {backup_mode}")
                self.update_progress(BackupStatus.FAILED)
                return False

        # 数据目录
        base_copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, base_copy_id)
        os.makedirs(base_copy_dir, exist_ok=True)
        monitor_info, parallel = self._monitor_info(dcs_address, dcs_port), max(1, os.cpu_count() // 4)
        if backup_mode == BackupTypeEnum.FULL_BACKUP.value:
            backup_content = self._backup_yml(monitor_info, base_copy_dir, ptrack=False)
            backup_cmd = (f'ha_ctl backup all -p {base_copy_dir} -l http://{dcs_address}:{dcs_port} '
                          f'--user {dcs_user} --password {dcs_pass} -a "-j {parallel}" -c {self._cluster_name}')
        else:
            backup_content = self._backup_yml(monitor_info, base_copy_dir, ptrack=True)
            backup_cmd = f'ha_ctl backup all -p {base_copy_dir} -l http://{dcs_address}:{dcs_port} ' \
                         f'--user {dcs_user} --password {dcs_pass} -a "-b ptrack" -c {self._cluster_name}'
        # 写备份配置文件
        self.log.info(f"backup_cmd: {backup_cmd}, backup_content: {backup_content}, base_copy_dir: {base_copy_dir}")
        self.write_backup_config(backup_content, base_copy_dir)
        return_code = self.try_execute_backup_cmd(backup_cmd, monitor_info, base_copy_dir, dcs_address, dcs_port)
        if return_code != ResultCode.SUCCESS:
            self.log.error(f"Execute backup failed. job id: {self._job_id}")
            self.update_progress(BackupStatus.FAILED)
            return False

        # 拷贝wal日志文件
        # 全量备份时，修补上一次全备日志文件
        self.copy_wal_files(backup_mode, base_copy_dir, base_copy_id)

        # 读取备份记录， 并存储备份记录至backup.history
        read_backup_result_cmd = (f'ha_ctl backup show -l http://{dcs_address}:{dcs_port} '
                                  f'-p {base_copy_dir} -c {self._cluster_name}')
        return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, read_backup_result_cmd)
        self.log.info(f"ha_ctl backup show: {read_backup_result_cmd} {std_out}")

        # 移动 wal 文件进备份目录
        self._move_wal_files(monitor_info, base_copy_dir)

        # 拆分备份文件
        # E6000 会抹去对全量副本的修改
        if backup_mode != BackupTypeEnum.FULL_BACKUP:
            backup_name = std_out.strip().split('\n')[3].split('|')[3].strip()
            try:
                copy_sub_dir_backup_name(base_copy_dir, cur_copy_dir, backup_name, self.get_start_backup_time())
            except Exception as err:
                self.log.error(f"copy sub dir, err: {err}")

        # 备份后操作，记录备份时间等信息
        self.after_backup(backup_mode, base_copy_dir, base_copy_id, std_out)
        return True

    def try_execute_backup_cmd(self, backup_cmd, monitor_info, base_copy_dir, dcs_address, dcs_port):
        # 发起备份并检查进度
        # 偶现 backup.yml not exist 错误，规避方法，重复尝试三次
        for _ in range(4):
            self._set_archive_command(monitor_info, base_copy_dir, f"http://{dcs_address}:{dcs_port}")
            execute_cmd(cmd_format("chmod -R 777 {}", base_copy_dir))
            execute_cmd("sync")
            self.log.warning("chmod done.")
            return_code, std_out, std_err = execute_cmd_by_user(self._user_name, self._env_file, backup_cmd)
            self.log.info(f"Get instance backup return code: {return_code}, std out: {std_out},  std err: {std_err}")
            if return_code == ResultCode.SUCCESS:
                self.log.info(f"hactl return success.")
                break
            elif return_code != ResultCode.SUCCESS and "backup.yml not exist" in std_out:
                self.log.warning(f"backup.yml not exist, retry backup.")
                time.sleep(20)
                continue
            break
        return return_code

    def set_backup_copy_dir(self, backup_mode):
        if backup_mode == BackupTypeEnum.LOG_BACKUP.value:
            cur_copy_dir = os.path.join(self._log_dir, CopyDirectory.INSTANCE_DIRECTORY)
        else:
            cur_copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, self._job_id)
        os.makedirs(cur_copy_dir, exist_ok=True)
        change_path_permission(cur_copy_dir, self._user_name)
        return cur_copy_dir

    def after_backup(self, backup_mode, base_copy_dir, base_copy_id, std_out):
        backup_history_info = []
        backup_info = std_out.strip().split('\n')[3].split('|')
        self.log.info(f"Get backup info {backup_info}")
        if len(backup_info) < 6:
            backup_info = [""] * 7
        dict_info = {
            "Instance": backup_info[1].strip(),
            "Version": backup_info[2].strip(),
            "ID": backup_info[3].strip(),
            "Recovery Time": backup_info[4].strip() or CopyInfoKey.NO_TIME,
            "Mode ": backup_info[5].strip(),
            "Status": backup_info[6].strip(),
            "JobID": self._job_id
        }
        backup_history_info.append(dict_info)
        backup_history_path = os.path.join(base_copy_dir, 'backup.history')
        last_backup_history_info = read_tmp_json_file(backup_history_path)
        backup_history_info.extend(last_backup_history_info)
        if os.path.exists(backup_history_path):
            os.remove(backup_history_path)
        self.log.info(f"Write backup history {backup_history_info} to {backup_history_path}")
        output_execution_result_ex(backup_history_path, backup_history_info)
        # 读取可恢复时间
        backup_recovery_time = self.get_baclup_recovery_time(backup_history_path)
        if backup_recovery_time == CopyInfoKey.NO_TIME:
            backup_recovery_timestamp = str(int(time.time()))
        else:
            backup_recovery_time = backup_recovery_time.split('.')[0]
            backup_recovery_timestamp = str(
                int(datetime.strptime(backup_recovery_time, '%Y-%m-%d %H:%M:%S').timestamp()) + 1)
        # 日志备份场景，存储可恢复时间文件
        if backup_mode == BackupTypeEnum.LOG_BACKUP:
            self.save_cmdb_backup_info(backup_recovery_timestamp, base_copy_id)
        self.save_recovery_timestamp(backup_recovery_timestamp, backup_mode)
        self.update_progress(BackupStatus.COMPLETED)

    def get_baclup_recovery_time(self, backup_history_path):
        backup_history_infos = []
        backup_history_infos.extend(read_tmp_json_file(backup_history_path))
        self.log.info(f"Get backup history info {backup_history_infos}")
        backup_history_last_info = backup_history_infos[0]
        backup_recovery_time = backup_history_last_info.get("Recovery Time", CopyInfoKey.NO_TIME)
        return backup_recovery_time

    def write_backup_config(self, backup_content, base_copy_dir):
        # 创建配置文件
        backup_config = os.path.join(base_copy_dir, 'backup.yml')
        if os.path.exists(backup_config):
            os.remove(backup_config)
        write_content_to_file(backup_config, backup_content)
        change_path_permission(backup_config, self._user_name)
        self.log.info(f"Write backup config {backup_content} to backup_config {backup_config}")

    def get_backup_host(self):
        local_ips = get_local_ips()
        self.log.info(f"Get local ips {local_ips}")
        cluster_nodes = list(set(self._resource_info.get_cluster_nodes()))
        self.log.info(f"Get cluster nodes {cluster_nodes}")
        union = set(local_ips) & set(cluster_nodes)
        union_list = list(union)
        backup_host = union_list[0]
        return backup_host

    def copy_wal_files(self, backup_mode, base_copy_dir, base_copy_id):
        if backup_mode == BackupTypeEnum.FULL_BACKUP.value:
            # 全量备份时，修补上一次全备日志文件
            last_full_copy = get_previous_copy_info(self._protect_obj, [RpcParamKey.FULL_COPY], self._job_id)
            self.log.info(f"Get last full copy: {last_full_copy}")
            last_full_copy_id = last_full_copy.get("id", None)
            if base_copy_id is not None and last_full_copy_id is not None:
                self.log.info(f"Start copy wal files from {base_copy_id} to {last_full_copy_id}")
                last_copy_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, last_full_copy_id)
                if os.path.exists(last_copy_dir):
                    self.copy_last_file_to_cur(base_copy_dir, last_copy_dir)
                else:
                    self.log.error(f"old path not exist {last_copy_dir}")

    def copy_last_file_to_cur(self, base_copy_dir, last_copy_dir):
        for data_dir in os.listdir(base_copy_dir):
            wal_path = os.path.join(base_copy_dir, str(data_dir), "wal")
            if os.path.exists(wal_path) and os.path.isdir(wal_path):
                old_wal_path = os.path.join(last_copy_dir, str(data_dir), "wal")
                if not os.path.exists(old_wal_path):
                    self.log.error(f"old wal path not exist {old_wal_path}")
                    break
                shutil.copytree(wal_path, old_wal_path, dirs_exist_ok=True)
        return_code, _, err_str = execute_cmd(cmd_format("chmod 755 {}", last_copy_dir))

    def save_recovery_timestamp(self, cur_archive_time, backup_mode):
        # 记录此次备份结束时间
        cur_back_time = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, "cur_back_time")
        if os.path.exists(cur_back_time):
            os.remove(cur_back_time)
        write_content_to_file(cur_back_time, cur_archive_time)
        self.log.info(f"Write cur bak timestamp: {cur_back_time} mode: {backup_mode} file: {cur_archive_time}")
        if backup_mode == BackupTypeEnum.INCRE_BACKUP:
            return

        # 记录下次日备起始时间
        meta_archived_time_file = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, "archived_time")
        if backup_mode == BackupTypeEnum.FULL_BACKUP:
            meta_archived_time_file = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY,
                                                   "full_backup_time")
        if os.path.exists(meta_archived_time_file):
            os.remove(meta_archived_time_file)
        write_content_to_file(meta_archived_time_file, cur_archive_time)
        self.log.info(f"Write archive time: {cur_archive_time} mode: {backup_mode} file: {meta_archived_time_file}")

    def get_archive_start_time(self):
        # 获取此次可恢复时间开始时间，按以下顺序： 1、上次日志备份可恢复结束时间 2、上次全量备份时间
        meta_archived_time = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, "archived_time")
        if not os.path.exists(meta_archived_time):
            meta_archived_time = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, "full_backup_time")
        archive_time = read_result_file(meta_archived_time)
        self.log.info(f"Success to get last archive time {archive_time}.")
        return archive_time

    def save_cmdb_backup_info(self, stop_time, base_copy_id):
        self.log.info(f"Start to get save log backup info, job id:{self._job_id}.")
        start_time = self.get_archive_start_time()

        copy_meta_info = {
            "copy_id": f"log_{self._job_id}",
            "stop_time": convert_timestamp_to_time(int(stop_time)),
            "start_time": convert_timestamp_to_time(int(start_time)),
            "parent_backup": [base_copy_id]
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
        self.log.info(f"Success to get and save last backup info, copy_meta_info:{copy_meta_info}.")
        return True

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
        ret, backup_index_id, copy_mode = self.present_copy_info()
        if ret and copy_mode == "LOG":
            return self._log_dir
        if ret and backup_index_id:
            return os.path.join(
                self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY,
                CopyInfoKey.BACKUPS, CopyInfoKey.BACKUP_INSTANCE, backup_index_id
            )
        return ""

    def get_cmdb_backup_path(self):
        instance_dir = os.path.join(self._backup_dir, CopyDirectory.INSTANCE_DIRECTORY, self._base_id)
        self.log.info(f"Get instance dir {instance_dir}")
        return instance_dir

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
            archive_dir = re.search(r"cp\s*%\S+\s+(\S+)", archive_info).group(1)
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

    # 备份本次备份过程中产生的日志文件
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
            self.log.error(f"Failed to exec cmd: {sql_cmd}, job id: {self._job_id}. {std_out, std_err}")
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