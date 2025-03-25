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
import shutil
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
    DBLogLevel, SubJobPolicyEnum, CMDResult, CMDResultInt
from common.file_common import get_user_info, delete_path
from common.job_const import JobNameConst
from common.util.check_user_utils import check_path_owner, check_os_user
from common.util.cmd_utils import get_livemount_path
from common.util.common_utils import change_dir_owner_recursively
from common.util.exec_utils import exec_overwrite_file, exec_mkdir_cmd
from common.util.scanner_utils import scan_dir_size

from goldendb.handle.common.const import GoldenDBJsonConst, Env, GoldenDBNodeType, ErrorCode, MountBindPath
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.common.const import ProgressInfo, GoldenSubJobName, ManagerPriority, GoldendbLabel
from goldendb.handle.common.goldendb_common import report_job_details_file_ex, write_progress_file, \
    get_backup_path, get_copy_info_param, extract_ip, mount_bind_backup_path, get_agent_uuids, umount_bind_path_list, \
    report_action_result, mount_bind_path, umount_bind_path, write_progress_file_ex, get_etc_ini_path, \
    verify_path_trustlist, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, check_task_on_all_managers, \
    exec_task_on_all_managers, get_recognized_err_from_sts_file, update_agent_sts, report_subjob_via_rpc, \
    update_agent_sts_general_after_exec, exec_chmod_dir_recursively, check_exec_on_cm, check_dbtool_start, \
    shutil_copy_tree, shutil_copy_file, get_dbtool_cm_result, get_active_tx_info_files, get_gtid_timestamps
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
        self._task_infos = TaskInfo(pid=self._pid, jobId=self._job_id, subJobId=self._sub_job_id,
                                    taskType=JobNameConst.RESTORE,
                                    logComm=self._log_comm, jsonParam=self._json_param_object)
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        self._meta_path = get_livemount_path(self._job_id, JsonParam.get_meta_path(json_param))
        self._data_path = get_livemount_path(self._job_id, JsonParam.get_data_path(json_param))
        if JsonParam.has_repository_path(json_param):
            if not verify_path_trustlist(self._cache_area):
                log.error(f"Invalid cache path: {self._cache_area}, {self._log_comm}.")
                raise Exception(f"Invalid cache path: {self._cache_area}, {self._log_comm}.")
            if not verify_path_trustlist(self._meta_path):
                log.error(f"Invalid meta path: {self._meta_path}, {self._log_comm}.")
                raise Exception(f'Invalid meta path: {self._meta_path}, {self._log_comm}.')
            if not verify_path_trustlist(self._data_path):
                log.error(f"Invalid data path: {self._data_path}, {self._log_comm}.")
                raise Exception(f"Invalid data path: {self._data_path}, {self._log_comm}.")
        self._restore_extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self._restore_time_stamp = self._restore_extend_info.get("restoreTimestamp", "")
        self._restore_target_location = self._restore_extend_info.get("targetLocation", "")
        log.info(f"Restore_time_stamp: {self._restore_time_stamp}, {self._log_comm}.")
        self._log_path = ""
        if self._restore_time_stamp:
            if JsonParam.has_repository_path(json_param):
                self._log_path = get_livemount_path(self._job_id, JsonParam.get_log_path(json_param))
                log.info(f"log path: {self._log_path}, {self._log_comm}.")
        self._job_status = SubJobStatusEnum.RUNNING.value
        self._resource_common = GoldenDBRestoreCommon(self._json_param_object)
        log.info(f"json_param: {json_param}, {self._log_comm}")

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
        if return_code != CMDResult.SUCCESS.value:
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
        if return_code != CMDResult.SUCCESS.value:
            log.error(f"The execute start {node_type} service cmd failed!")
            return False
        err_service_list = GoldenDBResourceInfo.check_service_status(os_user, node_type)
        # 如果存在异常服务列表，则表示启动服务失败
        if err_service_list:
            return False
        return True

    @staticmethod
    def check_db_proxy_node_state(cluster_id, manager_user):
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
        if return_code != CMDResult.SUCCESS.value:
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
            report_action_result(self._pid, self._job_id, self._sub_job_id, False, "Step 1: managers all offline")
            return False
        gtm_node_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GTM, [])
        # 检查gtm节点，有一个异常就报错
        no_gtm = any([node.get(GoldenDBJsonConst.PARENTUUID) not in agent_uuids for node in gtm_node_list])
        if no_gtm:
            report_action_result(self._pid, self._job_id, self._sub_job_id, False, "Step 1: gtm abnormal")
            return False
        group_list = self._resource_common.get_cluster_instance_info().get(GoldenDBJsonConst.GROUP, [])
        data_node_list = []
        for group in group_list:
            for node in group.get(GoldenDBJsonConst.MYSQLNODES, []):
                data_node_list.append(node)
        # 检查data节点，有一个异常就报错
        err_dn = any([node.get(GoldenDBJsonConst.PARENTUUID) not in agent_uuids for node in data_node_list])
        if err_dn:
            report_action_result(self._pid, self._job_id, self._sub_job_id, False, "Step 1: data nodes abnormal")
            return False
        log.info(f"Step 1: allow_restore_in_local_node success, {self._log_comm}.")
        return True

    def restore_prerequisite(self):
        """
        执行前置任务：检查集群和原集群的结构
        @return:
        """
        # 检查集群和原集群的结构
        log.info(f"Step 2: restore_prerequisite start, json_param: {self._json_param_object}, {self._log_comm}.")
        umount_bind_path_list([MountBindPath.DATA_FILE_PATH, MountBindPath.META_FILE_PATH])
        if not self._resource_common.check_restore_golden_db_structure():
            log.error(f"Step 2: check golden db cluster structure failed, {self._log_comm}.")
            job_status = SubJobStatusEnum.FAILED.value
            target_group_num = len(
                self._resource_common.get_cluster_info(self._resource_common.get_protect_object()).get(
                    GoldenDBJsonConst.GROUP, []))
            protect_group_num = len(
                self._resource_common.get_cluster_info(self._resource_common.get_target_object()).get(
                    GoldenDBJsonConst.GROUP, []))
            message = "The sharding num is "
            log_detail_param = [message + str(target_group_num), message + str(protect_group_num)]
            log.error(f"Step 2: check golden db cluster structure failed, msg:{log_detail_param}, {self._log_comm}.")
            self._resource_common.get_cluster_info(self._resource_common.get_target_object())
            # 设置self._logdetail
            self.set_logdetail_with_params(ReportDBLabel.PRE_REQUISIT_FAILED, self._sub_job_id,
                                           ErrorCode.ERROR_GOLDENDB_STRUCTURE, log_detail_param,
                                           DBLogLevel.ERROR.value)
        else:
            log.info(f"Step 2: check golden db cluster structure success, {self._log_comm}.")
            job_status = SubJobStatusEnum.COMPLETED.value
            self._logdetail = None
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=job_status, progress=100, logDetail=self._logdetail)
        log.info(f"Step 2: restore_prerequisite finished with details: {output}, {self._log_comm}.")
        report_job_details_file_ex(self._job_id, output.dict(by_alias=True))

    def restore_prerequisite_progress(self):
        """
        执行前置任务：空
        @return:
        """
        log.info(f"Step 2 restore_prerequisite_progress, {self._log_comm}.")
        pre_job_status = SubJobStatusEnum.COMPLETED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=pre_job_status, progress=100)
        output_result_file(self._pid, output.dict(by_alias=True))

    def get_progress(self):
        """
        解析恢复进度
        :return:
        """
        log.info(f'Query restore progress! {self._log_comm}.')
        status = SubJobStatusEnum.RUNNING.value
        progress = 0
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f'Get {progress_file}, {self._log_comm}.')
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return SubJobStatusEnum.FAILED.value, progress
        if not os.path.exists(progress_file):
            status = SubJobStatusEnum.FAILED.value
            log.error(f"Progress file: {progress_file} not exist")
            return status, progress
        log.info(f'Path exist: {progress_file}, {self._log_comm}.')
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
        log.info(f'Progress is {progress}, {self._log_comm}.')
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
        while self._job_status == SubJobStatusEnum.RUNNING.value:
            log.info(f"Start to report {sub_job_name} progress, {self._log_comm}.")
            task_status, progress = self.get_progress()
            progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.RUNNING.value,
                                          progress=0, logDetail=self._logdetail)
            if task_status == SubJobStatusEnum.FAILED.value and sub_job_name != GoldenSubJobName.SUB_EXEC:
                log.error(f"{sub_job_name} failed, {self._log_comm}.")
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
            GoldenSubJobName.SUB_BINLOG_MERGE: self.sub_binlog_merge
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
            log.error(f'Exec {sub_job_name} failed, invalid sub_job_name, {self._log_comm}.')
            return False

        sub_job_dict = self.rst_job_dict()
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        if len(job_infos) != 4:
            log.error(f'Exec {sub_job_name} failed, invalid job info, {self._log_comm}.')
            return False
        write_progress_file_ex(ProgressInfo.START, progress_file)
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()

        self.report_sub_start(sub_job_name)

        # 执行子任务
        log.info(f"Exec sub job {sub_job_name} begin. {self._log_comm}.")
        try:
            ret = self.get_rst_sub_job_result_no_report(sub_job_dict, sub_job_name)
        except Exception as ex:
            log.error(f"Exec sub job {sub_job_name} exception, {ex}. {self._log_comm}.")
            ret = False

        if not ret:
            log.error(f"Exec sub job {sub_job_name} failed. {self._log_comm}.")
            # 读取状态文件，进度文件，通过RPC报告错误，先上报，后关线程
            if sub_job_name == GoldenSubJobName.SUB_EXEC:
                self.report_manager_err()
            else:
                self.report_err_via_rpc_with_progress(f"Exec {sub_job_name} failed", progress_file)
            progress_thread.join()
            return False

        log.info(f"Exec sub job {sub_job_name} finished, start to write progress_file, {self._log_comm}.")
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        self.report_completed_with_data_size(log_detail, sub_job_name)
        log.info(f"Exec sub job {sub_job_name} success. {self._log_comm}.")
        # 先上报，后关线程
        progress_thread.join()
        os.remove(progress_file)
        log.info(f"Removed progress_file on {extract_ip()[0]}, {self._log_comm}.")
        return True

    def report_sub_start(self, sub_job_name):
        if sub_job_name in [GoldenSubJobName.SUB_BINLOG_MERGE, GoldenSubJobName.SUB_CHECK]:
            log_info = ReportDBLabel.RESTORE_SUB_START_PREPARE
        else:
            log_info = ReportDBLabel.RESTORE_SUB_START_COPY
        log_detail = LogDetail(logInfo=log_info, logInfoParam=[self._sub_job_id], logLevel=DBLogLevel.INFO.value)
        log.info(f"Exec {sub_job_name}, {self._log_comm}.")
        report_job_details_file_ex(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                       by_alias=True))

    def report_manager_err(self):
        log.info(f"Restore failed on all managers, will report err, {self._log_comm}.")
        # 从进度文件中获取错误日志详情，并更新至状态文件中
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        node_id = job_infos[3]
        rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        # 获取更新后的错误日志详情，优先取dbtool原生错误上报
        key_log_details = get_recognized_err_from_sts_file(node_id, rst_sts_file, JobNameConst.RESTORE)
        report_subjob_via_rpc(self._pid, SubJobModel(jobId=self._job_id, subJobId=self._sub_job_id), key_log_details)

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
        log.info(f"Get log detail from process file. {self._log_comm}.")
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
                                   logDetailParam=[JobNameConst.RESTORE, message])
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
            log.error(f"command injection detected in restore command! {self._log_comm}.")
            return ''
        if not check_path_owner(file_path, [manager_name]):
            log.warn("user authentication failed!")
            use_infos = pwd.getpwnam(manager_name)
            user_uid = use_infos.pw_uid
            user_groupid = use_infos.pw_gid
            change_dir_owner_recursively(
                os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}', "DATA_BACKUP"), user_uid, user_groupid)
        # 适配E6000
        exec_chmod_dir_recursively(os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}'), 0o777)
        exec_chmod_dir_recursively(os.path.join(MountBindPath.DATA_FILE_PATH, 'Active_TX_Info'), 0o777)

        on_cm = check_exec_on_cm(manager_name, self._job_id)
        if on_cm:
            log.info(f"Will exec log restore on mds, {self._log_comm}.")
            cmd = f'su - {manager_name} -c "dbtool -cm -restore -type=cluster -check-binlog=yes -consistence=yes ' \
                  f'-restore-clusterid={bc_id} -backup-clusterid={rc_id} -restore-start-time=yes ' \
                  f'-result-file="{file_path}" -backuproot-dir="{copy_path}""'
        else:
            log.info(f"Will exec data restore on mds, {self._log_comm}.")
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

    def get_exec_rst_result(self, agent_id, rst_sts_file):
        """
        功能描述：从单个管理节点中获取恢复结果，根据该结果，更新恢复状态文件。
        1. 执行成功
        2. dbtool原生异常已在执行后完成更新，只需处理其余异常。
        """
        rst_result = self.exec_restore_on_manager()
        update_agent_sts_general_after_exec(agent_id, rst_sts_file, JobNameConst.RESTORE, rst_result, self._log_comm)
        return rst_result

    def exec_restore_on_manager(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        restore_cmd = self.prepare_rst_cmd_via_task_type(progress_file)
        if not restore_cmd:
            log.error(f'Command error, will not be executed, {self._log_comm}.')
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        log.info(f"Step 4-3: start to exec restore via cmd, {self._log_comm}.")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        manager_name = job_infos[1]
        on_cm = check_exec_on_cm(manager_name, self._job_id)
        if on_cm:
            log.info(f"Step 4-3: exec restore on cm, {self._log_comm}.")
            result = self.get_dbtool_rst_result_from_cm(manager_name, progress_file, restore_cmd)
        else:
            log.info(f"Step 4-3: exec restore on mds, {self._log_comm}.")
            result = self.get_dbtool_rst_result_from_mds(progress_file, restore_cmd)
        log.info(f"Step 4-3: restore result: {result}, {self._log_comm}.")
        return result

    def get_dbtool_rst_result_from_cm(self, manager_name, progress_file, restore_cmd):
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        node_id = job_infos[3]
        rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
        return_code, return_info, err_info = execute_cmd(restore_cmd)
        log.info(f"CM return_code: {return_code}, return_info: {return_info}, err_info: {err_info}, {self._log_comm}.")
        result, restore_id, log_detail = check_dbtool_start(self._pid, self._job_id, self._sub_job_id,
                                                            (return_code, return_info), JobNameConst.RESTORE)
        # 检查发起恢复命令结果

        result, log_detail = get_dbtool_cm_result(JobNameConst.RESTORE, manager_name, (result, restore_id, log_detail),
                                                  self._job_id, self._sub_job_id)
        log.info(f"Check cm restore start result: {result}, restore id {restore_id}, {self._log_comm}.")
        if not result:
            log.error(f"Check restore result fail, {self._log_comm}.")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            sts_info = StatusInfo(status=SubJobStatusEnum.FAILED.value, logDetail=log_detail.log_detail,
                                  logDetailParam=log_detail.log_detail_param, logInfo=log_detail.log_info)
            update_agent_sts(node_id, rst_sts_file, sts_info)
            return result
        else:
            log.info(f"Check restore result success, {self._log_comm}.")
            write_progress_file(ProgressInfo.SUCCEED, progress_file)
            return result

    def get_dbtool_rst_result_from_mds(self, progress_file, restore_cmd):
        try:
            child = pexpect.spawn("/bin/bash", args=["-c", restore_cmd], timeout=None)
        except Exception as ex:
            log.exception(f"Step 4-3: Spawn except an exception {ex}, {self._log_comm}.")
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return False
        try:
            ret_code = child.expect(["", pexpect.TIMEOUT, pexpect.EOF])
            if ret_code != CMDResultInt.SUCCESS.value:
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
            if child.exitstatus != CMDResultInt.SUCCESS.value:
                log.error(f"Step 4-3: Fail to exec restore, out: {out_info}, err: {err_info}, {self._log_comm}.")
                write_progress_file(ProgressInfo.FAILED, progress_file)
                # 只更新执行恢复指令失败产生的报错
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
            log.info(f"Step 4-3: Timeout, err: {err}, {self._log_comm}.")
            return False
        data = process.stdout
        if process.returncode != CMDResultInt.SUCCESS.value:
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
                                  logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                  logDetailParam=[JobNameConst.RESTORE, message])
            log.error(f"Restore failed with recognized err: {message} on {node_id}, will update status via {sts_info}.")
            update_agent_sts(node_id, rst_sts_file, sts_info)

    def prepare_rst_cmd_via_task_type(self, progress_file):
        # 如果_restore_time_stamp为真，即日志恢复
        if self._restore_time_stamp:
            restore_cmd = self.prepare_binlog_restore_cmd(progress_file)
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
                    f"Goldendb cannot restore from higher {copy_version} to lower {version}. {self._log_comm}.")
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
        # 挂载备份路径至短路径，满足恢复指令长度限制
        mount_bind_backup_path(self._data_path, self._meta_path, self._job_id)
        # 获取任务信息
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        # 检查任务信息的有效性
        user_name = job_infos[1]
        node_type = job_infos[2]
        # 获取备份路径
        default_root = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        # 获取复制信息参数
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return False
        bc_id = param_dict.get("cluster_id", "")
        # 检查集群信息的有效性
        if not bc_id or not user_name or not node_type:
            log.error(f"Sub job check cluster info failed, {self._log_comm}.")
            return False
        # 修改用户属组和文件权限
        group_name, _ = get_user_info(user_name)
        # 根据节点类型设置备份目录
        data_rst_mnt_dir, prod_rst_mnt_dir = self.get_rst_dirs(bc_id, default_root, node_type, user_name)
        # 解挂载生产端挂载路径
        umount_bind_path(prod_rst_mnt_dir)
        # 在数据仓备份目录下，新建挂载路径
        mkdir_chmod_chown_dir_recursively(data_rst_mnt_dir, 0o770, user_name, group_name)
        # 在生产端备份目录下，新建挂载路径
        if not mkdir_chmod_chown_dir_recursively(prod_rst_mnt_dir, 0o770, user_name, group_name):
            log.error(f"Failed to make data backup path, {self._log_comm}.")
            return False
        # 将文件系统数据仓路径挂载到生产端路径
        if not mount_bind_path(data_rst_mnt_dir, prod_rst_mnt_dir, self._job_id):
            log.error(f"Mount bind data path failed, {self._log_comm}.")
            return False

        self.mount_gtm_etc_path(default_root, group_name, node_type, prod_rst_mnt_dir, user_name)

        # 检查CN计算节点是否关闭
        if node_type in GoldenDBNodeType.ZX_MANAGER_NODE and \
                not Restore.check_db_proxy_node_state(bc_id, user_name):
            log.info(f"Sub job check CN node is not close, {self._log_comm}.")
        # 检查数据节点是否关闭服务
        if node_type in GoldenDBNodeType.DATA_NODE:
            service_error_list = GoldenDBResourceInfo.check_service_status(user_name, node_type)
            if service_error_list:
                log.error(f"Sub job check data node is close, error services: {service_error_list}, {self._log_comm}.")
                write_progress_file(f"Data node process not running: {service_error_list}", progress_file)
                return False
        return True

    def mount_gtm_etc_path(self, default_root, group_name, node_type, prod_rst_mnt_dir, user_name):
        if node_type == GoldenDBNodeType.GTM_NODE:
            gtm_etc_root = get_etc_ini_path(user_name, GoldenDBNodeType.GTM_NODE, default_root, self._job_id)
            umount_bind_path(gtm_etc_root)
            if not mkdir_chmod_chown_dir_recursively(gtm_etc_root, 0o770, user_name, group_name):
                log.error(f"Failed to make gtm etc backup path, {self._log_comm}.")
            if not mount_bind_path(gtm_etc_root, prod_rst_mnt_dir, self._job_id):
                log.error(f"Mount bind gtm etc backup path failed, {self._log_comm}.")
            log.info(f"Mount bind gtm etc backup path finished, {self._log_comm}.")

    def get_rst_dirs(self, bc_id, default_root, node_type, user_name):
        rel_dir = 'LOGICAL_BACKUP' if node_type == GoldenDBNodeType.GTM_NODE else 'DATA_BACKUP'
        if self._restore_time_stamp:
            data_rst_mnt_dir = os.path.join(self._data_path, f'DBCluster_{bc_id}')
            prod_rst_mnt_dir = os.path.join(default_root, f'DBCluster_{bc_id}')
        else:
            data_rst_mnt_dir = os.path.join(self._data_path, f'DBCluster_{bc_id}', rel_dir)
            prod_rst_mnt_dir = os.path.join(default_root, f'DBCluster_{bc_id}', rel_dir)
        return data_rst_mnt_dir, prod_rst_mnt_dir

    def do_post(self):
        log.info(f'umount backup paths, {self._log_comm}.')
        umount_bind_backup_paths(self._job_id)
        if self._restore_time_stamp:
            log.info(f"remove backup_root_tmp, {self._log_comm}.")
            log_backup_root = os.path.join(self._log_path, 'backup_root_tmp')
            if os.path.exists(log_backup_root):
                delete_path(log_backup_root)
                log.info(f"remove backup_root_tmp done, {self._log_comm}.")
        tmp_path = os.path.join('/mnt', 'goldendb_' + self._job_id)
        if os.path.exists(tmp_path):
            delete_path(tmp_path)
        log.info(f'tmp path deleted, {self._log_comm}.')
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        if not verify_path_trustlist(progress_file):
            log.error(f"Invalid src path: {progress_file}.")
            return False
        if os.path.exists(progress_file):
            os.remove(progress_file)
            log.info(f"Removed progress_file on {extract_ip()[0]}, {self._log_comm}.")
        else:
            log.info(f'Progress file {progress_file} removed before post job! {self._log_comm}.')
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
            log.error(f"Step 3: invalid src path: {file_path}, {self._log_comm}.")
            return False
        sub_job_array = []

        # 子任务1：新位置恢复检查集群版本
        online_managers = [node for node in manager_node_list if node.get(GoldenDBJsonConst.PARENTUUID) in agent_uuids]
        if not online_managers:
            log.error(f"Step 3: failed to generate sub jobs, no available manager nodes, {self._log_comm}.")
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
        sub_job_array = self.gen_sub_job_binlog_array(manager_node_list, data_node_list, gtm_node_list)
        log.info(f"Binlog Sub-task splitting succeeded.sub-task num: {len(sub_job_array)}, {self._log_comm}.")
        exec_overwrite_file(file_path, sub_job_array)
        return True

    def gen_sub_job_binlog_array(self, manager_node_list, data_node_list, gtm_node_list):
        sub_job_array = []
        # 子任务1：新位置恢复检查集群版本
        sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1, GoldenSubJobName.SUB_VER_CHECK,
                                     manager_node_list[0])
        sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 1st subjob, {GoldenSubJobName.SUB_VER_CHECK} success, {self._log_comm}.")

        # 子任务2：管理节点，数据节点，gtm节点挂载数据备份副本
        for node in manager_node_list + data_node_list + gtm_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, GoldenSubJobName.SUB_CHECK, node)
            sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 2nd subjob, {GoldenSubJobName.SUB_CHECK} success, {self._log_comm}.")

        # 子任务3：管理节点组合多个日志副本数据
        for node in manager_node_list + data_node_list:
            sub_job = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, GoldenSubJobName.SUB_BINLOG_MERGE, node)
            sub_job_array.append(sub_job)
        log.info(f"Step 3: generate 3rd subjob, {GoldenSubJobName.SUB_BINLOG_MERGE} success, {self._log_comm}.")

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
        param_dict = get_copy_info_param(self._meta_path)
        bc_id = param_dict.get('cluster_id', '')
        # 1) 拷贝log仓中多个日志副本的Active_TX_Info、LOGICAL_BACKUP至data仓，由于data仓的路径已挂载短路径，短路径下会同步这些文件。
        id_list = self.get_associated_log_copy_ids()
        for log_copy_id in id_list:
            act_tx_info = os.path.join(self._log_path, log_copy_id, 'Active_TX_Info')
            act_tx_data = os.path.join(self._data_path, 'Active_TX_Info')
            if not shutil_copy_tree(act_tx_info, act_tx_data, f"{self._log_comm}, copy active_tx_info to data"):
                return False
            logical_bkp = os.path.join(self._log_path, log_copy_id, f'DBCluster_{bc_id}', 'LOGICAL_BACKUP')
            logical_bkp_tmp = os.path.join(self._data_path, f'DBCluster_{bc_id}', 'LOGICAL_BACKUP')
            if not shutil_copy_tree(logical_bkp, logical_bkp_tmp, f"{self._log_comm}, copy logical_bkp to data"):
                return False
        log.info(f"Copy Active_TX_Info, LOGICAL_BACKUP from file sys to data success, {self._log_comm}.")
        # 2) 拷贝生产环境中的Active_TX_Info到data仓下，由于data仓的路径已挂载短路径，短路径下会同步这些文件。
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        user_name = job_infos[1]
        node_type = job_infos[2]
        default_root = get_backup_path(user_name, node_type, self._json_param_object, GoldenDBJsonConst.TARGETOBJECT)
        if node_type == GoldenDBNodeType.ZX_MANAGER_NODE:
            prod_act_root = get_etc_ini_path(user_name, "active", default_root, self._job_id)
            log.info(f"Get prod act root success: {prod_act_root}, {self._log_comm}.")
            prod_act_tx = os.path.join(prod_act_root, 'Active_TX_Info')
            act_tx_data = os.path.join(self._data_path, 'Active_TX_Info')
            if not shutil_copy_tree(prod_act_tx, act_tx_data, f"{self._log_comm}, copy prod active_tx_info to data"):
                return False
            # 只需要检查一次active_tx_info
            log.info(f"Check active info, {self._log_comm}.")
            self.check_restore_time_via_gtid(prod_act_tx)
        log.info(f"Copy Active_TX_Info from prod to data success, {self._log_comm}.")

        # 3) 拷贝副本的事务信息Active_TX_Info目录数据节点的backup_root/Active_TX_Info目录
        if node_type == GoldenDBNodeType.DATA_NODE:
            log.info(f"copy Active_TX_Info to data node root, {self._log_comm}.")
            act_tx_data = os.path.join(self._data_path, 'Active_TX_Info')
            prod_act_tx = os.path.join(default_root, 'Active_TX_Info')
            # 对于管理节点，只需要将active info复制到backup_root下
            if not shutil_copy_tree(act_tx_data, prod_act_tx, f"{self._log_comm}, copy active tx info to manager root"):
                return False
            # 只需要检查一次active_tx_info
        group_name, _ = get_user_info(user_name)
        mkdir_chmod_chown_dir_recursively(os.path.join(self._data_path, 'Active_TX_Info'), 0o770, user_name, group_name)
        mkdir_chmod_chown_dir_recursively(os.path.join(self._data_path, f'DBCluster_{bc_id}'), 0o770, user_name,
                                          group_name)
        return True

    def check_restore_time_via_gtid(self, act_tx_root):
        # 日志恢复需要检查时间点
        active_tx_info_files = get_active_tx_info_files(act_tx_root)
        get_valid_gtid = False
        for active_tx_info_file in active_tx_info_files:
            gtid_times = get_gtid_timestamps(active_tx_info_file)
            for gtid_time in gtid_times:
                time_diff = abs(gtid_time - int(self._restore_time_stamp))
                # 活跃gtid记录的采集时间点与用户指定时间之间的时间差大于1分钟，恢复会失败
                if time_diff < 60:
                    log.info(f"Valid gtid {gtid_time}, restore time {self._restore_time_stamp}, {self._log_comm}.")
                    get_valid_gtid = True
                    break
            if get_valid_gtid:
                log.info(f"Restore time {self._restore_time_stamp} is valid, {self._log_comm}.")
                break
        if not get_valid_gtid:
            log.warn(f"Invalid restore time {self._restore_time_stamp}, no valid gtid, {self._log_comm}.")
            log_detail = LogDetail(logInfo=GoldendbLabel.active_tx_info_record_lost, logLevel=DBLogLevel.WARN.value)
            report_job_details_file_ex(self._pid,
                                       SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=95,
                                                     logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value).dict(by_alias=True))
        log.info(f"Check restore time via gtid finished with result: {get_valid_gtid}, {self._log_comm}.")

    def prepare_binlog_restore_cmd(self, progress_file):
        param_dict = get_copy_info_param(self._meta_path)
        if not param_dict:
            return ''
        bc_id = param_dict.get("cluster_id", "")
        task_id = param_dict.get("task_id", "")
        result_info_name = param_dict.get("resultinfo_name", "")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        rc_id = job_infos[0]
        manager_name = job_infos[1]
        log_backup_root = MountBindPath.DATA_FILE_PATH
        file_path = os.path.join(MountBindPath.DATA_FILE_PATH, f'DBCluster_{bc_id}', "DATA_BACKUP",
                                 task_id, "ResultInfo", result_info_name)
        os_user = get_env_variable(f"{Env.USER_NAME}_{self._pid}")
        cmd_parameters = CmdParameters(manager_name, bc_id, rc_id, log_backup_root, file_path, os_user, progress_file)
        if not bc_id.isnumeric():
            log.error(f"Invalid bc_id: {bc_id}, {self._log_comm}.")
            return ''
        if not rc_id.isnumeric():
            log.error(f"Invalid bc_id: {rc_id}, {self._log_comm}.")
            return ''
        if Restore.rst_cmd_injection(cmd_parameters):
            log.error(f"command injection detected in restore command! {self._log_comm}.")
            return ''
        if not check_path_owner(file_path, [manager_name]):
            log.warn("user authentication failed!")
            use_infos = pwd.getpwnam(manager_name)
            user_uid = use_infos.pw_uid
            user_groupid = use_infos.pw_gid
            change_dir_owner_recursively(os.path.join(log_backup_root, f'DBCluster_{bc_id}'), user_uid, user_groupid)
            change_dir_owner_recursively(os.path.join(log_backup_root, 'Active_TX_Info'), user_uid, user_groupid)
        time_stamp = time.localtime(int(self._restore_time_stamp))
        restore_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        on_cm = check_exec_on_cm(manager_name, self._job_id)
        if on_cm:
            log.info(f"Will exec log restore on cm, {self._log_comm}.")
            cmd = f'su - {manager_name} -c "dbtool -cm -restore -type=cluster -backup-clusterid={bc_id} ' \
                  f'-restore-clusterid={rc_id} ' \
                  f"-restore-time='{restore_time}' -backuproot-dir='{log_backup_root}' -result-file='{file_path}' " \
                  f'-check-binlog=yes -consistence=yes"'
        else:
            log.info(f"Will exec log restore on mds, {self._log_comm}.")
            cmd = f'su - {manager_name} -c "dbtool -mds -restore -bc={bc_id} -rc={rc_id} ' \
                  f"-t='{restore_time}' -d={log_backup_root} " \
                  f'-f={file_path} -no-check-binlog-time -user={os_user} -password" >> {progress_file}'
        log.info(f"Get timestamp restore cmd: {cmd}, {self._log_comm}.")
        return cmd

    def get_associated_log_copy_ids(self):
        if not self._log_path:
            log.error(f'Failed to get log copy path, {self._log_comm}.')
            return []
        associated_log_copies = self._json_param_object.get("job").get("extendInfo").get("associated_log_copies", {})
        if not associated_log_copies:
            log.warning(f'associated_log_copies is null, {self._log_comm}.')
            return []
        copy_id_set = set()
        for key, _ in associated_log_copies.items():
            copy_id_set.add(key)
        log.info(f'copy_id_set is {copy_id_set}, {self._log_comm}.')
        return list(copy_id_set)

    def check_restore_on_all_managers(self):
        """
        检查所有管理节点的恢复结果。
        """
        log.info(f"Step 4-2: exec restore failed on local, start to check result on other agents.")
        job_infos = self._json_param_object.get(GoldenDBJsonConst.SUBJOB, {}).get(GoldenDBJsonConst.JOBINFO,
                                                                                  "").split(" ")
        node_id = job_infos[3]
        try:
            rst_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_restore_status.json')
            log.info(f"Step 4-2: exec restore failed on {node_id}, start to check other results, {self._log_comm}.")
            # 在所有节点上检查任务结果
            ret = check_task_on_all_managers(node_id, rst_sts_file, f"Step 4-2: exec restore, {self._log_comm}.",
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
            log.error(f"Step 4: scan {self._data_path} failed, {self._log_comm}.")
            data_size = 0
        log.info(f"Step 4: data size of {self._data_path} is {data_size}, {self._log_comm}.")
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
        log.error(f"{message}, {self._log_comm}.")
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
