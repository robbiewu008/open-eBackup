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
import shutil
import subprocess

from common.common import output_result_file, output_execution_result
from common.common_models import ActionResult
from common.common_models import SubJobDetails, LogDetail
from common.const import RepositoryDataTypeEnum, SubJobStatusEnum, ExecuteResultEnum, ParamConstant, DBLogLevel
from tdsql.common.const import EnvName
from tdsql.common.const import ErrorCode
from tdsql.common.safe_get_information import ResourceParam
from tdsql.common.tdsql_common import get_std_in_variable
from tdsql.common.tdsql_common import report_job_details
from tdsql.common.util import get_mysql_version_path, get_backup_pre_from_defaults_file_path
from tdsql.handle.common.const import TDSQLReportLabel
from tdsql.handle.livemount.livemount import live_mount, cancel_live_mount
from tdsql.logger import log


class TdsqlLivemountService(object):
    """
    及时挂载相关接口
    """

    @staticmethod
    def live_mount(req_id, job_id, sub_id, std_in):
        log.info(f"livemount request parameter is req_id:{req_id} job_id:{job_id} sub_id:{sub_id} req_id:{std_in}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        log.info(f"livemount request param is {param}")
        result = ""
        try:
            lm_inst = LiveMount(req_id, job_id, sub_id, std_in, param)
            result = lm_inst.livemount_task()
            log.info(f"live mount status is {result}")
        except Exception as exception_str:
            log.error(f'livemount exception {exception_str}')
            log_detail_param = []
            log_detail = LogDetail(logInfo="plugin_live_mount_subjob_fail_label", logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
        if not result:
            log_detail_param = []
            log_detail = LogDetail(logInfo="plugin_live_mount_subjob_fail_label", logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            output.code = ExecuteResultEnum.INTERNAL_ERROR
            return ExecuteResultEnum.INTERNAL_ERROR
        else:
            log.info("begin to report job status")
            log_detail_param = []
            log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.INFO.value,
                                   logDetailParam=log_detail_param)
            ret = report_job_details(req_id,
                                     SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                   logDetail=[log_detail],
                                                   taskStatus=SubJobStatusEnum.COMPLETED.value).dict(
                                         by_alias=True))
            log.info(f"report job status is {ret}")
            log.info("end to report job status")

            return ExecuteResultEnum.SUCCESS

    @staticmethod
    def cancel_live_mount(req_id, job_id, sub_id, std_in):
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        lm_inst = LiveMount(req_id, job_id, sub_id, std_in, param)
        result = lm_inst.cancel_livemount_task()
        if not result:
            output.code = ExecuteResultEnum.INTERNAL_ERROR
            log_detail_param = []
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=log_detail_param)

            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
        output_result_file(req_id, output.dict(by_alias=True))
        log.info("begin to report job status")
        log_detail_param = []
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[sub_id],
                               logLevel=DBLogLevel.INFO.value,
                               logDetailParam=log_detail_param)
        ret = report_job_details(req_id,
                                 SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                               logDetail=[log_detail],
                                               taskStatus=SubJobStatusEnum.COMPLETED.value).dict(
                                     by_alias=True))
        log.info(f"report job status is {ret}")
        log.info("end to report job status")
        return ExecuteResultEnum.SUCCESS


def exec_rc_tool_cmd(cmd, in_path, out_path):
    cmd = f"{os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh')} {cmd} {in_path} {out_path}"
    ret, out = subprocess.getstatusoutput(cmd)
    if ret != 0:
        log.error(f"An error occur in execute cmd. ret:{ret} err:{out}")
        return False
    return True


class LiveMount:

    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")

        self._pid = pid
        self._job_id = job_id
        if not self._job_id:
            log.warn(f"self._job_id: {self._job_id}")
        self._sub_job_id = sub_job_id
        if not self._sub_job_id:
            log.warn(f"_sub_job_id: {self._sub_job_id}")
        self._std_in = data
        self._json_param_object = json_param
        log.info(f"the parameter is {self._json_param_object}")
        self._copy_info = self._json_param_object.get("job", {}).get("copy", [])[0]
        self._copy_id = self._json_param_object.get("job", {}).get("copy", [])[0].get("id", "")
        self._host_ip = self._json_param_object.get("job", {}).get("targetEnv", {}).get("endpoint", "")
        self._oss_url = self._copy_info.get("protectObject", {}).get("extendInfo", {}).get("ossUrl", "")
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._instance_id = self._json_param_object.get("job", {}).get("targetEnv", {}).get("id", "")
        log.warn(f"self._instance_id: {self._instance_id}")
        self._protect_obj_extend_info = self._json_param_object.get("job", {}).get("copy", [])[0]. \
            get("protectObject", {}).get("extendInfo", {})
        self._cluster_info_ = json.loads(self._protect_obj_extend_info.get("clusterInstanceInfo"))
        log.info("before self.nodes")
        self.nodes = self._cluster_info_.get("groups", [])[0].get("dataNodes")
        self._mysql_conf_path = self.nodes[0].get("defaultsFile", "")
        log.warn(f"nodes: {self.nodes}")
        self._set_id = self._cluster_info_.get("id")
        log.info(f"self._set_id: {self._set_id}")
        self._mysql_version = self.get_mysql_version()
        self._sub_job_name = self._json_param_object.get("job", {}).get("subJob", {}).get("jobName", "")
        self._data_area = self.get_repository(RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._job_status = SubJobStatusEnum.RUNNING
        EnvName.IAM_USERNAME = "job_protectObject_auth_authKey"
        EnvName.IAM_PASSWORD = "job_protectObject_auth_authPwd"
        self.user_name = get_std_in_variable(f"{EnvName.IAM_USERNAME}_{pid}")
        if not self.user_name:
            log.error(f"Livemount: failed fetch user_name.{self.user_name}")
        # 调测使用
        else:
            log.info(f"Livemount user_name: {self.user_name}.")

        # 组装资源接入请求体
        self._extend_info = self._json_param_object.get("job", {}).get("extendInfo", {})
        self.job_param = self._json_param_object.get("job", {}).get("jobParam", {})
        self.request_id = self._json_param_object.get("job", {}).get("requestId", {})
        self.target_env = self._json_param_object.get("job", {}).get("targetEnv", {})
        self.target_object = self._json_param_object.get("job", {}).get("targetObject", {})
        self.sub_job = self._json_param_object.get("job", {}).get("subJob", {})
        if not self._extend_info:
            log.error(f"self._extend_info: {self._extend_info}")

    def call_rc_tool_cmd(self, cmd, data_repository):
        json_str = {
            "user": "tdsql",
            "fileMode": "0755"
        }
        param_json = {
            "repository": [data_repository],
            "permission": json_str,
            "extendInfo": self._json_param_object.get("job", {})
        }
        param_path = os.path.join(ParamConstant.PARAM_FILE_PATH, f"param_{self._job_id}")
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result_{self._job_id}")
        ret = self.create_param_file(param_path, param_json)
        if not ret:
            return False

        ret = exec_rc_tool_cmd(cmd, param_path, result_path)
        if not ret:
            log.warning(f"Exec rc tool failed. pid:{self._pid} jobId:{self._job_id}")
            return False
        try:
            os.remove(param_path)
        except Exception as exception_str:
            # 删除参数文件失败 不认为命令执行失败
            log.warning(f"Remove param path failed. pid:{self._pid} jobId:{self._job_id}")
        try:
            os.remove(result_path)
        except Exception as exception_str:
            # 删除参数文件失败 不认为命令执行失败
            log.warning(f"Remove result path failed. pid:{self._pid} jobId:{self._job_id}")
        return True

    def create_param_file(self, param_path, param_json):
        if os.path.exists(param_path):
            try:
                os.remove(param_path)
            except Exception as exception_str:
                log.error(f"Rm file failed. err:{exception_str} \
                        pid:{self._pid} jobId:{self._job_id}")
                return False
        log.info(f"Write:{param_json} to param_path. pid:{self._pid} jobId:{self._job_id}")
        try:
            output_execution_result(param_path, param_json)
        except Exception:
            log.error(f"Create file failed. path:param_path  \
                    pid:{self._pid} jobId:{self._job_id}")
            return False
        return True

    def get_repository(self, repository_type):
        repositories = self._json_param_object.get("job", {}).get("copy", [])[0].get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_path = repository
                log.info(f"repositories {repositories_path}")
                break
        return repositories_path

    def mount_local_path(self):
        data_repository = self.get_repository(RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not data_repository:
            log.error(f"Get data respository failed. pid:{self._pid} jobId:{self._job_id}")
            return False
        path = os.path.join("/mnt", self._job_id)
        # 调用挂载脚本
        ret = self.mount_by_script(path, data_repository)
        if not ret:
            return False
        return True

    def mount_by_script(self, local_path, data_repository):
        # 调用挂载脚本执行挂载
        path = [local_path]
        data_repository["path"] = path
        remote_host_array = data_repository.get("remoteHost", [])
        data_repository["remoteHost"] = []
        if not remote_host_array:
            log.error(f"Remote host array error. pid:{self._pid} jobId:{self._job_id}")
            return False
        for remote_host in remote_host_array:
            data_repository["remoteHost"].append(remote_host)
            ret = self.call_rc_tool_cmd("MountRepositoryByPlugin", data_repository)
            if ret:
                log.info(f"Rctool mount success. pid:{self._pid} jobId:{self._job_id}")
                return True
        log.error(f"Rctool mount failed. pid:{self._pid} jobId:{self._job_id}")
        return False

    def unmount_local_path(self, option):
        path = ""
        if option == "cancellivemount":
            path = os.path.join("/mnt", self.target_object.get("extendInfo", "").get("mountJobId", "") + "_live_mount")
        elif option == "livemount":
            path = os.path.join("/mnt", self._job_id)
        cmd = f"mount |grep {path}"
        return_code, out_info = subprocess.getstatusoutput(cmd)
        if return_code == 0:
            # 调用卸载脚本
            data_repository = self.get_repository(RepositoryDataTypeEnum.DATA_REPOSITORY)
            if not data_repository:
                log.error(f"Get data respository failed. pid:{self._pid} jobId:{self._job_id}")
                return False
            path_array = [path]
            data_repository["path"] = path_array
            ret = self.call_rc_tool_cmd("UnMountRepositoryByPlugin", data_repository)
            if not ret:
                log.error(f"Unmount failed. pid:{self._pid} jobId:{self._job_id}")
                return False
        else:
            log.info(f"Mount not exist. pid:{self._pid} jobId:{self._job_id}")
        return True

    def livemount_task(self):
        mysql_port = self._extend_info.get("mysql_port", 33060)
        mysql_version = self._mysql_version
        backup_pre = get_backup_pre_from_defaults_file_path(self._mysql_conf_path)
        log.info(f"livemount_task backup_path is {backup_pre}")
        ret, cnf_path = subprocess.getstatusoutput(f"ls {backup_pre}/*/{mysql_version}/etc/my_*.cnf | head -1")
        log.info(f"the base config file is {cnf_path}")
        if not cnf_path or not cnf_path.strip().startswith(backup_pre):
            log_detail = LogDetail(logInfo=TDSQLReportLabel.TDSQL_LIVE_MOUNT_MYSQL_VERSION_NOT_MATCH_LABEL,
                                   logInfoParam=[self._sub_job_id], logLevel=DBLogLevel.ERROR)
            sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value)
            report_job_details(self._pid, sub_dict.dict(by_alias=True))
            log.error("can not find the base cnf path")
            return False

        ret, output = subprocess.getstatusoutput(f"netstat -anopl | grep {mysql_port} | grep -w LISTEN | grep -v grep "
                                                 f"| wc -l")
        log.info(f"checking port {mysql_port} is used")
        if ret or int(output) > 0:
            self.report_port_message(mysql_port)
            log.error(f"checking port {mysql_port} failed, maybe port {mysql_port} is being used")
            return False

        pool_size = 2048
        ret = self.mount_local_path()
        if not ret:
            self.unmount_local_path("livemount")
            log.error(f"Mount local path failed. pid:{self._pid} jobid:{self._job_id}")
            return False
        log.info(f"Mount local path success. pid:{self._pid} jobid:{self._job_id}")
        ret, temp = subprocess.getstatusoutput(f"ls /mnt/{self._job_id}/tdsqlbackup/*/full/xtrabackup_info | head -1")
        if ret:
            log.error("can not find the backup path")
        mount_path = temp[:temp.rindex("/")]
        log.info(f"the backup path is {mount_path}")
        ret, output = subprocess.getstatusoutput(f"chown -R tdsql /mnt/{self._job_id}")
        if ret:
            log.error(f"can not change path /mnt/{self._job_id}/tdsqlbackup to owner tdsql, "
                      f"ret is {ret}, output is {output}")
        ret = live_mount(mount_path, mysql_port, cnf_path, pool_size, mysql_version)
        return ret

    def cancel_livemount_task(self):
        mount_job_id = self.target_object.get("extendInfo", "").get("mountJobId", "") + "_live_mount"
        mysql_port = self._copy_info.get("extendInfo", {}).get("mysql_port", 33060)
        log.info(f"targetObject is {self.target_object}, mountJobId is {mount_job_id}")
        ret, temp = subprocess.getstatusoutput(f"ls /mnt/{mount_job_id}/tdsqlbackup/*/full/xtrabackup_info | head -1")
        if ret:
            log.error("can not find the backup path")
            return False
        mount_path = temp[:temp.rindex("/")]
        log.info(f"the backup path is {mount_path}")

        ret = cancel_live_mount(mount_path, self._pid, self._job_id, mysql_port)
        if not ret:
            log.error("stop mysqld failed")
            return ret
        ret = self.unmount_local_path("cancellivemount")
        if not ret:
            log.error("umount live mount path failed")
            return ret
        if os.path.exists(f"/mnt/{mount_job_id}"):
            shutil.rmtree(f"/mnt/{mount_job_id}")
        return True

    def report_port_message(self, mysql_port):
        log_detail = LogDetail(logInfo="plugin_live_mount_subjob_fail_label", logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.ERROR, logDetail=ErrorCode.ERR_LIVE_MOUNT_PORT_USED,
                               logDetailParam=[f"{mysql_port}"])
        sub_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                 logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value)
        report_job_details(self._pid, sub_dict.dict(by_alias=True))

    def get_mysql_version(self):
        mysql_version = self._copy_info.get("extendInfo", {}).get("mysql_version", "")
        if not mysql_version:
            mysql_version = get_mysql_version_path(self._oss_url, self._set_id, "livemount", self._pid)
        if not mysql_version:
            mysql_version = get_backup_pre_from_defaults_file_path(self._mysql_conf_path)
        return mysql_version
