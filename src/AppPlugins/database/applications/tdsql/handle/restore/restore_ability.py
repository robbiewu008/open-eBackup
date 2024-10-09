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

import os.path

from common.common import exter_attack, output_result_file
from common.common_models import ActionResult, SubJobDetails
from common.const import ExecuteResultEnum, SubJobStatusEnum
from common.parse_parafile import ParamFileUtil
from tdsql.handle.restore.exec_restore import Restore
from tdsql.logger import log


class RestoreAbility:
    @staticmethod
    @exter_attack
    def restore(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore task.")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        restore_inst = Restore(req_id, job_id, sub_id, data, body_param)
        result = restore_inst.restore_task()
        if not result:
            output.code = ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, output.dict(by_alias=True))
        return output.code

    @staticmethod
    @exter_attack
    def allow_restore_in_local_node(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the allow_restore_in_local_node task.")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        output_result_file(req_id, output.dict(by_alias=True))
        return output.code

    @staticmethod
    @exter_attack
    def restore_prerequisite(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_prerequisite task.")
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        ret, error_code = Restore(req_id, job_id, sub_id, data, body_param).restore_prerequisite()
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_prerequisite_progress(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_prerequisite_progress task.")
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        Restore(req_id, job_id, sub_id, data, body_param).restore_prerequisite_progress()
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_gen_sub_job(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_gen_sub_job task.")
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        try:
            Restore(req_id, job_id, sub_id, data, body_param).gen_sub_job()
        except Exception as ex:
            return ExecuteResultEnum.INTERNAL_ERROR
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_post_job(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_post_job task.")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        restore_inst = Restore(req_id, job_id, sub_id, data, body_param)
        result = restore_inst.do_post()
        if not result:
            output.code = ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_post_job_progress(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_post_job_progress task.")
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=100, dataSize=0, speed=0, extendInfo=None)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS
