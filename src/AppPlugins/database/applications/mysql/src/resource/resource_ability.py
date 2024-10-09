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

from common.common import exter_attack
from common.const import ParamConstant
from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import exec_overwrite_file
from mysql import log
from mysql.src.common.constant import MySQLClusterType
from mysql.src.common.error_code import MySQLErrorCode, MySQLCode
from mysql.src.resource.mysql_resource import PXCMySQLResource, APMySQLResource, EAPPMySQLResource, \
    ResourceParam, SingleMySQLResource, AAMySQLResource
from mysql.src.utils.common_func import build_body_param


def build_resource(param: ResourceParam):
    if param.cluster_type == MySQLClusterType.PXC:
        return PXCMySQLResource(param)
    elif param.cluster_type == MySQLClusterType.AP:
        return APMySQLResource(param)
    elif param.cluster_type == MySQLClusterType.AA:
        return AAMySQLResource(param)
    elif param.cluster_type == MySQLClusterType.EAPP:
        return EAPPMySQLResource(param)
    else:
        return SingleMySQLResource(param)


class ResourceAbility:
    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        try:
            with ResourceParam(req_id) as params_from_pm:
                resource = build_resource(params_from_pm)
                resource.check_application()
                result_param = build_body_param(0, 0, "Check connectivity!")
                log.info(f"Success to execute check application command. pid:{req_id}")
                exec_overwrite_file(result_path, result_param)
        except ErrCodeException as mysql_err:
            log.error(f"check_application error, mysql_err:{mysql_err.error_code}")
            result_param = build_body_param(MySQLCode.FAILED.value, mysql_err.error_code, mysql_err.error_message)
            exec_overwrite_file(result_path, result_param)
        except Exception as err:
            log.error(f"check_application error, err:{err},type_err:{type(err)}")
            result_param = build_body_param(MySQLCode.FAILED.value, MySQLErrorCode.SYSTEM_ERROR, "system error")
            exec_overwrite_file(result_path, result_param)

    @staticmethod
    @exter_attack
    def verify_cluster_node(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        try:
            with ResourceParam(req_id) as params_from_pm:
                resource = build_resource(params_from_pm)
                resource.verify_cluster_node()
                log.info(f"Success to verify_cluster_node. pid:{req_id}")
                message = f"Check cluster success!"
                result_param = build_body_param(0, 0, message)
                exec_overwrite_file(result_path, result_param)
        except ErrCodeException as mysql_err:
            log.error(f"verify_cluster_node error, mysql_err:{mysql_err.error_code}")
            result_param = build_body_param(MySQLCode.FAILED.value, mysql_err.error_code, mysql_err.error_message)
            exec_overwrite_file(result_path, result_param)
        except Exception as err:
            log.error(f"verify_cluster_node error, err:{err}")
            result_param = build_body_param(MySQLCode.FAILED.value, MySQLErrorCode.SYSTEM_ERROR, "system error")
            exec_overwrite_file(result_path, result_param)

    @staticmethod
    @exter_attack
    def query_cluster(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        try:
            with ResourceParam(req_id) as params_from_pm:
                resource = build_resource(params_from_pm)
                resource.query_cluster()
                result_param = resource.build_check_result()
                log.info(f"result_param:{result_param}")
                log.info(f"Success to execute query cluster command. pid:{req_id}, cluster:{result_param.get('id')}")
                exec_overwrite_file(result_path, result_param)
        except ErrCodeException as mysql_err:
            log.error(f"check_cluster error, mysql_err:{mysql_err.error_code}")
            exec_overwrite_file(result_path, {"extendInfo": {"error_code": mysql_err.error_code}})
        except Exception as err:
            log.error(f"check_cluster error, err:{err}")
            exec_overwrite_file(result_path, {"extendInfo": {"error_code": MySQLErrorCode.SYSTEM_ERROR}})

    @staticmethod
    @exter_attack
    def list_application_resource(req_id, job_id, sub_id, data):
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        try:
            with ResourceParam(req_id) as params_from_pm:
                resource = build_resource(params_from_pm)
                database_list = resource.list_application()
                log.info(f"Success to execute list application command. pid:{req_id}, result:{len(database_list)}")
                params = {"resourceList": database_list}
                exec_overwrite_file(result_path, params)
        except ErrCodeException as mysql_err:
            log.error(f"check_cluster error, mysql_err:{mysql_err.error_code}")
            result_param = build_body_param(MySQLCode.FAILED.value, mysql_err.error_code, mysql_err.error_message)
            exec_overwrite_file(result_path, result_param)
        except Exception as err:
            log.error(f"check_cluster error, err:{err}")
            result_param = build_body_param(MySQLCode.FAILED.value, MySQLErrorCode.SYSTEM_ERROR, "system error")
            exec_overwrite_file(result_path, result_param)


def write_error_param(application: dict, error_code, result_path):
    result_param = {
        "id": application.get("id"),
        "type": application.get("type"),
        "subType": application.get("subType"),
        "extendInfo": {
            "status": "1",
            "error_code": error_code
        }
    }
    log.info(f"result_param:{result_param}")
    exec_overwrite_file(result_path, result_param)
