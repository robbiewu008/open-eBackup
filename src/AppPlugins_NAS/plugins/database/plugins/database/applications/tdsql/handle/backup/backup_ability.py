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

from common.common import exter_attack, output_result_file
from common.common_models import ActionResult, SubJobDetails
from common.const import ExecuteResultEnum, SubJobStatusEnum
from tdsql.common.const import EnvName
from tdsql.common.safe_get_information import ResourceParam
from tdsql.handle.backup.tdsql_backup_service import TdsqlBackupService
from tdsql.handle.backup.exec_backup import BackUp


class BackupAbility:
    """
    备份相关接口
    """

    @staticmethod
    @exter_attack
    def check_backup_job_type(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.check_backup_job_type(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def allow_backup_in_local_node(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.allow_backup_in_local_node(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def backup_prerequisite(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.backup_pre_job(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup_prerequisite_progress(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.backup_prerequisite_progress(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup_gen_sub_job(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.backup_gen_sub_job(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup(req_id, job_id, sub_id, std_in):
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        EnvName.IAM_USERNAME = "job_protectEnv_auth_authKey"
        EnvName.IAM_PASSWORD = "job_protectEnv_auth_authPwd"
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, std_in, param)
        result = backup_inst.backup_task()
        if not result:
            output.code = ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def backup_post_job(req_id, job_id, sub_id, std_in):
        TdsqlBackupService.do_post_job(req_id, job_id, sub_id, std_in)
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def backup_post_job_progress(req_id, job_id, sub_id, std_in):
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=100, dataSize=0, speed=0, extendInfo=None)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def query_backup_copy(req_id, job_id, sub_id, std_in):
        return TdsqlBackupService.query_backup_copy(req_id, job_id, sub_id)
