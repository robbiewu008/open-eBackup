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

from common.common import exter_attack, output_execution_result_ex
from common.const import ParamConstant, ExecuteResultEnum
from tdsql.handle.common.const import ErrorCode, TDSQLQueryType
from tdsql.handle.common.tdsql_exception import ErrCodeException
from tdsql.handle.resource.parse_params import ResourceParam
from tdsql.handle.resource.resource_info import TDSQLResourceInfo
from tdsql.logger import log


class ResourceAbility:
    """
    资源接入相关接口
    """

    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        params_from_pm = ResourceParam(req_id)

        resource_info = TDSQLResourceInfo(req_id, params_from_pm)
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "Check connection success!"
        parameters = []
        try:
            resource_info.check_node_info()
        except ErrCodeException as err_code_ex:
            log.error(f"exec check_node_info failed , err_code_ex: {err_code_ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message_json
            parameters = json.loads(err_msg).get("parameters")
        except Exception as ex:
            log.error(f"exec check_node_info failed , ex: {ex}")
            err_code = ExecuteResultEnum.INTERNAL_ERROR.value
            body_err_code = ErrorCode.ERR_INPUT_STRING
            err_msg = "exception occurs."
        finally:
            resource_info.clear_auth()
            log.info(f"Check application end result is {err_msg}.")
            ResourceAbility.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg, parameters)

    @staticmethod
    @exter_attack
    def list_application_v2(req_id, job_id, sub_id, data):
        params_from_pm = ResourceParam(req_id)
        resource_info = TDSQLResourceInfo(req_id, params_from_pm)
        extend_info = params_from_pm.get_param().get("applications", [])[0].get("extendInfo", {})
        if extend_info.get("queryType") == TDSQLQueryType.RESOURCE:
            try:
                resource_info.get_cluster_host_info()
            except Exception as exception_str:
                log.error(f"List_application_v2 failed.{exception_str}")
        else:
            try:
                resource_info.get_data_nodes()
            except Exception as exception_str:
                log.error(f"List_application_v2 failed.{exception_str}")

        log.info(f"List_application_v2 success req:{req_id}.")

    @staticmethod
    @exter_attack
    def query_cluster(req_id, job_id, sub_id, data):
        params_from_pm = ResourceParam(req_id)
        resource_info = TDSQLResourceInfo(req_id, params_from_pm)
        try:
            resource_info.query_cluster()
        except Exception as exception_str:
            log.error(f"query_cluster failed.{exception_str}")
        log.info(f"query_cluster success req:{req_id}.")

    @staticmethod
    def write_error_param_to_result_file(path: str, code: int, error_code: int, message: str, parameters):
        output_execution_result_ex(path, {"code": code, "bodyErr": error_code, "message": message,
                                          "bodyErrParams": parameters})

    @staticmethod
    @exter_attack
    def remove_project(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        params_from_pm = ResourceParam(req_id)
        resource_info = TDSQLResourceInfo(req_id, params_from_pm)
        body_err_code = ExecuteResultEnum.SUCCESS.value
        err_code = ExecuteResultEnum.SUCCESS.value
        err_msg = "remove project connection success!"
        parameters = []
        if not resource_info.remove_project():
            log.error("Fail to clean persistent mount path")
            err_msg = "clean persistent mount path failed"
        log.info(f"remove project end result is {err_msg}.")
        ResourceAbility.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg, parameters)
