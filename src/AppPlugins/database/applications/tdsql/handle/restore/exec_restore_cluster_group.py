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
import random

from common.common import output_result_file, output_execution_result_ex, report_job_details, execute_cmd, \
    execute_cmd_list, is_clone_file_system
from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.const import ParamConstant, SubJobPolicyEnum, SubJobPriorityEnum, ReportDBLabel, CMDResult, \
    SubJobStatusEnum, DBLogLevel, SubJobTypeEnum
from common.util.exec_utils import exec_mkdir_cmd
from oceanbase.common.const import ArchiveType
from tdsql.common.const import TdsqlClusterGroupRestoreSubJobName, TdsqlRestoreStatus, RestoreParam, EnvNameValue
from tdsql.common.tdsql_common import report_job_details, extract_ip, chown_path_owner_to_tdsql, mount_bind_path, \
    umount_bind_path, check_ip, chown_owner_to_tdsql, get_log_uri, get_std_in_variable, get_remote_host_ips
from tdsql.handle.common.const import TDSQLRestoreTaskStatus, TDSQLReportLabel, TDSQLProtectKeyName
from tdsql.handle.requests.rest_requests import RestRequests
from tdsql.handle.restore.restore_common import get_cluster_group_sub_job_name, get_nodes_info, get_register_ip
from tdsql.logger import log
from tdsql.handle.common.tdsql_param import JsonParam


class RestoreClusterGroup:
    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        self._std_in = data
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param_object = json_param
        log.info(f"tdsql_group_restore _json_param_object: {self._json_param_object}")
        self._logdetail = None
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        self._meta_path = JsonParam.get_meta_path(json_param)
        self._data_path = JsonParam.get_data_path(json_param)
        self._log_path = JsonParam.get_log_path(json_param)
        self._job_status = SubJobStatusEnum.RUNNING
        self._restore_status = TdsqlRestoreStatus.RUNNING
        self._restore_progress = 0
        self._sub_job_name = ''
        self._job_json = self._json_param_object.get("job", {})
        self._group_id = self.get_copy_extend_info().get("group_id")
        self._job_extend_info = self._job_json.get("extendInfo", {})
        self._restore_time_stamp = self._job_extend_info.get("restoreTimestamp", "")
        self.is_new_cluster = self.check_is_new_cluster(self._json_param_object)
        log.info(f"tdsql_group_restore is_new_cluster: {self.is_new_cluster}")
        self._oss_nodes = self.get_oss_nodes()
        self._business_addr = f'http://{self._oss_nodes[0].get("ip")}:{self._oss_nodes[0].get("port")}/tdsql'
        self._restore_hosts = self.get_restore_hosts()
        self._repositories = json_param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        self._mount_type = self._job_extend_info.get("agentMountType")
        self._osad_ip_list = get_remote_host_ips(self._repositories)
        self._osad_auth_port = get_std_in_variable(f"{EnvNameValue.IAM_OSADAUTHPORT}_{self._pid}")
        self._osad_server_port = self._job_extend_info.get("OSADServerPort", "")

    @staticmethod
    def restore_prerequisite():
        error_code = SubJobStatusEnum.COMPLETED.value
        return True, error_code

    @staticmethod
    def check_is_new_cluster(json_param):
        protect_env = json_param.get("job").get("copies")[0].get("protectEnv")
        protect_cluster_id = protect_env.get("id", "")
        target_cluster_id = json_param.get("job", {}).get("targetEnv", {}).get("id", {})
        log.info(f"protect_cluster_id is {protect_cluster_id}, target_cluster_id is {target_cluster_id}")
        return protect_cluster_id != target_cluster_id

    @staticmethod
    def new_cluster_file_make_link(local_dir, dir_path, filenames):
        for file in filenames:
            local_file = os.path.join(local_dir, file)
            # 跳过文件名包含._COPYING_的文件
            if file.__contains__("_COPYING_"):
                continue
            dir_file = os.path.join(dir_path, file)
            if os.path.exists(local_file):
                log.info(f'new_cluster_file_make_link {local_file} exists')
                os.remove(local_file)
            if os.path.islink(local_file):
                log.info(f"new_cluster_file_make_link {local_file} islink")
                os.unlink(local_file)
            log.info(f'new_cluster_file_make_link local_file is {local_file}')
            cmd = f"ln -s {dir_file} {local_dir}"
            return_code, out_info, err_info = execute_cmd(cmd)
            if return_code != CMDResult.SUCCESS:
                log.error(f"new_cluster_file_make_link failed out_info: {out_info} err_info: {err_info}")
                return False
            # 删除本地local_dir目录下包含._COPYING_的文件
            delete_cmd = f"find {local_dir} -name '*_COPYING_*' -type f -delete"
            return_code, out_info, err_info = execute_cmd(delete_cmd)
            if return_code != CMDResult.SUCCESS:
                log.error(f"new_cluster_file_make_link delete_cmd failed out_info: {out_info} err_info: {err_info}")
                return False
        return True

    @staticmethod
    def make_link(local_group_path, dir_path, filenames, base_name):
        curr_set = os.path.basename(os.path.dirname(dir_path))
        local_dir = os.path.join(local_group_path, curr_set, base_name)
        for file in filenames:
            local_file = os.path.join(local_dir, file)
            # 跳过文件名包含._COPYING_的文件
            if file.__contains__("_COPYING_"):
                continue
            if not os.path.exists(local_file):
                # 如果本地目录不存在该日志副本文件，创建软链接
                dir_file = os.path.join(dir_path, file)
                if os.path.islink(local_file):
                    log.info(f"path islinked : {local_file}")
                    os.unlink(local_file)
                log.info(f'make_link local_file is {local_file}')
                cmd = f"ln -s {dir_file} {local_dir}"
                return_code, out_info, err_info = execute_cmd(cmd)
                if return_code != CMDResult.SUCCESS:
                    log.error(f"make_link failed out_info: {out_info} err_info: {err_info}")
                    return False
        return True

    @staticmethod
    def exec_unlink(dir_path, filenames):
        for file in filenames:
            file_path = os.path.join(dir_path, file)
            if os.path.islink(file_path):
                log.info(f"exec unlink path is : {file_path}")
                os.unlink(file_path)

    def allow_restore_in_local_node(self):
        # 如果是跨集群恢复，检查是否有zkmeta，没有就任务失败，提示没有zkmeta
        if self.is_new_cluster:
            extend_info_json = self.get_copy_extend_info()
            if not extend_info_json:
                log.error("get extend_info_json failed")
                return False
            has_zkmeta = extend_info_json.get("has_zkmeta", False)
            log.info(f"get has_zkmeta is {has_zkmeta}")
            if not has_zkmeta:
                log_detail = LogDetail(logInfo=TDSQLReportLabel.RESTORE_CHECK_ZKMETA_NOT_EXIT_LABEL,
                                       logInfoParam=[self._sub_job_id],
                                       logLevel=DBLogLevel.ERROR.value)
                report_job_details(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                       by_alias=True))
                return False
        return True

    def restore_prerequisite_progress(self):
        pre_job_status = SubJobStatusEnum.COMPLETED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=pre_job_status, progress=100, logDetail=self._logdetail)
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

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

    def write_progress_file(self, task_status, progress):
        log.info("start write_progress_file")
        log.info(f"start self._sub_job_name: {self._sub_job_name}")
        if task_status == SubJobStatusEnum.FAILED.value:
            log_detail_param = []
            self.set_logdetail_with_params(ReportDBLabel.SUB_JOB_FALIED, self._sub_job_id, 0,
                                           log_detail_param,
                                           DBLogLevel.ERROR.value)
        if task_status == SubJobStatusEnum.COMPLETED.value:
            self.set_logdetail_with_params(ReportDBLabel.SUB_JOB_SUCCESS, self._sub_job_id, 0, [],
                                           DBLogLevel.INFO.value)
            log.info("task_status == SubJobStatusEnum.COMPLETED.value")
        progress_str = SubJobDetails(taskId=self._job_id,
                                     subTaskId=self._sub_job_id,
                                     taskStatus=task_status,
                                     progress=progress,
                                     logDetail=self._logdetail)
        json_str = progress_str.dict(by_alias=True)
        if not os.path.exists(self._cache_area):
            exec_mkdir_cmd(self._cache_area)
        progress_file = os.path.join(self._cache_area, f"progress_{self._job_id}_{self._sub_job_id}")
        log.debug(f"Write file.{progress_str}{self.get_log_comm()}.")
        output_execution_result_ex(progress_file, json_str)

    def upload_restore_progress(self):
        # 定时上报恢复进度
        while self._job_status == SubJobStatusEnum.RUNNING:
            if self._sub_job_name == TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_RESTORE:
                if self._restore_status == TdsqlRestoreStatus.RUNNING:
                    progress = self._restore_progress
                    status = SubJobStatusEnum.RUNNING
                elif self._restore_status == TdsqlRestoreStatus.SUCCEED:
                    status = SubJobStatusEnum.COMPLETED
                    progress = 100
                else:
                    status = SubJobStatusEnum.FAILED
                    progress = 0
                log.info(f"progress_info.job(status): {self._restore_status}")
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
            log.info(f"Get progress_dict in upload_restore_progress.{self._job_status}")
            log.info(f"upload_restore_progress{self.get_log_comm()}")
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

    def restore_task(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}_{extract_ip()[0]}")
        log.info(f"restore_task progress_file {progress_file}")
        self.write_progress_file(SubJobStatusEnum.RUNNING, 0)
        sub_job_dict = {
            TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_MYSQL_VERSION: self.sub_check_mysql_version,
            TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_HOST_AGENT: self.sub_check_host_agent,
            TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_MOUNT: self.sub_exec_mount,
            TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_RESTORE: self.sub_exec_restore
        }
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()
        # 开始执行子任务
        self.report_before_sub_job()
        sub_job_name = get_cluster_group_sub_job_name(self._json_param_object)
        log.info(f"get sub job name {sub_job_name}")
        if not sub_job_name:
            self._job_status = SubJobStatusEnum.FAILED
            return False
        self._sub_job_name = sub_job_name
        log.info(f"Exec sub job {sub_job_name} begin.{self.get_log_comm()}.")
        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as err:
            log.error(f"do {sub_job_name} fail: {err}")
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail_param = []
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

    def build_sub_job(self, job_priority, job_type, job_name, node_id, job_info):
        return SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=job_type, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def get_oss_nodes(self):
        target_obj_extend_info = self._job_json.get("targetEnv", {}).get("extendInfo", {})
        target_cluster_info_str = target_obj_extend_info.get("clusterInfo", "")
        target_cluster_info = json.loads(target_cluster_info_str)
        target_cluster_oss_nodes = target_cluster_info.get("ossNodes", [])
        oss_nodes = []
        for target_oss_node in target_cluster_oss_nodes:
            oss_node = {}
            oss_node["ip"] = target_oss_node.get("ip")
            oss_node["parentUuid"] = target_oss_node.get("parentUuid")
            oss_node["port"] = target_oss_node.get("port")
            oss_nodes.append(oss_node)
        log.info(f"get_oss_nodes oss_nodes is {oss_nodes}")
        return oss_nodes

    def get_restore_hosts(self):
        restore_hosts = json.loads(self._job_extend_info.get("restoreHosts"))
        nodes = []
        for restore_host in restore_hosts:
            host_node = {}
            host_node["ip"] = restore_host.get("ip")
            host_node["parentUuid"] = restore_host.get("parentUuid")
            nodes.append(host_node)
        return nodes

    def gen_sub_job(self):
        log.info(f"step 2-3 gen_sub_job _restore_hosts is {self._restore_hosts}")
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        log.info("step 2-3 gen_sub_job sub_job_01")
        # 子任务1：检查mysql版本
        job_name = TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_MYSQL_VERSION
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        choice_node = random.choice(self._restore_hosts)
        sub_job = self.gen_sub_job_info(choice_node, job_priority, job_name)
        sub_job_array.append(sub_job)

        # 子任务1：检查主机ip是否在对应的agent上
        job_name = TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_HOST_AGENT
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        for restore_host in self._restore_hosts:
            sub_job = self.gen_sub_job_info(restore_host, job_priority, job_name)
            sub_job_array.append(sub_job)

        # 子任务2：执行mount bind挂载和软链接
        log.info("step 2-3 gen_sub_job sub_job_02")
        job_name = TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_MOUNT
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
        # 所有的数据节点都要执行挂载子任务
        for restore_host in self._restore_hosts:
            sub_job = self.gen_sub_job_info(restore_host, job_priority, job_name)
            sub_job_array.append(sub_job)

        # 子任务3：执行恢复数据子任务（随机选取数据节点执行oss恢复接口）
        log.info("step 2-3 gen_sub_job sub_job_03")
        job_name = TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_RESTORE
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
        choice_node = random.choice(self._restore_hosts)
        sub_job = self.gen_sub_job_info(choice_node, job_priority, job_name)
        sub_job_array.append(sub_job)

        log.info(f"step 2-3 Sub-job splitting succeeded. sub_job_array:{sub_job_array}")
        output_execution_result_ex(file_path, sub_job_array)
        return True

    def gen_sub_job_info(self, node, job_priority, job_name):
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        node_id = node.get("parentUuid", "")
        host = node.get("ip", "")
        job_info = f"{host}"
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, job_info)
        return sub_job

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

    def sub_check_host_agent(self):
        log.info("step 2-4 sub_check_host_agent start")
        host_ip = self._json_param_object.get("subJob", "").get("jobInfo", "")
        if not check_ip(host_ip):
            log.error(f"step 2-4 sub_check_host_agent host_ip is invalid, host_ip is {host_ip}")
            return False
        cmd_list = ["ifconfig", f"grep {host_ip}", "wc -l"]
        return_code, out_info, err_info = execute_cmd_list(cmd_list)
        if int(out_info) <= 0:
            log.error(f"step 2-4 sub_check_host_agent host agent is not match, host_ip is {host_ip}")
            log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_CHECK_HOST_AGENT_NOT_MATCH,
                                   logInfoParam=[host_ip],
                                   logLevel=DBLogLevel.ERROR.value)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            return False
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        log.info("step 2-4 sub_check_host_agent end")
        return True

    def sub_check_mysql_version(self):
        extend_info_json = self.get_copy_extend_info()
        if not extend_info_json:
            log.error("step 2-4 get copy_db_version failed")
            return False
        copy_db_version = extend_info_json.get("db_version", "")
        log.info(f"step 2-4 get copy_db_version is {copy_db_version}")
        rest_request = RestRequests()
        env_variable = TDSQLProtectKeyName.IAM_USERNAME_RESTORE + self._pid
        db_version_list = rest_request.query_db_version(self._business_addr, env_variable)
        log.info(f"step 2-4 db_version_list is {db_version_list}")
        for db_version in db_version_list:
            version = db_version.get("db_version")
            if copy_db_version == version:
                self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
                return True
        log.warn(f"step 2-4 no match mysql version in target tdsql cluster")
        return False

    def new_cluster_link_log(self, local_group_path, id_list, log_uri):
        local_group_path = os.path.join(local_group_path, "autocoldbackup/sets")
        for current_copy_id in id_list:
            if is_clone_file_system(self._json_param_object):
                log_copy_path = os.path.join(log_uri, current_copy_id)
                chown_cmd = f"chown -R tdsql: {log_copy_path}"
                execute_cmd(chown_cmd)
            log_copy_dir = os.path.join(log_uri, current_copy_id, self._group_id)
            log.info(f'step 2-5 new_cluster_link_log, log_copy_dir is {log_copy_dir}')
            if not self.new_cluster_make_link(local_group_path, log_copy_dir, "binlog"):
                log.error(f"step 2-5 new_cluster_link_log  failed")
                return False
        return True

    def new_cluster_make_link(self, local_group_path, log_copy_dir, base_name):
        for dir_path, _, filenames in os.walk(log_copy_dir):
            if os.path.basename(dir_path) == base_name:
                log.info(f"step 2-5 new_cluster_link_log path is : {dir_path}")
                curr_set = os.path.basename(os.path.dirname(dir_path))
                local_dir = os.path.join(local_group_path, curr_set, base_name)
                if not self.new_cluster_file_make_link(local_dir, dir_path, filenames):
                    log.error(f"step 2-5 new_cluster_make_link failed")
                    return False
        return True

    def sub_exec_mount(self):
        log.info(f'step 2-5 sub_exec_mount _restore_time_stamp is {self._restore_time_stamp}')
        local_group_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id)
        log.info(f'step 2-5 sub_exec_mount local_group_path is {local_group_path}')
        # 创建本地副本目录
        self.make_local_dirs(local_group_path)
        chown_owner_to_tdsql(local_group_path)
        if self.is_new_cluster:
            # 异集群全量副本恢复，mount --bind /mnt/data仓目录  /tdsqlbackup/tdsqlzk/groupid
            log.info(f'step 2-5 sub_exec_mount data_path is {self._data_path}')
            if not mount_bind_path(self._data_path, local_group_path, self._mount_type,
                                   [self._osad_ip_list, self._osad_auth_port, self._osad_server_port, self._job_id]):
                log.error(f"step 2-5 sub_exec_mount mount_bind_path failed")
                return False
            # 挂载后修改group_path属主为tdsql
            chown_path_owner_to_tdsql("/tdsqlbackup/tdsqlzk/")
            chown_path_owner_to_tdsql(local_group_path)
            if self._restore_time_stamp:
                # 异集群日志副本恢复，创建软链接
                id_list, log_uri, data_uri = get_log_uri(self._job_json.get("copies"))
                if not self.new_cluster_link_log(local_group_path, id_list, log_uri):
                    log.error(f"step 2-5 sub_exec_mount new_cluster_link_log failed")
                    return False
        else:
            if not self._restore_time_stamp:
                # 原集群全量副本恢复，创建软链接
                self.exec_mount_full_data(local_group_path)
            else:
                # 原集群日志副本恢复，创建软链接
                id_list, log_uri, data_uri = get_log_uri(self._job_json.get("copies"))
                local_group_path = os.path.join(local_group_path, "autocoldbackup/sets")
                self.log_copy_mlink(id_list, log_uri, local_group_path)
                self.data_copy_mlink(data_uri, local_group_path)
        self.write_progress_file(SubJobStatusEnum.COMPLETED, 100)
        return True

    def make_local_dirs(self, local_group_path):
        if self.is_new_cluster:
            umount_bind_path(local_group_path)
        if not os.path.exists(local_group_path):
            log.info(f'step 2-5 makedirs local_group_path: {local_group_path}')
            os.makedirs(local_group_path)

        # 创建/tdsqlbackup/tdsqlzk/groupid（原实例id）/autocoldbackup/sets/setid/xtrabackup目录和binlog目录
        cluster_group_info_str = self._job_json.get("copies", [])[0].get("protectObject", {}). \
            get("extendInfo", {}).get("clusterGroupInfo", "")
        cluster_group_info = json.loads(cluster_group_info_str)
        set_ids = cluster_group_info.get("group").get("setIds", {})
        if not set_ids:
            log.error("step 2-5 get set_ids failed")
            return False
        log.info(f"step 2-5 set_ids is {set_ids}")
        local_group_path = os.path.join(local_group_path, "autocoldbackup/sets")
        for set_id in set_ids:
            loca_data_path = os.path.join(local_group_path, set_id, "xtrabackup")
            if not os.path.exists(loca_data_path):
                log.info(f'step 2-5 makedirs loca_data_path: {loca_data_path}')
                os.makedirs(loca_data_path)
            loca_binlog_path = os.path.join(local_group_path, set_id, "binlog")
            if not os.path.exists(loca_binlog_path):
                log.info(f'step 2-5 makedirs loca_binlog_path: {loca_binlog_path}')
                os.makedirs(loca_binlog_path)
        return True

    def exec_mount_full_data(self, local_group_path):
        # 遍历dataclone仓/autocoldbackup/sets/set_id/xtrabackup目录和binlog下所有副本文件，创建软链接
        if is_clone_file_system(self._json_param_object):
            chown_owner_to_tdsql(self._data_path)
        local_group_path = os.path.join(local_group_path, "autocoldbackup/sets")
        data_path = os.path.join(self._data_path, "autocoldbackup/sets")
        log.info(f'step 2-5 exec_mount_full_data, data_path is {data_path}')
        for dir_path, _, filenames in os.walk(data_path):
            if os.path.basename(dir_path) == 'xtrabackup':
                log.info(f"step 2-5 exec_mount_full_data xtrabackup path is : {dir_path}")
                if not self.make_link(local_group_path, dir_path, filenames, "xtrabackup"):
                    log.error(f"make_link xtrabackup failed")
                    return False
            if os.path.basename(dir_path) == 'binlog':
                log.info(f"step 2-5 exec_mount_full_data binlog path is : {dir_path}")
                if not self.make_link(local_group_path, dir_path, filenames, "binlog"):
                    log.error(f"make_link binlog failed")
                    return False
        return True

    def log_copy_mlink(self, id_list, log_uri, local_group_path):
        # 日志副本创建软链接
        for current_copy_id in id_list:
            log_copy_path = os.path.join(log_uri, current_copy_id)
            if is_clone_file_system(self._json_param_object):
                chown_owner_to_tdsql(log_copy_path)
            log_copy_dir = os.path.join(log_uri, current_copy_id, self._group_id)
            log.info(f'step 2-5 log_copy_mlink, log_copy_dir is {log_copy_dir}')
            for dir_path, _, filenames in os.walk(log_copy_dir):
                if os.path.basename(dir_path) == 'binlog':
                    self.make_link(local_group_path, dir_path, filenames, "binlog")
        return True

    def data_copy_mlink(self, data_uri, local_group_path):
        # 全量副本创建软链接
        if is_clone_file_system(self._json_param_object):
            chown_owner_to_tdsql(data_uri)
        data_path = os.path.join(data_uri, "autocoldbackup/sets")
        log.info(f'step 2-5 data_copy_mlink, data_path is {data_path}')
        for dir_path, _, filenames in os.walk(data_path):
            if os.path.basename(dir_path) == 'xtrabackup':
                self.make_link(local_group_path, dir_path, filenames, "xtrabackup")
        return True

    def get_copy_extend_info(self):
        copy = self._job_json.get("copies", [])[0]
        copy_type = copy.get("type", "")
        if copy_type in ArchiveType.archive_array:
            # 如果是归档到磁带副本，需要从extendInfo里拿原始备份副本
            return copy.get("extendInfo", {}).get("extendInfo", {})
        return copy.get("extendInfo", {})

    def get_restore_time(self):
        if not self._restore_time_stamp:
            log.info("step 2-6 restore from full data copy")
            extend_info_json = self.get_copy_extend_info()
            if not extend_info_json:
                log.error("step 2-6 get restore time failed")
                return False
            backup_end_time = extend_info_json.get("backup_end_time", 0)
            time_stamp = time.localtime(backup_end_time)
            restore_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        else:
            log.info("step 2-6 restore from log copy")
            time_stamp = time.localtime(int(self._restore_time_stamp))
            restore_time = time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)
        return restore_time

    def sub_exec_restore(self):
        log.info(f"step 2-6 sub_exec_restore")
        restore_time = self.get_restore_time()
        node_ips = []
        for restore_host in self._restore_hosts:
            node_ips.append(restore_host.get("ip"))

        log.info(f"step 2-6 sub_exec_restore, node_ip is {node_ips}, restore_time is {restore_time}")
        env_variable = TDSQLProtectKeyName.IAM_USERNAME_RESTORE + self._pid
        if self.is_new_cluster:
            taskid = self.exec_restore_to_new(node_ips, restore_time, env_variable)
        else:
            taskid = self.exec_restore_to_original(node_ips, restore_time, env_variable)
        if taskid == -1:
            log.error("step 2-6 exec_restore_cmd failed")
            return False
        while True:
            log.info(f"step 2-6 exec_restore query_restore_status, taskid is {taskid}")
            rest_request = RestRequests()
            if self.is_new_cluster:
                task_status, task_step, err_msg = rest_request.query_restore_status_new(self._business_addr, taskid,
                                                                                        env_variable)
            else:
                task_status, task_step, err_msg = rest_request.query_restore_status_original(self._business_addr,
                                                                                             self._group_id, taskid,
                                                                                             env_variable)
            log.info(
                f"step 2-6 exec_restore task_status is {task_status}, task_step is {task_step}, err_msg is {err_msg}")
            if task_status == TDSQLRestoreTaskStatus.SUCCEED:
                self._restore_status = TdsqlRestoreStatus.SUCCEED
                self._restore_progress = 100
                break
            elif task_status == TDSQLRestoreTaskStatus.FAILED:
                self._restore_status = TdsqlRestoreStatus.FAILED
                self._restore_progress = 0
                log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_GROUP_RESTORE_FAIL_LABEL,
                                       logInfoParam=[err_msg],
                                       logLevel=DBLogLevel.ERROR.value)
                report_job_details(self._pid,
                                   SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                       by_alias=True))
                break
            elif task_status >= TDSQLRestoreTaskStatus.RUNNING:
                self._restore_status = TdsqlRestoreStatus.RUNNING
                self._restore_progress = 20
            else:
                log.error(f'step 2-6 query_restore_status status invalid : {task_status}')
                self._restore_status = TdsqlRestoreStatus.FAILED
                self._restore_progress = 0
                break
            time.sleep(15)
        log.info("step 2-6 end to sub_exec_restore")
        return True

    def exec_restore_to_new(self, node_ips, restore_time, env_variable):
        rest_request = RestRequests()
        log.info(f"step 2-6 exec_restore_to_new job_extend_info:  {self._job_extend_info}")
        restore_param = RestoreParam(node_ips=node_ips, group_id=self._group_id, restore_time=restore_time,
                                     request_url=self._business_addr, env_variable=env_variable,
                                     job_extend_info=self._job_extend_info)
        task_id = rest_request.start_restore_to_new(restore_param)
        log.info(f"step 2-6 exec_restore_to_new is task_id {task_id}")
        return task_id

    def exec_restore_to_original(self, node_ips, restore_time, env_variable):
        rest_request = RestRequests()
        log.info(f"step 2-6 exec_restore_to_original job_extend_info:  {self._job_extend_info}")
        restore_param = RestoreParam(node_ips=node_ips, group_id=self._group_id, restore_time=restore_time,
                                     request_url=self._business_addr, env_variable=env_variable,
                                     job_extend_info=self._job_extend_info)
        task_id = rest_request.start_restore_to_original(restore_param)
        log.info(f"step 2-6 exec_restore_to_original is task_id {task_id}")
        return task_id

    def do_post(self):
        if self.is_new_cluster:
            group_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id)
            log.info(f"step 3 post job, umount group_path {group_path}")
            umount_bind_path(group_path, self._mount_type)
            if self._restore_time_stamp:
                local_group_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id, "autocoldbackup/sets")
                self.unlink_files_by_basename(local_group_path)
        else:
            log.info(f"step 3 post job, exec unlink")
            local_group_path = os.path.join("/tdsqlbackup/tdsqlzk/", self._group_id, "autocoldbackup/sets")
            self.unlink_files_by_basename(local_group_path)
        log.info(f"step 3 post job success, job id: {self._job_id}")

    def unlink_files_by_basename(self, local_group_path):
        for dir_path, _, filenames in os.walk(local_group_path):
            if os.path.basename(dir_path) == 'binlog':
                self.exec_unlink(dir_path, filenames)
            elif os.path.basename(dir_path) == 'xtrabackup':
                self.exec_unlink(dir_path, filenames)
            else:
                log.info(f"No need to unlink this directory, job id: {self._job_id}")
        log.info(f"All files in {local_group_path} are unlinked, job id: {self._job_id}")
