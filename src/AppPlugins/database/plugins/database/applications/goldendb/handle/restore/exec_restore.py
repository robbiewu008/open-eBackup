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

import glob
import pwd
import collections
import functools
import os
import pathlib
import re
import subprocess
import threading
import time
import pexpect

from goldendb.logger import log
from common.cleaner import clear
from common.common import check_command_injection_exclude_quote, execute_cmd, output_result_file
from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.const import SubJobStatusEnum, ParamConstant, SubJobPriorityEnum, ReportDBLabel
from common.file_common import get_user_info, delete_path
from common.util.common_utils import change_dir_owner_recursively
from common.util.exec_utils import exec_overwrite_file, exec_mkdir_cmd, exec_cp_dir_no_user
from goldendb.handle.common.const import GoldenDBCode, CMDResult, GoldenDBJsonConst, Env, GoldenDBNodeType, \
    MountBindPath, ErrorCode, LogLevel
from common.util.check_user_utils import check_path_owner, check_os_user
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.common.const import ProgressInfo, SubJobPolicy, GoldenSubJobName, SubJobType
from goldendb.handle.common.goldendb_common import cp_active_folder, report_job_details, write_progress_file, \
    get_backup_path, get_copy_info_param, extract_ip, mount_bind_backup_path, \
    umount_bind_path_list, mount_bind_path, umount_bind_path, write_progress_file_ex, \
    verify_path_trustlist, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, check_repository_path
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo, get_env_variable
from goldendb.handle.restore.restore_common import GoldenDBRestoreCommon
from openGauss.common.common import safe_get_dir_size

CmdParameters = collections.namedtuple('CmdParameters',
                                       ['manager_name', 'bc_id', 'rc_id', 'copy_path', 'file_path', 'os_user',
                                        'progress_file'])


def handle_ex_with_return_bool_decorator(func):
    @functools.wraps(func)
    def inner(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except Exception as ex:
            log.exception("Execute task failed.")
            return False

    return inner


class Restore:
    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        Env.USER_NAME = 'job_targetObject_auth_authKey'
        Env.PASS_WORD = 'job_targetObject_auth_authPwd'
        self._std_in = data
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param_object = json_param
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        if not verify_path_trustlist(self._cache_area):
            log.error(f"Invalid path: {self._cache_area}, {self.get_log_comm()}")
            raise Exception(f'job id: {job_id}, invalid path: {self._cache_area}.')
        self._meta_path = JsonParam.get_meta_path(json_param)
        if not verify_path_trustlist(self._meta_path):
            log.error(f"Invalid path: {self._meta_path}, {self.get_log_comm()}")
            raise Exception(f'job id: {job_id}, invalid path: {self._meta_path}.')
        self._data_path = JsonParam.get_data_path(json_param)
        if not verify_path_trustlist(self._data_path):
            log.error(f"Invalid path: {self._data_path}, {self.get_log_comm()}")
            raise Exception(f'job id: {job_id}, invalid path: {self._data_path}.')
        self._restore_extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self._restore_time_stamp = self._restore_extend_info.get("restoreTimestamp", "")
        self._restore_target_location = self._restore_extend_info.get("targetLocation", "")
        if self._restore_time_stamp:
            self._log_path = JsonParam.get_log_path(json_param)
            if not verify_path_trustlist(self._log_path):
                log.error(f"Invalid path: {self._log_path}, {self.get_log_comm()}")
                raise Exception(f'job id: {job_id}, invalid path: {self._log_path}.')

        self._job_status = SubJobStatusEnum.RUNNING
        self._resource_common = GoldenDBRestoreCommon(self._json_param_object)

    @staticmethod
    def stop_golden_service(os_user, node_type):
        """
        停止golden db某个节点的某个用户下的服务
        @param os_user: 用户
        @param node_type: 节点类型
        @return: True 停止成功  False 停止失败
        """
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return False
        cmd = f"su - {os_user} -c 'dbmoni -stop"
        return_code, out_info, err_info = execute_cmd(cmd)
        # 执行命令停止服务失败
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute stop {node_type} service cmd failed!")
            return False
        err_service_list = GoldenDBResourceInfo.check_service_status(os_user, node_type)
        # 如果异常服务列表为空，则表示停止服务失败
        if not err_service_list:
            return False
        return True

    @staticmethod
    def start_golden_service(os_user, node_type):
        """
        停止golden db某个节点的某个用户下的服务
        @param os_user:用户
        @param node_type:节点类型
        @return:True 启动成功  False 启动失败
        """
        if check_command_injection_exclude_quote(os_user) or not check_os_user(os_user):
            log.error("command injection in os user! The os user(%s) is invalid.", os_user)
            return False
        cmd = f"su - {os_user} -c 'dbmoni -start"
        return_code, out_info, err_info = execute_cmd(cmd)
        # 执行命令启动服务失败
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute start {node_type} service cmd failed!")
            return False
        err_service_list = GoldenDBResourceInfo.check_service_status(os_user, node_type)
        # 如果存在异常服务列表，则表示启动服务失败
        if err_service_list:
            return False
        return True

    @staticmethod
    def check_db_proxy_node_statue(cluster_id, manager_user):
        """
        检查 CN节点的状态
        @return: False 失败/运行 True CN节点关闭
        """
        if check_command_injection_exclude_quote(manager_user) or not check_os_user(manager_user):
            log.error("command injection in manager_user! The manager_user(%s) is invalid.", manager_user)
            return False
        if check_command_injection_exclude_quote(cluster_id):
            log.error("command injection detected in cluster_id")
            return False
        cmd = f"su - {manager_user} -c 'dbtool -pm -showtrace {cluster_id}'"
        return_code, out_info, err_info = execute_cmd(cmd)
        if return_code != CMDResult.SUCCESS:
            log.info("The db proxy is closed.")
            return True
        return False

    @staticmethod
    def calculate_progress(progress_info, status, progress):
        for info in progress_info:
            if "%" in info:
                progress_a = int(re.findall("\d+\/\d+", info)[0].split('/')[0])
                progress_b = int(re.findall("\d+\/\d+", info)[0].split('/')[1])
                try:
                    progress = int(progress_a * 100 / progress_b)
                except ZeroDivisionError as err:
                    log.exception(f"Handling run-time error:', err: {err}")
                    return SubJobStatusEnum.FAILED, 0
                return status, progress
        return status, progress

    @staticmethod
    def rst_cmd_injection(cmd_parameters):
        if check_command_injection_exclude_quote(cmd_parameters.manager_name):
            log.error("command injection detected in manager_name!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.bc_id):
            log.error("command injection detected in bc_id!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.rc_id):
            log.error("command injection detected in rc_id!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.copy_path):
            log.error("command injection detected in copy_path!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.file_path):
            log.error("command injection detected in file_path!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.os_user):
            log.error("command injection detected in os_user!")
            return True
        if check_command_injection_exclude_quote(cmd_parameters.progress_file):
            log.error("command injection detected in progress_file!")
            return True
        return False

    @staticmethod
    def get_repository_path(copy, repository_type):
        repositories = copy.get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type and repository.__contains__('path'):
                path_list = repository.get("path", [])
                repositories_path = check_repository_path(path_list)
                break
        return repositories_path

    @staticmethod
    def get_log_copy_id_list(log_path_parent_dir, restore_copy_id):
        restore_copy_id_file = restore_copy_id + ".meta"
        dot_meta_path = os.path.join(log_path_parent_dir, restore_copy_id_file)
        id_list = []
        if not os.path.exists(dot_meta_path) or not dot_meta_path:
            return id_list
        with open(dot_meta_path, 'r', encoding='utf-8') as file_read:
            for line in file_read.readlines():
                key_value = line.strip('\n').split(";")
                key = key_value[0].strip()
                id_list.append(key)
        return id_list

    def allow_restore_in_local_node(self):
        """
        判断当前节点是否能执行任务
        @return:
        """
        log.info(f"start allow_restore_in_local_node, job id is {self._job_id}")
        return

    def restore_prerequisite(self):
        """
        执行前置任务：检查集群和原集群的结构
        @return:
        """
        # 检查集群和原集群的结构
        umount_bind_path_list([MountBindPath.DATA_FILE_PATH, MountBindPath.META_FILE_PATH])
        error_code = GoldenDBCode.SUCCESS.value
        if not self._resource_common.check_restore_golden_db_structure():
            log.error("Check golden db cluster structure failed.")
            return False, GoldenDBCode.FAILED.value
        # 如果结构不一致，不会创建进度文件
        result_file = os.path.join(self._cache_area, "BackupPrerequisiteProgress")
        if not verify_path_trustlist(result_file):
            log.error(f"Invalid src path: {result_file}.")
            return False, GoldenDBCode.FAILED.value
        pathlib.Path(result_file).touch()

        return True, error_code

    def restore_prerequisite_progress(self):
        """
        执行前置任务：查询结果文件如果存在即前置任务成功
        @return:
        """
        # 当前根据是否存在BackupPrerequisiteProgress文件来上报前置任务进度
        job_status = SubJobStatusEnum.COMPLETED.value
        file_path = os.path.join(self._cache_area, "BackupPrerequisiteProgress")
        if not verify_path_trustlist(file_path):
            raise Exception(f"Invalid src path: {file_path}.")
        if not os.path.exists(file_path):
            job_status = SubJobStatusEnum.FAILED.value
            target_group_num = len(
                self._resource_common.get_cluster_info(self._resource_common.get_protect_object()).get(
                    GoldenDBJsonConst.GROUP, []))
            protect_group_num = len(
                self._resource_common.get_cluster_info(self._resource_common.get_target_object()).get(
                    GoldenDBJsonConst.GROUP, []))
            message = "The sharding num is "
            log_detail_param = [message + str(target_group_num), message + str(protect_group_num)]

            self._resource_common.get_cluster_info(self._resource_common.get_target_object())
            self.set_logdetail_with_params("", self._sub_job_id, ErrorCode.ERROR_GOLDENDB_STRUCTURE, log_detail_param,
                                           LogLevel.ERROR.value)
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=job_status, progress=100, logDetail=self._logdetail)
        output_result_file(self._pid, output.dict(by_alias=True))

    def get_progress(self):
        """
        解析恢复进度
        :return:
        """
        log.info('Query restore progress!')
        status = SubJobStatusEnum.RUNNING
        progress = 0
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f'{progress_file}')
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return SubJobStatusEnum.FAILED, progress
        if not os.path.exists(progress_file):
            status = SubJobStatusEnum.FAILED
            log.error(f"Progress file: {progress_file} not exist")
            return status, progress
        log.info(f'Path exist')
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
        if ProgressInfo.FAILED in data:
            status = SubJobStatusEnum.FAILED
            progress = 100
            # 可设置错误码
            return status, progress
        if ProgressInfo.SUCCEED in data:
            status = SubJobStatusEnum.COMPLETED
            progress = 100
        elif "%" not in data:
            progress = 0
        else:
            progress_info = data.split("\n")
            progress_info.reverse()
            status, progress = Restore.calculate_progress(progress_info, status, progress)
        log.info(f'Progress is {progress}')
        return status, progress

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def set_logdetail(self, err_code):
        err_dict = LogDetail(logInfo='', logInfoParam=[], logTimestamp=0, logDetail=0, logDetailParam=[],
                             logDetailInfo=[], logLevel=LogLevel.ERROR.value)
        err_dict.log_detail = err_code
        self._logdetail = []
        self._logdetail.append(err_dict)
        return True

    def set_logdetail_with_params(self, log_label, sub_job_id, err_code=None, log_detail_param=None,
                                  log_level=LogLevel.INFO.value):
        err_dict = LogDetail(logInfo=log_label,
                             logInfoParam=[sub_job_id],
                             logTimestamp=int(time.time()),
                             logDetail=err_code,
                             logDetailParam=log_detail_param,
                             logLevel=log_level)
        self._logdetail = []
        self._logdetail.append(err_dict)
        return True

    def upload_restore_progress(self):
        # 定时上报恢复进度
        while self._job_status == SubJobStatusEnum.RUNNING:
            log.info("Start to report progress.")
            task_status, progress = self.get_progress()
            progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.RUNNING,
                                          progress=0, logDetail=self._logdetail)
            if task_status == SubJobStatusEnum.FAILED:
                self.set_logdetail_with_params("plugin_restore_subjob_fail_label", self._sub_job_id,
                                               0, [], LogLevel.ERROR.value)
            progress_dict.task_status = task_status
            progress_dict.progress = progress
            self._job_status = task_status
            report_job_details(self._job_id, progress_dict.dict(by_alias=True))
            time.sleep(self._query_progress_interval)

    def restore_task(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return False
        write_progress_file_ex(ProgressInfo.START, progress_file)
        sub_job_dict = {
            GoldenSubJobName.SUB_VER_CHECK: self.sub_ver_check,
            GoldenSubJobName.SUB_CHECK: self.sub_job_check,
            GoldenSubJobName.SUB_EXEC: self.sub_job_exec,
            GoldenSubJobName.SUB_BINLOG_MERGE: self.sub_binlog_merge,
            GoldenSubJobName.SUB_BINLOG_MOUNT: self.sub_binlog_mount,
        }
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()
        # 开始执行子任务
        sub_job_name = GoldenDBRestoreCommon.get_sub_job_name(self._json_param_object)
        if not sub_job_name:
            return False

        if sub_job_name in [GoldenSubJobName.SUB_BINLOG_MOUNT, GoldenSubJobName.SUB_CHECK]:
            log_info = ReportDBLabel.RESTORE_SUB_START_PREPARE
        else:
            log_info = ReportDBLabel.RESTORE_SUB_START_COPY

        log_detail = LogDetail(logInfo=log_info, logInfoParam=[self._sub_job_id], logLevel=LogLevel.INFO.value)
        report_job_details(self._pid,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                               by_alias=True))

        # 执行子任务
        log.info(f"Exec sub job {sub_job_name} begin.{self.get_log_comm()}.")
        if not sub_job_dict.get(sub_job_name)():
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail = self.report_error_result(progress_file, progress_thread, sub_job_name)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            progress_thread.join()
            os.remove(progress_file)
            return False

        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[self._sub_job_id], logLevel=1)

        data_size = 0
        # 数据量所有子任务只上报一次，因为UBC会叠加每个子任务上报的值
        if sub_job_name == GoldenSubJobName.SUB_EXEC:
            data_size = self.get_data_size()

        report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                    logDetail=[log_detail], dataSize=data_size,
                                                    taskStatus=SubJobStatusEnum.COMPLETED.value).dict(by_alias=True))
        log.info(f"Exec sub job {sub_job_name} success.{self.get_log_comm()}.")
        progress_thread.join()
        os.remove(progress_file)
        return True

    def report_error_result(self, progress_file, progress_thread, sub_job_name):
        write_progress_file(ProgressInfo.FAILED, progress_file)
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
        if ProgressInfo.FAILED in data and "The response message:" in data:
            message = data.split("The response message:")[1].replace(ProgressInfo.FAILED, "").replace('\n',
                                                                                                      ' ').strip()

            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=LogLevel.ERROR.value, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                   logDetailParam=["Restore", message])
        elif ProgressInfo.FAILED in data and 'Version Check Failed' in data:
            message = 'Version Check Failed'
            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=LogLevel.ERROR.value, logDetail=ErrorCode.ERR_NEW_LOC_RST_VER_CONFLICT,
                                   logDetailParam=["Restore", message])
        else:
            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=LogLevel.ERROR.value, logDetail=0)
        return log_detail

    def prepare_restore_cmd(self, progress_file):
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return ''
        bc_id = param_dict.get("cluster_id", "")
        task_id = param_dict.get("task_id", "")
        result_info_name = param_dict.get("resultinfo_name", "")
        file_content = self._json_param_object
        rc_id, manager_name, _ = file_content.get(GoldenDBJsonConst.SUBJOB) \
            .get(GoldenDBJsonConst.JOBINFO, "").split(" ")
        copy_path = MountBindPath.DATA_FILE_PATH
        file_path = os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}', "DATA_BACKUP",
                                 task_id, "ResultInfo", result_info_name)
        os_user = get_env_variable(f"{Env.USER_NAME}_{self._pid}")
        cmd_parameters = CmdParameters(manager_name, bc_id, rc_id, copy_path, file_path, os_user, progress_file)
        if not bc_id.isnumeric():
            log.error(f"Invalid bc_id: {bc_id}.")
            return ''
        if not rc_id.isnumeric():
            log.error(f"Invalid bc_id: {rc_id}.")
            return ''
        if Restore.rst_cmd_injection(cmd_parameters):
            log.error("command injection detected in restore command!")
            return ''
        if not check_path_owner(file_path, [manager_name]):
            log.warn("user authentication failed!")
            use_infos = pwd.getpwnam(manager_name)
            user_uid = use_infos.pw_uid
            user_groupid = use_infos.pw_gid
            change_dir_owner_recursively(
                os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}', "DATA_BACKUP"), user_uid, user_groupid)

        cmd = f'su - {manager_name} -c "dbtool -mds -restore -bc={bc_id} -rc={rc_id} -d={copy_path} ' \
              f'-f={file_path} -bakstart -user={os_user} -password" >> {progress_file}'
        return cmd

    @handle_ex_with_return_bool_decorator
    def sub_job_exec(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return False
        if self._restore_time_stamp:
            restore_cmd = self.prepare_binlog_restore_cmd(progress_file)
            log.info(f"prepare_binlog_restore_cmd: {restore_cmd}.")
        else:
            restore_cmd = self.prepare_restore_cmd(progress_file)

        if not restore_cmd:
            log.error(f'Command error, will not be executed, taskID: {self._job_id}')
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        try:
            child = pexpect.spawn("/bin/bash", args=["-c", restore_cmd], timeout=None)
        except Exception as ex:
            log.exception(f"Spawn except an exception {ex}.")
            return False
        try:
            ret_code = child.expect(["", pexpect.TIMEOUT, pexpect.EOF])
            if str(ret_code) != CMDResult.SUCCESS:
                log.error(f"Exec cmd failed.")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                return False
            os_pass = get_env_variable(f"{Env.PASS_WORD}_{self._pid}")
            child.sendline(os_pass)
            clear(os_pass)
            out_str = child.read()
            # 执行完要先关闭，否则child.exitstatus的结果为None
            child.close()
            if str(child.exitstatus) != CMDResult.SUCCESS:
                return_code, out_info, err_info = execute_cmd(f"cat {progress_file}")
                log.error(f"Fail to exec restore, err: {out_info}")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                return False
        finally:
            if child:
                child.close()
        try:
            process = subprocess.run(["/bin/cat", progress_file], timeout=5, shell=False, stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE, encoding="utf-8")
        except subprocess.TimeoutExpired as err:
            raise Exception("Timeout") from err
        data = process.stdout
        if str(process.returncode) != CMDResult.SUCCESS:
            log.error(f"Fail to get progress file, err: {process.stderr} data: {data}")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        log.info("Succeed to exec restore")
        return True

    def sub_ver_check(self):
        log.info(f"Start to check version. {self.get_log_comm()}.")
        # 新位置恢复检查集群版本，高版本不能恢复到老版本
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        _, user_name, _ = self._json_param_object.get(GoldenDBJsonConst.SUBJOB).get(GoldenDBJsonConst.JOBINFO,
                                                                                    "").split(" ")
        if self._restore_target_location == "new":
            copy_version = self._resource_common.get_copy_version()
            log.info(f"Check version for new location restore, copy version {copy_version}. {self.get_log_comm()}.")
            version = GoldenDBResourceInfo.get_cluster_version(user_name)
            if copy_version > version:
                log.error(
                    f"Goldendb cannot restore from higher {copy_version} to lower {version}. {self.get_log_comm()}")
                write_progress_file("Version Check Failed", progress_file)
                return False
            else:
                log.info(f"Version check success for new location restore. {self.get_log_comm()}.")
                return True
        log.info(f"No need to check version for origin location restore. {self.get_log_comm()}.")
        return True

    def sub_job_check(self):
        if not verify_path_trustlist(self._data_path):
            log.error(f"Invalid src path: {self._data_path}.")
            return False
        if not verify_path_trustlist(self._meta_path):
            log.error(f"Invalid src path: {self._meta_path}.")
            return False
        mount_bind_backup_path(self._data_path, self._meta_path, self._job_id)
        _, user_name, node_type = self._json_param_object.get(GoldenDBJsonConst.SUBJOB) \
            .get(GoldenDBJsonConst.JOBINFO, "").split(" ")
        root_path = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return False
        cluster_id = param_dict.get("cluster_id", "")
        data_path = os.path.join(self._data_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        # 修改用户属组和文件权限
        group_name, _ = get_user_info(user_name)
        mkdir_chmod_chown_dir_recursively(data_path, 0o770, user_name, group_name)

        prod_data_backup = os.path.join(root_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        # 解挂载备份根data_backup目录
        umount_bind_path(prod_data_backup)
        # 在备份根目录下，新建挂载路径
        if not mkdir_chmod_chown_dir_recursively(prod_data_backup, 0o770, user_name, group_name, True):
            log.error("fail to make data backup path")
            return False
        if not mount_bind_path(data_path, prod_data_backup, self._job_id):
            log.error("Mount bind data path failed")
            return False
        # 复制活跃事务文件夹至备份根目录
        if not cp_active_folder(self._data_path, root_path, user_name):
            log.error("copy active tx info folder failed.")
            return False
        if not cluster_id or not user_name or not node_type:
            log.error("Sub job check cluster info failed.")
            return False
        # 检查CN计算节点是否关闭
        if node_type in GoldenDBNodeType.ZX_MANAGER_NODE and \
                not Restore.check_db_proxy_node_statue(cluster_id, user_name):
            log.info("Sub job check CN node is not close.")
        # 检查数据节点是否关闭服务
        if node_type in GoldenDBNodeType.DATA_NODE and GoldenDBResourceInfo.check_node_type(user_name, node_type):
            log.info("Sub job check data node is not close.")
        return True

    def do_post(self):
        if self._restore_time_stamp:
            log.info("remove backup_root_tmp")
            log_backup_root = os.path.join(self._log_path, 'backup_root_tmp')
            delete_path(log_backup_root)
            log.info("remove backup_root_tmp done")

        umount_bind_backup_paths(self._job_id)
        tmp_path = os.path.join('/mnt', 'goldendb_' + self._job_id)
        if os.path.exists(tmp_path):
            delete_path(tmp_path)
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return False
        if os.path.exists(progress_file):
            os.remove(progress_file)
            return True
        else:
            log.error(f'Progress file {progress_file} not exist! failed to delete while doing post job!')
            return False

    def build_sub_job(self, job_priority, job_name, node):
        cluster_id = self._resource_common.get_target_cluster_id()
        user_name = node.get(GoldenDBJsonConst.OSUSER, "")
        golden_db_node_type = node.get(GoldenDBJsonConst.NODETYPE, "")
        node_id = node.get(GoldenDBJsonConst.PARENTUUID, "")
        job_info = f"{cluster_id} {user_name} {golden_db_node_type}"
        return SubJobModel(jobId=self._job_id, jobType=SubJobType.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=SubJobPolicy.FIXED_NODE.value,
                           jobInfo=job_info, ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        if self._restore_time_stamp:
            return self.gen_sub_job_binlog()
        else:
            return self.gen_sub_job_data()

    def gen_sub_job_data(self):
        manager_node = self._resource_common.get_manager_node_info()
        manager_node_list = manager_node.get(GoldenDBJsonConst.NODES, [])
        gtm_node_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GTM, [])
        group_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GROUP, [])
        data_node_list = []
        for group in group_list:
            for node in group.get(GoldenDBJsonConst.MYSQLNODES, []):
                data_node_list.append(node)

        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        if not verify_path_trustlist(file_path):
            log.error(f"Invalid src path: {file_path}.")
            return False
        sub_job_array = []

        # 子任务1：新位置恢复检查集群版本
        sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1, GoldenSubJobName.SUB_VER_CHECK,
                                     manager_node_list[0])
        sub_job_array.append(sub_job)

        # 子任务2：检查+挂载
        # 检查manager节点data节点，挂载目录
        for node in manager_node_list + data_node_list + gtm_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, GoldenSubJobName.SUB_CHECK, node)
            sub_job_array.append(sub_job)

        # 子任务3：执行恢复，管理节点执行恢复
        for node in manager_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, GoldenSubJobName.SUB_EXEC, node)
            sub_job_array.append(sub_job)

        log.info(f"Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        exec_overwrite_file(file_path, sub_job_array)
        return True

    def gen_sub_job_binlog(self):
        manager_node = self._resource_common.get_manager_node_info()
        manager_node_list = manager_node.get(GoldenDBJsonConst.NODES, [])
        gtm_node_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GTM, [])
        group_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GROUP, [])
        data_node_list = []
        for group in group_list:
            for node in group.get(GoldenDBJsonConst.MYSQLNODES, []):
                data_node_list.append(node)

        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        if not verify_path_trustlist(file_path):
            log.error(f"Invalid src path: {file_path}.")
            return False
        sub_job_array = self.gen_sub_job_array(manager_node_list, data_node_list, gtm_node_list)
        log.info(f"Binlog Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        exec_overwrite_file(file_path, sub_job_array)
        return True

    def gen_sub_job_array(self, manager_node_list, data_node_list, gtm_node_list):
        sub_job_array = []
        # 子任务1：新位置恢复检查集群版本
        sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1, GoldenSubJobName.SUB_VER_CHECK,
                                     manager_node_list[0])
        sub_job_array.append(sub_job)

        # 子任务2：管理节点组合多个日志副本数据
        for node in manager_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, GoldenSubJobName.SUB_BINLOG_MERGE, node)
            sub_job_array.append(sub_job)

        # 子任务3：所有节点执行挂载子任务
        for node in manager_node_list + data_node_list + gtm_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, GoldenSubJobName.SUB_BINLOG_MOUNT, node)
            sub_job_array.append(sub_job)

        # 子任务4：管理节点执行恢复命令
        for node in manager_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_4, GoldenSubJobName.SUB_EXEC, node)
            sub_job_array.append(sub_job)
        return sub_job_array

    def sub_binlog_merge(self):
        id_list = self.get_log_uri()
        param_dict = get_copy_info_param(self._meta_path)
        bc_id = param_dict.get('cluster_id', '')
        # 在log仓创建一个临时目录backup_root_tmp
        log_backup_root_tmp = os.path.join(self._log_path, 'backup_root_tmp')
        log_dbcluster = os.path.join(self._log_path, 'backup_root_tmp', f'DBCluster_{bc_id}')
        log.info(f"sub_binlog_merge log_dbcluster {log_dbcluster}")

        if not os.path.exists(log_dbcluster):
            exec_mkdir_cmd(log_dbcluster)

        # 把log仓多个日志副本的Active_TX_Info、LOGICAL_BACKUP拷贝到backup_root_tmp下
        for log_copy_id in id_list:
            active_tx_info = os.path.join(self._log_path, log_copy_id, 'Active_TX_Info')
            if not exec_cp_dir_no_user(active_tx_info, log_backup_root_tmp,
                                       is_check_white_list=False, is_overwrite=True):
                log.error("copy active_tx_info failed")
                return False
            logical_backup = os.path.join(self._log_path, log_copy_id, f'DBCluster_{bc_id}', 'LOGICAL_BACKUP')
            if not exec_cp_dir_no_user(logical_backup, log_dbcluster, is_check_white_list=False, is_overwrite=True):
                log.error("copy logical_backup failed")
                return False
        # 拷贝data仓的backup_result_info到backup_root_tmp
        backup_path = os.path.join(self._data_path, f'DBCluster_{bc_id}')
        filenames = glob.glob(os.path.join(backup_path, f"*backup_resultsinfo*"))
        for filename in filenames:
            if os.path.isfile(filename):
                if not exec_cp_dir_no_user(filename, log_dbcluster, is_check_white_list=False, is_overwrite=True):
                    log.error("copy backup_result_info failed")
                    return False
        # 拷贝data仓/DBCluster_id/DATA_BACKUP到backup_root_tmp/DBCluster_id/DATA_BACKUP
        data_backup_path = os.path.join(self._data_path, f'DBCluster_{bc_id}', 'DATA_BACKUP')
        if not exec_cp_dir_no_user(data_backup_path, log_dbcluster, is_check_white_list=False, is_overwrite=True):
            log.error("copy data_backup failed")
            return False

        _, user_name, _ = self._json_param_object.get(GoldenDBJsonConst.SUBJOB) \
            .get(GoldenDBJsonConst.JOBINFO, "").split(" ")
        group_name, _ = get_user_info(user_name)
        return mkdir_chmod_chown_dir_recursively(log_backup_root_tmp, 0o770, user_name, group_name)

    def sub_binlog_mount(self):
        # 挂载manager、gtm、dn节点的backup_root目录到log仓的backup_root_tmp目录
        local_log_path = os.path.join('/mnt', 'goldendb_' + self._job_id, 'log')
        log.info(f"sub_binlog_mount local_log_path, {local_log_path}")

        cluster_id, user_name, node_type = self._json_param_object.get(GoldenDBJsonConst.SUBJOB) \
            .get(GoldenDBJsonConst.JOBINFO, "").split(" ")
        group_name, _ = get_user_info(user_name)

        # 1:将副本挂载到/mnt/goldendb_{job_id)/log 共享目录
        mkdir_chmod_chown_dir_recursively(local_log_path, 0o770, user_name, group_name, True)
        log_backup_root_tmp = os.path.join(self._log_path, 'backup_root_tmp')
        if not mount_bind_path(log_backup_root_tmp, local_log_path, self._job_id):
            log.error("mount local_log_path failed")
            return False

        # 2: 将副本的DBCluster_{id}目录 挂载到 对应角色的backup_root/DBCluster_{id}
        root_path = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        log.info(f"sub_binlog_mount root_path, {root_path}")

        target_path = os.path.join(root_path, f"DBCluster_{cluster_id}")
        param_dict = get_copy_info_param(self._meta_path)
        bc_id = param_dict.get('cluster_id', '')
        source_path = os.path.join(log_backup_root_tmp, f"DBCluster_{bc_id}")
        if not mount_bind_path(source_path, target_path, self._job_id):
            log.error("mount root_path failed")
            return False

        # 3：拷贝副本的事务信息Active_TX_Info目录 到管理节点的backup_root/Active_TX_Info目录
        if node_type == GoldenDBNodeType.ZX_MANAGER_NODE:
            log.info("copy Active_TX_Info to manager")
            from_path = os.path.join(log_backup_root_tmp, "Active_TX_Info")
            return exec_cp_dir_no_user(from_path, root_path, is_check_white_list=False, is_overwrite=True)

        return True

    def prepare_binlog_restore_cmd(self, progress_file):
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return ''
        bc_id = param_dict.get("cluster_id", "")
        task_id = param_dict.get("task_id", "")
        result_info_name = param_dict.get("resultinfo_name", "")
        file_content = self._json_param_object
        rc_id, manager_name, _ = file_content.get(GoldenDBJsonConst.SUBJOB) \
            .get(GoldenDBJsonConst.JOBINFO, "").split(" ")
        log_backup_root = os.path.join('/mnt', 'goldendb_' + self._job_id, 'log')
        file_path = os.path.join(log_backup_root, f'DBCluster_{bc_id}', "DATA_BACKUP",
                                 task_id, "ResultInfo", result_info_name)
        os_user = get_env_variable(f"{Env.USER_NAME}_{self._pid}")
        cmd_parameters = CmdParameters(manager_name, bc_id, rc_id, log_backup_root, file_path, os_user, progress_file)
        if not bc_id.isnumeric():
            log.error(f"Invalid bc_id: {bc_id}.")
            return ''
        if not rc_id.isnumeric():
            log.error(f"Invalid bc_id: {rc_id}.")
            return ''
        if Restore.rst_cmd_injection(cmd_parameters):
            log.error("command injection detected in restore command!")
            return ''
        if not check_path_owner(file_path, [manager_name]):
            log.warn("user authentication failed!")
            use_infos = pwd.getpwnam(manager_name)
            user_uid = use_infos.pw_uid
            user_groupid = use_infos.pw_gid
            change_dir_owner_recursively(log_backup_root, user_uid, user_groupid)
        time_stamp = time.localtime(int(self._restore_time_stamp))
        restore_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        cmd = f'su - {manager_name} -c "dbtool -mds -restore -bc={bc_id} -rc={rc_id} ' \
              f"-t='{restore_time}' -d={log_backup_root} " \
              f'-f={file_path} -no-check-binlog-time -user={os_user} -password" >> {progress_file}'
        return cmd

    def get_log_uri(self):
        copies = self._json_param_object.get("job", {}).get("copies", [])
        restore_type = copies[len(copies) - 1]['type']
        id_list = []
        if restore_type == 'log':
            # 从后往前找到第一个不是日志的副本，然后记下id
            restore_copy_id = None
            for copy in copies[::-1]:
                restore_type = copy['type']
                if restore_type != 'log':
                    restore_copy_id = copy['id']
                    break
            id_list = self.get_log_copy_id_list(self._log_path, restore_copy_id)
            log.info(f'get_log_copy_id_list, id_list is {id_list}')
        return id_list

    def get_data_size(self):
        if self._restore_time_stamp:
            log_backup_root_tmp = os.path.join(self._log_path, 'backup_root_tmp')
            data_size = safe_get_dir_size(log_backup_root_tmp)
        else:
            data_size = safe_get_dir_size(self._data_path)
        return data_size
