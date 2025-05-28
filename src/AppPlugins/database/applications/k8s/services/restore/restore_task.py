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
import time
import json

from kubernetes.client import ApiException

from common.common import report_job_details
from common.common_models import LogDetail, SubJobDetails
from common.const import DBLogLevel, SubJobStatusEnum
from k8s.common.const import MetadataStatus, WORKLOAD_PLURAL_MAP, OCEAN_PROTECT_GROUP, \
    NO_NEED_RESTORE_RESOURCE_GROUP_AND_VER_AND_KIND, DAEMONSET, DEPLOYMENT, REPLICASET, STATEFULSET, JOB, CRONJOB, \
    DEPLOYMENTCONFIG, Progress, K8sReportLabel
from k8s.common.k8s_manager.pod_white_handler import PodWhiteListHandler
from k8s.common.label_const import LabelConst
from k8s.common.utils import read_file_in_dir
from k8s.common.kubernetes_client.k8s_api_class_custom_objects import ResourceCustomObjects
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.services.basic_job import BasicJob
from k8s.services.k8s_dispatcher import K8sDispatcher
from k8s.services.restore.restore_para_mgr import RestoreParamMgr
from k8s.logger import log


class RestoreConst:
    # workload资源
    workload_plural = ['jobs', 'cronjobs', 'statefulsets', 'replicasets', 'deployments', 'daemonsets']
    # 不恢复的cluster资源
    exclude_cluster_resources = ["nodes", "componentstatuses", "persistentvolumes"]
    # 不恢复的namespace资源
    exclude_namespace_resources = []


class RestoreTask(BasicJob):
    def __init__(self, restore_param: RestoreParamMgr):
        super().__init__(restore_param.job_id, restore_param.ns_name, restore_param.image_name,
                         restore_param.job_info)
        self._env_name_list = {}
        self.restore_param = restore_param
        self._pod_list = []
        self._pvc_num_per_pod = 1
        self.core_api_client = ResourceCoreApi(self.restore_param.job_info.resource.cluster_authentication)
        self.custom_res_client = ResourceCustomObjects(self.restore_param.job_info.resource.cluster_authentication)
        self._origin_name_space = None
        copy_namespace_dir = self._get_copy_namespace_dir()
        self._fill_origin_namespace(copy_namespace_dir)
        log.info(f"Namespace is {self._origin_name_space}")
        return

    @staticmethod
    def get_kind_metadata(k8s_copy_kind_path):
        # 包含group、 version、 plural、 namespace、kind
        res_kind_store_path = os.path.join(k8s_copy_kind_path, "kind.metadata")
        if not os.path.exists(res_kind_store_path):
            log.error(f"Not exist {res_kind_store_path}")
            raise Exception(f"Not found {res_kind_store_path}")
        with open(res_kind_store_path, 'r') as fh:
            res_kind_content = json.loads(fh.read())
        log.info(f"Kind metadata {k8s_copy_kind_path} is {res_kind_content}")
        return res_kind_content

    @staticmethod
    def _update_env(metadata_body, single_env, pos):
        for i, env in enumerate(metadata_body['spec']['template']['spec']['containers'][pos]['env']):
            if env.get('name') == single_env.get('key'):
                metadata_body['spec']['template']['spec']['containers'][pos]['env'][i] = \
                    {'name': single_env.get('key'), 'value': single_env.get('value')}
                return metadata_body
        log.warning(f'Env {single_env.get("key")} do not exists! Workload:{metadata_body.get("metadata").get("name")}')
        return metadata_body

    def prerequisite(self):
        self._check_namespace()
        if self._is_skip_restore():
            # 上报任务提示，数据集没有备份资源，跳过恢复
            log_detail = LogDetail(logInfo=K8sReportLabel.DATASET_WITH_NO_DATA_NEED_TO_SKIP_RESTORE,
                                   logInfoParam=[self.restore_param.exclude_labels, self.restore_param.labels],
                                   logLevel=DBLogLevel.WARN)
            report_job_details(self.restore_param.pid, SubJobDetails(taskId=self.restore_param.job_id,
                                                                     subTaskId=self.restore_param.sub_id,
                                                                     progress=Progress.PROGRESS_FORTY,
                                                                     logDetail=[log_detail],
                                                                     taskStatus=SubJobStatusEnum.COMPLETED.value))
            return True
        log.info(f"WhiteList Pod start, task id:{self.task_id}")
        PodWhiteListHandler.create_temp_pod_and_add_ip_white_list(self.restore_param.job_info)
        return True

    def restore(self):
        if self._is_skip_restore():
            return True
        status = MetadataStatus.PVC
        if not self.restore_param.pvc_maps:
            status = MetadataStatus.PRE
            self.restore_metadata(status)
            status = MetadataStatus.POST
        time.sleep(60)
        if not self._restore_pvc():
            return False
        self.restore_metadata(status)
        return True

    def post(self):
        if self._is_skip_restore():
            return True
        if not self._clean_pod():
            log.error(f'Clean pod failed! Task id:{self.task_id}')
            return False
        if not self._move_and_clean_cache(self.restore_param.job_info.cache_repo.local_path):
            log.error(f"Move_and_clean_cache failed! Task id:{self.task_id}")
            return False
        log.info(f'Action post succeed! Task id:{self.task_id}')
        return True

    def restore_namespace_res(self, group, version, plural, body):
        res_name = body.get("metadata", {}).get("name")
        if not res_name:
            log.error(f"No resource name in {body}")
            raise Exception(f"No resource name is {body}")
        if not self._process_meta_body(plural, body):
            return
        log.info(f"Group({group}), version({version}), plural({plural}) namespace({self.ns_name}, res name({res_name})")
        try:
            if group:
                self.custom_res_client.get_with_namespace(group, version, self.ns_name, plural, res_name)
            else:
                self.core_api_client.get_namespaced_resource_with_http_info(res_name, plural, self.ns_name)
            return
        except ApiException as ex:
            if ex.status == 404:
                log.info(f"Namespace resource({res_name}) does not exist, continue restore.")
        if group:
            log.info(f'Create namespace res:{group} {version} {self.ns_name} {plural}')
            self.custom_res_client.create_with_namespace(group, version, self.ns_name, plural, body)
        else:
            log.info(f'Create namespace res: {version} {self.ns_name} {plural}')
            self.core_api_client.create_namespaced_resource_with_http_info(plural, self.ns_name, body)
        return

    def restore_cluster_res(self, group, version, plural, body):
        res_name = body.get("metadata", {}).get("name")
        log.info(f"Group({group}, version({version}, plural({plural}, res name({res_name}")
        if not self._process_meta_body(plural, body):
            return
        try:
            if not group:
                log.warning(f'Core kind cluster resource {res_name}, skip! Task id:{self.task_id}')
                return
            self.custom_res_client.get_without_namespace(group, version, plural, res_name)
            log.info(f"Cluster resource({res_name}) exist, ignore restore.")
            return
        except ApiException as ex:
            if ex.status == 404:
                log.exception(f"Resource {res_name} does not exist, continue restore.")
        ret = self.custom_res_client.create_without_namespace(group, version, plural, body)
        log.info(f"Restore result {ret}")
        return

    def read_and_restore_metadata(self, scope, k8s_copy_kind_path):
        res_kind_content = self.get_kind_metadata(k8s_copy_kind_path)
        group = res_kind_content.get("group")
        version = res_kind_content.get("version")
        plural = res_kind_content.get("plural")
        namespace = res_kind_content.get("namespace")
        if not version or not plural:
            log.error(f"Get resource info failed, path{k8s_copy_kind_path}.")
            raise Exception(f"No version {version} and no plural {plural}")
        if plural in RestoreConst.exclude_cluster_resources or plural in RestoreConst.exclude_namespace_resources:
            log.info(f"The type of {plural} resource skip restore, task id: {self.task_id}")
            return

        for file in os.listdir(k8s_copy_kind_path):
            if file == "kind.metadata":
                continue
            log.info(f"Begin to restore {file}")
            meta_data_file = os.path.join(k8s_copy_kind_path, file)
            self.read_and_restore_metadata_single_file(scope, res_kind_content, meta_data_file)
        return

    def read_and_restore_metadata_single_file(self, scope, res_kind_content, meta_data_file):
        group = res_kind_content.get("group")
        version = res_kind_content.get("version")
        plural = res_kind_content.get("plural")
        if not self.is_need_restore(res_kind_content):
            return
        with open(meta_data_file, 'r') as fh:
            metadata_body = json.loads(fh.read())
            labels = metadata_body.get("metadata").get("labels")
            # 自己label的不恢复
            if labels and LabelConst.DPA_BACKUP_POD_COMMON_KEY in labels:
                return
            if "status" in metadata_body:
                metadata_body.pop("status")
        if scope == "namespace":
            metadata_body['metadata']['namespace'] = self.ns_name
            self._change_env_for_workload(plural, metadata_body)
            self.restore_namespace_res(group, version, plural, metadata_body)
        else:
            self._change_param_for_sc(res_kind_content, metadata_body)
            self.restore_cluster_res(group, version, plural, metadata_body)

    def restore_metadata_copy_data(self, scope, dir_path, status):
        prefer_restore_kind_list = [
            "CustomResourceDefinition", "APIService", "StorageClass", "Namespace", "ConfigMap", "Secret", "Service",
            "ServiceAccount"
        ]
        post_restore_kind_list = [
            DAEMONSET, DEPLOYMENT, REPLICASET, STATEFULSET, JOB, CRONJOB, "Pod", DEPLOYMENTCONFIG
        ]
        for k8s_group in os.listdir(dir_path):
            k8s_group_path = os.path.join(dir_path, k8s_group)
            if status == MetadataStatus.PRE:
                self.pre_metadata_restore(k8s_group_path, post_restore_kind_list, prefer_restore_kind_list, scope)
            elif status == MetadataStatus.POST:
                self.post_metadata_restore(k8s_group_path, post_restore_kind_list, scope)
            else:
                log.info(f'Pvc level restore, no need to restore metadata！Task id:{self.task_id}')
        return

    def post_metadata_restore(self, k8s_group_path, post_restore_kind_list, scope):
        log.info(f'Start restore workload metadata！ Task id:{self.task_id}')
        for post_kind in post_restore_kind_list:
            prefer_kind_path = os.path.join(k8s_group_path, post_kind)
            if os.path.isdir(prefer_kind_path):
                self.read_and_restore_metadata(scope, prefer_kind_path)

    def pre_metadata_restore(self, k8s_group_path, post_restore_kind_list, prefer_restore_kind_list, scope):
        log.info(f'Start restore metadata！ Task id:{self.task_id}')
        for prefer_kind in prefer_restore_kind_list:
            prefer_kind_path = os.path.join(k8s_group_path, prefer_kind)
            if os.path.isdir(prefer_kind_path):
                self.read_and_restore_metadata(scope, prefer_kind_path)
        for k8s_type in os.listdir(k8s_group_path):
            if k8s_type in prefer_restore_kind_list or k8s_type in post_restore_kind_list:
                continue
            k8s_type_path = os.path.join(k8s_group_path, k8s_type)
            if os.path.isdir(k8s_type_path):
                self.read_and_restore_metadata(scope, k8s_type_path)

    def restore_metadata(self, status):
        copy_cluster_dir = self._get_copy_cluster_dir()
        copy_namespace_dir = self._get_copy_namespace_dir()
        self.restore_metadata_copy_data("cluster", copy_cluster_dir, status)
        self._fill_origin_namespace(copy_namespace_dir)
        self.restore_metadata_copy_data("namespace", os.path.join(copy_namespace_dir, self._origin_name_space), status)
        log.info(f"Restore cluster and namespaced copy success.")
        return

    def is_need_restore(self, res_kind_content: dict):
        group_and_ver_and_kind = f"{res_kind_content.get('group', '')}/{res_kind_content.get('version', '')}" \
                                 f"|{res_kind_content.get('kind', '')}"
        if group_and_ver_and_kind in NO_NEED_RESTORE_RESOURCE_GROUP_AND_VER_AND_KIND:
            log.info(f"No need to restore group_and_ver_and_kind:{group_and_ver_and_kind} resource.")
            return False
        return True

    def _get_copy_cluster_dir(self):
        return os.path.join(self.restore_param.job_info.data_repo.local_path,
                            f"meta-file/{self.restore_param.job_info.copy_id}/cluster")

    def _get_copy_namespace_dir(self):
        return os.path.join(self.restore_param.job_info.data_repo.local_path,
                            f"meta-file/{self.restore_param.job_info.copy_id}/namespace")

    def _fill_origin_namespace(self, copy_namespace_dir):
        if not os.path.isdir(copy_namespace_dir):
            log.info(f"Not exist {copy_namespace_dir}, no need to restore.")
            return
        namespace_name = os.listdir(copy_namespace_dir)
        if not namespace_name:
            log.error(f"No namespace in {copy_namespace_dir}")
            raise Exception(f"Not namespace in {copy_namespace_dir}")
        self._origin_name_space = namespace_name[0]

    def _restore_pvc(self):
        log.info(f'Start restore_pvc, task id:{self.task_id}')
        if not self.restore_param.pvc_maps:
            log.warning(f'Pvc maps list is None! Task id:{self.task_id}')
            self._get_all_copy_pvc()
        restore_main_action = K8sDispatcher(self.restore_param.job_info, self.restore_param.pvc_maps)
        restore_main_action.start()
        log.info(f"Restore succeed! Task id:{self.task_id}")
        return True

    def _get_all_copy_pvc(self):
        pvc_path = f'{self.restore_param.job_info.data_repo.local_path}/pvc'
        pvc_list = read_file_in_dir(pvc_path)
        for pvc in pvc_list:
            self.restore_param.pvc_maps.update({pvc: pvc})

    def _process_meta_body(self, plural, body):
        """
        处理meta文件的内容
        如果处理后要继续执行后续恢复，则返回True,否则False

        :param plural:
        :param body:
        :return:
        """
        is_need_process_post = True
        if plural == 'events':
            body['involvedObject']['namespace'] = self.ns_name
        elif plural == 'services':
            body['spec'].pop('clusterIP', None)
            body['spec'].pop('clusterIPs', None)
            body['spec'].pop('ipFamilies', None)
        elif plural == 'persistentvolumeclaims':
            if body['metadata'].get('annotations'):
                body['metadata']['annotations'].pop('pv.kubernetes.io/bind-completed', None)
                body['metadata']['annotations'].pop('pv.kubernetes.io/bound-by-controller', None)
            if body['spec'].get('storageClassName'):
                body['spec'].pop('volumeName', None)
        elif plural in ['pods', 'jobs', 'replicasets']:
            is_need_process_post = self._process_resource_pods(body)
        elif plural == "namespaces":
            is_need_process_post = self._process_resource_namespace(body)
        return is_need_process_post

    def _process_resource_namespace(self, body):
        # 如果是命名空间直接恢复
        res_name = body.get("metadata", {}).get("name")
        log.warning(f'Restore namespace! {res_name}')
        if res_name != self._origin_name_space:
            log.warning(f'Namespace {res_name} not need to restore')
            return False
        try:
            self.core_api_client.get_namespace_with_http_info(namespace=self.ns_name)
            return False
        except ApiException as ex:
            if ex.status == 404:
                log.warning(f"Namespace ({res_name}) does not exist, continue restore.")
                body.get("metadata", {}).update({"name": self.ns_name})
                self.core_api_client.create_namespace_with_http_info(body)
                log.info(f'Namespace restored! Name:{res_name}')
                return False
            log.error(f'Create namespace {res_name} error! Task id:{self.task_id}')
            return False

    def _process_resource_pods(self, body):
        # 资源有ownerReferences，则不能被恢复
        if body.get("metadata").get("ownerReferences", {}):
            pod_name = body.get("metadata").get("name")
            log.info(f"{pod_name} has ownerReferences, skip restore, task_id: {self.task_id}")
            return False
        return True

    def _check_namespace(self):
        namespace_path = os.path.join(self._get_copy_cluster_dir(), OCEAN_PROTECT_GROUP, "Namespace")
        res_kind_content = self.get_kind_metadata(namespace_path)
        copy_namespace_dir = self._get_copy_namespace_dir()
        self._fill_origin_namespace(copy_namespace_dir)
        if self._is_skip_restore():
            return
        meta_data_file_path = os.path.join(namespace_path, self._origin_name_space)
        meta_data_file = meta_data_file_path + ".json"
        self.read_and_restore_metadata_single_file("cluster", res_kind_content, meta_data_file)

    def _is_backup_namespace(self):
        return self._origin_name_space is not None

    def _is_skip_restore(self):
        return not self._is_backup_namespace()

    def _write_env_into_metabody(self, metadata_body, container, env_dict):
        container_list = metadata_body.get('spec').get('template').get('spec').get('containers', [])
        if not container_list:
            return metadata_body
        pos = 0
        for singe_container in container_list:
            if singe_container.get('name') == container:
                if not metadata_body['spec']['template']['spec']['containers'][pos].get('env'):
                    metadata_body['spec']['template']['spec']['containers'][pos].update({'env': []})
                for single_env in env_dict:
                    self._update_env(metadata_body, single_env, pos)
                return metadata_body
            pos += 1
        return metadata_body

    def _change_env_for_workload(self, plural, metadata_body):
        workload_name = metadata_body.get('metadata').get('name')
        log.debug(f"plural:{plural}, workload_name:{workload_name}")
        if plural not in RestoreConst.workload_plural:
            log.warning(f"Not target workload type: {plural}")
            return metadata_body
        if workload_name in self.restore_param.job_info.change_env_dict.keys():
            if plural != WORKLOAD_PLURAL_MAP.get(
                    self.restore_param.job_info.change_env_dict[workload_name].get('workLoadType')):
                log.warning(f"Plural cannot match! Plural:{plural}")
                return metadata_body
            if plural == 'cronjobs' and metadata_body.get('spec').get('jobTemplate'):
                cron_metabody = metadata_body
                metadata_body = cron_metabody.get('spec').get('jobTemplate')
            self._write_env_into_metabody(metadata_body,
                                          self.restore_param.job_info.change_env_dict[workload_name].get(
                                              'containerName'),
                                          self.restore_param.job_info.change_env_dict[workload_name].get('envMap'))
            if plural == 'cronjobs' and metadata_body.get('spec').get('jobTemplate'):
                cron_metabody['spec']['jobTemplate'] = metadata_body
                metadata_body = cron_metabody
        return metadata_body

    def _change_param_for_sc(self, res_kind_content, metadata_body):
        group = res_kind_content.get("group")
        version = res_kind_content.get("version")
        kind = res_kind_content.get("kind")
        sc_name = metadata_body.get('metadata', {}).get('name', '')
        if 'storage.k8s.io/v1|StorageClass' != f"{group}/{version}|{kind}" \
                or sc_name not in self.restore_param.job_info.change_sc_dict.keys():
            return

        parameters = metadata_body.get('parameters', {})
        log.info(f"sc:{sc_name}, parameters:{parameters}")
        self._write_param_into_meta_body(metadata_body,
                                         self.restore_param.job_info.change_sc_dict[sc_name].get('paramsMap', {}))

    def _write_param_into_meta_body(self, metadata_body: dict, param_map: dict):
        parameters = metadata_body.get('parameters', {})
        if not parameters:
            metadata_body.update({'parameters': {}})
        metadata_body.get('parameters', {}).update(param_map)
        return metadata_body
