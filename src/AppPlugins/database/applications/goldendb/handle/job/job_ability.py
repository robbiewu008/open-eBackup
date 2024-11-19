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
import signal

import psutil

from common.common import exter_attack, output_result_file
from common.common_models import ActionResult, JobPermissionInfo
from common.const import ExecuteResultEnum
from common.job_const import JobNameConst
from goldendb.logger import log
from goldendb.handle.common.goldendb_param import JsonParam


class JobAbility:
    """
    任务相关接口
    """

    @staticmethod
    @exter_attack
    def query_job_permission(req_id, job_id, sub_id, data):
        """
        功能描述：设置文件系统权限, 主任务执行
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f"step 2: execute QueryJobPermission interface pid:{req_id} job_id:{job_id}")
        output = JobPermissionInfo(user="root", group="goldendb", fileMode="0770")
        output_result_file(req_id, output.dict(by_alias=True))
        log.info(f"step 2: execute QueryJobPermission interface success")

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_id, data):
        """
        功能描述：执行中止任务, 页面触发, 不会执行后置任务
        参数：
        @pid： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'execute to abort_job, job_id:{job_id},sub_job_id:{sub_id}')
        JsonParam.parse_param_with_jsonschema(req_id)
        if job_id is None or len(job_id) == 0:
            return
        processes = psutil.process_iter()
        for process in processes:
            cmd_lines = process.cmdline()
            if len(cmd_lines) < 5:
                continue
            if str(job_id) == str(cmd_lines[4]) and (
                    str(cmd_lines[2]) in [JobNameConst.BACKUP_PRE, JobNameConst.BACKUP_GEN_SUB_JOB,
                                          JobNameConst.BACKUP]):
                os.kill(process.pid, signal.SIGKILL)
                return
        output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))

    @staticmethod
    @exter_attack
    def pause_job(req_id, job_id, sub_id, data):
        """
        功能描述：执行暂停任务，任务流程中触发, 会执行后置任务
        参数：
        @pid： 请求ID
        @job_id： 主任务任务ID
        返回值：CommonBodyResponse
        """
        log.info(f'execute to pause_job, pid: {req_id}, job_id: {job_id}')
        JsonParam.parse_param_with_jsonschema(req_id)
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
