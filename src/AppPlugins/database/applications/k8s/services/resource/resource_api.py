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

from kubernetes.client.exceptions import ApiException
from urllib3.exceptions import MaxRetryError, HTTPError

from common.util.exec_utils import exec_overwrite_file
from k8s.common.utils import clean_sys_data
from k8s.logger import log
from k8s.services.resource.resource_params import ResourceParam
from k8s.services.resource.resource_service import ResourceInfo
from k8s.common.error_code import ErrorCode
from k8s.common.const import K8sType, K8sSubType
from common.const import ParamConstant
from common.common import exter_attack, output_result_file, output_execution_result_ex
from common.exception.common_exception import ErrCodeException
from common.common_models import ActionResult
from common.const import ExecuteResultEnum


class ResourceApi:
    @staticmethod
    @exter_attack
    def check_application(req_id, job_id, sub_id, data):
        """
        通用接口: 检查项目连通性
        """
        log.info("Start action: check_application!!!")
        params_info = ResourceParam(req_id)
        resource_info = ResourceInfo(req_id, params_info)
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{req_id}")
        """
        获取客户端, 调用生产K8S查询preferversion
        """
        try:
            body_err_code = ExecuteResultEnum.SUCCESS.value
            err_code = ExecuteResultEnum.SUCCESS.value
            err_msg = "Check connection success!"
            resource_info.do_check_cluster()
            log.info(f"Action check_application success req:{req_id}.")
        except ErrCodeException as err:
            body_err_code = err.error_code
            err_code = err.error_code
            err_msg = err.error_message
            log.error(f"Action check_application ErrCodeException failed.{err}")
        except ApiException as err:
            body_err_code = ErrorCode.AUTH_FAILED
            err_code = ErrorCode.AUTH_FAILED
            err_msg = "Action check_application ApiException failed."
            log.error(f"Action check_application ApiException failed.{err}")
        except HTTPError as err:
            body_err_code = ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT
            err_code = ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT
            err_msg = "Action check_application HTTPError failed."
            log.error(f"Action check_application HTTPError failed.{err}")
        except MaxRetryError as err:
            body_err_code = ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT
            err_code = ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT
            err_msg = "Action check_application HTTPError failed."
            log.error(f"Action check_application MaxRetryError failed.{err}")
        except Exception as err:
            body_err_code = ErrorCode.OPERATION_FAILED
            err_code = ErrorCode.OPERATION_FAILED
            err_msg = "Action check_application Exception failed."
            log.error(f"Action check_application Exception failed.{err}")
        finally:
            exec_overwrite_file(result_path, {"code": err_code, "bodyErr": body_err_code, "message": err_msg})
            params_info.clean_resource_auth_info()


    @staticmethod
    @exter_attack
    def query_cluster(req_id, job_id, sub_id, data):
        """
        通用接口: 查询集群版本
        """
        log.info("Start action: query_cluster!!!")
        params_info = ResourceParam(req_id)
        resource_info = ResourceInfo(req_id, params_info)
        try:
            response = resource_info.do_query_cluster()
            output_result_file(req_id, response.dict(by_alias=True))
            log.info(f"Action query_cluster success req:{req_id}.")
        except ErrCodeException as err:
            output_result_file(req_id, ResourceApi.load_err_into_result(err))
            log.error(f"Action query_cluster failed.{err}")
        except ApiException as err:
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                                    body_err=ErrorCode.AUTH_FAILED,
                                                    message=ErrorCode.AUTH_FAILED.value).dict(by_alias=True))
            log.error(f"Action query_cluster ApiException failed.{err}")
        except HTTPError as err:
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                                    body_err=ErrorCode.AUTH_FAILED,
                                                    message=ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT.value)
                               .dict(by_alias=True))
            log.error(f"Action query_cluster HTTPError failed.{err}")
        except MaxRetryError as err:
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                                    body_err=ErrorCode.AUTH_FAILED,
                                                    message=ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT.value)
                               .dict(by_alias=True))
            log.error(f"Action query_cluster MaxRetryError failed.{err}")
        except Exception as exception_str:
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                                    body_err=ErrorCode.ERROR_AGENT_INTERNAL_ERROR,
                                                    message="OPERATION_FAILED").dict(by_alias=True))
            log.error(f"Action query_cluster Exception failed.{exception_str}")
        finally:
            params_info.clean_resource_auth_info()

    @staticmethod
    @exter_attack
    def list_application_resource(req_id, job_id, sub_id, data):
        """
        通用接口: 查询资源列表
        """
        log.info("Start action: list_application_resource!!!")
        params_info = ResourceParam(req_id)
        resource_info = ResourceInfo(req_id, params_info)
        try:
            response = resource_info.do_list_resource()
            output_result_file(req_id, response)
            log.info(f"Action list_application_resource success req:{req_id}.")
        except ErrCodeException as err:
            output_result_file(req_id, ResourceApi.write_err_into_result(err))
            log.error(f"Action list_application_resource failed.{err}")
        except ApiException as err:
            ResourceApi.write_error_to_result_file(req_id)
            log.error(f"Action list_application_resource ApiException failed.{err}")
        except HTTPError as err:
            ResourceApi.write_error_to_result_file(req_id)
            log.error(f"Action list_application_resource HTTPError failed.{err}")
        except MaxRetryError as err:
            ResourceApi.write_error_to_result_file(req_id)
            log.error(f"Action list_application_resource MaxRetryError failed.{err}")
        except Exception as exception_str:
            ResourceApi.write_error_to_result_file(req_id)
            log.error(f"Action list_application_resource Exception failed.{exception_str}")
        finally:
            params_info.clean_resource_auth_info()
            clean_sys_data()

    @staticmethod
    def load_err_into_result(err: ErrCodeException):
        log.debug("Start load_err_into_result")
        err_dict = json.loads(err.error_message)
        result = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                              body_err=err_dict.get("errorCode"),
                              message=err_dict.get("errorCode").value).dict(by_alias=True)
        return result

    @staticmethod
    def write_err_into_result(err: ErrCodeException):
        log.debug("Start load_err_into_result")
        err_dict = json.loads(err.error_message)
        error_info = {
            "code": ExecuteResultEnum.INTERNAL_ERROR,
            "bodyErr": err_dict.get("errorCode"),
            "message": err_dict.get("errorCode").value
        }
        param = {
            "type": K8sType.TYPE_CLUSTER,
            "subType": K8sSubType.SUBTYPE_NAMESPACE,
            "extendInfo": {"cluster_list": json.dumps([])}
        }
        item = [param]
        result = {"exception": error_info, "resourceList": item}
        return result

    @staticmethod
    def write_error_to_result_file(pid):
        error_info = {
            "code": ErrorCode.AUTH_FAILED,
            "bodyErr": ErrorCode.AUTH_FAILED,
            "message": ErrorCode.AUTH_FAILED.value
        }
        param = {
            "type": K8sType.TYPE_CLUSTER,
            "subType": K8sSubType.SUBTYPE_NAMESPACE,
            "extendInfo": {"cluster_list": json.dumps([])}
        }
        item = [param]
        result_param = {"exception": error_info, "resourceList": item}
        output_result_file(pid, result_param)