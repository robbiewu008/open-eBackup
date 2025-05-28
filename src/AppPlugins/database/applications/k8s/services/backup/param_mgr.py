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

import subprocess
import yaml

from k8s.common.k8s_resource_map import K8SAPI
from k8s.common.parse_param import parse_param_with_jsonschema
from common.const import BackupTypeEnum, RepositoryDataTypeEnum, SubJobPolicyEnum, SubJobTypeEnum, SubJobPriorityEnum
from common.parse_parafile import ParamFileUtil
from common.exception.common_exception import ErrCodeException
from common.common_models import SubJobModel
from common.env_common import get_install_head_path
from k8s.logger import log
from k8s.common.const import K8sBackupKeyName, K8sSubType, ActionType, JobInfo, CONSISTENT_SCRIPT_TIMEOUT
from k8s.common.kubernetes_client.struct import ClusterAuthentication, Token, Resource, Namespace, DataSet, AuthType, \
    ConsistencyRules, Repositories
from k8s.common.error_code import ErrorCode
from k8s.common.utils import validate_auth_str, validate_port_str, \
    validate_ip_str, get_env_variable, clean_auth_info, clean_ssl_file, get_task_timeout


class ParamMgr:
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
            self._host_ip = self._body_param.get("job", {}).get("protectEnv", {}).get("endpoint", "")
            self._port = self._body_param.get("job", {}).get("protectEnv", {}).get("port", "")
            self._address = f"https://{self._host_ip}:{self._port}"
            self.ns_name = self._body_param.get("job", {}).get("protectObject", {}).get("parentName", "")
            ns_object = Namespace(name=self.ns_name)
            self._protect_object = self._body_param.get("job", {}).get("protectObject", {})
            backup_type = self._body_param.get("job", {}).get("jobParam", {}).get("backupType",
                                                                                  BackupTypeEnum.FULL_BACKUP)
            data_set = self._parse_dataset()
            cluster_name = self._body_param.get("job", {}).get("protectEnv", {}).get("name", "")
            log.info(f"Namespace {self.ns_name}, cluster_name {cluster_name}")
            self._resource = Resource(cluster_name=cluster_name, backup_type=backup_type, namespace=ns_object,
                                      dataset=data_set, cluster_authentication=self._auth)
            self._instance_id = self._body_param.get("job", {}).get("protectObject", {}).get("id", "")
            self._sub_type = self._body_param.get("job", {}).get("protectObject", {}).get("subType", "")
            cache_repo_obj = self.get_repository_info(RepositoryDataTypeEnum.CACHE_REPOSITORY)
            data_repo_obj = self.get_repository_info(RepositoryDataTypeEnum.DATA_REPOSITORY)
            meta_repo_obj = self.get_repository_info(RepositoryDataTypeEnum.META_REPOSITORY)
            self._node_id_list = self._parse_node_id()
            self._scripts = self._body_param.get("job", {}).get("jobParam", {}).get("scripts", {})
            extend_info = self._body_param.get("job", {}).get("extendInfo", {})
            extend_info = extend_info if extend_info is not None else {}
            copy_id = self._body_param.get("job", {}).get("copy", [])[0].get("id", "")
            task_id = self._body_param.get("job", {}).get("jobId", "")
            log.info(f"Task is {task_id}")
            _node_selector, _task_timeout, image_name, _consistent_timeout = self._get_param_from_extendinfo()
            self.job_info = JobInfo(resource=self._resource, consistency_rules=ConsistencyRules(), sub_id=sub_job_id,
                                    task_id=task_id, data_repo=data_repo_obj, cache_repo=cache_repo_obj,
                                    meta_repo=meta_repo_obj, image_name=image_name, copy_id=copy_id,
                                    pod_nums=self._pod_num, advanced_params=extend_info, request_id=pid,
                                    action_type=ActionType.BACKUP, job_type=backup_type, node_selector=_node_selector,
                                    task_timeout=_task_timeout, consistent_timeout=_consistent_timeout)
        except Exception as e:
            log.exception(f"Init para failed.")
            clean_ssl_file(self._auth)
            clean_auth_info(self._auth)
            raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="ResourceParam init error!") from e

    def get_repository_info(self, repository_type):
        repositories = self._body_param.get("job", {}).get("repositories", [])
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                log.info(f"Extend info is {repository['extendInfo']}")
                extend_info = repository.get('extendInfo', {})
                if 'storageLogicalIps' in extend_info:
                    logical_ip_list = extend_info['storageLogicalIps']
                else:
                # 执行命令获取IP列表
                    command = (
                        f"grep listen {get_install_head_path()}/DataBackup/ProtectClient/ProtectClient-E/nginx/conf/nginx.conf | "
                        "awk -F ' ' '{print $2}' | "
                        "awk -F ':' '{print $1}'"
                )
                try:
                    # 执行命令并捕获输出
                    result = subprocess.run(
                        command,
                        shell=True,
                        capture_output=True,
                        text=True,
                        check=True  # 检查命令是否成功执行
                    )
                    # 解析命令输出结果
                    logical_ip_list = result.stdout.strip().split()
                    log.info(f"Logical IP List: {logical_ip_list}")
                except subprocess.CalledProcessError as e:
                    # 命令执行失败时记录错误并抛出异常
                    log.error(f"Command execution failed: {e}")
                    raise Exception("Failed to retrieve IPs from nginx.conf")
                return Repositories(logical_ip_list=logical_ip_list,
                                    remote_path=repository["remotePath"],
                                    local_path=repository["path"][0])
        log.error(f"Not found repository {repository_type}")
        raise Exception("Not found repository")

    def check_backup_job_type(self):
        log.info(f'Start execute check_backup_job_type, pid: {self.pid}, job_id:{self.job_id}')
        backup_type = self._body_param.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type and (backup_type == BackupTypeEnum.FULL_BACKUP or backup_type == BackupTypeEnum.INCRE_BACKUP):
            return True
        log.error(f"Get wrong backup type! Type:{backup_type}")
        return False

    def get_sub_job(self):
        log.info("Step 5-1 start to gen_sub_job!")
        if self._sub_type == K8sSubType.SUBTYPE_NAMESPACE:
            log.debug(f"Generate namespace subjob model:jobid{self.job_id},subjob:{self.sub_id}")
            sub_job = SubJobModel(jobId=self.job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                  jobName="sub_ns", policy=SubJobPolicyEnum.ANY_NODE.value,
                                  jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1.value).dict(by_alias=True)
            log.debug(f"Sub job:{sub_job}")
            return sub_job
        elif self._sub_type == K8sSubType.SUBTYPE_DATASET:
            sub_job = SubJobModel(jobId=self.job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                  jobName="sub_ds", policy=SubJobPolicyEnum.ANY_NODE.value,
                                  jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1.value).dict(by_alias=True)
            return sub_job
        raise ErrCodeException(err_code=ErrorCode.PARAM_FAILED, message=str(ErrorCode.PARAM_FAILED.value))

    def get_path_ip_dict(self):
        repositories = self._body_param.get("job", {}).get("repositories", [])
        path_dict = dict()
        ip_dict = dict()
        for repository in repositories:
            path_dict.update({repository['repositoryType']: repository.get('remotePath')})
            ip_dict.update({repository['repositoryType']: repository.get('remoteHost')[0].get('ip')})
        return path_dict, ip_dict

    def clean_backup_auth_info(self):
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
        self._auth.auth_type = int(get_env_variable(K8sBackupKeyName.PROTECTENV_AUTH_AUTHTYPE + self.pid))
        log.info(f"Action update_auth_info, authtype={self._auth.auth_type}")
        if self._auth.auth_type == AuthType.TOKEN:
            if not self._get_token_info():
                log.error(f"Action get_token_info Failed! req_id:{self.pid}")
                raise ErrCodeException(ErrorCode.PARAM_FAILED, message="get_token_info error!")
        elif self._auth.auth_type == AuthType.CONFIGFILE:
            if not self._get_kube_config_info():
                log.error(f"Action get_config_info Failed! req_id:{self.pid}")
                raise ErrCodeException(ErrorCode.PARAM_FAILED, message="get_config_info error!")
        else:
            raise ErrCodeException(ErrorCode.PARAM_FAILED, message="Auth type error!")
        log.info("SUCCESS! update_auth_info")

    def _get_token_info(self):
        """
        从参数中获取k8s集群地址
        """
        log.info("Start get_token_info!!!")
        self._auth.token.address = self._body_param.get("job", {}).get("protectEnv", {}).get("endpoint", "")
        if not validate_ip_str(self._auth.token.address):
            log.error(f"Validate ip Failed! req_id:{self.pid}")
            return False
        self._auth.token.port = self._body_param.get("job", {}).get("protectEnv", {}).get("port", "")
        if not validate_port_str(self._auth.token.port):
            log.error(f"Validate port Failed! req_id:{self.pid}")
            return False
        self._auth.token.token_info = get_env_variable(K8sBackupKeyName.PROTECTENV_AUTH_EXTENDINFO_TOKEN + self.pid)
        if not validate_auth_str(self._auth.token.token_info):
            log.error(f"Validate token Failed! req_id:{self.pid}")
            return False
        self._auth.is_verify_ssl = self._body_param.get("job", {}).get("protectEnv", {}).get("extendInfo", {}).get(
                "isVerifySsl", "")
        if not validate_auth_str(self._auth.is_verify_ssl):
            log.error(f"Validate is_verify_ssl Failed! req_id:{self.pid}")
            return False
        self._auth.token.certificateAuthorityData = get_env_variable(
            K8sBackupKeyName.PROTECTENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA + self.pid
        )
        log.info("SUCCESS! get_token_info")
        return True

    def _get_kube_config_info(self):
        """
        从参数中获取k8s集群地址
        """
        log.info("Start get_config_info!!!")
        self._auth.is_verify_ssl = self._body_param.get("job", {}).get("protectEnv", {}).get("extendInfo", {}).get(
                "isVerifySsl", "")
        if not validate_auth_str(self._auth.is_verify_ssl):
            log.error(f"Validate is_verify_ssl Failed! req_id:{self.pid}")
            return False
        kube_config = get_env_variable(K8sBackupKeyName.PROTECTENV_AUTH_EXTENDINFO_CONFIG + self.pid)
        kube_config = str(b64decode(kube_config).decode("ascii"))
        if not validate_auth_str(kube_config):
            log.error(f"Validate config Failed! req_id:{self.pid}")
            return False
        self._auth.kube_config = yaml.safe_load(kube_config)
        log.info("SUCCESS! get_config_info")
        return True

    def _parse_dataset(self):
        data_set_name = self._protect_object.get("name", "")
        label_list = self._parse_label()
        data_set = DataSet(name=data_set_name, labels=label_list)
        return data_set

    def _parse_label(self):
        app_extend_info = self._body_param.get("job", {}).get("protectObject", {}).get("extendInfo", {})
        label_list = app_extend_info.get("labels", '')
        return label_list

    def _parse_node_id(self):
        node_id_list = []
        for node in self._body_param.get("job", {}).get("extendInfo", {}).get("agents", {}):
            node_id_list.append(node.get("id", "") if node.get("id", "") != "" else None)
        return node_id_list

    def _get_param_from_extendinfo(self):
        extend_info = self._body_param.get("job", {}).get("protectEnv", {}).get("extendInfo", {})
        image_name = extend_info.get("imageNameAndTag")
        log.info(f"Image name is {image_name}")
        self._pod_num = extend_info.get("jobNumOnSingleNode", 4)
        _node_selector = extend_info.get("nodeSelector", "")
        timeout = extend_info.get("taskTimeout", '')
        _task_timeout = get_task_timeout(timeout)
        log.info(f"Task time out is {timeout}, _task_timeout is {_task_timeout} task is {self.job_id}")
        consistent_timeout = extend_info.get("consistentScriptTimeout", '')
        _consistent_timeout = get_task_timeout(consistent_timeout, CONSISTENT_SCRIPT_TIMEOUT)
        log.info(f"Consistent time out is {consistent_timeout},"
                 f" _consistent_timeout is {_consistent_timeout} task is {self.job_id}")
        return _node_selector, _task_timeout, image_name, _consistent_timeout
