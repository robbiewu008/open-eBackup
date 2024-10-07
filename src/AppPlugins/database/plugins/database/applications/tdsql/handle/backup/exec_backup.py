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

import datetime
import json
import multiprocessing
import random
import os
import subprocess
import threading
import time

from common.cleaner import clear
from common.common import output_result_file, invoke_rpc_tool_interface, read_tmp_json_file
from common.common_models import SubJobDetails, LogDetail, SubJobModel, ActionResult
from common.const import ParamConstant, SubJobStatusEnum, RepositoryDataTypeEnum, ExecuteResultEnum, SubJobPolicyEnum, \
    SubJobTypeEnum, DBLogLevel, BackupTypeEnum, SubJobPriorityEnum, RpcParamKey
from common.util.scanner_utils import scan_dir_size
from oracle.common.common import write_tmp_json_file
from tdsql.common.const import TdsqlSubJobName, EnvName, TdsqlBackupStatus, \
    ErrorCode, LastCopyType, MySQLVersion, ConnectParam, JobInfo, InstanceConfigInfo, \
    NodeInfo, BackupPath, TdsqlBackTypeConstant, BackupParam
from tdsql.common.safe_get_information import ResourceParam
from tdsql.common.tdsql_common import output_execution_result_ex, report_job_details, \
    get_std_in_variable, write_file, delete_files_except, get_last_backup_time_and_id
from tdsql.common.util import check_status, can_exec_backup, backup_tdsql, monitor, backup_log, \
    is_job_finished, report_message, get_nodelist, get_tdsql_status, init_node_data, exec_sqlite_sql, \
    get_xtrabackup_tool_path, exec_sql
from tdsql.handle.common.const import TDSQLReportLabel
from tdsql.logger import log


class BackUp:

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
        self._host_ip = self._json_param_object.get("job", {}).get("protectEnv", {}).get("endpoint", "")
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._instance_id = self._json_param_object.get("job", {}).get("protectObject", {}).get("id", "")
        self._protect_obj_extend_info = self._json_param_object.get("job", {}).get("protectObject", {}).get(
            "extendInfo", {})
        self._cluster_info_str = self._protect_obj_extend_info.get("clusterInstanceInfo")
        self._cluster_info_ = json.loads(self._cluster_info_str)
        self.nodes = self._cluster_info_.get("groups", [])[0].get("dataNodes")
        self._mysql_socket = self.nodes[0].get("socket", "")
        self._mysql_conf_path = self.nodes[0].get("defaultsFile", "")
        log.info(f"nodes: {self.nodes}")
        self._set_id = self._cluster_info_.get("id")
        self._business_addr = self._protect_obj_extend_info.get("ossUrl", "")
        self._sub_job_name = ""
        self._cache_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self._data_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._meta_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.META_REPOSITORY)
        self._log_area = self.get_repository_path(json_param, RepositoryDataTypeEnum.LOG_REPOSITORY)
        self._job_status = SubJobStatusEnum.RUNNING
        self._backup_status = TdsqlBackupStatus.RUNNING
        self._err_code = 0
        self._backup_sla_str = json_param.get("job", {}).get("extendInfo", {}).get("backupTask_sla", "")
        self._backup_sla_ = json.loads(self._backup_sla_str)
        self.parallel = self.get_parallel()
        self.use_memory = self.get_use_memory()
        EnvName.IAM_USERNAME = "job_protectObject_auth_authKey"
        EnvName.IAM_PASSWORD = "job_protectObject_auth_authPwd"
        self.user_name = get_std_in_variable(f"{EnvName.IAM_USERNAME}_{pid}")
        self.user_pwd = get_std_in_variable(f"{EnvName.IAM_PASSWORD}_{pid}")
        # 组装资源接入请求体
        self._extend_info = self._json_param_object.get("job", {}).get("protectEnv", {}).get("extendInfo", {})
        self.backup_type = self._json_param_object.get("job", {}).get("jobParam", {}).get("backupType")
        self.delete_archived_log = self._json_param_object.get("job", {}).get("extendInfo", {}).get(
            "delete_archived_log", "false")
        log.debug(f"Param: {json_param}")

    @staticmethod
    def get_repository_path(file_content, repository_type):
        repositories = file_content.get("job", {}).get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                index = random.randint(0, len(repository["path"]) - 1)
                if repository_type != RepositoryDataTypeEnum.DATA_REPOSITORY:
                    index = 0
                repositories_path = repository["path"][index]
                log.info(f"repository_type is {repository_type}, index is {index}")
                break
        return repositories_path

    @staticmethod
    def get_params_by_key(param, json_const):
        param = param.get("job", {}).get("protectObject", {}).get("extendInfo", {}).get(json_const, "")
        if not param:
            log.error(f"Get param protectObject_extendInfo_json_const failed.")
        return param

    @staticmethod
    def set_error_response(response):
        response.code = ExecuteResultEnum.INTERNAL_ERROR.value
        response.body_err = ExecuteResultEnum.INTERNAL_ERROR.value

    @staticmethod
    def read_param_file(file_path):
        """
        解析参数文件
        :return:
        """
        if not os.path.isfile(file_path):
            raise Exception(f"File:{file_path} not exist")
        try:
            with open(file_path, "r", encoding='UTF-8') as f_content:
                json_dict = json.loads(f_content.read())
        except Exception as ex:
            raise Exception("parse param file failed") from ex
        return json_dict

    @staticmethod
    def get_cpu_number():
        exec_cmd = "cat /proc/cpuinfo | grep processor | wc -l"
        ret, output = subprocess.getstatusoutput(exec_cmd)
        if not ret:
            return int(output)
        else:
            return 8

    @staticmethod
    def query_size_and_speed(time_file, data_path, original_size_file, job_id):
        size = 0
        speed = 0
        original_size = 0
        with open(original_size_file, "r", encoding='UTF-8') as file:
            original_size = file.read().strip()
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
        if (timediff == 0):
            log.info(f"query_size_and_speed, timediff is {timediff}")
            return datadiff, speed
        try:
            speed = datadiff / timediff
        except Exception:
            log.error("Error while calculating speed! time difference is 0!")
            return 0, 0
        log.info(f"query_size_and_speed, datadiff: {datadiff}, timediff: {timediff}, speed: {speed}")
        return datadiff, speed

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

    def get_parallel(self):
        parallel = self._backup_sla_.get("policy_list", [])[0].get("ext_parameters", {}).get("channel_number", "")
        max_parallel = self.get_cpu_number()
        if parallel:
            parallel = int(parallel)
            if parallel > max_parallel:
                log.warn(f"Maximum supported channel_number is {max_parallel}!")
                parallel = max_parallel
        else:
            parallel = max_parallel
        return parallel

    def get_use_memory(self):
        use_memory = self._backup_sla_.get("policy_list", [])[0].get("ext_parameters", {}).get("use_memory", "")
        if use_memory:
            use_memory = int(use_memory)
            log.info(f"self.use_memory {use_memory}")
            if use_memory > 2048:
                log.warn(f"Maximum supported memory is 2048M!")
                use_memory = 2048
        else:
            use_memory = 1024
        return use_memory

    def get_last_copy_info(self, copy_type: int):
        # 获取上次数据备份副本信息
        log.info("start get_last_copy_info")
        last_copy_type = LastCopyType.last_copy_type_dict.get(copy_type)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{err_info}")
            return {}
        return result

    def get_last_full_copy_info(self):
        log.info("start get_last_full_copy_info")
        last_copy_type = LastCopyType.last_copy_type_dict.get(2)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{err_info}")
            return {}
        log.info(f"Get_last_full_copy_info: {result}")
        return result

    def get_last_any_copy_info(self):
        log.info("start get_last_any_copy_info")
        last_copy_type = LastCopyType.last_copy_type_dict.get(4)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get any last copy info fail.{err_info}")
            return {}
        log.info(f"Get_last_any_copy_info: {result}.")
        return result

    def get_last_any_copy_type(self):
        last_copy_info = self.get_last_any_copy_info()
        copy_type = last_copy_info.get("type", "")
        log.info(f"copy_type: {copy_type}")
        if not copy_type:
            return False, ""
        if copy_type == RpcParamKey.LOG_COPY:
            return True, "log"
        else:
            return True, "data"

    def get_last_data_copy_extend_info(self):
        last_copy_info = self.get_last_copy_info(1)
        extend_info = last_copy_info.get("extendInfo", {})
        log.info(f"Extend_info: {extend_info}.")
        last_copy_id = extend_info.get("copyId", "")
        backup_start_time = extend_info.get("backup_time", 0)
        exec_node = extend_info.get("exec_node", "")
        log.info(f"Last_copy_id: {last_copy_id}.")
        log.info(f"Backup_start_time: {backup_start_time}.")
        log.info(f"Exec_node: {exec_node}.")
        return backup_start_time, exec_node, [last_copy_id]

    def get_last_binlog_copy_info(self):
        log.info("start get_last_binlog_copy_info")
        last_copy_type = LastCopyType.last_copy_type_dict.get(3)
        input_param = {
            RpcParamKey.APPLICATION: self._json_param_object.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail: {err_info}.")
            return {}
        log.info(f"Get_last_binlog_copy_info: {result}.")
        return result

    def get_last_binlog_copy_extend_info(self):
        log.info("start to get_last_binlog_copy_extend_info")
        last_copy_info = self.get_last_binlog_copy_info()
        extend_info = last_copy_info.get("extendInfo", {})
        end_time = extend_info.get("endTime", 0)
        log.info(f"end_time: {end_time}  type:{type(end_time)}")
        last_copy_id = extend_info.get("associatedCopies", [])
        log.info(f"last_copy_id{last_copy_id}")
        return end_time, last_copy_id

    def check_backup_job_type(self):
        log.info("step2-2 start to check backup job type")

        # 当此次任务是增量量备份，且之前没做过全量备份，需要转全量
        # 当此次任务是日志备份，且之前没有任何备份副本，日志备份失败
        def check_last_copy_is_null():
            # 读取last_copy_info
            if self.backup_type == BackupTypeEnum.LOG_BACKUP:
                last_copy_info = self.get_last_any_copy_info()
            else:
                last_copy_info = self.get_last_copy_info(1)
            last_copy_info_file = os.path.join(self._meta_area, "lastCopyInfo", f"jobid_{self._job_id}")
            last_copy_info_path = os.path.join(self._meta_area, "lastCopyInfo")
            if not os.path.exists(last_copy_info_path):
                try:
                    os.makedirs(last_copy_info_path)
                except Exception as err:
                    log.error(f"Make dir for {last_copy_info_path} err: {err}.")
            if last_copy_info:
                output_execution_result_ex(last_copy_info_file, last_copy_info)
                log.info(f"save last copy info to {last_copy_info_file} successfully")
                return False
            else:
                log.info(f"succeed save last copy info: {last_copy_info_file} ")
                return True

        log.info(f'step 2-2: start execute check_backup_job_type, pid: {self._pid}, job_id:{self._job_id}')
        backup_type = self.backup_type
        log.info(f"check backup_type is {backup_type}")
        if not backup_type:
            return False
        if backup_type == BackupTypeEnum.FULL_BACKUP:
            return True

        if check_last_copy_is_null():
            log.error(f"check_last_copy_is_null.")
            if backup_type == BackupTypeEnum.INCRE_BACKUP:
                response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                        bodyErr=ErrorCode.ERROR_INCREMENT_TO_FULL,
                                        message="Can not apply this type backup job")
                output_result_file(self._pid, response.dict(by_alias=True))
                log.info(f"change backup_type increment to full")
                return False
            elif backup_type == BackupTypeEnum.LOG_BACKUP:
                # 日志备份，无任何副本，则任务失败
                response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                        message="Can not apply this type backup job")
                output_result_file(self._pid, response.dict(by_alias=True))
                log.info(f"Can not apply binlog backup")
                return False
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(self._pid, response.dict(by_alias=True))
        log.info(f'step 2-2: finish execute check_backup_job_type, pid: {self._pid}, job_id:{self._job_id}')
        return True

    def backup_pre_job(self):
        # 共享文档逻辑如下：
        # 1、node_info文件创建处，以job id唯一标记
        # 2、建立一个线程在备份，循环请求node info，进行更新，循环时间15S
        # 3、主从节点切换选择新节点：读取文件后进行选择，并更新共享文档
        # 4、主从节点切换或备份发生错误时，更新共享文档
        log.info("step2-3 start to backup_pre_job")
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        nodes_info_path = os.path.join(self._meta_area, "nodesInfo")
        if not os.path.exists(nodes_info_path):
            try:
                os.makedirs(nodes_info_path)
            except Exception as err:
                log.error(f"Make dir for {nodes_info_path} err: {err}.")

        nodes = self.nodes
        nodes_info = {}
        for node in nodes:
            ip = node.get("ip")
            port = str(node.get("port"))
            node_host = ip + ":" + port
            node_priority = node.get("priority")
            uuid = node.get("parentUuid")
            node_info = NodeInfo(nodeHost=node_host,
                                 setId=self._instance_id,
                                 agentUuid=uuid,
                                 priority=node_priority,
                                 )
            nodes_info[node_host] = node_info.dict(by_alias=True)
        log.info(f"step2-3- nodes_info: {nodes_info}")
        init_node_data(nodes_info_file, nodes_info, 100)
        log.info("step2-3 end to backup_pre_job")
        return

    def get_progress(self):
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        nodes_info = get_nodelist(nodes_info_file, 10)
        num = len(nodes_info)
        log.info("start get_progress")
        is_failed = 0
        for value in nodes_info:
            if value[4] == 1:
                log.info("job succeed")
                return TdsqlBackupStatus.SUCCEED
            elif value[4] == 2:
                is_failed += 1
        if is_failed == num and num != 0:
            log.info("job failed")
            return TdsqlBackupStatus.FAILED
        log.info("job running")
        return TdsqlBackupStatus.RUNNING

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def write_progress_file(self, task_status, progress):
        log.info("start write_progress_file")
        if task_status == SubJobStatusEnum.FAILED.value:
            log_detail_param = []
            if self._sub_job_name == TdsqlSubJobName.SUB_EXEC or self._sub_job_name == TdsqlSubJobName.SUB_BINLOG:
                self._err_code = None
                log_detail_param.append(self._instance_id)
                log.info(f"start self._sub_job_name: {self._sub_job_name}")
            self.set_log_detail_with_params("plugin_task_subjob_fail_label", self._sub_job_id, self._err_code,
                                            log_detail_param,
                                            DBLogLevel.ERROR.value)
        if task_status == SubJobStatusEnum.COMPLETED.value:
            self.set_log_detail_with_params("plugin_task_subjob_success_label", self._sub_job_id, 0, [],
                                            DBLogLevel.INFO.value)
            log.info("task_status == SubJobStatusEnum.COMPLETED.value")

        # 获取速度，数据量大小
        if self.backup_type == BackupTypeEnum.FULL_BACKUP or self.backup_type == BackupTypeEnum.INCRE_BACKUP:
            log.info(f"Data backup :query backup speed and size")
            time_file = os.path.join(self._cache_area, f'T{self._job_id}')
            original_size_file = os.path.join(self._cache_area, f'D{self._job_id}')
            size_all, speed = self.query_size_and_speed(time_file, self._data_area, original_size_file, self._job_id)
            node_num = len(self.nodes)
            if not node_num:
                size = size_all
            else:
                size = int(size_all / node_num)
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
        progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
        log.debug(f"Write file.{progress_str}{self.get_log_comm()}.")
        output_execution_result_ex(progress_file, json_str)

    def write_backup_progress(self):
        # 定时上报备份进度
        while self._backup_status == TdsqlBackupStatus.RUNNING:
            # 没有进度文件可能是还没有生成,不返回失败
            # 通过请求任务信息获取任务进度
            self._backup_status = self.get_progress()
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
            time.sleep(self._query_progress_interval)
            self.write_progress_file(status, progress)

    def upload_backup_progress(self):
        # 定时上报备份进度
        while self._job_status == SubJobStatusEnum.RUNNING:
            if self._sub_job_name == TdsqlSubJobName.SUB_EXEC:
                self._backup_status = self.get_progress()
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
            TdsqlSubJobName.SUB_OSS: self.sub_job_oss,
            TdsqlSubJobName.SUB_EXEC: self.sub_job_exec,
            TdsqlSubJobName.SUB_BINLOG: self.sub_job_binlog,
            TdsqlSubJobName.SUB_FLUSH_LOG: self.sub_flush_log,
            TdsqlSubJobName.SUB_RM_BINLOG: self.sub_rm_binlog
        }
        return sub_job_dict

    def backup_task(self):
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
        sub_job_name = ResourceParam.get_sub_job_name(self._json_param_object)
        if not sub_job_name:
            return False
        self._sub_job_name = sub_job_name

        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as err:
            log.error(f"do {sub_job_name} fail: {err}")
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail_param = []
            if sub_job_name == TdsqlSubJobName.SUB_EXEC or sub_job_name == TdsqlSubJobName.SUB_BINLOG:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[self._sub_job_id],
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
            if sub_job_name == TdsqlSubJobName.SUB_EXEC or sub_job_name == TdsqlSubJobName.SUB_BINLOG:
                log_detail_param.append(self._instance_id)
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            return False

        progress_thread.join()
        return True

    def build_sub_job(self, job_priority, policy, job_name, node_id, job_info):
        return SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=policy, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job_data(self):
        log.info("step 2-4 start to gen_sub_job for data backup")
        nodes = self.nodes
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        # 子任务1：OSS
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_OSS
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        for node in nodes:
            node_id = node.get("parentUuid", "")
            endpoint = node.get("ip", "")
            port = str(node.get("port"))
            host = endpoint + ":" + port
            job_info = f"{host}"
            sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        # 子任务2：执行
        # 在每个节点执行，具体执行权限在执行子任务中实现判断
        log.info("start to gen sub_exec")
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_EXEC
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        for node in nodes:
            node_id = node.get("parentUuid", "")
            endpoint = node.get("ip", "")
            port = str(node.get("port"))
            host = endpoint + ":" + port
            job_info = f"{host}"
            sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        log.info(f"gen_sub_job get sub_job_array: {sub_job_array}")
        log.info(f"step 2-4 Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        log.info("step 2-4 end to gen_sub_job for data backup")
        return True

    def gen_sub_job_binlog(self):
        # 日志备份生成子任务
        log.info("step 2-4 start to gen_sub_job for binlog backup")

        nodes = self.nodes
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []
        # 所有节点执行flush logs
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_FLUSH_LOG
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        for node in nodes:
            node_id = node.get("parentUuid", "")
            endpoint = node.get("ip", "")
            port = str(node.get("port"))
            host = endpoint + ":" + port
            job_info = f"{host}"
            sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        # 执行日志备份，在数据备份节点进行
        backup_start_time, last_exec_node, last_copy_id = self.get_last_data_copy_extend_info()
        log.info("step 2-4 binlog based on full backup copy")
        log.info(f"Last_exec_node: {last_exec_node}.")
        job_policy = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlSubJobName.SUB_BINLOG
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        for node in nodes:
            node_id = node.get("parentUuid", "")
            endpoint = node.get("ip", "")
            port = str(node.get("port"))
            host = endpoint + ":" + port
            log.info(f"host: {host}")
            job_info = f"{host}"
            # 在上次执行的节点执行
            if host == last_exec_node:
                log.info(f"Node_id: {node_id}.")
                sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
                sub_job_array.append(sub_job)
                break

        # 如果开启了删除归档日志，所有节点执行rm binlog
        if self.delete_archived_log == "true":
            job_policy = SubJobPolicyEnum.FIXED_NODE.value
            job_name = TdsqlSubJobName.SUB_RM_BINLOG
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
            for node in nodes:
                node_id = node.get("parentUuid", "")
                endpoint = node.get("ip", "")
                port = str(node.get("port"))
                host = endpoint + ":" + port
                job_info = f"{host}"
                sub_job = self.build_sub_job(job_priority, job_policy, job_name, node_id, job_info)
                sub_job_array.append(sub_job)

        log.info(f"gen_sub_job get sub_job_array: {sub_job_array}")
        log.info(f"step2-4 Sub-task splitting succeeded.sub-task num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        log.info("step2-4 end to gen_sub_job for binlog backup")
        return True

    def sub_flush_log(self):
        output = ""
        ip, port = self.fetch_current_host()
        mysql_version = self._mysql_conf_path.split("/")[4]
        log.info(f"Exec sub_flush_logs, mysql_version: {mysql_version}")
        if mysql_version.startswith(MySQLVersion.MARIADB):
            connect_param = ConnectParam(socket='', ip=ip, port=port)
        else:
            connect_param = ConnectParam(socket=self._mysql_socket, ip=ip, port=port)
        try:
            ret, output = exec_sql(self.user_name, self.user_pwd, connect_param, "flush logs")
        except Exception as exception_str:
            log.error(f"flush logs failed: {exception_str}")
            ret = False
        if not ret:
            log.error(f"Exec_sql failed. sql:flush logs ret:{ret} output is {output} "
                      f"pid:{self._pid} jobId{self._job_id}")
            return False
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def sub_rm_binlog(self):
        file_path = os.path.join(self._cache_area, f'copy_binlog_time_{self._job_id}')
        copy_binlog_time = read_tmp_json_file(file_path).get('copy_binlog_time')
        log.info(f"Exec sub_rm_binlog, copy_binlog_time: {copy_binlog_time}")
        output = ""
        ip, port = self.fetch_current_host()
        mysql_version = self._mysql_conf_path.split("/")[4]
        if mysql_version.startswith(MySQLVersion.MARIADB):
            connect_param = ConnectParam(socket='', ip=ip, port=port)
        else:
            connect_param = ConnectParam(socket=self._mysql_socket, ip=ip, port=port)
        try:
            ret, output = exec_sql(self.user_name, self.user_pwd, connect_param,
                                   f"purge binary logs before '{copy_binlog_time}'")
        except Exception as exception_str:
            log.error(f"sub_rm_binlog failed: {exception_str}")
            ret = False
        if not ret:
            log.error(f"Exec_sql failed. sql:delete_backup_binlog ret:{ret} output is {output} "
                      f"pid:{self._pid} jobId{self._job_id}")
            return False
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def sub_job_pre(self):
        # 录入备份开始时间
        start_time = str(int((time.time())))
        log.warn(f"start_time: {start_time}")
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
            write_file(original_data_size_path, str(original_size))
            log.info(f"success to write backup start data size {original_size} to {original_data_size_path}")
            log.info("step2-6 start to sub_job_pre")

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

    def gen_sub_job(self):
        if self.backup_type == BackupTypeEnum.LOG_BACKUP:
            self.gen_sub_job_binlog()
        else:
            self.gen_sub_job_data()

    def fetch_current_host(self):
        host = self._json_param_object["subJob"]["jobInfo"]
        ip_port = host.split(":")
        ip = ip_port[0]
        port = ip_port[1]
        log.info(f"the port is {port}")
        return ip, port

    def sub_job_oss(self):
        # 检查数据库服务子任务
        log.info("step2-5 start to sub_job_oss")
        # 获取当前端口，校验mysql服务状态
        ip, port = self.fetch_current_host()
        mysql_port = port
        mysql_status = check_status(mysql_port)
        if not mysql_status:
            response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                    bodyErr=ErrorCode.ERR_DB_SERVICES,
                                    message="Mysql service abnormal")
            output_result_file(self._pid, response.dict(by_alias=True))
            log.error("mysql数据库服务异常")
            return False
        log.info("step2-5 mysql service normal")
        log.info("step2-5 end to sub_job_oss")
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def fetch_copy_info(self):
        cluster_info = self._extend_info.get("clusterInfo")
        return cluster_info

    def data_backup_paths(self, port, version):
        paths = {}
        node = self.nodes[0]
        xtrabackup_tool_path = get_xtrabackup_tool_path(version)
        log.info(f"xtrabackup_tool_path {xtrabackup_tool_path}")
        paths["backup_tools"] = os.path.join(BackupPath.BACKUP_PRE, port, version, xtrabackup_tool_path)
        paths["cnf_path"] = node.get("defaultsFile")
        paths["backup_socket"] = node.get("socket")
        paths["backup_target"] = os.path.join(self._data_area, "tdsqlbackup", self._copy_id)

        # 获取last_copy_id
        if self.backup_type == BackupTypeEnum.FULL_BACKUP:
            target_dir = os.path.join(self._data_area, "tdsqlbackup", self._copy_id, "full")
            paths["target_dir"] = target_dir
            paths["base_dir"] = ""
        else:
            target_dir = os.path.join(self._data_area, "tdsqlbackup", self._copy_id, "incremental")
            paths["target_dir"] = target_dir
            last_copy_id_file = os.path.join(self._meta_area, "lastCopyId", "lastCopyId")
            last_copy_id = self.read_param_file(last_copy_id_file)
            paths["base_dir"] = os.path.join(self._data_area, "tdsqlbackup", last_copy_id, "full")
            log.info(f"lastCopyId: {last_copy_id}")
        if not os.path.exists(paths["target_dir"]):
            try:
                os.makedirs(paths["target_dir"])
            except Exception as err:
                log.error(f"Make dir for {target_dir} err: {err}.")
        log.info(f"data_backup_paths: {paths}")
        return paths

    def exec_backup(self, backup_param: BackupParam):
        backup_thread = ""
        # NodesInfo共享文档
        file_name = backup_param.file_name
        ip = backup_param.host
        port = backup_param.port
        set_id = backup_param.set_id
        # ossUrl
        url = backup_param.url
        timeout = 10

        while True:
            tdsql_body = get_tdsql_status(url, set_id, "backup", self._pid)
            log.info(f"step2-6 get tdsql_body")
            sql_lists = []
            for i in tdsql_body:
                for j in dict(i).get("db"):
                    curr_node = j.get('ip') + ":" + str(j.get('port'))
                    sql = f"update nodeinfo set is_master={j.get('master')},is_alive={j.get('alive')} " \
                          f"where node_host='{curr_node}'"
                    sql_lists.append(sql)
            exec_sqlite_sql(file_name, timeout, sql_lists)
            log.info("step2-6 before can_exec_backup")
            if can_exec_backup(backup_param, file_name, set_id, self._pid):
                log.info(f"step2-6 can_exec_backup")
                backup_pwd = get_std_in_variable(f"{EnvName.IAM_PASSWORD}_{self._pid}")
                backup_thread = multiprocessing.Process(target=backup_tdsql, args=(backup_param, backup_pwd, self._pid,
                                                                                   self._job_id, self._sub_job_id,))
                log.info(f"step2-6 after multiprocessing.Process")
                try:
                    backup_thread.start()
                except Exception as ex:
                    log.error(f"when start backup thread, exception {ex} occurs")
                finally:
                    clear(backup_pwd)
                next_node = ip + ":" + port
                nodes = ["", next_node]
                report_message(self._pid, self._job_id, self._sub_job_id, nodes, "info")
                log.info(f"step2-6  after backup_thread.start")
                if backup_thread:
                    log.info(f"True: backup_thread")
                    monitor_thread = multiprocessing.Process(target=monitor,
                                                             args=(backup_thread, backup_param, self._pid,
                                                                   self._job_id, self._sub_job_id,))
                    log.info(f"after monitor_thread.start")
                    monitor_thread.daemon = True
                    monitor_thread.start()
                backup_thread.join()
            if is_job_finished(file_name):
                instance_config = self.gen_instance_config_info(tdsql_body)
                self.report_copy_info(instance_config)
                break
            time.sleep(15)

    def gen_instance_config_info(self, tdsql_body):
        """
        用于将备份的实例的机型配置信息上报到副本中
        """
        for i in tdsql_body:
            instance_config_info = dict(i)
            instance_config = InstanceConfigInfo(dbversion=self.fetch_version(),
                                                 machine=instance_config_info.get("machine", ""),
                                                 cpu=instance_config_info.get("cpu", ""),
                                                 memory=instance_config_info.get("memory", ""),
                                                 data_disk=instance_config_info.get("data_disk", ""),
                                                 log_disk=instance_config_info.get("log_disk", ""))
            return instance_config

    def fetch_version(self):
        ip, port = self.fetch_current_host()
        for node in self.nodes:
            if node["ip"] == ip:
                path = node["defaultsFile"]
                version = path.split("/")[4]
        return version

    def report_copy_info(self, instance_config):
        if self.backup_type == BackupTypeEnum.FULL_BACKUP:
            xtrabackup_info_file = os.path.join(self._data_area, "tdsqlbackup", self._copy_id, "full",
                                                "xtrabackup_info")
        elif self.backup_type == BackupTypeEnum.INCRE_BACKUP:
            xtrabackup_info_file = os.path.join(self._data_area, "tdsqlbackup", self._copy_id, "incremental",
                                                "xtrabackup_info")
        if not os.path.exists(xtrabackup_info_file):
            log.error(f"report_copy_info check_is_all_node_not_alive")
            log_detail = LogDetail(logInfo=TDSQLReportLabel.CHECK_IS_ALL_NODE_NOT_ALIVE,
                                   logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR)
            sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                     logDetail=[log_detail],
                                     taskStatus=SubJobStatusEnum.FAILED.value)
            report_job_details(self._job_id, sub_dict.dict(by_alias=True))
            return

        backup_start_time = ""
        with open(xtrabackup_info_file, "r", encoding='UTF-8') as f_content:
            xt_info = f_content.readlines()
        for xtra_info in xt_info:
            if xtra_info.startswith("start_time"):
                backup_start_time = xtra_info.split("=")[1].strip()
                break

        exec_node = ""
        job_id_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")
        sql_lists = ["select node_host from nodeinfo where is_exec_node=1"]
        ret, results = exec_sqlite_sql(job_id_file, 10, sql_lists)
        if ret and len(results) > 0:
            exec_node = results[0][0]

        # 删除合成前的增量路径
        if self.backup_type == BackupTypeEnum.INCRE_BACKUP:
            incre_path = os.path.join(self._data_area, "tdsqlbackup", self._copy_id, "incremental")
            if os.path.exists(incre_path):
                try:
                    delete_files_except("xtrabackup_info", incre_path)
                except Exception as err:
                    log.error(f"Err: {err}")

        backup_start_time = datetime.datetime.strptime(backup_start_time, "%Y-%m-%d %H:%M:%S")
        log.info(f"Backup_start_time: {backup_start_time}.")
        start_time = backup_start_time.timestamp()
        log.info(f"Backup_start_time timestamp: {start_time}.")
        json_copy = {
            "extendInfo": {
                "copy_id": self._copy_id, "exec_node": exec_node, "backup_time": start_time,
                "mysql_version": self.fetch_version(), "instance_config_info": instance_config.dict(by_alias=True)
            }
        }
        copy_info = {"copy": json_copy, "jobId": self._job_id}
        log.debug(f"Copy_info: {copy_info}.")
        invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        log.debug(f"Finish report copy_info!")

    def build_log_backup_copy_info(self, start_time, end_time, last_copy_id, curr_exec_node):
        """
        组装日志副本上报信息
        :return:
        """
        log.info("Start to build_log_backup_copy_info")
        mysql_version = self.fetch_version()
        out_put_info = {
            "extendInfo": {
                "backupTime": start_time,
                "beginTime": start_time,
                "endTime": end_time,
                "exec_node": curr_exec_node,
                "beginSCN": None,
                "endSCN": None,
                "backupset_dir": '',
                "backupSetName": "",
                "backupType": "",
                "baseBackupSetName": "",
                "dbName": "",
                "groupId": '',
                "tabal_space_info": [],
                "associatedCopies": last_copy_id,
                "logDirName": self._log_area,
                "mysql_version": mysql_version
            }
        }
        log.info(f"build_log_backup_copy_info: {out_put_info}")
        return out_put_info

    def report_copy_info_binlog(self, start_time, end_time, last_copy_id, curr_exec_node):
        log.info(f"Start to report_copy_info_binlog.")
        json_copy = self.build_log_backup_copy_info(start_time, end_time, last_copy_id, curr_exec_node)
        copy_info = {"copy": json_copy, "jobId": self._job_id}
        log.debug(f"Copy_info: {copy_info}.")
        try:
            invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Report copy info fail.err: {err_info},{self.get_log_comm()}")
            return False
        log.info(f"Report copy info succ {copy_info}.{self.get_log_comm()}")
        return True

    def sub_job_exec(self):
        # 执行数据备份子任务
        log.info("step 2-6 start to exec_back_up")
        # 发送备份请求
        log.info("begin to get ip and port")
        ip, port = self.fetch_current_host()
        log.info("begin to get version")
        version = self.fetch_version()
        log.info("begin to prepare data_backup_paths")
        data_backup_paths = self.data_backup_paths(port, version)
        nodes_info_file = os.path.join(self._meta_area, "nodesInfo", f"job_id_{self._job_id}")

        log.info(f"step2-6 ip: {ip}, port: {port}")
        if self.backup_type == BackupTypeEnum.FULL_BACKUP:
            backup_type = TdsqlBackTypeConstant.FULL
        elif self.backup_type == BackupTypeEnum.INCRE_BACKUP:
            backup_type = TdsqlBackTypeConstant.INCREMENTAL_FOREVER
        log.info(f"step2-6 backup_type: {backup_type}")
        # 备份参数
        backup_param = BackupParam(path=data_backup_paths.get("backup_target"), url=self._business_addr,
                                   file_name=nodes_info_file, host=ip, port=port, set_id=self._set_id,
                                   backup_type=backup_type, backup_tools=data_backup_paths.get("backup_tools"),
                                   defaults_file=data_backup_paths.get("cnf_path"),
                                   socket=data_backup_paths.get("backup_socket"), user=self.user_name,
                                   parallel=self.parallel, use_memory=self.use_memory,
                                   target_dir=data_backup_paths.get("target_dir"),
                                   base_dir=data_backup_paths.get("base_dir"), mysql_version=version)
        self.exec_backup(backup_param)

        # 保存此次 copy_id
        if self.backup_type == BackupTypeEnum.FULL_BACKUP:
            last_copy_id_file = os.path.join(self._meta_area, "lastCopyId", "lastCopyId")
            last_copy_id_path = os.path.join(self._meta_area, "lastCopyId")
            log.info(f"current task copyId: {self._copy_id}")
            if not os.path.exists(last_copy_id_path):
                try:
                    os.makedirs(last_copy_id_path)
                except Exception as err:
                    log.error(f"Make dir for {last_copy_id_path} err: {err}.")
            output_execution_result_ex(last_copy_id_file, self._copy_id)

        # 保存当前实例的集群信息，用于恢复时使用
        copy_info = self.fetch_copy_info()
        copy_info_file = os.path.join(self._meta_area, self._copy_id, "copy_info")
        copy_info_path = os.path.join(self._meta_area, self._copy_id)
        if not os.path.exists(copy_info_path):
            try:
                os.makedirs(copy_info_path)
            except Exception as err:
                log.error(f"Make dir for {copy_info_path} err: {err}.")
        output_execution_result_ex(copy_info_file, copy_info)
        log.info("step2-6 end to sub_job_exec")
        return True

    def sub_job_binlog(self):
        # 执行日志备份子任务
        log.info("step2-6 start to sub_job_binlog")
        node = self.nodes[0]
        cnf_path = node.get("defaultsFile")
        target_dir = os.path.join(self._log_area, "tdsqlbackup", self._copy_id, "binlog")
        if not os.path.exists(target_dir):
            try:
                os.makedirs(target_dir)
            except Exception as err:
                log.error(f"Make dir for {target_dir} err: {err}.")
        _, curr_exec_node, _ = self.get_last_data_copy_extend_info()
        last_copy_info = self.get_log_backup_last_copy_info(curr_exec_node)
        if not last_copy_info:
            log.error(f"Fail to get previous copy info")
            return False
        start_time, last_copy_id = get_last_backup_time_and_id(last_copy_info)
        log.info(f"Start_time: {start_time}.")
        start_time_str = datetime.datetime.fromtimestamp(start_time, tz=datetime.timezone.utc)
        log.info(f"Start_time_str(UTC time): {start_time_str}.")
        job_info = JobInfo(pid=self._pid, job_id=self._job_id, sub_job_id=self._sub_job_id, copy_id=self._copy_id)
        binlog_ret, copy_time = backup_log(job_info, cnf_path, target_dir, start_time_str)
        end_time = int(copy_time.timestamp())
        copy_time = copy_time.strftime("%Y-%m-%d %H:%M:%S")
        copy_binlog_time = {
            "copy_binlog_time": copy_time
        }
        file_path = os.path.join(self._cache_area, f'copy_binlog_time_{self._job_id}')
        log.info(f"file_path:{file_path}")
        write_tmp_json_file(file_path, self._job_id, copy_binlog_time)
        if not binlog_ret:
            log.error("step2-6 sub_job_binlog failed")
            self._job_status == SubJobStatusEnum.FAILED
            return False
        else:
            log.info("step2-6 sub_job_binlog success")
            # 上报日志副本信息（确保上报时间段包括副本生成时间）
            self.report_copy_info_binlog(start_time, end_time, last_copy_id, curr_exec_node)
            self._job_status == SubJobStatusEnum.COMPLETED
        log.info("step2-6 end to sub_job_binlog")
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def do_post_job(self):
        log.info(f"step 2-7 start to do_post_job job_id {self._job_id} sub_job_id {self._sub_job_id}")
        log.info("step 2-7 end to do_post_job")
        return True

    def get_log_backup_last_copy_info(self, curr_exec_node):
        # 以上次日志备份作为起点，如果找不到上次日志备份或上次日志备份执行节点和当前不一致，则以上一次数据备份作为起点
        log.info("start get_log_backup_last_copy_info")
        last_copy_info = self.get_last_copy_info(3)
        if not last_copy_info:
            log.warning("This is the first log backup.")
            last_copy_info = self.get_last_copy_info(1)
        else:
            last_exec_node = last_copy_info.get("extendInfo", {}).get("exec_node", "")
            if last_exec_node != curr_exec_node:
                log.warning(f"last_exec_node {last_exec_node}, curr_exec_node {curr_exec_node}, not equal.")
                last_copy_info = self.get_last_copy_info(1)
        log.info(f"last copy info is {last_copy_info}")
        return last_copy_info
