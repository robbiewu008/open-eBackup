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
import json
import math
import os
import random
import threading
import pwd
import time
import datetime

from common.common import execute_cmd, invoke_rpc_tool_interface, execute_cmd_list
from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.util.exec_utils import exec_mkdir_cmd
from common.const import ParamConstant, SubJobStatusEnum, RepositoryDataTypeEnum, BackupTypeEnum, ReportDBLabel, \
    CMDResult, RpcParamKey, SubJobPolicyEnum, SubJobTypeEnum, DBLogLevel, SubJobPriorityEnum, PathConstant
from common.parse_parafile import get_env_variable
from common.util.scanner_utils import scan_dir_size
from tdsql.common.const import TdsqlSubJobName, TdsqlBackupStatus, LastCopyType, ErrorCode, \
    EnvNameValue, MountType
from tdsql.common.safe_get_information import ResourceParam
from tdsql.common.tdsql_common import output_execution_result_ex, report_job_details, write_file, mount_bind_path, \
    umount_bind_path, get_std_in_variable, get_remote_host_ips
from tdsql.handle.common.const import TDSQLGroupBackupTaskStatus, TDSQLReportLabel, TDSQLProtectKeyName
from tdsql.handle.common.tdsql_restful import request_post
from tdsql.handle.requests.rest_requests import RestRequests
from tdsql.logger import log


class TdsqlGroupBackUp:

    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        self._std_in = data
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param_object = json_param
        self._copy_id = self._json_param_object.get("job", {}).get("copy", [])[0].get("id", "")
        self._repositories = self._json_param_object.get("job", {}).get("repositories", [])
        self._logdetail = None
        self._query_progress_interval = 15
        self._instance_id = self._json_param_object.get("job", {}).get("protectObject", {}).get("id", "")
        self._protect_obj_extend_info = self._json_param_object.get("job", {}).get("protectObject", {}).get(
            "extendInfo", {})
        self._extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self._cluster_group_info_str = self._protect_obj_extend_info.get("clusterGroupInfo")
        self._cluster_group_info_ = json.loads(self._cluster_group_info_str)
        self.nodes = self._cluster_group_info_.get("group").get("dataNodes")
        log.info(f"tdsql_group_backup nodes: {self.nodes}")
        self._group_id = self._cluster_group_info_.get("id")
        self._set_ids = self._cluster_group_info_.get("group").get("setIds")
        log.info(f"tdsql_group_backup set_ids: {self._set_ids}")
        self._sub_job_name = ""
        self._cache_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self._data_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._meta_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.META_REPOSITORY)
        self._log_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.LOG_REPOSITORY)
        self._job_status = SubJobStatusEnum.RUNNING
        self._backup_status = TdsqlBackupStatus.RUNNING
        self._backup_start_time = 0
        self._backup_end_time = 0
        self._max_data_size = 0
        self._err_code = 0
        self._mysql_version = ''
        self._mount_type = self._extend_info.get("agentMountType")
        self._osad_ip_list = get_remote_host_ips(self._repositories)
        self._osad_auth_port = get_std_in_variable(f"{EnvNameValue.IAM_OSADAUTHPORT}_{self._pid}")
        self._osad_server_port = self._extend_info.get("OSADServerPort", "")
        # 组装资源接入请求体
        self._oss_nodes = self.get_oss_nodes()
        self.backup_type = self._json_param_object.get("job", {}).get("jobParam", {}).get("backupType")
        log.debug(f"tdsql_group_backup json_param_object: {json_param}")

    @staticmethod
    def get_repository_path(file_content, repository_type):
        repositories = file_content.get("job", {}).get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_path = repository["path"][0]
                break
        return repositories_path

    @staticmethod
    def query_size_and_speed_binlog(time_file, data_file, data_path, job_id):
        size = 0
        speed = 0
        # 读取初始时间
        if os.path.islink(time_file):
            log.error(f"Link file:{time_file},stop writing.")
            return size, speed
        if not os.path.exists(time_file):
            log.error(f"Time file: {time_file} not exist")
            return size, speed
        log.info('Time path exist')
        with open(time_file, "r", encoding='UTF-8') as time_file:
            start_time = time_file.read().strip()

        # 读取初始大小
        if os.path.islink(data_file):
            log.error(f"Link file:{data_file},stop writing.")
            return size, speed
        if not os.path.exists(data_file):
            log.error(f"Time file: {data_file}, not exist")
            return size, speed
        log.info('data path exist')
        with open(data_file, "r", encoding='UTF-8') as data_file:
            original_data_size = data_file.read().strip()
        log.info(f"query_size_and_speed, start_time: {start_time}, original_data_size: {original_data_size}")

        _, size = scan_dir_size(job_id, data_path)
        new_time = int((time.time()))
        datadiff = int((size - int(original_data_size)))
        timediff = new_time - int(start_time)
        if timediff == 0:
            log.info(f"query_size_and_speed, timediff is {timediff}")
            return datadiff, speed
        try:
            speed = datadiff / timediff
        except Exception:
            log.error("Error while calculating speed! time difference is 0!")
            return 0, 0
        log.info(f"query_size_and_speed, datadiff: {datadiff}, timediff: {timediff}, speed: {speed}")
        return datadiff, speed

    @staticmethod
    def chown_backup_path_owner(backup_path):
        stat_info = os.stat(backup_path)
        user = pwd.getpwuid(stat_info.st_uid)[0]
        if user == "tdsql":
            log.info(f"chown_backup_path_owner already tdsql")
            return
        os.lchown(backup_path, pwd.getpwnam("tdsql").pw_uid, pwd.getpwnam("tdsql").pw_gid)
        log.info(f"chown_backup_path_owner to tdsql")
        return

    @staticmethod
    def mount_bind_tdsqlbackup():
        # 创建/tdsqlbackup 目录
        tdsqlbackup_path = "/tdsqlbackup"
        if not os.path.exists(tdsqlbackup_path):
            log.info(f"/tdsqlbackup not exit")
            os.makedirs(tdsqlbackup_path)
        # 判断/tdsqlbackup目录是否已挂载
        cmd_list = ["mount", "grep tdsqlbackup", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if return_code == CMDResult.SUCCESS and int(out_info) > 0:
            log.info(f"/tdsqlbackup already mount bind")
            # 已挂载，执行cd /tdsqlbackup
            return_code, out_info, err_info = execute_cmd(f"cd {tdsqlbackup_path}")
            if return_code != CMDResult.SUCCESS:
                # 执行cd /tdsqlbackup 异常，解挂载后重新挂载
                if not mount_bind_path("/mnt", tdsqlbackup_path):
                    log.error(f"mount_bind_tdsqlbackup re mount_bind_path failed")
                    return False
        else:
            # 没挂载,执行mount --bind /mnt /tdsqlbackup
            if not mount_bind_path("/mnt", "/tdsqlbackup"):
                log.error(f"mount_bind_tdsqlbackup mount_bind_path failed")
                return False
        return True

    def get_oss_nodes(self):
        extend_info = self._json_param_object.get("job", {}).get("protectEnv", {}).get("extendInfo", {})
        oss_nodes = (json.loads(extend_info.get("clusterInfo", {}))).get("ossNodes", [])
        oss_node_ips = []
        for oss_node in oss_nodes:
            oss_node_ips.append(oss_node.get("ip"))
        log.info(f"oss_node_ips {oss_node_ips}")
        return oss_node_ips

    def build_sub_job(self, job_priority, policy, job_name, node_id, job_info):
        return SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=policy, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        if self.backup_type == BackupTypeEnum.LOG_BACKUP:
            self.gen_sub_job_binlog()
        else:
            self.gen_sub_job_data()

    def gen_sub_job_binlog(self):
        # 日志备份生成子任务
        log.info("step 2-4 start to gen_sub_job for binlog backup")

        nodes = self.nodes
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        # 随机选取一个节点，执行日志备份
        log.info("step 2-4 binlog based on full backup copy")
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_GROUP_BINLOG
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        choice_node = random.choice(nodes)
        node_id = choice_node.get("parentUuid", "")
        host = choice_node.get("ip", "")
        job_info = f"{host}"
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
        sub_job_array.append(sub_job)

        log.info(f"gen_sub_job get sub_job_array: {sub_job_array}")
        log.info(f"step2-4 Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        log.info("step2-4 end to gen_sub_job for binlog backup")
        return True

    def gen_sub_job_data(self):
        log.info("step 2-4 start to gen_sub_job for data backup")
        nodes = self.nodes
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        # 子任务1：所有数据节点挂载 /tdsqlbackup目录
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_GROUP_MOUNT_BIND
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        for node in nodes:
            node_id = node.get("parentUuid", "")
            host = node.get("ip", "")
            job_info = f"{host}"
            sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        # 子任务2：随机选取一个节点，执行数据备份
        log.info("start to gen sub_exec")
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_GROUP_EXEC
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        choice_node = random.choice(nodes)
        node_id = choice_node.get("parentUuid", "")
        host = choice_node.get("ip", "")
        job_info = f"{host}"
        sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
        sub_job_array.append(sub_job)

        log.info(f"gen_sub_job get sub_job_array: {sub_job_array}")
        log.info(f"step 2-4 Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        log.info("step 2-4 end to gen_sub_job for data backup")
        return True

    def query_size_and_speed(self, time_file, data_path, original_size_file, job_id):
        size = 0
        speed = 0
        with open(original_size_file, "r", encoding='UTF-8') as file:
            original_size = file.read().strip()
        if not original_size:
            original_size = 0
        if os.path.islink(time_file):
            log.error(f"Link file:{time_file},stop writing.")
            return size, speed
        if not os.path.exists(time_file):
            log.error(f"Time file: {time_file} not exist")
            return size, speed
        log.info('Time path exist')
        with open(time_file, "r", encoding='UTF-8') as file:
            start_time = file.read().strip()
        ret, data_size = scan_dir_size(job_id, data_path)
        if ret:
            if data_size > self._max_data_size:
                self._max_data_size = data_size
            else:
                data_size = self._max_data_size
            size = data_size - int(original_size)

        new_time = int((time.time()))
        if size == 0:
            write_file(time_file, str(new_time))
            return 0, 0
        try:
            speed = int((data_size - int(original_size)) / (new_time - int(start_time)))
        except Exception:
            log.error("Error while calculating speed! time difference is 0!")
            return 0, 0
        return size, speed

    def set_log_detail_with_params(self, log_label, sub_job_id, err_code=None, log_detail_param=None,
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

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def write_progress_file(self, task_status, progress):
        log.info("start write_progress_file")
        if task_status == SubJobStatusEnum.FAILED.value:
            log_detail_param = []
            if self._sub_job_name == TdsqlSubJobName.SUB_GROUP_EXEC \
                    or self._sub_job_name == TdsqlSubJobName.SUB_GROUP_BINLOG:
                self._err_code = None
                log_detail_param.append(self._instance_id)
                log.info(f"start self._sub_job_name: {self._sub_job_name}")
            log.info(f"start write_progress_file self._logdetail: {self._logdetail}")
            if not self._logdetail:
                self.set_log_detail_with_params(ReportDBLabel.SUB_JOB_FALIED, self._sub_job_id, self._err_code,
                                                log_detail_param,
                                                DBLogLevel.ERROR.value)
        if task_status == SubJobStatusEnum.COMPLETED.value:
            self.set_log_detail_with_params(ReportDBLabel.SUB_JOB_SUCCESS, self._sub_job_id, 0, [],
                                            DBLogLevel.INFO.value)
            log.info("task_status == SubJobStatusEnum.COMPLETED.value")

        # 获取速度，数据量大小
        if self.backup_type == BackupTypeEnum.FULL_BACKUP or self.backup_type == BackupTypeEnum.INCRE_BACKUP:
            log.info(f"Data backup :query backup speed and size")
            time_file = os.path.join(self._cache_area, f'T{self._job_id}')
            original_size_file = os.path.join(self._cache_area, f'D{self._job_id}')
            size, speed = self.query_size_and_speed(time_file, self._data_area, original_size_file, self._job_id)
            log.info(f"Data backup :query  backup speed {speed}, and size {size}")
        elif self.backup_type == BackupTypeEnum.LOG_BACKUP:
            log.info(f"Binlog backup :query backup speed and size")
            time_file = os.path.join(self._cache_area, f'T{self._job_id}')
            data_file = os.path.join(self._cache_area, f'D{self._job_id}')
            path_all = os.path.abspath(os.path.join(self._log_area, "..", ".."))
            size, speed = self.query_size_and_speed_binlog(time_file, data_file, path_all, self._job_id)
        progress_str = SubJobDetails(taskId=self._job_id,
                                     subTaskId=self._sub_job_id,
                                     taskStatus=task_status,
                                     progress=progress,
                                     dataSize=size,
                                     speed=speed,
                                     logDetail=self._logdetail)
        json_str = progress_str.dict(by_alias=True)
        if self._sub_job_name == TdsqlSubJobName.SUB_GROUP_EXEC and task_status == SubJobStatusEnum.RUNNING.value \
                and self._backup_status == TdsqlBackupStatus.FAILED:
            log.error(f"task_status is running but backup_status is failed")
            return
        progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
        log.debug(f"Write file.{progress_str}{self.get_log_comm()}.")
        output_execution_result_ex(progress_file, json_str)

    def upload_backup_progress(self):
        # 定时上报备份进度
        while self._job_status == SubJobStatusEnum.RUNNING:
            if self._sub_job_name == TdsqlSubJobName.SUB_GROUP_EXEC:
                if self._backup_status == TdsqlBackupStatus.RUNNING:
                    progress = 20
                    status = SubJobStatusEnum.RUNNING
                elif self._backup_status == TdsqlBackupStatus.SUCCEED:
                    status = SubJobStatusEnum.COMPLETED
                    progress = 100
                else:
                    status = SubJobStatusEnum.FAILED
                    progress = 0
                log.info(f"progress_info.job(status): {self._backup_status}")
                log.info(f"status：{status}   progress: {progress}")
                self.write_progress_file(status, progress)

            log.info("Start to report progress.")
            progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
            # 没有进度文件可能是还没有生成,不返回失败
            comm_progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                               taskStatus=SubJobStatusEnum.RUNNING,
                                               progress=0, logDetail=self._logdetail)
            if not os.path.exists(progress_file):
                report_job_details(self._job_id, comm_progress_dict.dict(by_alias=True))
                time.sleep(self._query_progress_interval)
                continue
            with open(progress_file, "r") as f_object:
                progress_dict = json.loads(f_object.read())

            self._job_status = progress_dict.get("taskStatus")
            log.info(f"Get progress_dict in upload_backup_progress.{self._job_status}")
            log.info(f"upload_backup_progress{self.get_log_comm()}")
            if not self._job_status:
                log.error(f"Failed to obtain the task status.{self.get_log_comm()}")
                self._job_status = SubJobStatusEnum.FAILED
                fail_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.FAILED, progress=100,
                                          logDetail=self._logdetail)
                progress_dict = fail_dict.dict(by_alias=True)
            log.info(f"progress_dict{progress_dict}")

            time.sleep(self._query_progress_interval)
            report_job_details(self._job_id, progress_dict)

    def backup_task_subjob_dict(self):
        sub_job_dict = {
            TdsqlSubJobName.SUB_GROUP_MOUNT_BIND: self.sub_job_mount_bind,
            TdsqlSubJobName.SUB_GROUP_BINLOG: self.sub_job_binlog,
            TdsqlSubJobName.SUB_GROUP_EXEC: self.sub_job_exec
        }
        return sub_job_dict

    def backup_task(self):
        if not self.get_db_version():
            log.error(f"get_db_version failed")
            return False

        if self.backup_type == BackupTypeEnum.LOG_BACKUP:
            self.sub_job_pre_binlog()
        else:
            self.sub_job_pre()

        self.write_progress_file(SubJobStatusEnum.RUNNING, 0)
        # 启动一个线程查询备份进度
        sub_job_dict = self.backup_task_subjob_dict()
        progress_thread = threading.Thread(name='pre_progress', target=self.upload_backup_progress)
        progress_thread.daemon = True
        progress_thread.start()
        # 执行子任务
        sub_job_name = ResourceParam.get_group_sub_job_name(self._json_param_object)
        if not sub_job_name:
            return False
        self._sub_job_name = sub_job_name

        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as err:
            log.error(f"do {sub_job_name} fail: {err}")
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail_param = []
            if sub_job_name == TdsqlSubJobName.SUB_GROUP_EXEC or sub_job_name == TdsqlSubJobName.SUB_GROUP_BINLOG:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            return False
        if not ret:
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail_param = []
            if sub_job_name == TdsqlSubJobName.SUB_GROUP_EXEC or sub_job_name == TdsqlSubJobName.SUB_GROUP_BINLOG:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            return False

        progress_thread.join()
        return True

    def sub_job_pre_binlog(self):
        start_time = str(int((time.time())))
        start_time_path = os.path.join(self._cache_area, f'T{self._job_id}')
        write_file(start_time_path, start_time)
        log.info(f"binlog success to write backup start time {start_time} to {start_time_path}")
        path_all = os.path.abspath(os.path.join(self._log_area, "..", ".."))
        # 录入初始仓库大小
        _, original_size = scan_dir_size(self._job_id, path_all)
        original_data_size_path = os.path.join(self._cache_area, f'D{self._job_id}')
        write_file(original_data_size_path, str(original_size))
        log.info(f"binlog success to write backup start data size {original_size} to {original_data_size_path}")

    def sub_job_pre(self):
        # 录入备份开始时间
        curr_time = int((time.time()))
        start_time = str(curr_time)
        self._backup_start_time = curr_time
        log.info(f"start_time: {start_time}")
        start_time_file = os.path.join(self._cache_area, f'T{self._job_id}')
        if not os.path.isfile(start_time_file):
            write_file(start_time_file, start_time)
            log.info(f"success to write backup start time {start_time} to {start_time_file}")
        # 录入初始仓库大小
        original_data_size_path = os.path.join(self._cache_area, f'D{self._job_id}')
        if not os.path.isfile(original_data_size_path):
            original_size = 0
            ret, size = scan_dir_size(self._job_id, self._data_area)
            if ret:
                original_size = size
                self._max_data_size = original_size
            write_file(original_data_size_path, str(original_size))
            log.info(f"success to write backup start data size {original_size} to {original_data_size_path}")
            log.info("step2-6 start to sub_job_pre")

    def exec_close_xtrabackup_cmd(self):
        # 首次备份关闭物理备份
        user = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        passwd = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHPWD + self._pid)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.ModifyBackupChoose",
                "para": {
                    "groupid": self._group_id,
                    "id": "",
                    "xtrabackup": 0,
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        executed_nodes = []
        oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        while oss_url:
            executed_nodes.append(oss_url)
            log.info(f'exec_close_xtrabackup_cmd executed_nodes : {executed_nodes}')
            # 调用oss关闭物理备份接口,接口失败重试3次，每次间隔3s
            retry_nums = 0
            while retry_nums < 3:
                retry_nums += 1
                if retry_nums != 1:
                    time.sleep(3)
                ret, ret_body, ret_header = request_post(oss_url, request_body, request_header)
                log.info(f'exec_close_xtrabackup_cmd ret_body : {ret_body}, retry_nums:  {retry_nums}')
                if not ret:
                    log.error(f'Failed exec_close_xtrabackup_cmd, request_body is : {request_body}')
                    continue
                if ret_body.get("returnMsg") != "ok":
                    log.error(f'exec_close_xtrabackup_cmd error with return: {ret_body.get("returnMsg")}')
                    continue
                return True
            oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        return False

    def query_backup_progress(self, taskid):
        # 调用oss接口查询备份任务进度
        user = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        passwd = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHPWD + self._pid)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QueryGroupManualBackup",
                "para": {
                    "taskid": taskid,
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        return_msg = ''
        executed_nodes = []
        oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        while oss_url:
            executed_nodes.append(oss_url)
            log.info(f'query_backup_progress executed_nodes : {executed_nodes}')
            retry_nums = 0
            while retry_nums < 3:
                retry_nums += 1
                if retry_nums != 1:
                    time.sleep(3)
                ret, ret_body, ret_header = request_post(oss_url, request_body, request_header)
                log.info(f'query_backup_progress ret_body : {ret_body}, retry_nums:  {retry_nums}')
                return_msg = ret_body.get("returnMsg")
                if not ret:
                    log.error(f'Failed query_backup_progress, request_body is : {request_body}')
                    continue
                if return_msg != "ok":
                    log.error(f'query_backup_progress error with return: {return_msg}')
                    continue
                return ret_body.get("returnData").get("status"), return_msg
            oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        return "-1", return_msg

    def check_zkmeta_is_exist(self):
        # 间隔30s检查一次zkmeta是否存在，如果查询超时1800s（30min）还不存在，则在副本元数据记录没有zkmeta
        log.info("step2-6 start to check zkmeta")
        timeout_time = int(time.time()) + 1800
        zkmeta_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id, "autocoldbackup", "group_zkmetas")
        while int(time.time()) <= timeout_time:
            if os.path.exists(zkmeta_path):
                return True
            time.sleep(30)
        log.info(f"step2-6 end to check zkmeta not exist")
        return False

    def get_backup_end_time(self):
        log.info(f"step2-6 get_backup_end_time")
        group_sets_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id, "autocoldbackup/sets")
        backup_end_time = 0
        for dirpath, _, _ in os.walk(group_sets_path):
            if os.path.basename(dirpath) == 'xtrabackup':
                file_paths = sorted(glob.glob(os.path.join(dirpath, "*")), key=os.path.getmtime)
                last_file = file_paths[-1]
                log.info(f"step2-6 get_backup_end_time last_file is : {last_file}")
                last_filename = os.path.basename(last_file)
                filename_list = last_filename.split('+')
                end_date = filename_list[7]
                end_time = filename_list[8]
                time_string = end_date + " " + end_time
                date_obj = datetime.datetime.strptime(time_string, "%Y%m%d %H%M%S")
                time_int = int(time.mktime(date_obj.timetuple()))
                backup_end_time = max(backup_end_time, time_int)
        return backup_end_time

    def delete_old_xtrabackup_data(self):
        # 删除/tdsqlback/tdsqlzk/group_id/autocoldbackup/sets/set_id/xtraback目录下旧的备份数据
        log.info("step2-6 start to deleate_old_xtrabackup_data")
        curr_time = int(time.time())
        interval_time = math.ceil((curr_time - self._backup_start_time) / 60)
        log.info(
            f"step2-6 delete_old_xtrabackup_data curr_time is : {curr_time}, "
            f"_backup_start_time is {self._backup_start_time}, interval_time is {interval_time}")
        group_sets_path = os.path.join(f"/tdsqlbackup/tdsqlzk/{self._group_id}/", "autocoldbackup/sets")
        log.info(f"step2-6 delete_old_xtrabackup_data sets path is : {group_sets_path}")
        for dirpath, _, _ in os.walk(group_sets_path):
            log.info(f"step2-6 delete_old_xtrabackup_data dirpath is : {dirpath}")
            if os.path.basename(dirpath) == 'xtrabackup':
                delete_cmd_list = [f"find {dirpath} -type f -mmin +{interval_time}", "xargs rm -rf"]
                log.info(f"step2-6 delete_old_xtrabackup_data cmd is : {delete_cmd_list}")
                # 找到目录下本次备份命令执行之前的文件并删除
                return_code, out_info, err_info = execute_cmd_list(delete_cmd_list)
                if return_code != CMDResult.SUCCESS:
                    log.info(
                        f"delete_old_xtrabackup_data delete_cmd_list out_info: {out_info} err_info: {err_info}")
                    return False
        return True

    def exec_shard_binlog(self):
        rest_request = RestRequests()
        env_variable = TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid
        executed_nodes = []
        oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        while oss_url:
            executed_nodes.append(oss_url)
            log.info(f'exec_shard_binlog executed_nodes : {executed_nodes}')
            if rest_request.shard_binlog(oss_url, self._group_id, env_variable):
                return True
            oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        log.error("step 2-6 shard_binlog failed")
        return True

    def sub_job_exec(self):
        # 执行数据备份子任务
        log.info("step 2-6 start to exec_back_up")
        if not self.exec_close_xtrabackup_cmd():
            log.error("step 2-6 exec_close_xtrabackup_cmd failed")
        taskid = self.exec_backup_cmd()
        if taskid == -1:
            log.error("step 2-6 exec_backup_cmd failed")
            return False
        time.sleep(5)
        # 15s定时查询备份任务状态
        while True:
            log.info(f"step 2-6 exec_backup query_backup_progress, taskid is {taskid}")
            task_status, return_msg = self.query_backup_progress(taskid)
            log.info(f"step 2-6 exec_backup query_backup_progress task_status: {task_status}")
            if task_status == TDSQLGroupBackupTaskStatus.SUCCEED:
                self._backup_end_time = self.get_backup_end_time()
                log.info(f"step 2-6 exec_backup backup_end_time: {self._backup_end_time}")
                zkmeta_is_exist = self.check_zkmeta_is_exist()
                if not zkmeta_is_exist:
                    log_detail = LogDetail(logInfo=TDSQLReportLabel.CHECK_ZKMETA_NOT_EXIT_LABEL,
                                           logInfoParam=[self._sub_job_id],
                                           logLevel=DBLogLevel.WARN.value)
                    report_job_details(self._pid,
                                       SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                     logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                           by_alias=True))
                if not self.exec_shard_binlog():
                    log.error(f"step 2-6 exec_shard_binlog failed")
                    return False
                self.delete_old_xtrabackup_data()
                self.report_copy_info(zkmeta_is_exist)
                self._backup_status = TdsqlBackupStatus.SUCCEED
                break
            elif task_status == TDSQLGroupBackupTaskStatus.FAILED:
                self._backup_status = TdsqlBackupStatus.FAILED
                log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_GROUP_FULL_BACKUP_FAIL_LABEL,
                                       logInfoParam=[self._sub_job_id],
                                       logLevel=DBLogLevel.ERROR.value)
                report_job_details(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                       by_alias=True))
                break
            elif task_status == TDSQLGroupBackupTaskStatus.RUNNING:
                self._backup_status = TdsqlBackupStatus.RUNNING
            else:
                log.error(f'query_backup_progress status invalid : {task_status}, return_msg is {return_msg}')
                self._backup_status = TdsqlBackupStatus.FAILED
                if return_msg:
                    log_detail_param = ["Backup", return_msg]
                    self.set_log_detail_with_params(ReportDBLabel.SUB_JOB_FALIED, self._sub_job_id,
                                                    ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                                    log_detail_param,
                                                    DBLogLevel.ERROR.value)
                break
            time.sleep(15)
        log.info("step 2-6 end to sub_job_exec")
        return True

    def sub_job_binlog(self):
        # 执行日志备份子任务
        log.info("step2-6 start to sub_job_binlog")
        # 在log仓创建groupid/set1/binlog、groupid/set2/binlog...的目标文件夹
        if not self.mkdir_copy_dest():
            log.error("step2-6 mkdir_copy_dest error")
            return False
        oss_url = self.get_oss_url(self._oss_nodes, [])
        if oss_url:
            log.info("step2-6 exec shard_binlog")
            RestRequests().shard_binlog(oss_url, self._group_id,
                                        TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        # 调用shard_binlog接口后等待45s，等binlog刷新
        time.sleep(45)
        binlog_ret, last_copy_time, end_time = self.backup_binlog()
        if not binlog_ret:
            log.error("step2-6 sub_job_binlog failed")
            self._job_status == SubJobStatusEnum.FAILED
            return False
        else:
            log.info("step2-6 sub_job_binlog success")
            # 上报日志副本信息（确保上报时间段包括副本生成时间）
            self.report_copy_info_binlog(last_copy_time, end_time)
            self._job_status == SubJobStatusEnum.COMPLETED
        log.info("step2-6 end to sub_job_binlog")
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def backup_binlog(self):
        last_backup_time = self.get_log_last_backup_time()
        log.info(f"backup_binlog last_backup_time is {last_backup_time}")
        # 拷贝从上次备份到现在的日志文件（多拷贝10min日志，避免数据不完整）
        time_period_min = (int(time.time()) - last_backup_time) / 60 + 10
        group_sets_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id, "autocoldbackup/sets")
        log.info(f"step2-6 group sets path is : {group_sets_path}, interval_time is {time_period_min}")
        target_root_path = os.path.join(self._log_area, self._group_id)
        end_time = 0
        for dirpath, _, _ in os.walk(group_sets_path):
            if os.path.basename(dirpath) == 'binlog':
                log.info(f"step2-6 backup_binlog set dirpath is : {dirpath}")
                curr_set = os.path.basename(os.path.dirname(dirpath))
                target_dir = os.path.join(target_root_path, curr_set, "binlog")
                os.chdir(dirpath)
                log.info(f"step2-6 backup_binlog target_dir is : {target_dir}")
                cp_cmd_list = [
                    f"find ./ -mmin -{time_period_min} -type f ! -name '*_COPYING_*'",
                    "xargs -i cp --parents -r {} " + target_dir
                ]
                log.info(f"step2-6 backup_binlog cp_cmd_list is : {cp_cmd_list}")
                return_code, out_info, err_info = execute_cmd_list(cp_cmd_list)
                if return_code != CMDResult.SUCCESS:
                    log.error(f'execute copy binlog cmd failed, message: {out_info}, err: {err_info}')
                    return False, last_backup_time, end_time
                file_paths = sorted(glob.glob(os.path.join(target_dir, "*")), key=os.path.getmtime)
                if len(file_paths) < 1:
                    log.error(f'step2-6 backup_binlog failed, log file is null')
                    return False, last_backup_time, end_time
                last_file = file_paths[-1]
                # 删除最后一个binlog文件
                if len(file_paths) > 1:
                    os.remove(last_file)
                    last_file = file_paths[-2]
                    log.info(f"step2-6 backup_binlog last_file is : {last_file}")
                last_filename = os.path.basename(last_file)
                binlog_time = int(last_filename.split('+')[1])
                end_time = max(end_time, binlog_time)
        last_full_backup_time = int(self.get_log_last_full_backup_time())
        log.info(f"step2-6 backup_binlog last_full_backup_time is : {last_full_backup_time}, end_time is {end_time}")
        if end_time <= last_full_backup_time:
            # 最新日志文件的时间小于上次全量备份副本的时间，备份失败
            log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_GROUP_LOG_BACKUP_FAIL_LABEL,
                                   logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                   by_alias=True))
            return False, last_backup_time, end_time
        return True, last_backup_time, end_time

    def sub_job_mount_bind(self):
        # 目录挂载子任务
        log.info("step2-5 start to sub_job_mount_bind")
        # mount bind /tdsqlbackup目录
        if not self.mount_bind_tdsqlbackup():
            log.error(f"mount_bind_tdsqlbackup failed")
            return False
        # 挂载分布式实例备份目录到X8000文件系统
        # mount 8.40.168.103:/home/nfs/nfs_server /tdsqlbackup/tdsqlzk/group_id
        if not self.mount_group_backup_path():
            log.error(f"mount_group_backup_path failed")
            return False
        log.info("step 2-5 end to sub_job_mount_bind")
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def mount_group_backup_path(self):
        # 创建分布式实例备份目录mkdir -p /tdsqlbackup/tdsqlzk/group_id
        backup_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id)
        log.info(f"step 2-5 mkdir backup_path: {backup_path}")
        return_code, out_info, err_info = execute_cmd(f"mkdir -p {backup_path}")
        if return_code != CMDResult.SUCCESS:
            log.error(f"mkdir -p {backup_path} out_info: {out_info} err_info: {err_info}")
            umount_bind_path(backup_path, self._mount_type)
        # 获取X8000文件系统目录
        data_repo = None
        for repository in self._repositories:
            if repository.get("repositoryType", "") == RepositoryDataTypeEnum.DATA_REPOSITORY.value:
                data_repo = repository
        data_remote_path = data_repo.get("remotePath", "")
        remote_ip = data_repo.get("remoteHost")[0].get("ip")
        log.info(f"remote_ip : {remote_ip}, data_remote_path: {data_remote_path}")
        if not os.path.ismount(backup_path):
            log.info(f"backup path not mounted")
            # 执行挂载命令
            if self._mount_type == MountType.FUSE:
                # 挂载前修改backup_path属主为tdsql
                self.chown_backup_path_owner("/tdsqlbackup/tdsqlzk/")
                self.chown_backup_path_owner(backup_path)
                mount_cmd_str = f"{PathConstant.FILE_CLIENT_PATH} --add --mount_point={backup_path} " \
                                f"--source_id={self._job_id} --osad_ip_list={self._osad_ip_list} " \
                                f"--osad_auth_port={self._osad_auth_port}  --osad_server_port={self._osad_server_port}"
            else:
                mount_cmd_str = f"sudo mount {remote_ip}:{data_remote_path} {backup_path}"
            return_code, out_info, err_info = execute_cmd(mount_cmd_str)
            if return_code != CMDResult.SUCCESS:
                log.error(f"mount group_backup_path out_info: {out_info} err_info: {err_info}")
        if self._mount_type != MountType.FUSE:
            # 挂载后修改backup_path属主为tdsql
            self.chown_backup_path_owner("/tdsqlbackup/tdsqlzk/")
            self.chown_backup_path_owner(backup_path)
        return True

    def do_post_job(self):
        log.info(f"step 2-7 start to do_post_job job_id {self._job_id} sub_job_id {self._sub_job_id}")
        log.info("step 2-7 end to do_post_job")
        return True

    def report_copy_info(self, zkmeta_is_exist):
        json_copy = {
            "extendInfo": {"copy_id": self._copy_id, "has_zkmeta": zkmeta_is_exist, "db_version": self._mysql_version,
                           "backup_time": self._backup_end_time, "backup_end_time": self._backup_end_time,
                           "group_id": self._group_id, "node_num": len(self.nodes)}
        }
        copy_info = {"copy": json_copy, "jobId": self._job_id}
        log.info(f"report_copy_info: {copy_info}.")
        try:
            invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Report copy info fail.err: {err_info},{self.get_log_comm()}")
            return False
        log.info(f"Report copy info succ {copy_info}.{self.get_log_comm()}")
        return True

    def report_copy_info_binlog(self, last_copy_time, end_time):
        log.info(f"Start to report_copy_info_binlog. end_time is {end_time}")
        json_copy = self.build_log_backup_copy_info(last_copy_time, end_time)
        copy_info = {"copy": json_copy, "jobId": self._job_id}
        log.info(f"report_copy_info_binlog: {copy_info}.")
        try:
            invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Report copy info fail.err: {err_info},{self.get_log_comm()}")
            return False
        log.info(f"Report copy info succ {copy_info}.{self.get_log_comm()}")
        return True

    def get_last_copy_info(self, copy_type: int):
        # 获取上次数据备份副本信息
        log.info(f"start get_last_copy_info copy_type {copy_type}")
        last_copy_type = LastCopyType.last_copy_type_dict.get(copy_type)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        log.info(f"start get_last_copy_info input_param {input_param}")
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{err_info}")
            return {}
        return result

    def get_log_last_backup_time(self):
        # 以上次日志备份作为起点，如果找不到上一次日志备份，则以上一次数据备份作为起点
        log.info("start get_log_backup_start_time")
        last_any_copy_info = self.get_last_copy_info(4)
        if not last_any_copy_info:
            log.info(f"This is the first log backup.last data copy info: {last_any_copy_info}")
            raise Exception("last copy not exist")
        log.info(f"last log copy info is : {last_any_copy_info}")
        extend_info = last_any_copy_info.get("extendInfo")
        last_time = extend_info.get("endTime", "")
        if not last_time:
            last_time = extend_info.get("backup_end_time")
            log.info(f"get_log_backup_start_time last_time is {last_time}")
        return last_time

    def get_log_last_full_backup_time(self):
        # 查找上一次数据备份的时间戳
        log.info("start get_log_last_full_backup_time")
        last_any_copy_info = self.get_last_copy_info(2)
        if not last_any_copy_info:
            log.info(f"This is the first log backup.last data copy info: {last_any_copy_info}")
            raise Exception("last copy not exist")
        log.info(f"get_log_last_full_backup_time copy info is : {last_any_copy_info}")
        extend_info = last_any_copy_info.get("extendInfo")
        last_time = extend_info.get("backup_time")
        log.info(f"get_log_last_full_backup_time backup_time {last_time}")
        return last_time

    def mkdir_copy_dest(self):
        repo_path = self._log_area
        for set_id in self._set_ids:
            copy_dest = os.path.join(repo_path, self._group_id, set_id, "binlog")
            log.info(f'mkdir_copy_dest copy_dest is: {copy_dest}')
            if not os.path.exists(copy_dest):
                log.info(f'mkdir_copy_dest makedirs: {copy_dest}')
                os.makedirs(copy_dest)
            ret = os.path.exists(copy_dest)
            if not ret:
                log.error(f'mkdir_copy_dest failed! {copy_dest}')
                return False
        return True

    def build_log_backup_copy_info(self, last_backup_time, end_time):
        """
        组装日志副本上报信息
        :return:
        """
        log.info(f"Start to build_log_backup_copy_info {last_backup_time}, end_time is {end_time}")
        out_put_info = {
            "extendInfo": {
                "backupTime": last_backup_time,
                "beginTime": last_backup_time,
                "endTime": end_time,
                "beginSCN": None,
                "endSCN": None,
                "associatedCopies": [],
                "logDirName": self._log_area,
                "copy_id": self._copy_id
            }
        }

        log.info(f"build_log_backup_copy_info: {out_put_info}")
        return out_put_info

    def exec_backup_cmd(self):
        # 调用oss全量备份接口执行分布式实例备份
        user = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        passwd = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHPWD + self._pid)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GroupManualBackup",
                "para": {
                    "backup_type": "xtrabackup",
                    "file_system": "local",
                    "groupid": self._group_id
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        executed_nodes = []
        oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        while oss_url:
            executed_nodes.append(oss_url)
            log.info(f'exec_backup_cmd executed_nodes : {executed_nodes}')
            retry_nums = 0
            # 调用oss全量备份接口,接口失败重试3次，每次间隔3s
            while retry_nums < 3:
                retry_nums += 1
                if retry_nums != 1:
                    time.sleep(3)
                ret, ret_body, ret_header = request_post(oss_url, request_body, request_header)
                log.info(f'exec_backup_cmd ret_body : {ret_body}, retry_nums:  {retry_nums}')
                if not ret:
                    log.error(f'Failed exec_backup_cmd, request_body is : {request_body}')
                    continue
                if ret_body.get("returnMsg") != "ok":
                    log.error(f'exec_backup_cmd error with return: {ret_body.get("returnMsg")}')
                    continue
                return ret_body.get("returnData").get("taskid")
            oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        return -1

    def get_db_version(self):
        # 获取实例的数据库版本
        log.info(f"start get_db_version")
        user = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        passwd = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHPWD + self._pid)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.GetGroup",
                "para": {
                    "groups": [{"id": self._group_id}]
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        executed_nodes = []
        oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        while oss_url:
            executed_nodes.append(oss_url)
            log.info(f'get_db_version executed_nodes : {executed_nodes}')
            retry_nums = 0
            while retry_nums < 3:
                retry_nums += 1
                if retry_nums != 1:
                    time.sleep(3)
                ret, ret_body, ret_header = request_post(oss_url, request_body, request_header)
                log.info(f'get_db_version ret_body : {ret_body}, retry_nums:  {retry_nums}')
                if not ret:
                    log.error(f'Failed get_db_version, request_body is : {request_body}')
                    continue
                if ret_body.get("returnMsg") != "ok":
                    log.error(f'get_db_version error with return: {ret_body.get("returnMsg")}')
                    continue
                self._mysql_version = ret_body.get("returnData").get("groups")[0].get("db_version")
                log.info(f"get_db_version _mysql_version is {self._mysql_version}")
                return True
            oss_url = self.get_oss_url(self._oss_nodes, executed_nodes)
        return False

    def get_oss_url(self, oss_nodes, executed_nodes):
        for oss_node in oss_nodes:
            oss_url = f'http://{oss_node}:8080/tdsql'
            if oss_url not in executed_nodes:
                if not self.check_oss_node_connect(oss_url):
                    log.error(f"{oss_url} can not connect")
                    continue
                return oss_url
        return ''

    def check_oss_node_connect(self, oss_url):
        user = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHKEY + self._pid)
        passwd = get_env_variable(TDSQLProtectKeyName.PROTECTENV_AUTH_AUTHPWD + self._pid)
        timestamp = int(time.time())
        request_body = {
            "callee": "TDSQL", "caller": user, "eventId": 101,
            "interface": {
                "interfaceName": "TDSQL.QuerySpec",
                "para": {
                }
            },
            "password": passwd, "timestamp": timestamp, "version": "1.0"
        }
        request_header = {'Content-type': 'application/json'}
        try:
            ret, ret_body, ret_header = request_post(oss_url, request_body, request_header)
        except Exception as ex:
            log.error(f"check_oss_node_connect Exception, error is {ex}")
            return False
        log.info(f"check_oss_node_connect ret is {ret}, ret_body is {ret_body}")
        if not ret:
            return False
        return True
