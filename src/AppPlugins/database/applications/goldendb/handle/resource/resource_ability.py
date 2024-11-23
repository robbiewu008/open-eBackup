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

from goldendb.logger import log
from common.common import exter_attack, output_result_file
from common.const import ParamConstant
from common.util.exec_utils import exec_overwrite_file
from goldendb.handle.common.const import GoldenDBSubType, GoldenDBCode, ErrorCode
from goldendb.handle.common.goldendb_exception import ErrCodeException
from goldendb.handle.resource.parse_params import ResourceParam
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo


class ResourceAbility:
    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        params_from_pm = ResourceParam(req_id)

        resource_info = GoldenDBResourceInfo(req_id, params_from_pm)
        node_info = resource_info.param.get_node_info()
        node_type = node_info.get("nodeType")
        body_err_code = GoldenDBCode.SUCCESS.value
        err_code = GoldenDBCode.SUCCESS.value
        err_dict = {"msg": "Check connection success!", "nodeType": node_type}
        err_msg = json.dumps(err_dict)
        try:
            resource_info.check_node_status()
        except ErrCodeException as err_code_ex:
            err_code = GoldenDBCode.FAILED.value
            err_dict = {"msg": err_code_ex.error_message_json, "nodeType": node_type}
            body_err_code = err_code_ex.error_code
            err_msg = json.dumps(err_dict)
        except Exception as ex:
            err_code = GoldenDBCode.FAILED.value
            body_err_code = ErrorCode.ERR_INPUT_STRING
            err_dict = {"msg": "exception occurs.", "nodeType": node_type}
            err_msg = json.dumps(err_dict)
        finally:
            resource_info.clear_auth()
            log.info(f"Check application end result is {err_msg}.")
            ResourceAbility.write_error_param_to_result_file(result_path, err_code, body_err_code, err_msg)

    @staticmethod
    @exter_attack
    def list_application_v2(req_id, job_id, sub_id, data):
        params_from_pm = ResourceParam(req_id)
        resource_fun = GoldenDBResourceInfo(req_id, params_from_pm)

        ResourceAbility.get_param_browse_cluster(req_id, resource_fun, params_from_pm)

        log.info(f"List_application_v2 success req:{req_id}.")

    @staticmethod
    def get_param_browse_cluster(req_id, resource_fun, params_from_pm):
        resource_list = []
        cluster_infos = resource_fun.get_clusters()
        version = resource_fun.get_cluster_version(params_from_pm.get_os_user_when_brows())
        for cluster_info in cluster_infos:
            resource = dict()
            resource["type"] = GoldenDBSubType.TYPE
            resource["subType"] = GoldenDBSubType.SUBTYPE_CLUSTER_INSTANCE
            resource["name"] = cluster_info.get("name")
            resource["id"] = cluster_info.get("id")
            resource["version"] = version
            resource["extendInfo"] = {"version": version, "clusterInfo": json.dumps(cluster_info)}
            resource_list.append(resource)
        params = {"resourceList": resource_list}
        output_result_file(req_id, params)

    @staticmethod
    def write_error_param_to_result_file(path: str, code: int, error_code: int, message: str):
        exec_overwrite_file(path, {"code": code, "bodyErr": error_code, "message": message})
