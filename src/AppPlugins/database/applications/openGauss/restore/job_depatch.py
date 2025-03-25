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

import os
import sys

from common.common import exter_attack
from common.common_models import ActionResult, SubJobDetails
from common.const import ExecuteResultEnum, SubJobStatusEnum, DeployType
from common.logger import Logger
from openGauss.backup.resource_info import ResourceInfo
from openGauss.common.common import get_value_from_dict, get_dbuser_gname, check_path, safe_get_environ
from openGauss.common.common_models import JobPermission
from openGauss.common.const import ParamKey, Status, JobType, ProgressPercentage, NodeRole, AuthKey
from openGauss.common.error_code import OpenGaussErrorCode
from openGauss.restore.database_restore import DatabaseRestore


class JobDepatch:
    log = Logger().get_logger()

    def __init__(self, pid, job_id, param):
        self._pid = pid
        self._job_id = job_id
        self._param = param
        self._user_name = ""
        self._resource_obj = None
        self.init_environment()
        self.depatch = {
            JobType.QUERY_JOB_PERMISSION: self.query_job_permission,
            JobType.ALLOW_RESTORE_IN_LOCAL_NODE: self.allow_restore_in_local_node,
            JobType.RESTORE_PREREQUISITE: self.restore_prerequisite,
            JobType.RESTORE: self.restore,
            JobType.RESTORE_POST: self.restore_post,
            JobType.PREREQUISITE_PROGRESS: self.prerequisite_progress,
            JobType.RESTORE_PROGRESS: self.restore_progress,
            JobType.POST_PROGRESS: self.post_progress
        }

    def init_environment(self):
        self._user_name = safe_get_environ(f"{AuthKey.APPLICATION_ENV}{self._pid}")
        if not self._user_name:
            self._user_name = safe_get_environ(f"{AuthKey.TARGET_ENV}{self._pid}")
        ret, env_file = get_value_from_dict(self._param, ParamKey.JOB, ParamKey.TARGET_ENV, ParamKey.EXTEND_INFO,
                                            ParamKey.ENV_FILE)
        if isinstance(env_file, str) and os.path.isfile(env_file) and check_path(env_file):
            self._resource_obj = ResourceInfo(self._user_name, env_file)
        else:
            self._resource_obj = ResourceInfo(self._user_name)

    @exter_attack
    def query_job_permission(self):
        """
        查询任务权限
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f'Execute query job permission job. job id: {self._job_id}')
        group_name = get_dbuser_gname(self._user_name)
        if not group_name:
            self.log.error(f"Get user name or group name failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Get user name faild")
            return False, output.dict(by_alias=True)
        output = JobPermission(user=self._user_name, group=group_name, fileMode="0700")
        self.log.info(f"Execute query job permission job success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    @exter_attack
    def allow_restore_in_local_node(self):
        """
        检查本节点是否支持恢复
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """

        self.log.info(f"Execute allow restore in local node job. job id: {self._job_id}")

        restore_obj = DatabaseRestore(self._pid, self._job_id, self._param)
        ret, code = restore_obj.check_db_info()
        if not ret:
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, bodyErr=code,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)

        endpoint = self._resource_obj.get_local_endpoint()
        allow_status = self._resource_obj.get_node_status(endpoint)
        if allow_status and self._resource_obj.get_node_status(endpoint) != Status.NORMAL:
            self.log.error(f"Node status is abnormal. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  bodyErr=OpenGaussErrorCode.ERR_DATABASE_STATUS,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        if self._resource_obj.get_deploy_type() == DeployType.CLUSTER_TYPE:
            if self._resource_obj.get_node_role(endpoint) != NodeRole.PRIMARY:
                self.log.error(f"Database restore must in primary node. job id: {self._job_id}")
                output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                      message="The current can not exec restore job")
                return False, output.dict(by_alias=True)
        elif self._resource_obj.get_deploy_type() == DeployType.SHARDING_TYPE:
            self.log.info("Start CMDB dist database allow_restore_in_local_node")
            if not self._resource_obj.get_local_cn_port():
                self.log.error(f"Database restore must in node with cn. job id: {self._job_id}")
                output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR)
                return False, output.dict(by_alias=True)
        self.log.info(f"Execute allow restore in local node job success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        return True, output.dict(by_alias=True)

    @exter_attack
    def restore_prerequisite(self):
        """
        恢复前置
         :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Execute restore prerequisite. job id: {self._job_id}")
        restore_obj = DatabaseRestore(self._pid, self._job_id, self._param)
        if not restore_obj.restore_prerequisite(self._param):
            self.log.error(f"Execute restore prerequisite failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute restore prerequisite success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        return True, output.dict(by_alias=True)

    @exter_attack
    def restore(self):
        """
        恢复
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Execute restore. job id: {self._job_id}")
        restore_obj = DatabaseRestore(self._pid, self._job_id, self._param)
        if not restore_obj.restore():
            self.log.error(f"Execute restore failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute restore success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        return True, output.dict(by_alias=True)

    @exter_attack
    def restore_post(self):
        """
        恢复后置
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Execute restore post. job id: {self._job_id}")
        restore_obj = DatabaseRestore(self._pid, self._job_id, self._param)
        if not restore_obj.restore_post():
            self.log.error(f"Execute restore post failed. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        self.log.info(f"Execute restore post success. job id: {self._job_id}")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        return True, output.dict(by_alias=True)

    @exter_attack
    def prerequisite_progress(self):
        """
        恢复前置进度
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Get prerequisite progress. job id: {self._job_id}")
        output = SubJobDetails(taskId=self._job_id, subTaskId="", taskStatus=SubJobStatusEnum.COMPLETED.value,
                               progress=ProgressPercentage.COMPLETE_PROGRESS.value)
        self.log.info(f"Get prerequisite progress success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    @exter_attack
    def restore_progress(self):
        """
        恢复进度
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Get restore progress. job id: {self._job_id}")
        restore_obj = DatabaseRestore(self._pid, self._job_id, self._param)
        speed, progress, status, progress_detail = restore_obj.restore_progress()
        if len(sys.argv) < 5:
            self.log.error(f"Bad sys agrv. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        sub_task_id = sys.argv[4]
        output = SubJobDetails(taskId=self._job_id, subTaskId=sub_task_id, taskStatus=status,
                               progress=progress, speed=speed)
        if progress_detail:
            output.log_detail = [progress_detail]
        self.log.info(f"Get restore progress finish. job id: {self._job_id}")
        return True, output.dict(by_alias=True)

    @exter_attack
    def post_progress(self):
        """
        恢复后置进度
        :return:
               ret: 执行成功返回True，执行失败返回false
               output: 结果输出字典
        """
        self.log.info(f"Get post progress. job id: {self._job_id}")
        if len(sys.argv) < 5:
            self.log.error(f"Bad sys agrv. job id: {self._job_id}")
            output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                  message="The current can not exec restore job")
            return False, output.dict(by_alias=True)
        sub_task_id = sys.argv[4]
        output = SubJobDetails(taskId=self._job_id, subTaskId=sub_task_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                               progress=ProgressPercentage.COMPLETE_PROGRESS.value)
        self.log.info(f"Get post progress success. job id: {self._job_id}")
        return True, output.dict(by_alias=True)
