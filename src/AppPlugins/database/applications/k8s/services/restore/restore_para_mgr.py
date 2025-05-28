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
from base64 import b64decode

import yaml

from k8s.common.k8s_resource_map import K8SAPI
from k8s.common.parse_param import parse_param_with_jsonschema
from common.common_models import SubJobModel
from common.const import RepositoryDataTypeEnum, SubJobPriorityEnum, SubJobPolicyEnum, SubJobTypeEnum
from common.parse_parafile import ParamFileUtil
from common.exception.common_exception import ErrCodeException
from k8s.common.const import K8sBackupKeyName, RestoreType, ActionType, JobInfo, RestoreCommandParam
from k8s.logger import log
from k8s.common.kubernetes_client.struct import ClusterAuthentication, AuthType, Token, Repositories, \
    Resource, Namespace
from k8s.common.error_code import ErrorCode
from k8s.common.utils import get_env_variable, clean_auth_info, clean_ssl_file, get_task_timeout


class RestoreParamMgr:
    def __init__(self, pid, job_id, sub_job_id, is_only_fill_auth_info=False):
        self.pid = pid
        self.job_id = job_id
        self.sub_id = sub_job_id
        self._auth = ClusterAuthentication(auth_type=AuthType.TOKEN.value)
        try:
            self._body_param = parse_param_with_jsonschema(self.pid)
            self._auth.id = pid
            self._update_auth_info()
            if is_only_fill_auth_info:
                log.info(f"Only need to check connection of node, no need more param.")
                return
            # targetObject下参数
            param_target_bject = self._body_param.get("job", {}).get("targetObject", {})
            self.ns_name = param_target_bject.get("parentName", "")
            self._sub_type = param_target_bject.get("subType", "")
            extend_info = param_target_bject.get("extendInfo", {})
            self.exclude_labels = extend_info.get("excludeLabels", "")
            self.labels = extend_info.get("labels", "")

            # targetEnv下参数
            param_target_env = self._body_param.get("job", {}).get("targetEnv", {})
            cluster_name = param_target_env.get("name", "")
            _node_selector = param_target_env.get("extendInfo", {}).get("nodeSelector", "")
            timeout = param_target_env.get("extendInfo", {}).get("taskTimeout", '')
            _task_timeout = get_task_timeout(timeout)
            log.info(f"Task time out is {timeout}, _task_timeout is {_task_timeout} task is {self.job_id}")
            enable_change_env = param_target_env.get("extendInfo", {}).get('isEnableChangeEnv', 'false')
            change_env_list = param_target_env.get("extendInfo", {}).get(
                'advancedConfigReqList', '') if enable_change_env == 'true' else ''
            if change_env_list:
                change_env_dict = self._set_env_name_list(change_env_list)
            else:
                change_env_dict = {}

            enable_change_sc = param_target_env.get("extendInfo", {}).get('isEnableChangeScParameter', 'false')
            change_sc_list = param_target_env.get("extendInfo", {}).get(
                'scParameterList', '') if enable_change_sc == 'true' else ''
            log.info(f"Sc switching mode is {enable_change_sc}, change_sc_list is {change_sc_list} "
                     f"task is {self.job_id}")
            if change_sc_list:
                change_sc_dict = self._set_sc_name_list(change_sc_list)
            else:
                change_sc_dict = {}

            self.image_name = param_target_env.get("extendInfo", {}).get("imageNameAndTag", "")
            self._pod_nums = param_target_env.get("extendInfo", {}).get("jobNumOnSingleNode", 4)

            # 剩余参数
            restore_type = self._get_restore_type()
            repositories = self._body_param.get("job", {}).get("copies", [])[0].get("repositories", [])
            data_repo_obj, meta_repo_obj, cache_repo_obj = self.get_repository_info(repositories)
            self.copy_id = self._body_param.get("job", {}).get("extendInfo", {}).get("originBackupId", '')

            self.pvc_maps = {}
            self._generate_pvc_maps()

            log.info(f"Image name is {self.image_name}")
            log.info(f"Namespace {self.ns_name}, cluster_name {cluster_name}")
            self.job_info = JobInfo(resource=Resource(cluster_name=cluster_name,
                                                      cluster_authentication=self._auth,
                                                      namespace=Namespace(name=self.ns_name)),
                                    task_id=self.job_id, sub_id=sub_job_id, request_id=pid,
                                    data_repo=data_repo_obj, cache_repo=cache_repo_obj, meta_repo=meta_repo_obj,
                                    image_name=self.image_name, copy_id=self.copy_id, pod_nums=self._pod_nums,
                                    action_type=ActionType.RESTORE, job_type=restore_type,
                                    node_selector=_node_selector, change_env_dict=change_env_dict,
                                    change_sc_dict=change_sc_dict, task_timeout=_task_timeout)
        except Exception as e:
            log.exception(f"Init para failed.")
            clean_ssl_file(self._auth)
            clean_auth_info(self._auth)
            raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="ResourceParam init error!") from e
        return

    @staticmethod
    def get_repository_info(repositories):
        data_repo_obj = None
        meta_repo_obj = None
        cache_repo_obj = None
        for repository in repositories:
            log.info(f"Extend info is {repository['extendInfo']}")
            if repository['repositoryType'] == RepositoryDataTypeEnum.DATA_REPOSITORY:
                data_repo_obj = Repositories(logical_ip_list=repository['extendInfo']["storageLogicalIps"],
                                             remote_path=repository["remotePath"],
                                             local_path=repository["path"][0])
            elif repository['repositoryType'] == RepositoryDataTypeEnum.META_REPOSITORY:
                meta_repo_obj = Repositories(logical_ip_list=repository['extendInfo']["storageLogicalIps"],
                                             remote_path=repository["remotePath"],
                                             local_path=repository["path"][0])
            elif repository['repositoryType'] == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                cache_repo_obj = Repositories(logical_ip_list=repository['extendInfo']["storageLogicalIps"],
                                              remote_path=repository["remotePath"],
                                              local_path=repository["path"][0])
        if data_repo_obj is None or cache_repo_obj is None or meta_repo_obj is None:
            raise Exception("Not found repo.")
        return data_repo_obj, meta_repo_obj, cache_repo_obj

    @staticmethod
    def _set_env_name_list(change_env_list):
        change_env_dict = {}
        change_list = json.loads(change_env_list)
        for workload in change_list:
            change_env_dict.update({workload.get('workLoadName'): workload})
        return change_env_dict

    @staticmethod
    def _set_sc_name_list(change_sc_list):
        change_sc_dict = {}
        change_list = json.loads(change_sc_list)
        for sc in change_list:
            change_sc_dict.update({sc.get('scName'): sc})
        return change_sc_dict

    def get_pod_nums(self):
        return self._pod_nums

    def get_target_pvc_list(self):
        pvc_names = []
        for sub_object in self._body_param.get("job", {}).get("restoreSubObjects", []):
            pvc_names.append(sub_object.get("name"))
        return pvc_names

    def get_sub_job(self):
        log.info("Restore start to gen_sub_job!")
        log.debug(f"Generate namespace subjob model:jobid{self.job_id},subjob:{self.sub_id}")
        sub_job = SubJobModel(jobId=self.job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                              jobName="sub_k8s", policy=SubJobPolicyEnum.ANY_NODE.value,
                              jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1.value).dict(by_alias=True)
        log.debug(f"Sub job:{sub_job}")
        return sub_job

    def clean_restore_auth_info(self):
        clean_ssl_file(self._auth)
        clean_auth_info(self._auth)

    def do_check_cluster(self):
        client = self.get_resource_instance("Namespace")  # 可以使用ResourceNamespace来检查连通性
        if client is None:
            log.error(f"Get client failed! Req_id = {self.pid}")
            raise ErrCodeException(ErrorCode.AUTH_FAILED, message="Get client error!")
        api_list = []
        for api in client.k8s_client.get_prefer_versions():
            api_list.append(api)
        if len(api_list) < 2:  # 默认有CustomObjectsApi和VersionApi
            log.error(f"Get prefer_versions failed! Req_id = {self.pid}")
            raise ErrCodeException(ErrorCode.K8S_API_FAILED, message="Get prefer_versions error!")
        log.info(f"Action check_application Succeed! Req_id = {self.pid}")

    def get_resource_instance(self, resource_kind):
        resource_instance = K8SAPI[resource_kind](self._auth)
        if resource_instance:
            log.info("SUCCESS! get_resource_instance")
            return resource_instance
        raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="get_resource_instance error!")

    def _update_auth_info(self):
        self._auth.token = Token()
        self._auth.auth_type = int(get_env_variable(K8sBackupKeyName.TARGETENV_AUTH_AUTHTYPE + self.pid))
        log.info(f"Action update auth_info, auth type={self._auth.auth_type}")
        if self._auth.auth_type == AuthType.TOKEN:
            self._get_token_info()
        elif self._auth.auth_type == AuthType.CONFIGFILE:
            self._get_kube_config_info()
        else:
            raise ErrCodeException(ErrorCode.PARAM_FAILED, message="Auth type error!")
        log.info("Success update auth info.")
        return

    def _get_token_info(self):
        log.info("Start get_token_info!!!")
        self._auth.token.address = self._body_param.get("job", {}).get("targetEnv", {}).get("endpoint", "")
        self._auth.token.port = self._body_param.get("job", {}).get("targetEnv", {}).get("port", "")
        self._auth.token.token_info = get_env_variable(K8sBackupKeyName.TARGETENV_AUTH_EXTENDINFO_TOKEN + self.pid)
        self._auth.token.certificateAuthorityData = get_env_variable(
            K8sBackupKeyName.TARGETENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA + self.pid
        )
        self._auth.is_verify_ssl = self._body_param.get("job", {}).get("targetEnv", {}).\
            get("extendInfo", {}).get("isVerifySsl", "")
        log.info("SUCCESS! get_token_info")
        return

    def _get_kube_config_info(self):
        log.info("Start get_config_info!!!")
        kube_config = get_env_variable(K8sBackupKeyName.TARGETENV_AUTH_EXTENDINFO_CONFIG + self.pid)
        kube_config = str(b64decode(kube_config).decode("ascii"))
        self._auth.is_verify_ssl = self._body_param.get("job", {}).get("targetEnv", {}).\
            get("extendInfo", {}).get("isVerifySsl", "")
        self._auth.kube_config = yaml.safe_load(kube_config)
        log.info("SUCCESS! get_config_info")
        return

    def _generate_pvc_maps(self):
        for pvc_restore_info in self._body_param.get('job').get('restoreSubObjects', []):
            log.info(f'{pvc_restore_info}')
            name_info = pvc_restore_info.get('name')
            pvc_info = json.loads(name_info)
            pvc_need_restore = pvc_info.get('name')
            target_pvc = pvc_info.get('extendInfo').get('pvc')
            self.pvc_maps.update({target_pvc: pvc_need_restore})
            log.debug(f'Pvc map:{pvc_need_restore}---{target_pvc}')
        log.info(f'Generate pvc maps:{self.pvc_maps}')
        return

    def _get_restore_type(self):
        restore_option = int(self._body_param.get("job", {}).get("extendInfo", {}).get("restoreOption", '0'))
        if restore_option == RestoreType.IGNORE_EXIST.value:
            log.debug(f'Restore option is Ignore exist! Task id:{self.job_id}')
            restore_type = RestoreCommandParam.IGNORE_EXIST.value
        elif restore_option == RestoreType.OVERWRITE.value:
            log.debug(f'Restore option is Overwrite! Task id:{self.job_id}')
            restore_type = RestoreCommandParam.OVERWRITE.value
        else:
            restore_type = RestoreCommandParam.IGNORE_EXIST.value
        return restore_type
