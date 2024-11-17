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
from common.common import check_command_injection_exclude_quote, execute_cmd, output_result_file, \
    output_execution_result_ex
from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.const import SubJobStatusEnum, ParamConstant, SubJobPriorityEnum, ReportDBLabel, SubJobTypeEnum, \
    DBLogLevel, SubJobPolicyEnum
from common.file_common import get_user_info, delete_path
from common.util.check_user_utils import check_path_owner, check_os_user
from common.util.common_utils import change_dir_owner_recursively
from common.util.exec_utils import exec_overwrite_file, exec_mkdir_cmd, exec_cp_dir_no_user
from common.util.scanner_utils import scan_dir_size
from goldendb.handle.common.const import GoldenDBCode, CMDResult, GoldenDBJsonConst, Env, GoldenDBNodeType, ErrorCode, \
    MountBindPath
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.common.const import ProgressInfo, GoldenSubJobName, ManagerPriority
from goldendb.handle.common.goldendb_common import cp_active_folder, report_job_details_file_ex, write_progress_file, \
    get_backup_path, get_copy_info_param, extract_ip, mount_bind_backup_path, get_agent_uuids, umount_bind_path_list, \
    report_err_via_output_file, mount_bind_path, umount_bind_path, write_progress_file_ex, get_bkp_root_dir_via_role, \
    verify_path_trustlist, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, check_task_on_all_managers, \
    exec_task_on_all_managers, get_recognized_err_from_sts_file, report_err_via_rpc, update_agent_sts, \
    update_agent_sts_general_after_exec, exec_chmod_dir_recursively
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo, get_env_variable
from goldendb.handle.restore.restore_common import GoldenDBRestoreCommon
from goldendb.schemas.glodendb_schemas import TaskInfo, StatusInfo

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
        self._log_comm = f"pid: {self._pid}, job_id: {self._job_id}, sub job id: {self._sub_job_id}"
        self._json_param_object = json_param
        self._task_infos = TaskInfo(pid=self._pid, jobId=self._job_id, subJobId=self._sub_job_id, taskType="Restore",
                                    logComm=self._log_comm, jsonParam=self._json_param_object)
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        self._meta_path = JsonParam.get_meta_path(json_param)
        self._data_path = JsonParam.get_data_path(json_param)
        if JsonParam.has_repository_path(json_param):
            if not verify_path_trustlist(self._cache_area):
                log.error(f"Invalid path: {self._cache_area}, {self._log_comm}.")
                raise Exception(f"Invalid path: {self._cache_area}, {self._log_comm}.")
            if not verify_path_trustlist(self._meta_path):
                log.error(f"Invalid path: {self._meta_path}, {self._log_comm}.")
                raise Exception(f'Invalid path: {self._meta_path}, {self._log_comm}.')
            if not verify_path_trustlist(self._data_path):
                log.error(f"Invalid path: {self._data_path}, {self._log_comm}.")
                raise Exception(f"Invalid path: {self._data_path}, {self._log_comm}.")
        self._restore_extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self._restore_time_stamp = self._restore_extend_info.get("restoreTimestamp", "")
        self._restore_target_location = self._restore_extend_info.get("targetLocation", "")
        if self._restore_time_stamp:
            self._log_path = JsonParam.get_log_path(json_param)
            if JsonParam.has_repository_path(json_param):
                if not verify_path_trustlist(self._log_path):
                    log.error(f"Invalid path: {self._log_path}, {self._log_comm}.")
                    raise Exception(f"Invalid path: {self._log_path}, {self._log_comm}.")

        self._job_status = SubJobStatusEnum.RUNNING.value
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
                    return SubJobStatusEnum.FAILED.value, 0
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
        功能描述：是否允许本地运行, 支持可靠性, 异常时主动上报
        @return:
        """
        log.info(f"Step 1: start allow_restore_in_local_node, {self._log_comm}.")
        agent_uuids = get_agent_uuids(self._json_param_object)
        manager_node = self._resource_common.get_manager_node_info()
        manager_node_list = manager_node.get(GoldenDBJsonConst.NODES, [])
        # 检查管理节点，有一个可用即可
        no_manager = all([node.get(GoldenDBJsonConst.PARENTUUID) not in agent_uuids for node in manager_node_list])
        if no_manager:
            report_err_via_output_file(self._pid, self._job_id, self._sub_job_id, "Step 1: manager nodes all offline")
            return False
        gtm_node_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GTM, [])
        # 检查gtm节点，有一个异常就报错
        no_gtm = any([node.get(GoldenDBJsonConst.PARENTUUID) not in agent_uuids for node in gtm_node_list])
        if no_gtm:
            report_err_via_output_file(self._pid, self._job_id, self._sub_job_id, "Step 1: gtm nodes abnormal")
            return False
        group_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GROUP, [])
        data_node_list = []
        for group in group_list:
            for node in group.get(GoldenDBJsonConst.MYSQLNODES, []):
                data_node_list.append(node)
        # 检查data节点，有一个异常就报错
        err_dn = any([node.get(GoldenDBJsonConst.PARENTUUID) not in agent_uuids for node in data_node_list])
        if err_dn:
            report_err_via_output_file(self._pid, self._job_id, self._sub_job_id, "Step 1: data nodes abnormal")
            return False
        log.info(f"Step 1: allow_restore_in_local_node success, {self._log_comm}.")
        return True

    def restore_prerequisite(self):
        """
        执行前置任务：检查集群和原集群的结构
        @return:
        """
        # 检查集群和原集群的结构
        umount_bind_path_list([MountBindPath.DATA_FILE_PATH, MountBindPath.META_FILE_PATH])
        error_code = GoldenDBCode.SUCCESS.value
        if not self._resource_common.check_restore_golden_db_structure():
            log.error(f"Step 2: check golden db cluster structure failed, {self._log_comm}.")
            return False, GoldenDBCode.FAILED.value
        # 如果结构不一致，不会创建进度文件
        result_file = os.path.join(self._cache_area, "BackupPrerequisiteProgress")
        if not verify_path_trustlist(result_file):
            log.error(f"Step 2: invalid src path: {result_file}, {self._log_comm}.")
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
            raise Exception(f"Invalid src path: {file_path}, {self._log_comm}.")
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
                                           DBLogLevel.ERROR.value)
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=job_status, progress=100, logDetail=self._logdetail)
        output_result_file(self._pid, output.dict(by_alias=True))

    def get_progress(self):
        """
        解析恢复进度
        :return:
        """
        log.info('Query restore progress!')
        status = SubJobStatusEnum.RUNNING.value
        progress = 0
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f'{progress_file}')
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return SubJobStatusEnum.FAILED.value, progress
        if not os.path.exists(progress_file):
            status = SubJobStatusEnum.FAILED.value
            log.error(f"Progress file: {progress_file} not exist")
            return status, progress
        log.info(f'Path exist')
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
        if ProgressInfo.FAILED in data:
            status = SubJobStatusEnum.FAILED.value
            progress = 100
            # 可设置错误码
            return status, progress
        if ProgressInfo.SUCCEED in data:
            status = SubJobStatusEnum.COMPLETED.value
            progress = 100
        elif "%" not in data:
            progress = 0
        else:
            progress_info = data.split("\n")
            progress_info.reverse()
            status, progress = Restore.calculate_progress(progress_info, status, progress)
        log.info(f'Progress is {progress}')
        return status, progress

    def set_logdetail_with_params(self, log_label, sub_job_id, err_code=None, log_detail_param=None,
                                  log_level=DBLogLevel.INFO.value):
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
        sub_job_name = GoldenDBRestoreCommon.get_sub_job_name(self._json_param_object)
        # 定时上报数据恢复子任务以外的子任务进度
        if sub_job_name != GoldenSubJobName.SUB_EXEC:
            while self._job_status == SubJobStatusEnum.RUNNING.value:
                log.info("Start to report progress.")
                task_status, progress = self.get_progress()
                progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                              taskStatus=SubJobStatusEnum.RUNNING.value,
                                              progress=0, logDetail=self._logdetail)
                if task_status == SubJobStatusEnum.FAILED.value:
                    self.set_logdetail_with_params(ReportDBLabel.RESTORE_SUB_FAILED, self._sub_job_id,
                                                   0, [], DBLogLevel.ERROR.value)
                progress_dict.task_status = task_status
                progress_dict.progress = progress
                self._job_status = task_status
                report_job_details_file_ex(self._job_id, progress_dict.dict(by_alias=True))
                time.sleep(self._query_progress_interval)

    def rst_job_dict(self):
        sub_job_dict = {
            GoldenSubJobName.SUB_VER_CHECK: self.sub_ver_check,
            GoldenSubJobName.SUB_CHECK: self.sub_job_check,
            GoldenSubJobName.SUB_EXEC: self.sub_job_exec,
            GoldenSubJobName.SUB_BINLOG_MERGE: self.sub_binlog_merge,
            GoldenSubJobName.SUB_BINLOG_MOUNT: self.sub_binlog_mount,
        }
        return sub_job_dict

    def restore_task(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return False
        # 开始执行子任务
        sub_job_name = GoldenDBRestoreCommon.get_sub_job_name(self._json_param_object)
        if not sub_job_name:
            return False

        sub_job_dict = self.rst_job_dict()
        write_progress_file_ex(ProgressInfo.START, progress_file)
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()

        if sub_job_name in [GoldenSubJobName.SUB_BINLOG_MOUNT, GoldenSubJobName.SUB_CHECK]:
            log_info = ReportDBLabel.RESTORE_SUB_START_PREPARE
        else:
            log_info = ReportDBLabel.RESTORE_SUB_START_COPY

        log_detail = LogDetail(logInfo=log_info, logInfoParam=[self._sub_job_id], logLevel=DBLogLevel.INFO.value)
        report_job_details_file_ex(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                       by_alias=True))

        # 执行子任务
        log.info(f"Exec sub job {sub_job_name} begin. {self._log_comm}.")
        try:
            ret = self.get_rst_sub_job_result_no_report(sub_job_dict, sub_job_name)
        except Exception as ex:
            log.error(f"Exec sub job {sub_job_name} exception, {ex}. {self._log_comm}.")
            ret = False

        if not ret:
            log.error(f"Exec sub job {sub_job_name} failed. {self._log_comm}.")
            # 读取进度文件，通过RPC报告错误，先上报，后关线程
            if sub_job_name == GoldenSubJobName.SUB_EXEC:
                self.report_manager_err(progress_file)
            else:
                self.report_err_via_rpc_with_progress(f"Exec {sub_job_name} failed", progress_file)
            progress_thread.join()
            return False

        log.info(f"Exec sub job {sub_job_name} finished, start to write progress_file, {self._log_comm}.")
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[self._sub_job_id], logLevel=1)

        self.report_completed_with_data_size(log_detail, sub_job_name)
        log.info(f"Exec sub job {sub_job_name} success. {self._log_comm}.")
        # 先上报，后关线程
        progress_thread.join()
        os.remove(progress_file)
        log.info(f"Removed progress_file on {extract_ip()[0]}, {self._log_comm}.")
        return True

    def report_manager_err(self, progress_file):
        log.info(f"Restore failed on all managers, will report err, {self._log_comm}.")
        # 从进度文件中获取错误日志详情，并更新至状态文件中
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        node_id = job_infos[3]
        rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        # 获取更新后的错误日志详情，优先取dbtool原生错误上报
        log_detail, log_detail_param, log_info = get_recognized_err_from_sts_file(node_id, rst_sts_file,
                                                                                  "Restore")
        log_details = [log_detail, log_detail_param, log_info]
        report_err_via_rpc(self._pid, self._job_id, self._sub_job_id, log_details)

    def get_rst_sub_job_result_no_report(self, sub_job_dict, sub_job_name):
        """
        获取子任务结果，无上报告。

        参数:
        progress_file: 进度文件，用于记录任务进度。
        sub_job_dict: 子任务字典，包含子任务名称和对应的执行函数。
        sub_job_name: 子任务名称，用于从子任务字典中获取对应的执行函数。

        返回值:
        ret: 执行结果，如果子任务执行成功则返回True，否则返回False。

        异常描述:
        如果子任务执行失败，会通过RPC报告错误。
        """
        if sub_job_name == GoldenSubJobName.SUB_EXEC:
            sub_job_ret = sub_job_dict.get(sub_job_name)()
            log.info(f"Get sub_exec result, {sub_job_ret}, {self._log_comm}.")
            # 执行子任务，如果执行失败，则在所有管理节点上检查恢复结果
            if not sub_job_ret:
                # 当前管理节点恢复失败，开始检查其他管理节点的恢复结果
                sub_job_ret = self.check_restore_on_all_managers()
        else:
            sub_job_ret = sub_job_dict.get(sub_job_name)()
            log.info(f"Get {sub_job_name} result, {sub_job_ret}, {self._log_comm}.")
            if not sub_job_ret:
                log.error(f"Exec sub job {sub_job_name} failed, start to report error, {self._log_comm}.")
        return sub_job_ret

    def get_err_log_detail_via_process_file(self, progress_file):
        """
        通过处理文件获取错误日志详情

        参数:
        progress_file: 进度文件路径

        返回:
        log_detail: 日志详情对象

        """
        log.info(f"Get log detail from process file. {self._log_comm}")
        data = ""
        if os.path.exists(progress_file):
            write_progress_file(ProgressInfo.FAILED, progress_file)
            with open(progress_file, "r", encoding='UTF-8') as file_stream:
                data = file_stream.read()
        if ProgressInfo.FAILED in data and "The response message:" in data:
            message = data.split("The response message:")[1].replace(ProgressInfo.FAILED, "").replace('\n',
                                                                                                      ' ').strip()

            log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                   logDetailParam=["Restore", message])
        elif ProgressInfo.FAILED in data and 'Version Check Failed' in data:
            log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.ERR_NEW_LOC_RST_VER_CONFLICT)
        elif ProgressInfo.FAILED in data and 'Data node process not running' in data:
            err_pattern = 'Data node process not running: \[(.*?)\]'
            err_services_list = re.findall(err_pattern, data)
            log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.ERR_DB_SERVICES,
                                   logDetailParam=err_services_list)
        else:
            log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=0)
        return log_detail

    def prepare_data_restore_cmd(self, progress_file):
        log.info(f"Start prepare data restore cmd, {self._log_comm}.")
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return ''
        bc_id = param_dict.get("cluster_id", "")
        task_id = param_dict.get("task_id", "")
        result_info_name = param_dict.get("resultinfo_name", "")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Prepare data restore cmd failed, invalid job info, {self._log_comm}.')
            return ''
        rc_id = job_infos[0]
        manager_name = job_infos[1]
        copy_path = MountBindPath.DATA_FILE_PATH
        file_path = os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}', "DATA_BACKUP",
                                 task_id, "ResultInfo", result_info_name)
        os_user = get_env_variable(f"{Env.USER_NAME}_{self._pid}")
        log.info(f"Get os user success, {self._log_comm}.")
        cmd_parameters = CmdParameters(manager_name, bc_id, rc_id, copy_path, file_path, os_user, progress_file)
        if not bc_id.isnumeric():
            log.error(f"Invalid bc_id: {bc_id}, {self._log_comm}.")
            return ''
        if not rc_id.isnumeric():
            log.error(f"Invalid bc_id: {rc_id}, {self._log_comm}.")
            return ''
        if Restore.rst_cmd_injection(cmd_parameters):
            log.error(f"command injection detected in restore command! {self._log_comm}")
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
        log.info(f"Prepare restore cmd success, {self._log_comm}.")
        return cmd

    @handle_ex_with_return_bool_decorator
    def sub_job_exec(self):
        restore_status_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        log.info(f"Step 4-3: Start restore on all managers, {self._log_comm}.")
        result = exec_task_on_all_managers(self.get_exec_rst_result, restore_status_file, f"Step 4-3: exec restore",
                                           self._task_infos)
        return result

    def get_exec_rst_result(self, agent_id, restore_status_file):
        """
        功能描述：从单个管理节点中获取恢复结果，根据该结果，更新恢复状态文件。
        1. 执行成功
        2. dbtool原生异常已在执行后完成更新，只需处理其余异常。
        """
        restore_result = self.exec_restore_on_manager()
        update_agent_sts_general_after_exec(agent_id, restore_status_file, "Restore", restore_result, self._log_comm)
        return restore_result

    def exec_restore_on_manager(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        restore_cmd = self.prepare_rst_cmd_via_task_type(progress_file)
        if not restore_cmd:
            log.error(f'Command error, will not be executed, {self._log_comm}.')
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        log.info(f"Step 4-3: start to exec restore via cmd, {self._log_comm}.")
        try:
            child = pexpect.spawn("/bin/bash", args=["-c", restore_cmd], timeout=None)
        except Exception as ex:
            log.exception(f"Step 4-3: Spawn except an exception {ex}, {self._log_comm}.")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        try:
            ret_code = child.expect(["", pexpect.TIMEOUT, pexpect.EOF])
            if str(ret_code) != CMDResult.SUCCESS:
                log.error(f"Step 4-3: Exec cmd failed, {self._log_comm}.")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                return False
            os_pass = get_env_variable(f"{Env.PASS_WORD}_{self._pid}")
            child.sendline(os_pass)
            log.info(f"Step 4-3: exec restore, {self._log_comm}.")
            clear(os_pass)
            out_str = child.read()
            # 执行完要先关闭，否则child.exitstatus的结果为None
            child.close()
            return_code, out_info, err_info = execute_cmd(f"cat {progress_file}")
            if str(child.exitstatus) != CMDResult.SUCCESS:
                log.error(f"Step 4-3: Fail to exec restore, out: {out_info}, err: {err_info}, {self._log_comm}.")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                self.update_sts_file(out_info)
                return False
            log.info(f"Step 4-3: Succeed to exec restore cmd, out: {out_info}, err: {err_info}, {self._log_comm}.")
        finally:
            if child:
                child.close()
        try:
            process = subprocess.run(["/bin/cat", progress_file], timeout=5, shell=False, stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE, encoding="utf-8")
        except subprocess.TimeoutExpired as err:
            write_progress_file(ProgressInfo.FAILED, progress_file)
            log.info(f"Step 4-3: Timeout, err: {err}")
            return False
        data = process.stdout
        if str(process.returncode) != CMDResult.SUCCESS:
            log.error(f"Step 4-3: Fail to get progress file, err: {process.stderr} data: {data}, {self._log_comm}.")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        log.info(f"Step 4-3: Succeed to exec restore, {self._log_comm}.")
        return True

    def update_sts_file(self, out_info):
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        node_id = job_infos[3]
        rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        if "The response message:" in out_info:
            message = out_info.split("The response message:")[1].replace(ProgressInfo.FAILED, "").replace('\n',
                                                                                                          ' ').strip()
            message = message if message else "exec dbtool failed"
            sts_info = StatusInfo(status=SubJobStatusEnum.FAILED.value, logInfo=ReportDBLabel.RESTORE_SUB_FAILED,
                                  logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL, logDetailParam=["Restore", message])
            log.error(f"Restore failed with recognized err: {message} on {node_id}, will update status via {sts_info}.")
            update_agent_sts(node_id, rst_sts_file, sts_info)

    def prepare_rst_cmd_via_task_type(self, progress_file):
        # 如果_restore_time_stamp为真，即日志恢复
        if self._restore_time_stamp:
            restore_cmd = self.prepare_binlog_restore_cmd(progress_file)
            log.info(f"prepare_binlog_restore_cmd: {restore_cmd}, {self._log_comm}.")
        else:
            restore_cmd = self.prepare_data_restore_cmd(progress_file)
            log.info(f"prepare_restore_cmd: {restore_cmd}, {self._log_comm}.")
        return restore_cmd

    def sub_ver_check(self):
        """
        功能描述：
        检查目标集群版本能否恢复下发的副本，高版本的副本不能恢复到老版本的集群。
        如果恢复目标位置为新位置，则检查新位置的版本。
        如果恢复目标位置不是新位置，则不需要检查版本。
        无上报异常操作，发现异常，将异常写入进度文件。

        参数：
        无

        返回值：
        如果版本检查通过，返回True，否则返回False。

        异常描述：
        如果恢复目标位置为新位置，且待会恢复的副本版本高于目标集群版本，则会抛出错误。
        """
        log.info(f"Start to check version. {self._log_comm}.")
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Check version for restore failed, invalid job info, {self._log_comm}.')
            return False
        user_name = job_infos[1]
        # 如果恢复目标位置为新位置
        if self._restore_target_location == "new":
            # 获取待恢复副本对应的goldendb版本
            copy_version = self._resource_common.get_copy_version()
            log.info(f"Check version for new location restore, copy version {copy_version}. {self._log_comm}.")
            # 获取目标集群版本
            version = GoldenDBResourceInfo.get_cluster_version(user_name)
            # 如果副本版本高于集群版本，则记录错误日志，写入进度文件，并返回False
            if copy_version > version:
                log.error(
                    f"Goldendb cannot restore from higher {copy_version} to lower {version}. {self._log_comm}")
                write_progress_file("Version Check Failed", progress_file)
                return False
            # 如果复制版本不高于集群版本，则记录成功日志并返回True
            else:
                log.info(f"Version check success for new location restore. {self._log_comm}.")
                return True
        # 如果恢复目标位置不是新位置，则不需要检查版本，记录日志并返回True
        log.info(f"No need to check version for origin location restore. {self._log_comm}.")
        return True

    def sub_job_check(self):
        """
        检查计算节点，数据节点的服务，并进行相关的文件和目录的创建和挂载操作。
        无上报异常操作，发现异常，将异常写入进度文件。

        返回值:
        bool: 如果所有检查和操作都成功，返回True，否则返回False。

        异常描述:
        如果路径验证失败，或者文件信息无效，或者挂载、创建目录等操作失败，会记录错误日志，并将异常写入进度文件返回False。
        """
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        # 挂载备份路径
        mount_bind_backup_path(self._data_path, self._meta_path, self._job_id)
        # 获取任务信息
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        # 检查任务信息的有效性
        if len(job_infos) != 4:
            log.error(f'Job check failed, invalid job info, {self._log_comm}.')
            return False
        user_name = job_infos[1]
        node_type = job_infos[2]
        # 获取备份路径
        default_root = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        # 获取复制信息参数
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return False
        cluster_id = param_dict.get("cluster_id", "")
        # 检查集群信息的有效性
        if not cluster_id or not user_name or not node_type:
            log.error(f"Sub job check cluster info failed, {self._log_comm}.")
            return False
        # 修改用户属组和文件权限
        group_name, _ = get_user_info(user_name)
        ini_bkp_root_dir = get_bkp_root_dir_via_role(node_type, default_root, self._job_id)
        # 根据节点类型设置备份目录
        if node_type == GoldenDBNodeType.GTM_NODE:
            data_rst_mnt_dir = os.path.join(self._data_path, f'DBCluster_{cluster_id}', 'LOGICAL_BACKUP')
            prod_rst_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'LOGICAL_BACKUP')
        else:
            data_rst_mnt_dir = os.path.join(self._data_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
            prod_rst_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        # 解挂载生产端挂载路径
        umount_bind_path(prod_rst_mnt_dir)
        # 在数据仓备份目录下，新建挂载路径
        mkdir_chmod_chown_dir_recursively(data_rst_mnt_dir, 0o770, user_name, group_name)
        # 在生产端备份目录下，新建挂载路径
        if not mkdir_chmod_chown_dir_recursively(prod_rst_mnt_dir, 0o770, user_name, group_name, True):
            log.error("fail to make data backup path")
            return False
        # 将文件系统数据仓路径挂载到生产端路径
        if not mount_bind_path(data_rst_mnt_dir, prod_rst_mnt_dir, self._job_id):
            log.error("Mount bind data path failed")
            return False
        # 将文件系统中的活跃事务文件夹复制到生成端的备份根目录下
        if not cp_active_folder(self._data_path, ini_bkp_root_dir, user_name):
            log.error("copy active tx info folder failed.")
            return False
        # 检查CN计算节点是否关闭
        if node_type in GoldenDBNodeType.ZX_MANAGER_NODE and \
                not Restore.check_db_proxy_node_statue(cluster_id, user_name):
            log.info("Sub job check CN node is not close.")
        # 检查数据节点是否关闭服务
        if node_type in GoldenDBNodeType.DATA_NODE:
            service_error_list = GoldenDBResourceInfo.check_service_status(user_name, node_type)
            if service_error_list:
                log.error(f"Sub job check data node is close, error services: {service_error_list}.")
                write_progress_file(f"Data node process not running: {service_error_list}", progress_file)
                return False
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
            log.info(f"Removed progress_file on {extract_ip()[0]}")
        else:
            log.info(f'Progress file {progress_file} removed before post job!')
        return True

    def build_sub_job(self, job_priority, job_name, node):
        cluster_id = self._resource_common.get_target_cluster_id()
        user_name = node.get(GoldenDBJsonConst.OSUSER, "")
        golden_db_node_type = node.get(GoldenDBJsonConst.NODETYPE, "")
        node_id = node.get(GoldenDBJsonConst.PARENTUUID, "")
        job_info = f"{cluster_id} {user_name} {golden_db_node_type} {node_id}"
        return SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=SubJobPolicyEnum.FIXED_NODE.value,
                           jobInfo=job_info, ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        if self._restore_time_stamp:
            return self.gen_sub_job_binlog()
        else:
            return self.gen_sub_job_data()

    def gen_sub_job_data(self):
        log.info(f"Step 3: start generate sub job for data restore, {self._log_comm}.")
        agent_uuids = get_agent_uuids(self._json_param_object)
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
            log.error(f"Step 3: invalid src path: {file_path}, {self._log_comm}")
            return False
        sub_job_array = []

        # 子任务1：新位置恢复检查集群版本
        online_managers = [node for node in manager_node_list if node.get(GoldenDBJsonConst.PARENTUUID) in agent_uuids]
        if not online_managers:
            log.error(f"Step 3: failed to generate sub jobs, no available manager nodes, {self._log_comm}")
            return False
        sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1, GoldenSubJobName.SUB_VER_CHECK,
                                     online_managers[0])
        sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 1st subjob, {GoldenSubJobName.SUB_VER_CHECK} success, {self._log_comm}.")

        # 子任务2：检查+挂载
        # 检查manager节点data节点，挂载目录
        online_gtms = [node for node in gtm_node_list if node.get(GoldenDBJsonConst.PARENTUUID) in agent_uuids]
        online_dns = [node for node in data_node_list if node.get(GoldenDBJsonConst.PARENTUUID) in agent_uuids]
        for node in online_managers + online_dns + online_gtms:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, GoldenSubJobName.SUB_CHECK, node)
            sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 2nd subjob, {GoldenSubJobName.SUB_CHECK} success, {self._log_comm}.")

        # 子任务3：执行恢复，管理节点执行恢复
        manager_node_priority = ManagerPriority.priority
        restore_status_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        restore_status = {}
        for node in online_managers:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, GoldenSubJobName.SUB_EXEC, node)
            sub_job_array.append(sub_job)
            node_id = node.get(GoldenDBJsonConst.PARENTUUID, "")
            sts_info = StatusInfo(priority=manager_node_priority, status=SubJobStatusEnum.RUNNING.value)
            # 恢复子任务排优先级
            restore_status.update({node_id: {'priority': sts_info.priority, 'status': sts_info.status,
                                             'log_info': sts_info.log_info,
                                             'log_detail_param': sts_info.log_detail_param,
                                             'log_detail': sts_info.log_detail}})
            manager_node_priority += 1
        log.info(f"Step 3: generate 3rd subjob, {GoldenSubJobName.SUB_EXEC} success, {self._log_comm}.")
        output_execution_result_ex(restore_status_file, restore_status)

        log.info(f"Step 3: Sub-task splitting succeeded. sub-task num:{len(sub_job_array)}, {self._log_comm}.")
        exec_overwrite_file(file_path, sub_job_array)
        return True

    def gen_sub_job_binlog(self):
        log.info(f"Step 3: start generate sub job for log restore, {self._log_comm}.")
        manager_node = self._resource_common.get_manager_node_info()
        manager_node_list = manager_node.get(GoldenDBJsonConst.NODES, [])
        gtm_node_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GTM, [])
        group_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GROUP, [])
        data_node_list = []
        for group in group_list:
            for node in group.get(GoldenDBJsonConst.MYSQLNODES, []):
                data_node_list.append(node)
        log.info(f"Step 3: success to get all nodes, {self._log_comm}.")
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
        log.info(f"Step 3: generate 1st subjob, {GoldenSubJobName.SUB_VER_CHECK} success, {self._log_comm}.")

        # 子任务2：管理节点组合多个日志副本数据
        for node in manager_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, GoldenSubJobName.SUB_BINLOG_MERGE, node)
            sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 2nd subjob, {GoldenSubJobName.SUB_BINLOG_MERGE} success, {self._log_comm}.")

        # 子任务3：所有节点执行挂载子任务
        for node in manager_node_list + data_node_list + gtm_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, GoldenSubJobName.SUB_BINLOG_MOUNT, node)
            sub_job_array.append(sub_job)

        log.info(f"Step 3: generate 3rd subjob, {GoldenSubJobName.SUB_BINLOG_MOUNT} success, {self._log_comm}.")

        # 子任务4：管理节点执行恢复命令
        manager_node_priority = ManagerPriority.priority
        restore_status_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        restore_status = {}
        for node in manager_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_4, GoldenSubJobName.SUB_EXEC, node)
            sub_job_array.append(sub_job)
            # 恢复子任务排优先级
            sts_info = StatusInfo(priority=manager_node_priority, status=SubJobStatusEnum.RUNNING.value)
            node_id = node.get(GoldenDBJsonConst.PARENTUUID, "")
            restore_status.update({node_id: {'priority': sts_info.priority, 'status': sts_info.status,
                                             'log_info': sts_info.log_info,
                                             'log_detail_param': sts_info.log_detail_param,
                                             'log_detail': sts_info.log_detail}})
        log.info(f"Step 3: generate 4th subjob, {GoldenSubJobName.SUB_EXEC} success, {self._log_comm}.")
        output_execution_result_ex(restore_status_file, restore_status)
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

        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Exec sub_binlog_merge failed, invalid job info, {self._log_comm}.')
            return False
        user_name = job_infos[1]
        group_name, _ = get_user_info(user_name)
        return mkdir_chmod_chown_dir_recursively(log_backup_root_tmp, 0o770, user_name, group_name)

    def sub_binlog_mount(self):
        # 挂载manager、gtm、dn节点的backup_root目录到log仓的backup_root_tmp目录
        local_log_path = os.path.join('/mnt', 'goldendb_' + self._job_id, 'log')
        log.info(f"sub_binlog_mount local_log_path, {local_log_path}")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Exec sub_binlog_mount failed, invalid job info, {self._log_comm}.')
            return False
        cluster_id = job_infos[0]
        user_name = job_infos[1]
        node_type = job_infos[2]
        group_name, _ = get_user_info(user_name)

        # 1:将副本挂载到/mnt/goldendb_{job_id)/log 共享目录
        mkdir_chmod_chown_dir_recursively(local_log_path, 0o770, user_name, group_name, True)
        log_backup_root_tmp = os.path.join(self._log_path, 'backup_root_tmp')
        if not mount_bind_path(log_backup_root_tmp, local_log_path, self._job_id):
            log.error("mount local_log_path failed")
            return False

        # 2: 将副本的DBCluster_{id}目录 挂载到 对应角色的backup_root/DBCluster_{id}
        default_root = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        root_path = get_bkp_root_dir_via_role(node_type, default_root, self._job_id)
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
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Exec prepare_binlog_restore_cmd, invalid job info, {self._log_comm}.')
            return ''
        rc_id = job_infos[0]
        manager_name = job_infos[1]
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

    def check_restore_on_all_managers(self):
        """
        检查所有管理节点的恢复结果。
        """
        log.info(f"Step 4-2: exec restore failed on local, start to check result on other agents.")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Step 4-2: exec restore failed, invalid job info, {self._log_comm}.')
            return False
        node_id = job_infos[3]
        try:
            rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
            log.info(f"Step 4-2: exec restore failed on {node_id}, start to check other results, {self._log_comm}.")
            # 在所有节点上检查任务结果
            ret = check_task_on_all_managers(node_id, rst_sts_file, f"Step 4-2: exec restore, {self._log_comm}",
                                             self._task_infos)
        except Exception as err:
            log.error(f"Step 4-2: exec restore failed to check other results, err_msg: {err}, {self._log_comm}.")
            return False
        return ret

    def report_completed_with_data_size(self, log_detail, sub_job_name):
        """
        如果子任务名称为SUB_EXEC，通过rpc主动上报数据量。
        """
        data_size = 0
        # 数据量所有子任务只上报一次，因为UBC会叠加每个子任务上报的值
        if sub_job_name == GoldenSubJobName.SUB_EXEC:
            data_size = self.get_data_size()
        # 上报子任务详情，包括任务ID、子任务ID、进度、日志详情、数据大小和任务状态
        report_job_details_file_ex(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail], dataSize=data_size,
                                                 taskStatus=SubJobStatusEnum.COMPLETED.value).dict(by_alias=True))

    def get_data_size(self):
        # 获取数据量，如果_restore_time_stamp为真，即日志恢复，则扫描日志仓下对应的路径，否则扫描数据仓。
        if self._restore_time_stamp:
            log_backup_root_tmp = os.path.join(self._log_path, 'backup_root_tmp')
            ret, data_size = scan_dir_size(self._job_id, log_backup_root_tmp)
        else:
            ret, data_size = scan_dir_size(self._job_id, self._data_path)
        # 如果扫描失败，将数据大小设为0
        if not ret:
            log.error(f"Step 4: scan {self._data_path} failed, {self._log_comm}")
            data_size = 0
        log.info(f"Step 4: data size of {self._data_path} is {data_size}, {self._log_comm}")
        return data_size

    def report_err_via_rpc_with_progress(self, message, progress_file=None):
        """
        通过RPC报告错误信息。
        进度文件不为空时，读取进度文件，完成上报。
        进度文件为空时，根据错误码完成上报，如果错误码为空，使用默认标签和错误码上报。

        参数:
        message: str, 错误信息
        progress_file: str, 进度文件路径，默认为None
        err_code: str, 错误代码，默认为None

        返回值:
        无

        异常描述:
        无
        """
        # 记录错误信息
        log.error(f"{message}, {self._log_comm}")
        log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.ERROR_INTERNAL)
        if progress_file:
            if os.path.exists(progress_file):
                log_detail = self.get_err_log_detail_via_process_file(progress_file)
        report_job_details_file_ex(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                       by_alias=True))
