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

from common.common import exter_attack, output_result_file, output_execution_result_ex
from common.const import ParamConstant
from oceanbase.common.const import OceanBaseSubType, OceanBaseCode, ErrorCode, OceanBaseQueryType
from oceanbase.common.oceanbase_exception import ErrCodeException
from oceanbase.handle.resource.resource_info import OceanBaseResourceInfo
from oceanbase.logger import log


class ResourceAbility:
    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        resource_info = OceanBaseResourceInfo(req_id)
        body_err_code = OceanBaseCode.SUCCESS.value
        err_code = OceanBaseCode.SUCCESS.value
        err_msg = "Check connection success!"
        parameters = []
        try:
            resource_info.check()
        except ErrCodeException as err_code_ex:
            err_code = OceanBaseCode.FAILED.value
            body_err_code = err_code_ex.error_code
            err_msg = err_code_ex.error_message_json
            parameters = json.loads(err_msg).get("parameters")
        except Exception as ex:
            err_code = OceanBaseCode.FAILED.value
            body_err_code = ErrorCode.ERR_INPUT_STRING
            err_msg = f"exception occurs. {ex}"
        finally:
            log.info(f"Check application end result is {err_msg}.")
            ResourceAbility.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg, parameters)

    @staticmethod
    @exter_attack
    def list_application_v2(req_id, job_id, sub_id, data):
        resource_info = OceanBaseResourceInfo(req_id)
        resource_list = []
        if resource_info.query_type and resource_info.query_type == OceanBaseQueryType.POOL:
            pool = resource_info.get_resource_pool()
            resource = dict()
            pool_dict = {"pools": pool}
            resource["extendInfo"] = {"clusterInfo": json.dumps(pool_dict)}
            resource_list.append(resource)
            log.info(f"resource {resource}")
        else:
            cluster_infos = resource_info.get_cluster_info()
            ret, version = resource_info.check_cluster_version()
            for cluster_info in cluster_infos:
                resource = dict()
                resource["type"] = OceanBaseSubType.TYPE
                resource["subType"] = OceanBaseSubType.SUBTYPE_CLUSTER
                resource["name"] = cluster_info.get("cluster_name", "")
                resource["id"] = cluster_info.get("cluster_id", "")
                resource["version"] = version
                tenants = resource_info.get_tenant()
                observers = resource_info.get_observer()
                cluster_info_dict = {"version": version, "tenantInfos": tenants, "observers": observers}
                cluster_info_dict.update(cluster_info)
                resource["extendInfo"] = {"clusterInfo": json.dumps(cluster_info_dict)}
                resource_list.append(resource)
                log.info(f"resource {resource}")
        params = {"resourceList": resource_list}
        log.info(f"req_id {req_id} params {params}")
        output_result_file(req_id, params)
        log.info(f"List_application_v2 success req:{req_id}.")

    @staticmethod
    @exter_attack
    def remove_project(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        resource_info = OceanBaseResourceInfo(req_id)
        body_err_code = OceanBaseCode.SUCCESS.value
        err_code = OceanBaseCode.SUCCESS.value
        err_msg = "remove project connection success!"
        parameters = []
        if not resource_info.remove_project():
            log.error("Fail to clean persistent mount path")
            err_msg = "clean persistent mount path failed"
        log.info(f"remove project end result is {err_msg}.")
        ResourceAbility.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg, parameters)

    @staticmethod
    def write_error_param_to_result_file(path: str, code: int, error_code: int, message: str, parameters):
        output_execution_result_ex(path, {"code": code, "bodyErr": error_code, "message": message,
                                          "bodyErrParams": parameters})
