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
import os.path
import threading
import time

from common.cleaner import clear
from common.common import output_result_file, output_execution_result_ex, check_command_injection, clean_dir, \
    read_tmp_json_file
from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.const import ParamConstant, SubJobTypeEnum, DBLogLevel, SubJobPolicyEnum, SubJobPriorityEnum, \
    ExecuteResultEnum, SubJobStatusEnum
from common.parse_parafile import get_env_variable
from common.util.cmd_utils import cmd_format
from oracle.common.common import write_tmp_json_file
from tdsql.common.const import ErrorCode, TdsqlRestoreSubJobName, ActionResponse, HostParam, ArchiveType, \
    ConnectParam, MySQLVersion, BackupPath
from tdsql.common.const import TdsqlJsonConst
from tdsql.common.tdsql_common import report_job_details, extract_ip, execute_cmd_list, get_log_uri
from tdsql.common.util import is_slave, exec_sql, get_tdsql_status, get_version_path
from tdsql.handle.common.const import TDSQLReportLabel, TDSQLProtectKeyName, TDSQLDataNodeStatus, TDSQLRestoreTaskStatus
from tdsql.handle.common.tdsql_param import JsonParam
from tdsql.handle.requests.rest_requests import RestRequests
from tdsql.handle.restore.restore_common import check_mysql_status, get_sub_job_name, \
    need_log_restore, write_progress_file, execute_command, check_permission, start_mysql_service, \
    stop_mysql_service, stop_mysqlagent, xtrabackup_prepare, xtrabackup_restore, get_data_dir, get_dblogs_path, \
    mv_data_and_log_dir, get_master, get_url, get_port_and_ip, clean_backup_dir, copy_back_cnf_and_pem_file, \
    get_register_ip, get_nodes_info, create_deploy_conf, remove_deploy_conf, create_binlog_index
from tdsql.logger import log


class Restore:
    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        self._std_in = data
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param_object = json_param
        log.info(json_param)
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        self._meta_path = JsonParam.get_meta_path(json_param)
        self._data_path = JsonParam.get_data_path(json_param)
        self._job_status = SubJobStatusEnum.RUNNING
        self._copy_id = self._json_param_object.get("job", {}).get("copies", [{}, {}])[0].get("id", "")
        self._host_ip = self._json_param_object.get("job", {}).get("targetEnv", {}).get("endpoint", "")
        self._instance_id = self._json_param_object.get("job", {}).get("targetObject", {}).get("id", "")
        self._sub_name = self._json_param_object.get("subJob", {}).get("jobName", "")

        self._job_extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self._target_obj_extend_info = self._json_param_object.get("job", {}).get("targetObject", {}).get(
            "extendInfo", {})
        self._create_new_instance = self._json_param_object.get("job", {}).get("extendInfo", {}).get(
            "create_new_instance", "false")
        if self._create_new_instance == "true" and self._sub_name in (
                TdsqlRestoreSubJobName.SUB_EXEC_PREPARE, TdsqlRestoreSubJobName.SUB_CHECK_MYSQL_VERSION,
                TdsqlRestoreSubJobName.SUB_EXEC_RESTORE):
            self._instance_info_ = self.read_cluster_instance_info()
        else:
            self._instance_info_str = self._target_obj_extend_info.get("clusterInstanceInfo", "")
            self._instance_info_ = json.loads(self._instance_info_str)
        self._set_id = self._instance_info_.get("id", "")
        self.nodes = self._instance_info_.get("groups", [])[0].get("dataNodes", "")

        self._mysql_socket = self.nodes[0].get("socket", "")
        self._mysql_conf_path = self.nodes[0].get("defaultsFile", "")
        self._cluster_id = self._json_param_object.get("job", {}).get("targetEnv", {}).get("id", {})
        self._extend_info = self._json_param_object.get("job", {}).get("targetEnv", {}).get("extendInfo", {})

        self._cluster_info_str = self._extend_info.get("clusterInfo", "")
        self._cluster_info = json.loads(self._cluster_info_str)
        self._scheduler_nodes = self._cluster_info.get("schedulerNodes", "")
        self._oss_nodes = self._cluster_info.get("ossNodes", "")
        self.restore_type = self._json_param_object.get("job", {}).get("jobParam", {}).get("restoreType", "")

        self.user_name = "tdsql"
        self._mysql_ip = "127.0.0.1"
        self._mysql_port = 4002
        self._job_json = self._json_param_object.get("job", {})
        self._auth = self._json_param_object.get("job", {}).get("targetObject", {}).get("auth", "")
        self._max_change_slave_status_times = 3

    @staticmethod
    def restore_prerequisite():
        """
        执行前置任务：空
        ERROR_CODE待确认
        @return:
        """
        error_code = SubJobStatusEnum.COMPLETED.value
        return True, error_code

    def allow_restore_in_local_node(self):
        ret, master_user, master_password = self.get_master_user_and_password()
        mysql_version = self._mysql_conf_path.split("/")[4]
        if mysql_version.startswith(MySQLVersion.MARIADB):
            connect_param = ConnectParam(socket='', ip=self._mysql_ip, port=str(self._mysql_port))
        else:
            connect_param = ConnectParam(socket=self._mysql_socket, ip=self._mysql_ip, port=str(self._mysql_port))
        try:
            ret, output = exec_sql(master_user, master_password, connect_param, "show databases")
        except Exception as exception_str:
            log.error(f"master password incorrect {exception_str}")
            ret = False
        finally:
            clear(master_password)
        if not ret:
            response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                      bodyErr=ErrorCode.ERR_TDSQLSYS_REPL_PWD,
                                      message="master password incorrect")
            output_result_file(self._pid, response.dict(by_alias=True))
            log.error(f"master password incorrect ret:{ret} pid:{self._pid} jobId{self._job_id}")
            return False
        return True

    def restore_prerequisite_progress(self):
        """
        执行前置任务：空
        @return:
        """
        pre_job_status = SubJobStatusEnum.COMPLETED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=pre_job_status, progress=100, logDetail=self._logdetail)
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    def get_progress(self):
        log.info('Query restore progress!')
        status = SubJobStatusEnum.RUNNING
        progress = 0
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f'get_progress progress_file {progress_file}')
        if not os.path.exists(progress_file):
            status = SubJobStatusEnum.FAILED
            log.error(f"Progress file: {progress_file} not exist")
            return status, progress
        log.info(f'Path exist')
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
        if 'FAILED' in data:
            status = SubJobStatusEnum.FAILED
            progress = 100
            # 可设置错误码
            return status, progress
        if 'SUCCEED' in data:
            status = SubJobStatusEnum.COMPLETED
            progress = 100
        else:
            progress = 0
        log.info(f'Progress is {progress}')
        return status, progress

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def set_logdetail(self, err_code):
        err_dict = LogDetail(logInfo='', logInfoParam=[], logTimestamp=0, logDetail=0, logDetailParam=[],
                             logDetailInfo=[], logLevel=DBLogLevel.ERROR.value)
        err_dict.log_detail = err_code
        self._logdetail = []
        self._logdetail.append(err_dict)
        return True

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
        while self._job_status == SubJobStatusEnum.RUNNING:
            log.info("Start to report progress.")
            task_status, progress = self.get_progress()
            progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.RUNNING,
                                          progress=0, logDetail=self._logdetail)
            if task_status == SubJobStatusEnum.FAILED:
                self.set_logdetail_with_params("plugin_restore_subjob_fail_label", self._sub_job_id,
                                               0, [], DBLogLevel.ERROR.value)
            progress_dict.task_status = task_status
            progress_dict.progress = progress
            self._job_status = task_status
            report_job_details(self._job_id, progress_dict.dict(by_alias=True))
            time.sleep(self._query_progress_interval)

    def restore_task(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f"restore_task progress_file {progress_file}")
        try:
            write_progress_file('START', progress_file)
        except Exception as ex:
            log.error(f"write progress file failed {ex}")
            return False
        sub_job_dict = {
            TdsqlRestoreSubJobName.SUB_EXEC_CREATE_INSTANCE: self.sub_job_create_instance,
            TdsqlRestoreSubJobName.SUB_EXEC_PREPARE: self.sub_job_prepare,
            TdsqlRestoreSubJobName.SUB_CHECK_MYSQL_VERSION: self.sub_job_check,
            TdsqlRestoreSubJobName.SUB_EXEC_RESTORE: self.sub_job_exec
        }
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()
        # 开始执行子任务
        self.report_before_sub_job()
        # 执行子任务
        sub_job_name = get_sub_job_name(self._json_param_object)
        log.info(f"get sub job name {sub_job_name}")
        if not sub_job_name:
            return False
        log.info(f"Exec sub job {sub_job_name} begin.{self.get_log_comm()}.")
        try:
            # 新建实例子任务时未创建实例，不获取ip和port
            if not (self._create_new_instance == "true" and self._sub_name == "sub_job_cre_instance"):
                ret, self._mysql_port, self._mysql_ip = get_port_and_ip(self.nodes)
                if not ret:
                    log.error("get port and ip from target env failed")
                    raise Exception("Get port and ip from target env failed.")
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as ex:
            log.error(f"Exec sub job {sub_job_name} failed. {ex} {self.get_log_comm()}.")
            ret = False
        if not ret:
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            # 任务失败清理oc_agent deploy.conf文件
            remove_deploy_conf(self._mysql_port)
            log_detail = self.report_error_result(progress_file)
            report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                        logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value
                                                        ).dict(by_alias=True))
            progress_thread.join()
            os.remove(progress_file)
            return False
        write_progress_file('SUCCEED', progress_file)
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[self._sub_job_id], logLevel=1)
        report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                    logDetail=[log_detail],
                                                    taskStatus=SubJobStatusEnum.RUNNING.value).dict(by_alias=True))
        log.info(f"Exec sub job {sub_job_name} success.{self.get_log_comm()}.")
        progress_thread.join()
        return True

    def report_before_sub_job(self):
        host_ip_list = get_nodes_info(self._json_param_object)
        reg_ip = get_register_ip(host_ip_list)
        log.debug(f'reg_ip {reg_ip}')
        if reg_ip:
            local_ip = reg_ip
        else:
            local_ip = extract_ip()[0]
        log_detail = LogDetail(logInfo="agent_start_execute_sub_task_success_label",
                               logInfoParam=[local_ip, self._sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self._pid,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                               by_alias=True))

    def report_error_result(self, progress_file):
        write_progress_file('FAILED', progress_file)
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
        log.info(f"report_error_result data {data}")
        if 'FAILED' in data and "The response message:" in data:
            message = data.split("The response message:")[1].replace('FAILED', "").replace('\n',
                                                                                           ' ').strip()

            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                   logDetailParam=["Restore", message])
        else:
            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value, logDetail=0)
        return log_detail

    def build_sub_job(self, job_priority, job_type, job_name, node_id, job_info):
        return SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=job_type, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        """
        执行生成子任务
        @return:
        """
        cluster_id = self._cluster_id
        data_nodes_list = self.nodes if self._create_new_instance != "true" else self.gen_new_instance_nodes()

        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []
        log.info("gen_sub_job sub_job_01")

        if self._create_new_instance == "true":
            # 子任务0：执行创建新实例操作
            job_type = SubJobPolicyEnum.FIXED_NODE.value
            job_name = TdsqlRestoreSubJobName.SUB_EXEC_CREATE_INSTANCE
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
            node = data_nodes_list[0]
            node_type = node.get(TdsqlJsonConst.NODETYPE, "")
            node_id = node.get(TdsqlJsonConst.PARENTUUID, "")
            job_info = f"{cluster_id} {node_type}"
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, job_info)
            log.info(f"sub job is {sub_job}")
            sub_job_array.append(sub_job)

        # 子任务1：执行prepare操作
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlRestoreSubJobName.SUB_EXEC_PREPARE
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        for node in data_nodes_list:
            node_type = node.get(TdsqlJsonConst.NODETYPE, "")
            node_id = node.get(TdsqlJsonConst.PARENTUUID, "")
            job_info = f"{cluster_id} {node_type}"
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, job_info)
            log.info(f"sub job is {sub_job}")
            sub_job_array.append(sub_job)
            break
        log.info(f"prepare sub job is {sub_job_array}")

        # 子任务2：检查mysql版本和服务状态
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlRestoreSubJobName.SUB_CHECK_MYSQL_VERSION
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
        for node in data_nodes_list:
            node_type = node.get(TdsqlJsonConst.NODETYPE, "")
            node_id = node.get(TdsqlJsonConst.PARENTUUID, "")
            job_info = f"{cluster_id} {node_type}"
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        # 子任务3：执行恢复数据子任务
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TdsqlRestoreSubJobName.SUB_EXEC_RESTORE
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_4
        for node in data_nodes_list:
            node_type = node.get(TdsqlJsonConst.NODETYPE, "")
            node_id = node.get(TdsqlJsonConst.PARENTUUID, "")
            job_info = f"{cluster_id} {node_type}"
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, job_info)
            sub_job_array.append(sub_job)

        log.info(f"Sub-job splitting succeeded.sub-job num:{len(sub_job_array)}")
        output_execution_result_ex(file_path, sub_job_array)
        return True

    def sub_job_check(self):
        log.info("check_mysql_version")
        if not self.check_mysql_version(self._mysql_conf_path):
            log.error("check mysql version failed")
            return False

        # 检查数据库是否停用
        log.info("check_mysql_status")
        if check_mysql_status(self._mysql_port):
            # 在/data/oc_agent/log/目录创建{port}_deploy.conf文件防止oc_agent将mysql自动拉起
            log.info("create {port}_deploy.conf")
            try:
                if not create_deploy_conf(self._mysql_port):
                    log.error(f"create {self._mysql_port}_deploy.conf failed.")
                    return False
            except Exception as ex:
                log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_ORIGINAL_LOCATION_RESTORE_FAIL,
                                       logInfoParam=[str(ex)],
                                       logLevel=DBLogLevel.ERROR.value)
                report_job_details(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value).dict(by_alias=True))
                log.error(f"create {self._mysql_port}_deploy.conf failed, no space left on device")
                return False
            log.info("stop_mysql_service")
            if not stop_mysql_service(self._mysql_port, self._mysql_conf_path):
                log.error("stop mysql failed.")
                return False
            log.warn("re-check mysql status")
            check_mysql_status(self._mysql_port)
        return True

    def sub_job_prepare(self):
        # prepare只执行一次，如果其他节点已经执行过则跳过
        log.info("xtrabackup prepare")
        target_dir = self.get_target_dir()
        if not target_dir:
            log.error("xtrabackup prepare get target_dir failed")
            return False

        if not xtrabackup_prepare(port=self._mysql_port, target_dir=target_dir, mysql_conf_path=self._mysql_conf_path):
            log.error("xtrabackup prepare fail")
            return False
        log.info("xtrabackup prepare success")
        return True

    def sub_job_create_instance(self):
        log.info("Start create new instance to restore")
        rest_request = RestRequests()
        ret, request_url = get_url(self._oss_nodes)
        if not ret:
            return False
        env_variable = TDSQLProtectKeyName.IAM_USERNAME_RESTORE + self._pid
        instance_info = self.get_instance_conf()
        # 创建实例
        ret, task_id = rest_request.create_tdsql_instance(request_url, env_variable, instance_info)
        if not ret:
            return False
        # 查询实例创建进度
        ret, instance_set_id = self.query_create_instance_progress(request_url, env_variable, task_id)
        if not ret:
            return False
        log.info(f"Begin init instance.")
        # 初始化实例
        ret, task_id = rest_request.init_tdsql_instance(request_url, env_variable, instance_set_id)
        if not ret:
            return False
        # 查询初始化实例进度
        ret, return_data = rest_request.query_init_instance(request_url, env_variable, task_id)
        if not ret:
            log.info(f"Initialize instance fail. pid:{self._pid}")
            return False
        # 将新建实例信息写入cache仓保存
        ret = self.write_cluster_instance_info(url=request_url, instance_set_id=instance_set_id)
        if not ret:
            log.info(f"Save new instance information fail. pid:{self._pid}")
            return False
        log.info(f"Create instance success. Set_id is {instance_set_id}. Return_data is {return_data}.")
        log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_CREATE_NEW_INSTANCE_SUCCESS_LABEL,
                               logInfoParam=[instance_set_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self._pid,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail],
                                         taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                               by_alias=True))
        return True

    def query_create_instance_progress(self, request_url, env_variable, task_id):
        log.info(f"step2-6 exec_restore query_restore_status, taskid is {task_id}")
        while True:
            rest_request = RestRequests()
            task_status, ret_data = rest_request.query_instance_progress(request_url, env_variable, task_id)
            task_step = ret_data.get("cur_step", "")
            err_msg = ret_data.get("err_msg", "")
            if task_status == TDSQLRestoreTaskStatus.SUCCEED:
                instance_set_id = ret_data.get("set")
                log.info(f'Create new instance success.')
                return True, instance_set_id
            elif task_status == TDSQLRestoreTaskStatus.FAILED:
                log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_CREATE_NEW_INSTANCE_FAIl_LABEL,
                                       logInfoParam=[err_msg],
                                       logLevel=DBLogLevel.ERROR.value)
                report_job_details(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                       by_alias=True))
                log.error(f"Create new instance failed.Task_step is {task_step},err_msg is {err_msg}. pid:{self._pid}")
                return False, ""
            elif task_status >= TDSQLRestoreTaskStatus.RUNNING:
                log.info(f'Create new instance job is running.Task_step is {task_step}, err_msg is {err_msg}')
            else:
                log.error(f'step 2-6 query_restore_status status invalid : {task_status}, err_msg is {err_msg}')
                return False, ""
            time.sleep(15)

    def get_instance_conf(self):
        """
        用于生成在新建实例时所需要的必需的配置信息
        """
        defined_config_info_str = self._job_extend_info.get("extParameter", "")
        defined_instance_config_info = defined_config_info_str if not defined_config_info_str \
            else json.loads(defined_config_info_str)
        default_instance_config_info = self._json_param_object.get(
            "job", {}).get("copies", [{}, {}])[0].get("extendInfo", {}).get("instance_config_info", {})
        new_instance_nodes_list = self.gen_new_instance_nodes()
        assign_ip = ";".join([node.get("ip") for node in new_instance_nodes_list])
        node_num = len(new_instance_nodes_list)
        instance_config_info = {
            "manual": False,
            "idc_flag": True,
            "lock": 1,
            "assign_ip": assign_ip,
            "nodeNum": node_num
        }
        new_instance_name = self._json_param_object.get("job", {}).get("extendInfo", {}).get("newInstanceName", "")
        if new_instance_name:
            instance_config_info.update({"instance_name": new_instance_name})
        dbversion = default_instance_config_info.get("dbversion", "")
        if not defined_instance_config_info:
            if not dbversion:
                instance_config_info.update(default_instance_config_info)
                return instance_config_info
            instance_config_info.update(default_instance_config_info)
            instance_config_info["dbversion"] = dbversion.split("-")[-1]
            return instance_config_info
        add_config_info = {
            "machine": defined_instance_config_info.get("machine"),
            "cpu": str(int(defined_instance_config_info.get("cpu")) * 100),
            "memory": str(int(defined_instance_config_info.get("memory")) * 1000),
            "data_disk": str(int(defined_instance_config_info.get("dataDisk")) * 1000),
            "log_disk": str(int(defined_instance_config_info.get("logDisk")) * 1000)
        }
        if not dbversion:
            instance_config_info.update(add_config_info)
            return instance_config_info
        instance_config_info.update(add_config_info)
        instance_config_info["dbversion"] = dbversion.split("-")[-1]
        return instance_config_info

    def get_target_dir(self):
        target_dir = os.path.join(self._data_path, "tdsqlbackup", self._copy_id, "full")
        if not os.path.exists(target_dir):
            ret, last_copy_id = self.get_last_copy_id()
            if not ret:
                log.error("get last copy id failed")
                return {}
            target_dir = os.path.join(self._data_path, "tdsqlbackup", last_copy_id, "full")
        return target_dir

    def sub_job_exec(self):
        data_dir = get_data_dir(self._mysql_socket)
        ret, dblogs_path = get_dblogs_path(self._mysql_conf_path)
        if not ret:
            return False

        log.info("01.mv_data_and_log_dir")
        if not mv_data_and_log_dir(data_dir=data_dir, dblogs_path=dblogs_path):
            log.error("01.mv_data_and_log_dir fail")
            return False

        target_dir = self.get_target_dir()
        if not target_dir:
            log.error("xtrabackup restore get target_dir failed")
            return False
        log.info("02.xtrabackup_restore")
        if not xtrabackup_restore(port=self._mysql_port, target_dir=target_dir, mysql_conf_path=self._mysql_conf_path):
            log.error("02.xtrabackup_restore fail")
            return False

        if not self.operate_file_power_after_restore(data_dir=data_dir, dblogs_path=dblogs_path):
            return False

        if not self.change_master_and_restart_cluster():
            return False

        if need_log_restore(self._pid, self._job_id, self._json_param_object):
            try:
                log.info("10.exec_restore_binlog")
                ret = self.exec_restore_binlog()
            except Exception as ex:
                log.error(f"exec_restore_binlog failed , ex: {ex}")
                return False
            if not ret:
                return False

        log.info("11. clean backup dir")
        if not clean_backup_dir(data_dir=data_dir, dblogs_path=dblogs_path):
            return False

        return True

    def do_post(self):
        xtrabackup_prepare_path = os.path.join(self._meta_path, "xtrabackup_prepare")
        if os.path.exists(xtrabackup_prepare_path):
            clean_dir(xtrabackup_prepare_path)
            log.info("clean dir xtrabackup_prepare_path")

        return True

    def xtrabackup_prepare_check(self):
        xtrabackup_prepare_file = os.path.join(self._meta_path, "xtrabackup_prepare", "xtrabackup_prepare")
        xtrabackup_prepare_path = os.path.join(self._meta_path, "xtrabackup_prepare")
        if not os.path.exists(xtrabackup_prepare_path):
            try:
                os.mkdir(xtrabackup_prepare_path)
            except Exception as err:
                log.error(f"Make dir for {xtrabackup_prepare_path} err: {err}.")
        shared_file = read_tmp_json_file(xtrabackup_prepare_file)
        log.info(f'shared_file {shared_file}')
        if not shared_file:
            output_execution_result_ex(xtrabackup_prepare_file, self._pid)
            return True
        time.sleep(5)
        return False

    def exec_restore_binlog(self):
        ret, copy_start_time, copy_start_time_stamp = self.get_copy_start_time()
        if not ret:
            log.error("get copy start time failed")
            return False
        log.info(f"Copy start time: {copy_start_time}")
        ret, restore_time_stamp = self.parse_restore_log_parameter()
        if not ret:
            log.error("Get restore time stamp failed.")
            return False
        time_stamp = time.localtime(int(restore_time_stamp))
        date_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        log.info(f"Get time_stamp: {time_stamp}, date_time: {date_time}")

        restore_cmd = "mysqlbinlog --no-defaults "
        id_list, log_uri, data_uri = get_log_uri(self._job_json.get("copies"))
        log.info(f"exec_restore_binlog log_uri {log_uri}, id_list {id_list}")
        log_files = ""
        for log_id in id_list:
            log_path = os.path.join(log_uri, log_id)
            ret, log_file_names = self.get_log_file_names(int(copy_start_time_stamp), int(restore_time_stamp), log_path)
            if not ret:
                return False
            log_files += log_file_names
        log.info(f"exec_restore_binlog log_files {log_files}")
        restore_cmd += log_files
        if not self.log_restore_exec_cmd(restore_cmd, copy_start_time, date_time):
            return False
        log.info(f"Restore log success. pid:{self._pid} jobId{self._job_id}")
        return True

    def get_restore_mount_path(self, repositories_type):
        job_json = self._json_param_object.get(TdsqlJsonConst.JOB, {})
        result_path = ""
        if not job_json:
            return ""
        copies_json = job_json.get(TdsqlJsonConst.COPIES, [])
        if not copies_json:
            return ""
        # 适配UBC改动，取data仓时，取最后一个副本的path，因为日志恢复，原生格式UBC也会下发依赖的
        # 所有副本，但是只有最后一个副本会克隆
        for copy in copies_json:
            repositories_json = copy.get(TdsqlJsonConst.REPORITTORIES, [])
            for repo in repositories_json:
                if repo.get(TdsqlJsonConst.REPORITORYTYPE, "") == repositories_type and \
                        repo.get(TdsqlJsonConst.PATH, "") != "":
                    path = repo.get(TdsqlJsonConst.PATH, [""])[0]
                    result_path = path if path else result_path
        if not result_path:
            log.info(f"Path is empty. pid:{self._pid} jobId{self._job_id}")
        return result_path

    def log_restore_exec_cmd(self, restore_cmd, copy_start_time, date_time):
        """
        日志恢复，命令行执行
        :return:
        """
        authkey = "job_copies_0_protectObject_auth_authKey_"
        authpwd = "job_copies_0_protectObject_auth_authPwd_"
        user = get_env_variable(authkey + self._pid)
        password = get_env_variable(authpwd + self._pid)
        if check_command_injection(copy_start_time):
            log.error(f"Check copy start time command injection. pid:{self._pid} jobId:{self._job_id}")
            return False
        if check_command_injection(date_time):
            log.error(f"Check date time command injection. pid:{self._pid} jobId:{self._job_id}")
            return False
        if check_command_injection(user):
            log.error(f"Check user command injection. pid:{self._pid} jobId:{self._job_id}")
            return False
        mysql_version = self._mysql_conf_path.split("/")[4]
        skip_gtid_cmd = ""
        if not mysql_version.startswith(MySQLVersion.MARIADB):
            skip_gtid_cmd = "--skip-gtids"
        # 密码无法用命令注入来校验，使用shell自身的引号安全机制
        restore_cmd += cmd_format(" --disable-log-bin {} --start-datetime={} \
                                                  --stop-datetime={} | mysql -f --user={} --password={} -S {}",
                                  skip_gtid_cmd, copy_start_time, date_time, user, password, self._mysql_socket)
        try:
            ret = execute_command(restore_cmd, None)
        except Exception as ex:
            log.debug(f"start_mysql_service failed ex:{ex}")
            ret = False
        finally:
            clear(password)
            clear(restore_cmd)
        return ret

    def get_log_file_names(self, copy_start_time_stamp, end_time_stamp, log_path):
        """
        获取所有日志文件名
        """
        log_path_parent_dir = os.path.dirname(log_path)
        timestamp_id_dict = self.get_timestamp_id_dict(log_path_parent_dir)
        if not timestamp_id_dict:
            log.error(f"Get timestamp id dict is empty. pid:{self._pid} jobId:{self._job_id}")
            return False, ""

        file_names = ""
        for sub_dir in os.listdir(log_path_parent_dir):
            sub_dir_path = os.path.join(log_path_parent_dir, sub_dir)
            log.info(f"sub_dir_path is {sub_dir_path}")
            # 非文件、时间点恢复的开始时间大于等于时间戳结束，时间点恢复的结束时间小于等于时间戳开始，这三种情况不满足条件
            if not os.path.isdir(sub_dir_path):
                continue
            if sub_dir not in timestamp_id_dict:
                continue
            time_span = timestamp_id_dict[sub_dir]
            if not time_span:
                continue
            start_time_temp = int(time_span[:time_span.find("~")])
            if start_time_temp >= end_time_stamp:
                continue
            end_time_temp = int(time_span[time_span.rfind("~") + 1:])
            if end_time_temp <= copy_start_time_stamp:
                continue
            ret, partial_file_name = self.get_binlog_name(sub_dir_path, sub_dir)
            if not ret:
                log.error(f"Check new path command injection. pid:{self._pid} jobId:{self._job_id}")
                return False, ""
            file_names += partial_file_name

        return True, file_names

    def get_timestamp_id_dict(self, log_path_parent_dir):
        """
        从副本id.meta文件，组装timestamp和id的字典，如下
        {'788dee39-7123-453e-a697-af6a94896bfb': '1664939072~1665040898'}
        """
        restore_copy_id = self._json_param_object.get(TdsqlJsonConst.JOB, {}).get(TdsqlJsonConst.EXTENDINFO, {}).get(
            TdsqlJsonConst.RESTORE_COPY_ID, "")
        log.info(f"Get restore copy id: {restore_copy_id}. pid:{self._pid} jobId:{self._job_id}")

        restore_copy_id_file = restore_copy_id + ".meta"
        dot_meta_path = os.path.join(log_path_parent_dir, restore_copy_id_file)
        timestamp_id_dict = {}
        if not os.path.exists(dot_meta_path) or not dot_meta_path:
            log.error(f"Get timestamp id dict is empty. pid:{self._pid} jobId:{self._job_id}")
            return timestamp_id_dict
        try:
            with open(dot_meta_path, 'r', encoding='utf-8') as file_read:
                for line in file_read.readlines():
                    key_value = line.strip('\n').split(";")
                    key = key_value[0].strip()
                    value = key_value[1].strip()
                    timestamp_id_dict[key] = value
        except Exception as exception_str:
            log.error(f"Open meta file failed. pid:{self._pid} jobId:{self._job_id}")
            return {}
        log.info(f"Get timestamp id dict is {timestamp_id_dict}. pid:{self._pid} jobId:{self._job_id}")
        return timestamp_id_dict

    def get_host_sn_in_log_backup(self, log_path_parent_dir, end_time_stamp, timestamp_id_dict):
        """
        从日志副本中查找节点的hostsn
        """
        for sub_dir in os.listdir(log_path_parent_dir):
            sub_dir_path = os.path.join(log_path_parent_dir, sub_dir)
            # 非文件、时间点恢复的结束时间大于时间戳结束，这两种情况不满足条件
            if not os.path.isdir(sub_dir_path):
                continue
            if sub_dir not in timestamp_id_dict:
                continue
            time_span = timestamp_id_dict[sub_dir]
            if not time_span:
                continue
            end_time_temp = time_span[time_span.rfind("~") + 1:]
            if end_time_stamp > end_time_temp:
                continue
            for host_sn_path in os.listdir(sub_dir_path):
                log.debug(f"Find host sn in path. pid:{self._pid} jobId:{self._job_id}")
                return host_sn_path[host_sn_path.rfind("_") + 1:]
        return ""

    def get_binlog_name(self, binlog_parent_path, copy_id):
        """
        获取日志文件名
        """
        binlog_path = os.path.join(binlog_parent_path, "tdsqlbackup", copy_id, "binlog")
        log.info(f"binlog_path is {binlog_path}")
        log.info(f"sub dir list is {os.listdir(binlog_path)}")
        binlog_index_file = "binlog.index"
        index_file_path = os.path.join(binlog_path, binlog_index_file)
        if not os.path.exists(index_file_path) or not index_file_path:
            log.error(
                f"get binlog name failed,index file {index_file_path} not exists. pid:{self._pid} jobId:{self._job_id}")
            return False, ""

        file_names = ""
        with open(index_file_path, 'r', encoding='utf-8') as file_read:
            for line in file_read.readlines():
                binlog_file_name = line.rsplit('/', 1)[1].strip()
                log.info(f"binlog_file_name is {binlog_file_name}")
                binlog_file_path = os.path.join(binlog_path, binlog_file_name)
                if not os.path.exists(binlog_file_path):
                    log.warn(f"binlog file {binlog_file_path} not exists, pid:{self._pid} jobId:{self._job_id}")
                    continue
                file_names += binlog_file_path
                file_names += " "
        log.info(f"get binlog names is {file_names}. pid:{self._pid} jobId:{self._job_id}")
        return True, file_names

    def get_copy_start_time(self):
        """
        从参数中获取副本开始时间
        :return: copy_start_time(以%Y-%m-%d %H:%M:%S格式返回)， copy_start_time_stamp(以时间戳格式返回)
        """
        copy_start_time_stamp = 0
        if not self._json_param_object:
            return False, "", ""
        if not self._job_json:
            log.error(f"Get job json failed. pid:{self._pid} jobId{self._job_id}")
            return False, "", ""
        # 日志恢复 下发的副本至少是两个 这里如果拿到的少于两个 就认为不正常 直接返回失败
        copies_json = self._job_json.get("copies", [])
        if len(copies_json) < 2:
            log.error(f"Get copies json failed. pid:{self._pid} jobId{self._job_id}")
            return False, "", ""
        # 日志恢复需要最后一个数据副本来恢复
        copy = copies_json[-2]
        extendinfo_json = copy.get("extendInfo", {})
        if not extendinfo_json:
            log.error(f"Get extendinfo json failed.")
            return False, "", ""
        backup_time = extendinfo_json.get("backup_time", 0)
        if backup_time != 0:
            copy_start_time_stamp = backup_time
        if copy_start_time_stamp == 0:
            log.error(f"Get copy start time stamp failed. pid:{self._pid} jobId{self._job_id}")
            return False, "", ""
        time_stamp = time.localtime(copy_start_time_stamp)
        copy_start_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        log.info(f"Get start time stamp: {time_stamp}, date time: {copy_start_time}. \
            pid:{self._pid} jobId{self._job_id}")
        return True, copy_start_time, copy_start_time_stamp

    def parse_restore_log_parameter(self):
        extend_info_json = self._job_json.get("extendInfo", {})
        if not extend_info_json:
            log.error(f"Get extend info failed. pid:{self._pid} jobId{self._job_id}")
            return False, ""
        restore_time_stamp = extend_info_json.get("restoreTimestamp", "")
        if not restore_time_stamp:
            log.error(f"Get restore time stamp failed. pid:{self._pid} jobId{self._job_id}")
            return False, ""
        log.debug(f"Get restore time stamp： {restore_time_stamp}")
        return True, restore_time_stamp

    def stop_slave(self, oss_url, agent_user, agent_pwd):
        output = ""
        mysql_version = self._mysql_conf_path.split("/")[4]
        if mysql_version.startswith(MySQLVersion.MARIADB):
            connect_param = ConnectParam(socket='', ip=self._mysql_ip, port=str(self._mysql_port))
        else:
            connect_param = ConnectParam(socket=self._mysql_socket, ip=self._mysql_ip, port=str(self._mysql_port))
        try:
            ret, output = exec_sql(agent_user, agent_pwd, connect_param, "stop slave")
        except Exception as exception_str:
            log.error(f"stop slave failed: {exception_str}")
            ret = False
        if not ret:
            log.error(f"Exec_sql failed. sql:stop slave ret:{ret} output is {output} "
                      f"pid:{self._pid} jobId{self._job_id}")
            return False

        try:
            ret, output = exec_sql(agent_user, agent_pwd, connect_param, "reset slave all")
        except Exception as exception_str:
            log.error(f"reset slave failed: {exception_str}")
            ret = False
        if not ret:
            log.error(f"Exec failed, sql:reset slave. ret:{ret} pid:{self._pid} jobId{self._job_id}")
            return False

        self.change_master(oss_url, agent_user, agent_pwd)

        try:
            ret, output = exec_sql(agent_user, agent_pwd, connect_param, "start slave")
        except Exception as exception_str:
            log.error(f"start slave failed, output {exception_str}")
            ret = False

        if not ret:
            log.error(f"Exec failed, sql:start slave. ret:{ret} pid:{self._pid} jobId{self._job_id}")
            return False
        return True

    def change_master(self, oss_url, agent_user, agent_pwd):
        ret, master_host = get_master(url=oss_url, set_id=self._set_id, task_type="restore", pid=self._pid)
        if not ret:
            log.error(f"get master failed. pid:{self._pid} jobId{self._job_id}")
            return False

        ret, master_user, master_password = self.get_master_user_and_password()
        if not ret:
            return False

        cmd_str = f"change master to master_host='{master_host}', master_port={self._mysql_port}, " \
                  f"master_user='{master_user}', master_password='{master_password}'"
        mysql_version = self._mysql_conf_path.split("/")[4]
        if not mysql_version.startswith(MySQLVersion.MARIADB):
            cmd_str += ", master_auto_position=1"
        log.info(f"change master cmd_str {cmd_str}")

        if mysql_version.startswith(MySQLVersion.MARIADB):
            connect_param = ConnectParam(socket='', ip=self._mysql_ip, port=str(self._mysql_port))
        else:
            connect_param = ConnectParam(socket=self._mysql_socket, ip=self._mysql_ip, port=str(self._mysql_port))
        try:
            ret, output = exec_sql(agent_user, agent_pwd, connect_param, cmd_str)
        except Exception as exception_str:
            log.error(f"change master failed output {exception_str}")
            ret = False
        finally:
            clear(master_password)
            clear(cmd_str)
        if not ret:
            log.error(f"Exec_sql failed. sql:change master. ret:{ret} pid:{self._pid} jobId{self._job_id}")
            return False
        return True

    def get_last_copy_id(self):
        last_copy_id_file = os.path.join(self._meta_path, "lastCopyId", "lastCopyId")
        log.info(last_copy_id_file)
        last_copy_id = read_tmp_json_file(last_copy_id_file)
        log.info(last_copy_id)
        if last_copy_id:
            return True, last_copy_id
        log.info("get last copy id failed")
        return False, ""

    def operate_file_power_after_restore(self, data_dir, dblogs_path):
        log.info("04.copy back conf and pem file")
        if not copy_back_cnf_and_pem_file(data_dir=data_dir):
            log.error("04.copy back cnf and pem file fail")
            return False

        # 5.x版本需要在dblogs_path下手动创建bin/binlog.index文件
        if not create_binlog_index(dblogs_path, self._mysql_conf_path):
            log.error("04.create binlog index file fail")
            return False

        log.info("05-1.check_permission")
        if not check_permission(data_dir=data_dir, os_user=self.user_name):
            log.error("05-1.check_permission fail")
            return False
        log.info("05-2.check_permission")
        if not check_permission(data_dir=dblogs_path, os_user=self.user_name):
            log.error("05-2.check_permission fail")
            return False
        return True

    def change_master_and_restart_cluster(self):
        log.info("06.start_mysql_service")
        if not start_mysql_service(self._mysql_port, self._mysql_conf_path):
            log.error("06.start_mysql_service fail")
            return False
        log.info("07.change master")
        ret, url = get_url(self._oss_nodes)
        if not ret:
            return False
        log.info(f'url:{url} ip:{self._mysql_ip} port:{self._mysql_port} set_id:{self._set_id}')
        host_param = HostParam(url=url, ip=self._mysql_ip, port=str(self._mysql_port))
        if is_slave(host_param, set_id=self._set_id, task_type="restore", pid=self._pid):
            log.info('07-1.stop_mysqlagent')
            if not stop_mysqlagent(self._mysql_port):
                log.error("stop mysqlagent failed.")
                return False

            log.info('07-2.change slave status')
            agent_user = get_env_variable("job_copies_0_protectObject_auth_authKey_" + self._pid)
            agent_pwd = get_env_variable("job_copies_0_protectObject_auth_authPwd_" + self._pid)
            change_result = self.stop_slave(url, agent_user, agent_pwd)
            clear(agent_pwd)
            if not change_result:
                log.error("change slave status failed.")
                return False

        log.info("08.remove oc_agent deploy.conf file")
        if not remove_deploy_conf(self._mysql_port):
            log.error("08.remove oc_agent deploy.conf file failed")
            return False
        return True

    def get_master_user_and_password(self):
        if self._create_new_instance == "true":
            authreplpwd = "job_copies_0_protectObject_auth_extendInfo_replPassword_"
        else:
            authreplpwd = "job_targetObject_auth_extendInfo_replPassword_"
        master_user = "tdsqlsys_repl"
        master_password = get_env_variable(authreplpwd + self._pid)

        if not master_password:
            log.error(f"get master user_name,user_password failed. pid:{self._pid} jobId:{self._job_id}")
            log_detail = LogDetail(logDetail=ErrorCode.ERR_TDSQLSYS_REPL_PWD,
                                   logInfoParam=[self._sub_job_id],
                                   logLevel=DBLogLevel.ERROR.value)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                   by_alias=True))
            return False, "", ""
        return True, master_user, master_password

    def check_mysql_version(self, mysql_conf_path):
        extend_info_json = self.get_copy_extend_info()
        if not extend_info_json:
            log.error("step 2-4 get copy_mysql_version failed, copy extend_info is empty")
            return False
        copy_mysql_version = extend_info_json.get("mysql_version", "")
        log.info(f"step 2-4 get copy_mysql_version is {copy_mysql_version}")
        if not copy_mysql_version:
            # 兼容副本中mysql版本未上报的场景
            log.warn("copy_mysql_version is empty")
            return True
        current_version = mysql_conf_path.split("/")[4]
        log.info(f"step 2-4 current_version is {current_version}")
        if copy_mysql_version != current_version:
            log.error(f"copy_mysql_version {copy_mysql_version} not match instance version {current_version}")
            log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_INSTANCE_RESTORE_VERSION_CHECK_FAIL_LABEL,
                                   logInfoParam=[copy_mysql_version, current_version], logLevel=DBLogLevel.ERROR.value)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            return False
        return True

    def get_copy_extend_info(self):
        copy = self._job_json.get("copies", [])[0]
        copy_type = copy.get("type", "")
        if copy_type in ArchiveType.archive_array:
            # 如果是归档到磁带副本，需要从extendInfo里拿原始备份副本
            return copy.get("extendInfo", {}).get("extendInfo", {})
        return copy.get("extendInfo", {})

    def gen_new_instance_info(self, instance_list, instance_set_id):
        cluster_instance_info = {}
        cluster_instance_info["id"] = instance_set_id
        cluster_instance_info["name"] = instance_set_id
        cluster_instance_info["cluster"] = self._cluster_id
        groups = []
        for instance in instance_list:
            group = dict()
            set_id = instance.get("id")
            data_nodes = []
            db_version = instance.get("db_version")
            for db in instance.get("db"):
                port = str(db.get("port"))
                data_node = dict()
                data_node["setId"] = set_id
                data_node["ip"] = db.get("ip")
                data_node["port"] = port
                data_node["isMaster"] = db.get("master")
                conf_file = "my_" + port + ".cnf"
                log.info(f"port:{port}")
                version_path = get_version_path(db_version)
                defaults_file = os.path.join(BackupPath.BACKUP_PRE, port, f"{version_path}/etc", conf_file)
                data_node["defaultsFile"] = defaults_file
                # 改成从my.conf文件读取
                exec_cmd_list = [f"cat {defaults_file}", "grep mysql.sock", "head -n 1", "awk -F '=' '{print $2}'"]
                _, out_info, _ = execute_cmd_list(exec_cmd_list)
                data_node["socket"] = out_info.strip()
                data_node["linkStatus"] = TDSQLDataNodeStatus.ONLINE if db.get(
                    "alive") == 0 else TDSQLDataNodeStatus.OFFLINE
                data_nodes.append(data_node)
            group["setId"] = set_id
            group["dataNodes"] = data_nodes
            groups.append(group)
        cluster_instance_info["groups"] = groups
        log.info(f"Create new instance clusterInstanceInfo to restore success. set_id is {instance_set_id}")
        return cluster_instance_info

    def write_cluster_instance_info(self, url, instance_set_id):
        file_path = os.path.join(self._cache_area, f'clusterInstanceInfo_{self._job_id}')
        log.info(f"file_path:{file_path}")
        instance_list = get_tdsql_status(url, instance_set_id, "restore", self._pid)
        if not instance_list:
            return False
        cluster_instance_info = self.gen_new_instance_info(instance_list, instance_set_id)
        write_tmp_json_file(file_path, self._job_id, cluster_instance_info)
        return True

    def read_cluster_instance_info(self):
        file_path = os.path.join(self._cache_area, f'clusterInstanceInfo_{self._job_id}')
        cluster_instance_info = read_tmp_json_file(file_path)
        return cluster_instance_info

    def gen_new_instance_nodes(self):
        new_instance_nodes_str = self._json_param_object.get("job", {}).get(
            "extendInfo", {}).get("restoreHosts", "")
        new_instance_nodes = new_instance_nodes_str if not new_instance_nodes_str else json.loads(
            new_instance_nodes_str)
        return new_instance_nodes
