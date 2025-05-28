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

from base64 import b64decode

import yaml
from k8s.common.parse_param import parse_param_with_jsonschema
from k8s.common.utils import get_env_variable, validate_ip_str, validate_port_str, validate_auth_str, clean_auth_info, \
    clean_ssl_file
from k8s.logger import log
from k8s.common.kubernetes_client.struct import ClusterAuthentication, AuthType, Token
from k8s.common.const import K8sJobKind, ConditionsInfo, K8sResourceKeyName
from k8s.common.error_code import ErrorCode
from k8s.common.k8s_resource_map import K8SAPI
from common.parse_parafile import ParamFileUtil
from common.exception.common_exception import ErrCodeException


class ResourceParam:

    def __init__(self, pid):
        self.pid = pid
        self._auth = ClusterAuthentication(auth_type=6)
        try:
            self._body_param = parse_param_with_jsonschema(self.pid)
            self._auth.id = pid
            self.update_auth_info()
        except ErrCodeException as e:
            clean_auth_info(self._auth)
            raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="ResourceParam init error!") from e
        if not self._body_param:
            raise Exception(f"Failed to parse job param file is none")

    def update_auth_info(self):
        """
        从参数中获取k8s集群鉴权信息
        """
        self._auth.token = Token()
        self._auth.auth_type = int(get_env_variable(K8sResourceKeyName.APPENV_AUTH_AUTHTYPE + self.pid))
        log.info(f"undate_auth_info, authtype={self._auth.auth_type}")
        if self._auth.auth_type == AuthType.TOKEN:
            if not self.get_token_info():
                log.error(f"Action get_token_info Failed! req_id:{self.pid}")
                raise ErrCodeException(ErrorCode.PARAM_FAILED, message="get_token_info error!")
        elif self._auth.auth_type == AuthType.CONFIGFILE:
            if not self.get_kube_config_info():
                log.error(f"Action get_config_info Failed! req_id:{self.pid}")
                raise ErrCodeException(ErrorCode.PARAM_FAILED, message="get_config_info error!")
        else:
            raise ErrCodeException(ErrorCode.PARAM_FAILED, message="Auth type error!")
        log.info("SUCCESS! update_auth_info")

    def get_token_info(self):
        """
        从参数中获取k8s集群地址
        """
        log.info("Start get_token_info!!!")
        self._auth.is_verify_ssl = self._body_param.get("appEnv").get("extendInfo").get("isVerifySsl")
        if not validate_auth_str(self._auth.is_verify_ssl):
            log.error(f"Validate is_verify_ssl Failed! req_id:{self.pid}")
            return False
        self._auth.token.address = self._body_param.get("appEnv").get("endpoint")
        if not validate_ip_str(self._auth.token.address):
            log.error(f"Validate ip Failed! req_id:{self.pid}")
            return False
        self._auth.token.port = self._body_param.get("appEnv").get("port")
        if not validate_port_str(self._auth.token.port):
            log.error(f"Validate port Failed! req_id:{self.pid}")
            return False
        self._auth.token.token_info = get_env_variable(K8sResourceKeyName.APPENV_AUTH_EXTENDINFO_TOKEN + self.pid)
        if not validate_auth_str(self._auth.token.token_info):
            log.error(f"Validate token Failed! req_id:{self.pid}")
            return False
        self._auth.token.certificateAuthorityData = get_env_variable(
            K8sResourceKeyName.APPENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA + self.pid
        )
        log.info("SUCCESS! get_token_info")
        return True

    def get_kube_config_info(self):
        """
        从参数中获取k8s集群地址
        """
        log.info("Start get_config_info!!!")
        self._auth.is_verify_ssl = self._body_param.get("appEnv").get("extendInfo").get("isVerifySsl")
        if not validate_auth_str(self._auth.is_verify_ssl):
            log.error(f"Validate is_verify_ssl Failed! req_id:{self.pid}")
            return False
        kube_config = get_env_variable(K8sResourceKeyName.APPENV_AUTH_EXTENDINFO_CONFIG + self.pid)
        kube_config = str(b64decode(kube_config).decode("ascii"))
        if not validate_auth_str(kube_config):
            log.error(f"Validate token Failed! req_id:{self.pid}")
            return False
        self._auth.kube_config = yaml.safe_load(kube_config)
        log.info("SUCCESS! get_config_info")
        return True

    def get_resource_instance(self, resource_kind):
        resource_instance = K8SAPI[resource_kind](self._auth)
        if resource_instance:
            log.info("SUCCESS! get_resource_instance")
            return resource_instance
        raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="get_resource_instance error!")

    def get_workload_resource_instance(self, workload_type):
        resource_instance = K8SAPI["Workload"](self._auth, workload_type)
        if resource_instance:
            log.info("SUCCESS! get_workload_instance")
            return resource_instance
        raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="get_workload_resource_instance error!")

    def get_conditions_value(self):
        """
        获取condition参数，判断任务类型（namespace，workload，pvc）
        """
        log.debug("Start get_conditions_value")
        conditions_value = ConditionsInfo(kind="")
        conditions = self._body_param.get("condition")
        conditions_value.kind = conditions.get("conditions").get("kind", "")
        log.debug(f"Kind={conditions_value.kind}")
        if conditions_value.kind == K8sJobKind.NAMESPACE:
            return conditions_value
        elif conditions_value.kind in K8sJobKind.WORKLOAD:
            conditions_value.params_for_workload = conditions.get("conditions").get("namespace", "")
            if conditions_value.params_for_workload == "":
                log.error(f"Params error: no namespace_value! Req_id = {self.pid}")
                raise ErrCodeException(ErrorCode.PARAM_FAILED, message="Get no namespace value for list workload！")
            return conditions_value
        elif conditions_value.kind == K8sJobKind.PVC:
            conditions_value.params_for_pvc = []
            namespace = conditions.get("conditions").get("namespace", "")
            super_info = conditions.get("conditions").get("super", "")
            if namespace == "" or super_info == [] or super_info == "":
                log.debug(f"Start list all/namespace pvc!")
                pvc_params = {
                    "namespace": namespace,
                    "workload": "",
                    "workload_kind": ""
                }
            else:
                log.debug(f"Start list workload pvc!")
                pvc_params = {
                    "namespace": namespace,
                    "workload": super_info[0].get("value"),
                    "workload_kind": super_info[0].get("kind")
                }
            conditions_value.params_for_pvc = pvc_params
            return conditions_value
        log.error("Conditions kind error!")
        raise ErrCodeException(ErrorCode.PARAM_FAILED, message="No param kind fit!")

    def get_pages_info(self):
        """
        获取分页信息，用于查询资源
        """
        log.debug(f"Start get_pages_info, req_id:{self.pid}")
        condition = self._body_param.get("condition")
        pages_no = condition.get("pageNo")
        page_size = condition.get("pageSize")
        page_start = pages_no * page_size
        log.debug(f"Page_no:{pages_no}, page_size:{page_size}, page_start:{page_start}")
        return page_start, page_size, pages_no

    def get_selector_info(self):
        condition = self._body_param.get("condition")
        field = condition.get("conditions").get("fieldSelector")
        label = condition.get("conditions").get("labelSelector")
        log.debug(f"Get selector, field={field}, label={label}")
        return field, label

    def clean_resource_auth_info(self):
        clean_auth_info(self._auth)

    def clean_resource_ssl_file(self):
        clean_ssl_file(self._auth)
