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

from goldendb.logger import log
from common.common import exter_attack, output_result_file
from common.common_models import ActionResult, SubJobDetails
from common.const import ExecuteResultEnum, SubJobStatusEnum
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.restore.exec_restore import Restore


class RestoreAbility:
    @staticmethod
    @exter_attack
    def restore(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore task.")
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
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
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as err_code:
            log.error(f"Failed to parse job param file for {str(err_code)}")
            raise Exception(f"Failed to parse job param file.") from err_code
        if not body_param:
            log.error(f"Failed to parse job param file is none.")
            raise Exception(f"Failed to parse job param file is none.")
        try:
            # 异常时，上报
            ret = Restore(req_id, job_id, sub_id, data, body_param).allow_restore_in_local_node()
        except Exception as ex:
            log.error(f"Failed to execute allow_restore_in_local_node, {str(ex)}")
            raise Exception(f"Allow restore in local node failed.") from ex
        if not ret:
            log.error(f"Failed to execute allow_restore_in_local_node.")
            return ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_prerequisite(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_prerequisite task.")
        try:
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        Restore(req_id, job_id, sub_id, data, body_param).restore_prerequisite()
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    @exter_attack
    def restore_prerequisite_progress(req_id, job_id, sub_id, data):
        log.info(f"Start to execute the restore_prerequisite_progress task.")
        try:
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
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
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
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
            body_param = JsonParam.parse_param_with_jsonschema(req_id)
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
                                      logDetail=[], progress=100, dataSize=0, speed=0)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS
