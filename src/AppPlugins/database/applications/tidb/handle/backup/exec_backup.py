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
import math
import os
import pwd
import shutil
import stat
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime
from functools import wraps

from common.common import execute_cmd, execute_cmd_list, execute_cmd_with_expect, invoke_rpc_tool_interface, \
    output_execution_result_ex, output_result_file
from common.common_models import ActionResult, LogDetail, SubJobDetails, SubJobModel
from common.const import BackupTypeEnum, CMDResult, CMDResultInt, CopyDataTypeEnum, DBLogLevel, ExecuteResultEnum, \
    ParamConstant, RepositoryDataTypeEnum, RpcParamKey, SubJobStatusEnum, SubJobTypeEnum, SubJobPriorityEnum, \
    SubJobPolicyEnum, ReportDBLabel
from common.file_common import change_mod, change_owner_by_name, change_path_permission, create_dir_recursive
from common.job_const import JobNameConst
from common.util.checkout_user_utils import get_path_owner
from common.util.exec_utils import exec_append_file
from common.util.scanner_utils import scan_dir_size
from tidb.common.const import BackupGranEnum, ClusterRequiredHost, ErrorCode, LastCopyType, SqliteServiceField, \
    TiDBConst, TiDBDataBaseFilter, TidbSubJobName
from tidb.common.safe_get_information import ResourceParam
from tidb.common.tidb_common import convert_tso_time_to_ts, create_file_append, drop_db, exec_rc_tool_cmd, \
    get_backup_tso_validate, get_cluster_user, get_db, get_err_msg, get_log_path, get_log_stat_info, \
    get_status_up_role_one_host, get_table, get_tidb_structure, get_uids, parse_tso_to_time, report_job_details, \
    check_paths_valid, get_db_total_size, get_db_size, get_tab_size
from tidb.handle.backup.tidb_sqlite_service import TidbSqliteService
from tidb.logger import log
from tidb.schemas.tidb_schemas import TidbBkpResultInfo


class BackUp:

    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Pare params obj is null.")
            raise Exception("Parse params obj is null.")
        self._std_in = data
        self.pid = pid
        self.job_id = job_id
        self.sub_job_id = sub_job_id
        self._json_param_object = json_param
        self._logdetail = None
        self._err_info = {}
        self.query_progress_interval = 15
        self._instance_id = self._json_param_object.get("job", {}).get("protectObject", {}).get("id", "")
        self._protect_obj_extend_info = self._json_param_object.get("job", {}).get("protectObject", {}).get(
            "extendInfo", {})
        self._copy_id = self._json_param_object.get("job", {}).get("copy", [])[0].get("id", "")
        self._cluster_info_str = self._protect_obj_extend_info.get("clusterInfoList")
        self._cluster_info_ = get_tidb_structure(self._cluster_info_str)
        self._cluster_name = self._protect_obj_extend_info.get("clusterName")
        self._tiup_uuid = self._protect_obj_extend_info.get("tiupUuid")
        self._sub_job_name = ""
        self.cache_path = self.get_repository_path(json_param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self.data_path = self.get_repository_path(json_param, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._meta_path = self.get_repository_path(json_param, RepositoryDataTypeEnum.META_REPOSITORY)
        self.log_path = self.get_repository_path(json_param, RepositoryDataTypeEnum.LOG_REPOSITORY)
        self._job_status = SubJobStatusEnum.RUNNING
        self._err_code = 0
        self._tiup_path = self._protect_obj_extend_info.get("tiupPath", '/root/.tiup/bin/tiup')

        # 组装资源接入请求体
        self.backup_type = self._json_param_object.get("job", {}).get("jobParam", {}).get("backupType")
        self._bkp_gran = self._json_param_object.get("job", {}).get("protectObject", {}).get("subType", "")
        log.debug(f"Param: {json_param}")

    class BackupDecorator:
        @staticmethod
        def report_speed(func):
            @wraps(func)
            def wrapper(self, *args, **kwargs):
                message = str(int((time.time())))
                time_file = os.path.join(self.cache_path, f'T{self.job_id}')
                exec_append_file(time_file, message)
                if self.backup_type == BackupTypeEnum.LOG_BACKUP:
                    path = self.log_path
                    log_msg = 'step 5-2 log backup failed;'
                    thread_name_prefix = 'tidb-log-backup'
                else:
                    path = os.path.join(self.data_path, f"tidb_{self.job_id}")
                    log_msg = 'step 5-3 data backup failed;'
                    thread_name_prefix = 'tidb-data-backup'
                original_size = 0
                ret, size = scan_dir_size(self.job_id, path)
                if ret:
                    original_size = size
                process = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=0,
                                        taskStatus=SubJobStatusEnum.RUNNING)
                log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY,
                                       logInfoParam=[process.sub_task_id],
                                       logLevel=DBLogLevel.INFO.value)
                process.log_detail = [log_detail]
                report_job_details(self.pid, process.dict(by_alias=True))
                pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix=thread_name_prefix)
                copy_feature = pool.submit(func, self, *args, **kwargs)
                while not copy_feature.done():
                    time.sleep(self.query_progress_interval)
                    if not self.report_backup_process(original_size, process, path):
                        log.error(log_msg + " backup root error.")
                        pool.shutdown()
                        return False
                if not copy_feature.result()[2]:
                    if self.backup_type == BackupTypeEnum.LOG_BACKUP:
                        msg = "copy error"
                    else:
                        err_msgs = copy_feature.result()[1]
                        msg = get_err_msg(err_msgs)
                    log.error(f'{log_msg} {msg}')
                    self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR,
                                                  ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL, msg)
                    return False
                data_size = scan_dir_size(self.job_id, path)[1] - original_size
                process.speed = self.query_size_and_speed(time_file, original_size, path)[1]
                process.data_size = data_size
                process.task_status = SubJobStatusEnum.COMPLETED
                process.progress = 100
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[process.sub_task_id],
                                       logLevel=DBLogLevel.INFO.value)
                process.log_detail = [log_detail]
                report_job_details(self.pid, process.dict(by_alias=True))
                return True

            return wrapper

    @staticmethod
    def get_repository_path(file_content, repository_type):
        """
        获取挂载x8000文件系统路径
        :param file_content: 任务下发参数
        :param repository_type: 0:META;1: DATA;2: CACHE; 3: LOG; 4: INDEX; 5: LOG_META
        :return:
        """
        repositories = file_content.get("job", {}).get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_path = repository["path"][0]
                break
        return repositories_path

    @staticmethod
    def delete_directory(directory):
        for root_path, dirs, files in os.walk(directory):
            for file in files:
                os.remove(os.path.join(root_path, file))
            for dir_in_path in dirs:
                shutil.rmtree(os.path.join(root_path, dir_in_path), ignore_errors=True)

    def get_log_comm(self):
        return f"pid:{self.pid} jobId:{self.job_id} subjobId:{self.sub_job_id}"

    def record_deploy_user(self):
        deploy_user = get_cluster_user(self._cluster_name, self._tiup_path)
        deploy_user_file = os.path.join(self.cache_path, f"bkp_{self.job_id}_deploy_user.json")
        output_execution_result_ex(deploy_user_file, {"deploy_user": deploy_user})

    def build_sub_job(self, job_priority, policy, job_name, node_id, job_info):
        return SubJobModel(jobId=self.job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=policy, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        if self.backup_type == BackupTypeEnum.LOG_BACKUP:
            self.gen_log_bkp_sub_job()
        else:
            self.gen_data_bkp_sub_job()

    def backup_task_subjob_dict(self):
        sub_job_dict = {
            TidbSubJobName.SUB_DEPLOY_USER: self.exec_deploy_user_sub_job,
            TidbSubJobName.SUB_CHECK_UP: self.exec_check_up_sub_job,
            TidbSubJobName.SUB_RECORD_UID: self.exec_record_uid_sub_job,
            TidbSubJobName.SUB_CHECK_UID: self.exec_check_uid_sub_job,
            TidbSubJobName.SUB_CREATE: self.exec_create_sub_job,
            TidbSubJobName.SUB_EXEC: self.exec_backup_sub_job,
            TidbSubJobName.SUB_RECORD: self.exec_record_sub_job,
            TidbSubJobName.SUB_UP_LOG: self.exec_up_log_sub_job,
            TidbSubJobName.SUB_KF_LOG: self.exec_kf_log_sub_job
        }
        return sub_job_dict

    def gen_log_bkp_sub_job(self):
        log.info(f"Step 4: start to gen_sub_job for log backup, {self.get_log_comm()}.")
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        sub_job_array = []
        up_job_info = ''
        # 添加子任务1, 记录tiup节点deploy user
        sub_job_tiup_deploy_user = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1,
                                                      SubJobPolicyEnum.FIXED_NODE.value,
                                                      TidbSubJobName.SUB_DEPLOY_USER, self._tiup_uuid, "")
        sub_job_array.append(sub_job_tiup_deploy_user)
        # 添加子任务2, 记录tiup，tikv, tiflash节点uid
        sub_job_tiup_uid = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, SubJobPolicyEnum.FIXED_NODE.value,
                                              TidbSubJobName.SUB_RECORD_UID, self._tiup_uuid, "")
        sub_job_array.append(sub_job_tiup_uid)
        kvs = self._cluster_info_.tikv_nodes
        self.add_sub_job_to_node(sub_job_array, kvs, SubJobPriorityEnum.JOB_PRIORITY_2, TidbSubJobName.SUB_RECORD_UID)
        flashes = self._cluster_info_.tiflash_nodes
        self.add_sub_job_to_node(sub_job_array, flashes, SubJobPriorityEnum.JOB_PRIORITY_2,
                                 TidbSubJobName.SUB_RECORD_UID)
        # 添加子任务3, 检查tikv, tiflash节点uid
        sub_job_check_kf = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, SubJobPolicyEnum.FIXED_NODE.value,
                                              TidbSubJobName.SUB_CHECK_UID, self._tiup_uuid, "")
        sub_job_array.append(sub_job_check_kf)
        # 添加子任务4, up节查日志备份状态，时间
        sub_job_up_log = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_4, SubJobPolicyEnum.FIXED_NODE.value,
                                            TidbSubJobName.SUB_UP_LOG, self._tiup_uuid, up_job_info)
        sub_job_array.append(sub_job_up_log)
        # 添加子任务5, tikv, tiflash节点预处理，检查日志备份路径，新建数据备份路径
        kvs = self._cluster_info_.tikv_nodes
        self.add_sub_job_to_node(sub_job_array, kvs, SubJobPriorityEnum.JOB_PRIORITY_5, TidbSubJobName.SUB_KF_LOG)
        flashes = self._cluster_info_.tiflash_nodes
        self.add_sub_job_to_node(sub_job_array, flashes, SubJobPriorityEnum.JOB_PRIORITY_5, TidbSubJobName.SUB_KF_LOG)
        output_execution_result_ex(file_path, sub_job_array)
        log.info(f"Step 4: execute gen_log_bkp_sub_job success, sub_job_array {sub_job_array}, {self.get_log_comm()}.")
        return True

    def gen_data_bkp_sub_job(self):
        log.info(f"Step 4: start to gen_sub_job for data backup, {self.get_log_comm()}.")
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self.pid}")
        sub_job_array = []
        # 添加子任务1, up节查库表信息, 查询集群用户名
        sub_job_check = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_1, SubJobPolicyEnum.FIXED_NODE.value,
                                           TidbSubJobName.SUB_CHECK_UP, self._tiup_uuid, "")
        sub_job_array.append(sub_job_check)
        # 添加子任务2, 记录tiup，tikv, tiflash节点uid
        sub_job_tiup = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_2, SubJobPolicyEnum.FIXED_NODE.value,
                                          TidbSubJobName.SUB_RECORD_UID, self._tiup_uuid, "")
        sub_job_array.append(sub_job_tiup)
        kvs = self._cluster_info_.tikv_nodes
        self.add_sub_job_to_node(sub_job_array, kvs, SubJobPriorityEnum.JOB_PRIORITY_2, TidbSubJobName.SUB_RECORD_UID)
        flashes = self._cluster_info_.tiflash_nodes
        self.add_sub_job_to_node(sub_job_array, flashes, SubJobPriorityEnum.JOB_PRIORITY_2,
                                 TidbSubJobName.SUB_RECORD_UID)
        # 添加子任务3, 检查tikv, tiflash节点uid
        sub_job_check_kf = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_3, SubJobPolicyEnum.FIXED_NODE.value,
                                              TidbSubJobName.SUB_CHECK_UID, self._tiup_uuid, "")
        sub_job_array.append(sub_job_check_kf)
        # 添加子任务4, tikv, tiflash, tiup节点新建数据备份路径
        self.add_sub_job_to_node(sub_job_array, kvs, SubJobPriorityEnum.JOB_PRIORITY_4, TidbSubJobName.SUB_CREATE)
        self.add_sub_job_to_node(sub_job_array, flashes, SubJobPriorityEnum.JOB_PRIORITY_4, TidbSubJobName.SUB_CREATE)
        sub_job_create = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_4, SubJobPolicyEnum.FIXED_NODE.value,
                                            TidbSubJobName.SUB_CREATE, self._tiup_uuid, "")
        sub_job_array.append(sub_job_create)
        # 添加子任务5, tiup节点执行数据备份
        sub_job_exec = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_5, SubJobPolicyEnum.FIXED_NODE.value,
                                          TidbSubJobName.SUB_EXEC, self._tiup_uuid, "")
        sub_job_array.append(sub_job_exec)
        # 添加子任务6, tiup记录数据备份结果
        sub_job_record = self.build_sub_job(SubJobPriorityEnum.JOB_PRIORITY_6, SubJobPolicyEnum.FIXED_NODE.value,
                                            TidbSubJobName.SUB_RECORD, self._tiup_uuid, "")
        sub_job_array.append(sub_job_record)
        output_execution_result_ex(file_path, sub_job_array)
        log.info(f"Step 4: execute backup_gen_sub_job success, sub_job_array {sub_job_array}, {self.get_log_comm()}.")
        return True

    def add_sub_job_to_node(self, sub_job_array, nodes, priority, sub_job_name):
        for node in nodes:
            uuid = node['hostManagerResourceUuid']
            sub_job = self.build_sub_job(priority, SubJobPolicyEnum.FIXED_NODE.value, sub_job_name, uuid, uuid)
            sub_job_array.append(sub_job)

    def exec_sub_jobs(self):
        sub_job_name = ResourceParam.get_sub_job_name(self._json_param_object)
        sub_job_dict = self.backup_task_subjob_dict()
        # 进程上报备份进度
        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as err:
            log.error(f"Do {sub_job_name} fail: {err}, {self.get_log_comm()}.")
            ret = False
        if not ret:
            return False
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[self.sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self.pid,
                           SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=100,
                                         logDetail=[log_detail],
                                         taskStatus=SubJobStatusEnum.COMPLETED.value).dict(
                               by_alias=True))
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(self.pid, response.dict(by_alias=True))
        log.info(f"Do {sub_job_name} success, {self.get_log_comm()}.")
        return True

    def exec_deploy_user_sub_job(self):
        # 日志备份子任务1，记录集群的用户名
        log.info(f"Step 5-1 record tidb deploy user, {self.get_log_comm()}.")
        try:
            self.record_deploy_user()
        except Exception as err:
            log.info(f"Step 5-1 fail to record deploy user: {err}, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        log.info(f"Step 5-1 finish to record tidb deploy user, success, {self.get_log_comm()}.")
        return True

    def exec_up_log_sub_job(self):
        """
        日志备份子任务4，
        1. 删除备份失败产生的临时库
        2. 检查日志状态
        3. 检查时间
        4. 复制日志备份路径中meta信息
        """
        # 删除备份失败产生的临时库
        log.info(f"Step 5-4 start to execute sub job 4, drop temporary db, {self.get_log_comm()}.")
        ret, code = drop_db(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP,
                            TiDBDataBaseFilter.TIDB_BR_TEMP)
        if not ret:
            log.error(f"Step 5-4 drop temporary db failed, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, code,
                                          f'failed to drop {TiDBDataBaseFilter.TIDB_BR_TEMP}')
            return False
        # up检查日志状态
        log.info(f"Step 5-4 start to execute sub job 4, check log bkp time, {self.get_log_comm()}.")
        pd_id = get_status_up_role_one_host(self._cluster_name, self._tiup_path, ClusterRequiredHost.PD)
        log_status = get_log_stat_info(pd_id, self._tiup_path, TiDBConst.LOG_STATUS)
        if log_status != TiDBConst.LOG_NORMAL:
            log.error(f"Step 5-4 log backup fail, log status is abnormal, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_LOG_BKP_STATUS,
                                          'log backup failed')
            return False
        # 取日志备份路径
        log_bkp_path = get_log_path(pd_id, self._tiup_path)
        # 查日志备份时间
        log_start_ts = convert_tso_time_to_ts(get_log_stat_info(pd_id, self._tiup_path, TiDBConst.START))
        log_checkpoint_ts = convert_tso_time_to_ts(get_log_stat_info(pd_id, self._tiup_path, TiDBConst.CHECKPOINT))
        if not log_checkpoint_ts > log_start_ts:
            log.error(
                f'Step 5-4 time invalid, {self.get_log_comm()}, checkpoint time:{log_checkpoint_ts} must earlier than '
                f'log start time: {log_start_ts}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_TIME_INVALID, "invalid time")
            return False
        begin_ts, copy_start_ts, end_ts, last_bkp_ts, last_copy_id = self.get_log_info(log_start_ts, log_checkpoint_ts)
        # 检查时间，tidb中，日志归档状态回显的开始时间要早于上一次备份的时间，否则无法保证时间轴连续无空洞。
        if last_bkp_ts < log_start_ts:
            log.error(
                f'Step 5-4 time invalid, {self.get_log_comm()}, last backup time: {last_bkp_ts} is earlier than '
                f'log start time: {log_start_ts}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_TIME_INVALID, "invalid time")
            return False
        if not self.cp_files(log_bkp_path, self.log_path):
            log.error(f'Step 5-4 fail to copy files, {self.get_log_comm()}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_COPY, "copy error")
            return False
        # 记录begin_ts, end_ts, last_copy_id, copy_start_ts至cache仓
        log_bkp_file = os.path.join(self.cache_path, "log_bkp_info")
        output_execution_result_ex(log_bkp_file,
                                   {"log_bkp_path": log_bkp_path, "begin_ts": begin_ts, "end_ts": end_ts,
                                    "last_copy_id": last_copy_id, "copy_start_ts": copy_start_ts})
        return True

    def get_log_info(self, log_start_ts, log_checkpoint_ts):
        """
        获取日志备份信息

        参数:
        log_start_ts: tidb日志归档状态中回显的start（日志开始）时间戳
        log_checkpoint_ts: tidb日志归档状态中回显的checkpoint[global]（日志检查点）时间戳


        返回值:
        begin_ts: 开始时间戳， 用于完成本次备份后，上报UBC
        copy_start_ts: 备份开始时间戳
        end_ts: 结束时间戳， 用于完成本次备份后，上报UBC，作为本次日志备份副本的结束时间
        last_bkp_ts: 上一次备份时间戳
        last_copy_id: 上一次备份产生备份的ID， 用于完成本次备份后，上报UBC

        """
        # 获取上一次备份上报的副本信息
        last_bkp_info = self.get_log_backup_last_copy_info(log_start_ts)
        # 获取上一次备份的类型
        last_bkp_type = last_bkp_info.get('type', '')
        # 如果上一次备份备份类型为全量备份
        if last_bkp_type == CopyDataTypeEnum.FULL_COPY:
            # 获取上一次备份的副本ID
            last_copy_id = last_bkp_info.get('extendInfo', {}).get("copy_id", str())
            # 获取上一次备份（全量备份）的时间
            last_bkp_ts = last_bkp_info.get('extendInfo', {}).get("backup_time", 0)
            # 设置开始时间戳为tidb日志归档状态中显示的日志开始时间戳
            begin_ts = log_start_ts
        else:
            # 如果上一次备份类型不是全量备份，获取关联的副本ID
            last_copy_id = last_bkp_info.get('extendInfo', {}).get("associatedCopies", str())
            # 获取上一次备份（日志备份）的结束时间
            last_bkp_ts = last_bkp_info.get('extendInfo', {}).get("endTime", 0)
            # 设置开始时间戳为上一次备份的结束时间
            begin_ts = last_bkp_ts
        # 设置开始复制日志归档副本的时间为上一次备份的时间
        copy_start_ts = last_bkp_ts
        # 设置结束时间戳为日志检查点时间戳
        end_ts = log_checkpoint_ts
        # 返回开始时间戳、备份开始时间戳、结束时间戳、最后一次备份时间戳和最后一次备份ID
        return begin_ts, copy_start_ts, end_ts, last_bkp_ts, last_copy_id

    def exec_kf_log_sub_job(self):
        """
        日志备份子任务5，
        1. 检查日志路径
        2. 复制日志备份信息
        """
        log.info(f"Step 5-5 start to check log backup directory, {self.get_log_comm()}.")
        log_bkp_file = os.path.join(self.cache_path, "log_bkp_info")
        try:
            with open(log_bkp_file, "r", encoding='utf-8') as f:
                result = json.loads(f.readlines()[0])
        except Exception as err:
            log.error(f"Step 5-5 fail to get log backup path: {err}, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        log_bkp_path = result.get("log_bkp_path", "")
        if not os.path.exists(log_bkp_path):
            log.error(f'Step 5-5 log backup directory does not exist, {self.get_log_comm()}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_DIRECTORY,
                                          "invalid directory")
            return False
        log.info(f'Step 5-5 log backup directory exists, {self.get_log_comm()}.')
        start_ts = result.get("copy_start_ts", sys.maxsize)
        deploy_user_file = os.path.join(self.cache_path, f"bkp_{self.job_id}_deploy_user.json")
        result = self.get_json_content(deploy_user_file, "Step 5-5 fail to get deploy user")
        deploy_user = result.get("deploy_user", "")
        log_path_owner = get_path_owner(log_bkp_path)
        if log_path_owner != deploy_user:
            log.error(f'Step 5-5 wrong directory owner, {self.get_log_comm()}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_DIRECTORY, "invalid owner")
            return False
        log.info(f'Step 5-5 directory owner is correct, {self.get_log_comm()}.')
        cur_ts = int(time.time())
        if cur_ts < start_ts:
            log.error(f'Step 5-5 time invalid, current time:{cur_ts} is earlier than start copy time: {start_ts}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_TIME_INVALID, "invalid time")
            return False
        diff_ts = datetime.fromtimestamp(cur_ts) - datetime.fromtimestamp(start_ts)
        period = math.ceil(diff_ts.total_seconds() / 60) + 10
        if not self.cp_logs(log_bkp_path, self.log_path, period):
            log.error(f'Step 5-5 fail to copy log files in last {period} minutes, {self.get_log_comm()}.')
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.ERR_COPY, "copy error")
            return False
        log.info(f"Step 5-5 log files in last {period} minutes copied success, {self.get_log_comm()}.")
        return True

    def exec_check_up_sub_job(self):
        # 数据备份子任务1， 删除备份失败产生的临时库，检查库表是否存在
        log.info(f"step 5-1 drop temporary dbs, {self.get_log_comm()}.")
        if not drop_db(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP,
                       TiDBDataBaseFilter.TIDB_BR_TEMP):
            log.error(f"step 5-1 drop temporary db failed, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.BKP_DB_TAB_NOT_EXIST,
                                          f'failed to drop {TiDBDataBaseFilter.TIDB_BR_TEMP}')
            return False
        log.info(f"step 5-1 start to check database and table status, {self.get_log_comm()}.")
        cluster_dbs = get_db(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP)
        if self._bkp_gran == BackupGranEnum.DB_BACKUP.value:
            db = self._protect_obj_extend_info.get("databaseName", '')
            if db not in cluster_dbs:
                log.error(f"step 5-1 database backup fail, the target database does not exist, {self.get_log_comm()}.")
                self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.BKP_DB_TAB_NOT_EXIST,
                                              f'target database {db} does not exist')
                return False
        if self._bkp_gran == BackupGranEnum.TABLE_BACKUP.value:
            db = self._protect_obj_extend_info.get("databaseName", '')
            tabs_str = self._protect_obj_extend_info.get("tableNameList", "")
            tabs = json.loads(tabs_str)
            if db not in cluster_dbs:
                log.error(f"step 5-1 database backup fail, the target database does not exist, {self.get_log_comm()}.")
                self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.BKP_DB_TAB_NOT_EXIST,
                                              f"target database {db} does not exist")
                return False
            for tab in tabs:
                if tab not in get_table(self.pid, self._cluster_name, self._tiup_path, db, JobNameConst.BACKUP):
                    log.info(f"step 5-1 database backup fail, the target table does not exist, {self.get_log_comm()}.")
                    self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.BKP_DB_TAB_NOT_EXIST,
                                                  f"target table {tab} does not exist")
                    return False
        # 记录集群的用户名
        try:
            self.record_deploy_user()
        except Exception as err:
            log.info(f"Step 5-1 fail to record deploy user: {err}, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        log.info(f"Step 5-1 check database and table, record tidb deploy user success, {self.get_log_comm()}.")
        return True

    def exec_record_uid_sub_job(self):
        # 数据备份子任务2， 日志备份子任务2， 记录tiup节点，kv节点，flash节点的uuid
        log.info(f"Step 5-2 record uid of tikv, tiflash, {self.get_log_comm()}.")
        deploy_user_file = os.path.join(self.cache_path, f"bkp_{self.job_id}_deploy_user.json")
        result = self.get_json_content(deploy_user_file, "Step 5-2 fail to get deploy user")
        deploy_user = result.get("deploy_user", "")
        if not deploy_user:
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        log.info(f"Step 5-2 get deploy user success, {self.get_log_comm()}.")
        # 记录集群用户的id
        try:
            uid = pwd.getpwnam(deploy_user).pw_uid
            log.info(uid)
        except Exception as err:
            log.error(f"Step 5-2 Failed get uid err: {err}, {self.get_log_comm()}.")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        userid_path = os.path.join(self.cache_path, f"bkp_{self.job_id}_uid.json")
        create_file_append(userid_path, {"uid": uid})
        log.info(f"Step 5-2 record uid success, {self.get_log_comm()}.")
        return True

    def exec_check_uid_sub_job(self):
        # 数据备份子任务3，日志备份子任务3，检查kv节点，flash节点的uuid
        log.info(f"Step 5-3 check uid, {self.get_log_comm()}.")
        uids = get_uids(self.cache_path, f'bkp_{self.job_id}_uid')
        if len(uids) > 1:
            log.error(f"Step 5-3 uid check failed! {self.get_log_comm()}")
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.UID_INCONSISTENCY,
                                          "check uuid failed")
            return False
        log.info(f"Step 5-3 uid check success! {self.get_log_comm()}.")
        return True

    def exec_create_sub_job(self):
        # 数据备份子任务4， 新建数据备份路径
        log.info(f"Step 5-4 create data backup path, {self.get_log_comm()}.")
        deploy_user_file = os.path.join(self.cache_path, f"bkp_{self.job_id}_deploy_user.json")
        result = self.get_json_content(deploy_user_file, "Step 5-4 fail to get deploy user")
        deploy_user = result.get("deploy_user", "")
        if not deploy_user:
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            return False
        log.info(f"Step 5-4 get deploy user success, {self.get_log_comm()}.")
        data_bkp_path = os.path.join(self.data_path, f"tidb_{self.job_id}")
        create_dir_recursive(data_bkp_path)
        change_path_permission(data_bkp_path, deploy_user,
                               stat.S_IRWXU | stat.S_IXGRP | stat.S_IRGRP | stat.S_IXOTH | stat.S_IROTH)
        log.info(f"Step 5-4 create data backup path success, {self.get_log_comm()}.")
        return True

    def exec_backup_sub_job(self):
        # 数据备份子任务5， 执行数据备份
        log.info(f"Step 5-5 start to execute data backup, {self.get_log_comm()}.")
        if not self.exec_data_bkp():
            return False
        log.info(f'Step 5-5:finish to execute data backup, {self.get_log_comm()}.')
        return True

    def exec_record_sub_job(self):
        # 数据备份子任务6，记录备份的集群以及库表信息，保存为json格式，rpc上报, 将库表信息写入sqlite
        log.info(f"Step 5-6 start to record databackup result, {self.get_log_comm()}.")
        clu_db_tab_info_list = []
        if self._bkp_gran == BackupGranEnum.CLUSTER_BACKUP.value:
            db_list = get_db(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP)
            for db in db_list:
                table_list = get_table(self.pid, self._cluster_name, self._tiup_path, db, JobNameConst.BACKUP)
                bkp_info = TidbBkpResultInfo(copy_id=self._copy_id, job_id=self.job_id,
                                             cluster_name=self._cluster_name, db=db, tables=table_list).dict(
                    by_alias=True)
                clu_db_tab_info_list.append(bkp_info)
        elif self._bkp_gran == BackupGranEnum.DB_BACKUP.value:
            db = self._protect_obj_extend_info.get("databaseName", '')
            table_list = get_table(self.pid, self._cluster_name, self._tiup_path, db, JobNameConst.BACKUP)
            bkp_info = TidbBkpResultInfo(copy_id=self._copy_id, job_id=self.job_id, cluster_name=self._cluster_name,
                                         db=db, tables=table_list).dict(by_alias=True)
            clu_db_tab_info_list.append(bkp_info)
        elif self._bkp_gran == BackupGranEnum.TABLE_BACKUP.value:
            db = self._protect_obj_extend_info.get("databaseName", '')
            tabs_str = self._protect_obj_extend_info.get("tableNameList", "")
            tabs = json.loads(tabs_str)
            bkp_info = TidbBkpResultInfo(copy_id=self._copy_id, job_id=self.job_id, cluster_name=self._cluster_name,
                                         db=db, tables=tabs).dict(by_alias=True)
            clu_db_tab_info_list.append(bkp_info)
        output_file = os.path.join(self._meta_path, "tidb_bkp_info")
        if os.path.exists(output_file):
            os.remove(output_file)
        output_execution_result_ex(output_file, clu_db_tab_info_list)
        log.info(f"Step 5-6 finish to record cluster, database, table, success, {self.get_log_comm()}.")
        # rpc上报
        result, cmd, std_err = self.report_databackup_result_via_rpc()
        if not result:
            log.error(f"Step 5-6 fail to report backup time, {self.get_log_comm()}.")
            self.report_error(cmd, std_err)
            return False
        log.info(f"Step 5-6 finish to report backup time via rpc, success, {self.get_log_comm()}.")
        # 将库表信息写入sqlite
        total_size = str(get_db_total_size(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP))
        TidbSqliteService.write_metadata_to_sqlite_file(self._meta_path, self._cluster_name,
                                                        SqliteServiceField.TYPE_CLUSTER.value, "/", total_size)
        db_size_list = get_db_size(self.pid, self._cluster_name, self._tiup_path, JobNameConst.BACKUP)
        for db_size in db_size_list:
            TidbSqliteService.write_metadata_to_sqlite_file(self._meta_path, db_size[0],
                                                            SqliteServiceField.TYPE_DATABASE.value,
                                                            "/" + self._cluster_name, db_size[1])
            tab_size_list = get_tab_size(self.pid, self._cluster_name, self._tiup_path, db_size[0],
                                         JobNameConst.BACKUP)
            for tab_size in tab_size_list:
                TidbSqliteService.write_metadata_to_sqlite_file(self._meta_path, tab_size[0],
                                                                SqliteServiceField.TYPE_TABLE.value,
                                                                f"/{self._cluster_name}/{db_size[0]}", tab_size[1])
        log.info(f"Step 5-6 finish to write metadata to sqlite, success, {self.get_log_comm()}.")
        return True

    def report_error(self, cmd, std_err):
        if cmd:
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR, ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                          std_err, cmd=cmd)
        else:
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)

    def report_backup_process(self, original_size, process, path):
        time_file = os.path.join(self.cache_path, f'T{self.job_id}')
        size, speed = self.query_size_and_speed(time_file, original_size, path)
        process.progress = 50
        process.speed = speed
        report_job_details(self.pid, process.dict(by_alias=True))
        return True

    def query_size_and_speed(self, time_file, original_size, path):
        size = 0
        speed = 0
        if os.path.islink(time_file):
            return size, speed
        if not os.path.exists(time_file):
            return size, speed
        with open(time_file, "r", encoding='UTF-8') as file:
            start_time = file.read().strip()
        ret, data_size = scan_dir_size(self.job_id, path)
        if ret:
            size = data_size
        new_time = int((time.time()))
        try:
            speed = int((size - original_size) / (new_time - int(start_time)))
        except Exception:
            log.error(f"Error while calculating speed! time difference is 0! {self.get_log_comm()}.")
            return 0, 0
        return size, speed

    def do_post_job(self):
        if self.backup_type == BackupTypeEnum.LOG_BACKUP:
            self.report_copy_info_binlog()
        return True

    def get_log_backup_last_copy_info(self, log_start_ts):
        """
        以上次日志备份作为起点，如果找不到上一次日志备份，则以上一次数据备份作为起点，
        通过上次日志备份的开始时间和本次日志的开始时间，判断两者是否为同一次日志备份，
        如果不是同一次日志备份，以上一次数据备份为起点
        :log_start_ts, 本次日志备份的开始时间
        :return: dict
        """
        log.info(f"start get_log_backup_last_copy_info, {self.get_log_comm()}.")
        last_copy_info = self.get_last_copy_info(2)
        if not last_copy_info:
            log.warning(f"This is the first log backup, {self.get_log_comm()}.")
            last_copy_info = self.get_last_copy_info(1)
        else:
            last_log_start_ts = last_copy_info.get('extendInfo', {}).get('beginTime', 0)
            if last_log_start_ts < log_start_ts:
                log.warning(f"This is a different log backup, {self.get_log_comm()}.")
                last_copy_info = self.get_last_copy_info(1)
        log.info(f"last copy info is {last_copy_info}, {self.get_log_comm()}.")
        return last_copy_info

    def get_last_copy_info(self, copy_type: int):
        log.info(f"start get_last_copy_info, {self.get_log_comm()}.")
        last_copy_type = LastCopyType.last_copy_type_dict.get(copy_type)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self.job_id
        }
        try:
            result = exec_rc_tool_cmd(self.job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get any last copy info fail.{err_info}, {self.get_log_comm()}.")
            return {}
        log.info(f"Get_last_any_copy_info: {result}, {self.get_log_comm()}.")
        return result

    def report_copy_info_binlog(self):
        log_bkp_file = os.path.join(self.cache_path, "log_bkp_info")
        try:
            with open(log_bkp_file, "r", encoding='utf-8') as f:
                result = json.loads(f.readlines()[0])
        except Exception as err:
            log.error(f"step 6 fail to get log_bkp_info: {err}")
            return False
        begin_ts = result["begin_ts"]
        end_ts = result["end_ts"]
        last_copy_id = result["last_copy_id"]
        out_put_info = {
            "extendInfo": {
                "backupTime": begin_ts,
                "beginTime": begin_ts,
                "endTime": end_ts,
                "beginSCN": None,
                "endSCN": None,
                "backupset_dir": '',
                "backupSetName": '',
                "backupType": '',
                "baseBackupSetName": "",
                "dbName": "",
                "groupId": '',
                "tabal_space_info": [],
                "associatedCopies": last_copy_id,
                "logDirName": self.log_path
            }
        }
        copy_info = {"copy": out_put_info, "jobId": self.job_id}
        try:
            exec_rc_tool_cmd(self.job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Step 6 report copy info fail.err: {err_info}, {self.get_log_comm()}.")
            return False
        log.info(f"Step 6 report copy info succ {copy_info}, {self.get_log_comm()}.")
        return True

    def report_databackup_result_via_rpc(self):
        pd_id = get_status_up_role_one_host(self._cluster_name, self._tiup_path, ClusterRequiredHost.PD)
        data_bkp_path = os.path.join(self.data_path, f"tidb_{self.job_id}")
        backup_tso, br_cmd, std_err = get_backup_tso_validate(self._tiup_path, data_bkp_path)
        if not backup_tso:
            log.error(f'Failed to get backup time, {self.get_log_comm()}.')
            return False, br_cmd, std_err
        backup_time, ctl_cmd, std_err = parse_tso_to_time(self._tiup_path, pd_id, backup_tso)
        if not backup_time:
            log.error(f'Failed to parse tso to time, {self.get_log_comm()}.')
            return False, ctl_cmd, std_err
        backup_ts = convert_tso_time_to_ts(backup_time)
        bkp_info = {"extendInfo": {"copy_id": self._copy_id, "backup_time": backup_ts}}
        copy_info = {"copy": bkp_info, "jobId": self.job_id}
        invoke_rpc_tool_interface(self.job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        return True, '', ''

    def report_error_job_details(self, response_code, err_code=0, message='', log_info="plugin_task_subjob_fail_label",
                                 cmd='Backup'):
        response = ActionResult(code=response_code, bodyErr=err_code, message=message)
        log.info(f'report_sub_job_details: {response}, {self.get_log_comm()}.')
        log_detail_param = [cmd, message]
        log_detail = LogDetail(logInfo=log_info, logInfoParam=[self.sub_job_id],
                               logLevel=DBLogLevel.ERROR.value, logDetail=err_code, logDetailParam=log_detail_param)
        log.info(f'log_detail: {log_detail}, {self.get_log_comm()}.')
        sub_job_details = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=100,
                                        logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value)
        log.info(f'sub_job_details: {sub_job_details}, {self.get_log_comm()}.')
        report_job_details(self.pid, sub_job_details.dict(by_alias=True))
        output_result_file(self.pid, response.dict(by_alias=True))

    def get_json_content(self, path, err_msg="Read json file failed."):
        """
        读取json文件内容
        :path : 文件路径
        :err_msg: 报错信息
        :return: str
        """
        try:
            with open(path, "r", encoding='utf-8') as f:
                result = json.loads(f.readlines()[0])
        except Exception as err:
            self.report_error_job_details(ExecuteResultEnum.INTERNAL_ERROR)
            raise Exception(f"{err_msg}") from err
        return result

    @BackupDecorator.report_speed
    def exec_data_bkp(self):
        # 运行数据备份指令
        db_tab_info = ''
        bkp_type = ''
        if self._bkp_gran == BackupGranEnum.DB_BACKUP.value:
            bkp_type = 'db'
            db = self._protect_obj_extend_info.get("databaseName", '')
            db_tab_info = '--db ' + db
        elif self._bkp_gran == BackupGranEnum.TABLE_BACKUP.value:
            bkp_type = 'table'
            db = self._protect_obj_extend_info.get("databaseName", '')
            tabs_str = self._protect_obj_extend_info.get("tableNameList", "")
            tabs = json.loads(tabs_str)
            if len(tabs) == 1:
                db_tab_info = f'--db {db} --table {tabs[0]}'
            if len(tabs) > 1:
                bkp_type = 'full'
                db_tab_info = ''
                for tab in tabs:
                    db_tab_info += f"--filter '{db}.{tab}' "
                db_tab_info = db_tab_info.rstrip()
        elif self._bkp_gran == BackupGranEnum.CLUSTER_BACKUP.value:
            bkp_type = 'full'
        pd_id = get_status_up_role_one_host(self._cluster_name, self._tiup_path, ClusterRequiredHost.PD)
        data_bkp_path = os.path.join(self.data_path, f"tidb_{self.job_id}")
        if not check_paths_valid(data_bkp_path):
            log.error(f"The data_bkp_path {data_bkp_path} verification fails")
            return "", "", False
        br_log_path = os.path.join(self.cache_path, f"{self.job_id}_br.log")
        self.delete_directory(data_bkp_path)
        backup_cmd = f"{self._tiup_path} br backup {bkp_type} {db_tab_info} --pd {pd_id} -s {data_bkp_path}" \
                     f" --check-requirements=true --log-file {br_log_path}"
        rate_limit = self._json_param_object.get("job", {}).get("extendInfo", {}).get("rate_limit")
        if rate_limit:
            backup_cmd = f"{backup_cmd} --ratelimit {rate_limit}"
        backup_cmd = f"su - {get_path_owner(self._tiup_path)} -c '{backup_cmd}'"
        return_code, out_info, std_err = execute_cmd_with_expect(backup_cmd, "", None)
        log.info(f'result: {(return_code, out_info)}')
        if return_code != CMDResultInt.SUCCESS:
            log.error(f"Step 5-5 bkp data err, cmd:{backup_cmd}, out:{out_info}, err:{std_err}, {self.get_log_comm()}.")
            return return_code, out_info, False
        return return_code, out_info, True

    @BackupDecorator.report_speed
    def cp_files(self, path, des_path):
        src_path = os.path.join(path, '.')
        cp_cmd = f'cp -ar {src_path}  {des_path}'
        return_code, out_info, std_err = execute_cmd(cp_cmd)
        if return_code != CMDResult.SUCCESS:
            log.error(f'Step 5-4 execute copy cmd failed, message: {out_info}, err: {std_err}, {self.get_log_comm()}.')
            return return_code, out_info, False
        return return_code, out_info, True

    @BackupDecorator.report_speed
    def cp_logs(self, src_path, des_path, period):
        agent_uuid = self._json_param_object.get("subJob", "").get("jobInfo", "")
        des_path = os.path.join(des_path, agent_uuid)
        if not os.path.exists(des_path):
            os.makedirs(des_path)
        log.info(f"Step 5-5 cp_logs src_path is {src_path}, des_path is {des_path}, {self.get_log_comm()}.")
        os.chdir(src_path)
        find_cmd = f"find . -mmin -{period} -type f"
        grep_cmd = "grep -v '.tmp'"
        cp_cmd = "xargs -i /bin/cp --parents -r {} " + des_path
        cp_cmd_list = [find_cmd, grep_cmd, cp_cmd]
        return_code, out_info, std_err = execute_cmd_list(cp_cmd_list)
        if return_code != CMDResult.SUCCESS:
            log.error(f'Step 5-5 execute copy cmd failed, message: {out_info}, err: {std_err}, {self.get_log_comm()}.')
            return return_code, out_info, False
        deploy_user_file = os.path.join(self.cache_path, f"bkp_{self.job_id}_deploy_user.json")
        result = self.get_json_content(deploy_user_file, "Step 5-5 fail to get deploy user")
        deploy_user = result.get("deploy_user", "")
        if not deploy_user:
            return "", "", False
        log.info(f"Step 5-5 get deploy user success, {self.get_log_comm()}.")
        change_owner_by_name(des_path, deploy_user, deploy_user)
        change_mod(des_path, stat.S_IRWXU | stat.S_IXGRP | stat.S_IRGRP | stat.S_IXOTH | stat.S_IROTH)
        for root, dirs, files in os.walk(des_path):
            # 修改子路径权限
            for directory in dirs:
                change_mod(os.path.join(root, directory),
                           stat.S_IRWXU | stat.S_IXGRP | stat.S_IRGRP | stat.S_IXOTH | stat.S_IROTH)
                change_owner_by_name(os.path.join(root, directory), deploy_user, deploy_user)

            # 修改文件权限
            for file in files:
                change_mod(os.path.join(root, file),
                           stat.S_IRWXU | stat.S_IXGRP | stat.S_IRGRP | stat.S_IXOTH | stat.S_IROTH)
                change_owner_by_name(os.path.join(root, file), deploy_user, deploy_user)
        log.info(f"Step 5-5 permission changed success, {self.get_log_comm()}.")
        return return_code, out_info, True
