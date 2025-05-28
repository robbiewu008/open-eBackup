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

import time
import uuid

from typing import Optional
from pydantic import BaseModel

from kubernetes.client import V1PersistentVolumeClaimVolumeSource, V1VolumeMount, V1NFSVolumeSource

from common.common import report_job_details
from common.common_models import LogDetail, SubJobDetails
from common.const import DBLogLevel, SubJobStatusEnum
from common.exception.common_exception import ErrCodeException
from k8s.common.const import DATA_REPO, META_REPO, CACHE_REPO, NFS_DATA_PATH, NFS_META_PATH, \
    NFS_CACHE_PATH, BackupPodType, JobInfo, Event, PREFIX, K8sReportLabel, Progress
from k8s.common.error_code import ErrorCode
from k8s.common.k8s_manager.event_manager import EventManager
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_backup_pod_api import BackupPod
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.kubernetes_client.k8s_api_class_custom_objects import ResourceCustomObjects
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.kubernetes_client.struct import PodPhase
from k8s.common.label_const import LabelConst
from k8s.logger import log
from k8s.common.kubernetes_client.k8s_api import InitKubernetesApiError


class ImageNameIllegalError(Exception):
    def __init__(self):
        super().__init__()
        log.error('Image Name get failed, name illegal.')


class CreatePodInfo(BaseModel):
    node_name: str
    pvc_name_list: Optional[list]
    pod_name: str
    command: list
    pod_type: BackupPodType
    labels: Optional[dict]
    logic_ip: Optional[str]


class BackupPodManager:
    @staticmethod
    def get_node_dict(job_info: JobInfo):
        label_selector = "" if job_info.node_selector is None else job_info.node_selector
        node_list = ResourceCoreApi(job_info.resource.cluster_authentication) \
            .list_cluster_resource_with_http_info('nodes', label_selector=label_selector)
        node_dict = {}
        no_driver_node_list = BackupPodManager.get_no_csi_driver_node_list(job_info, node_list.get("items"))
        for node in node_list.get("items"):
            if node.get("metadata").get("name") in no_driver_node_list:
                continue
            node_dict[node.get("metadata").get("name")] = 'Ready'
        if not node_dict:
            raise ErrCodeException(ErrorCode.OPERATION_FAILED)
        return node_dict

    @staticmethod
    def get_no_csi_driver_node_list(job_info, node_list):
        rco = ResourceCustomObjects(job_info.resource.cluster_authentication)
        no_driver_node_list = []
        for node in node_list:
            log.info(f"csi_node is {node['metadata']['name']}")
            csi_node = rco.get_without_namespace(group='storage.k8s.io', version='v1', plural='csinodes',
                                                 name=node['metadata']['name'])
            if not csi_node.get("spec").get("drivers"):
                no_driver_node_list.append(node['metadata']['name'])
        return no_driver_node_list

    @staticmethod
    def create_backup_pods(job_info: JobInfo, create_pod_infos: list):
        """
        :param job_info: JobInfo
        :param create_pod_infos: CreatePodInfo的list
        :return: timeout时间内pod的启动情况
        """
        # pod_name->True
        pod_running_result = {}
        if not create_pod_infos:
            return pod_running_result
        for create_pod_info in create_pod_infos:
            BackupPodManager._create_backup_pod(job_info, create_pod_info)
            pod_running_result[create_pod_info.pod_name] = False
        resource_pod = ResourcePod(job_info.resource.cluster_authentication)
        timeout = 60
        while timeout >= 0:
            for create_pod_info in create_pod_infos:
                if pod_running_result.get(create_pod_info.pod_name, False):
                    continue
                try:
                    pod_info = resource_pod.read(create_pod_info.pod_name, job_info.resource.namespace.name)
                    BackupPodManager._check_pod_status(pod_info, pod_running_result, create_pod_info, job_info)
                except ErrCodeException as err:
                    raise ErrCodeException(err.error_code, *err.parameter_list, message=err.error_message) from err
                except Exception:
                    log.exception(f"Pod create failed, pod is {create_pod_info.pod_name}")
            if BackupPodManager.check_all_running(pod_running_result):
                break
            time.sleep(5)
            timeout -= 5
        if not BackupPodManager.check_all_running(pod_running_result):
            log.warn(f"pod can not be running in time: {timeout}s")
        return pod_running_result

    @staticmethod
    def check_all_running(pod_running_result):
        for key in pod_running_result:
            if not pod_running_result[key]:
                return False
        return True

    @staticmethod
    def del_backup_pods(cluster_authentication, namespace, pod_name_list):
        pod_api = ResourcePod(cluster_authentication)
        for pod in pod_name_list:
            pod_api.delete(pod, namespace)
            log.debug(f'Delete pod:{pod}')
        return

    @staticmethod
    def get_logical_ip(job_info: JobInfo, node_dict):
        logical_ip_list = BackupPodManager._get_all_logic_ip(job_info)
        log.info(f"Start get logical_ip_list: {logical_ip_list}, task is {job_info.task_id}")
        # 分别在节点上创建pod，创建pod时挂载数据仓。查询pod的挂载失败事件
        pod_info_list = BackupPodManager._get_creat_pod_info_list(job_info, node_dict, len(logical_ip_list))
        logic_ip_node_dict = dict(zip(logical_ip_list, pod_info_list))
        on_use_logical_ip_list = []
        pod_dict = {}
        init_pod_nums = 0
        left_pod_nums = len(logic_ip_node_dict)
        max_pod_nums = 10 if len(logic_ip_node_dict) > 10 else len(logic_ip_node_dict)
        # 每次创建10个pod
        for logical_ip, pod_info in logic_ip_node_dict.items():
            pod_info.logic_ip = logical_ip
            # 默认创建3次pod，3次都没创建成功则记录对应逻辑端口的日志
            for _ in range(3):
                BackupPodManager._create_backup_pod(job_info, pod_info)
                if BackupPodManager._pod_is_create(job_info, pod_info):
                    pod_dict.update({logical_ip: pod_info.pod_name})
                    break
            if not BackupPodManager._pod_is_create(job_info, pod_info):
                log.warn(f"Create logic pod failed, logic ip is {logical_ip}, pod is {pod_info.pod_name},"
                         f" task is {job_info.task_id}")
            init_pod_nums += 1
            if init_pod_nums == max_pod_nums:
                init_pod_nums = 0
                left_pod_nums -= 10
                max_pod_nums = 10 if left_pod_nums - 10 > 10 else left_pod_nums
                # 获取所有创建的pod最长等待时间3分钟，pod状态为running则挂载成功，返回该逻辑端口
                if BackupPodManager._retry_and_get_logic_ip(pod_dict, job_info, on_use_logical_ip_list):
                    return on_use_logical_ip_list
        return on_use_logical_ip_list

    @staticmethod
    def _retry_and_get_logic_ip(pod_dict: {}, job_info: JobInfo, on_use_logical_ip_list: []):
        log.info(f"Start sleep 10 seconds.")
        time.sleep(10)
        retry_time = 600
        init_time = 0
        while init_time < retry_time:
            log.info(f"Retry {init_time} s.")
            if BackupPodManager._is_mount_success(pod_dict, job_info, on_use_logical_ip_list):
                return on_use_logical_ip_list
            time.sleep(5)
            init_time += 5
            report_job_details(job_info.request_id,
                               SubJobDetails(taskId=job_info.task_id,
                                             subTaskId=job_info.sub_id,
                                             progress=0,
                                             logDetail=[],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
        pod_dict.clear()
        # 清理逻辑端口pod
        BackupPodManager.del_backup_pods(job_info.resource.cluster_authentication,
                                         job_info.resource.namespace.name,
                                         pod_dict.values())
        return []

    @staticmethod
    def _is_mount_success(pod_dict: {}, job_info: JobInfo, on_use_logical_ip_list: []):
        for logical_ip, pod_name in pod_dict.items():
            if BackupPodManager._pod_is_running(job_info, pod_name):
                on_use_logical_ip_list.append(logical_ip)
                # 清理逻辑端口pod
                BackupPodManager.del_backup_pods(job_info.resource.cluster_authentication,
                                                 job_info.resource.namespace.name,
                                                 pod_dict.values())
                log.info(f"End get logical_ip_list: {on_use_logical_ip_list}, task is {job_info.task_id}")
                return True
        return False

    @staticmethod
    def _create_backup_pod(job_info: JobInfo, create_pod_info):
        node_name = create_pod_info.node_name
        pvc_name_list = create_pod_info.pvc_name_list
        pod_name = create_pod_info.pod_name
        command = create_pod_info.command
        pod_type = create_pod_info.pod_type

        pod_uuid = str(uuid.uuid4())

        task_id = job_info.task_id
        backup_pod = BackupPod(job_info.resource.cluster_authentication)
        log.info(f"**************** Pod name is {pod_name}")
        backup_pod.set_job_name(pod_name)
        backup_pod.set_labels(create_pod_info.labels)
        if create_pod_info.logic_ip:
            backup_pod = BackupPodManager._handle_nfs_volume_for_check_logic_ip(backup_pod, job_info,
                                                                                create_pod_info.logic_ip)
            backup_pod.set_resource_requirement()
        else:
            backup_pod = BackupPodManager._handle_nfs_volume(backup_pod, job_info, pod_type)
        BackupPodManager._handle_pvc_volume(backup_pod, pvc_name_list)

        # 升级场景，防止之前已经绕过前端门禁成功注册的镜像名称进行备份和恢复，并给出提示。
        try:
            backup_pod.set_container_image(job_info.image_name)
        except InitKubernetesApiError:
            raise ImageNameIllegalError

        backup_pod.set_container_command(command)
        _ns_name = job_info.resource.namespace.name
        backup_pod.set_job_namespace(_ns_name)
        log.info(f"Backup pod name space is {_ns_name}")
        backup_pod.set_node_name(node_name)
        backup_pod.create_backup_pod()
        log.info(f'Create backup pod {pod_name} result task id is {task_id}.')

    @staticmethod
    def _handle_pvc_volume(backup_pod, pvc_name_list):
        if not pvc_name_list:
            return
        for pvc in pvc_name_list:
            pvc_item = V1PersistentVolumeClaimVolumeSource(claim_name=pvc, read_only=False)
            backup_pod.add_snapshot_pvc_volume(pvc, pvc_item)
            pvc_mount = V1VolumeMount(name=pvc, mount_path=f"/mnt/source/{pvc}")
            backup_pod.set_volume_mount_path(volume_name=pvc, volume_mount=pvc_mount)
            log.info(f"Pvc name is {pvc}")

    @staticmethod
    def _handle_nfs_volume(backup_pod, job_info: JobInfo, pod_type):
        if pod_type != BackupPodType.BACKUP_PVC_TYPE:
            return backup_pod
        task_id = job_info.task_id
        _repo_dict = {
            DATA_REPO: job_info.data_repo,
            META_REPO: job_info.meta_repo,
            CACHE_REPO: job_info.cache_repo
        }
        log.info(f"Begin to add date remote path {_repo_dict[DATA_REPO].remote_path}.")
        backup_pod = BackupPodManager._add_nfs_volume(f'k8s-{job_info.task_id}', DATA_REPO, backup_pod, _repo_dict)
        log.info(f"Begin to add meta remote path {_repo_dict[META_REPO].remote_path}.")
        backup_pod = BackupPodManager._add_nfs_volume(f'k8s-{task_id}-meta', META_REPO, backup_pod, _repo_dict)
        log.info(f"Begin to add cache remote path {_repo_dict[CACHE_REPO].remote_path}.")
        backup_pod = BackupPodManager._add_nfs_volume(f'k8s-{task_id}-cache', CACHE_REPO, backup_pod, _repo_dict)
        return backup_pod

    @staticmethod
    def _handle_nfs_volume_for_check_logic_ip(backup_pod, job_info: JobInfo, logic_ip):
        task_id = job_info.task_id
        log.info(f"Begin to add data remote path {job_info.data_repo.remote_path}.")
        nfs_name = f'k8s-{task_id}'
        nfs_item = V1NFSVolumeSource(server=logic_ip,
                                     path=job_info.data_repo.remote_path)
        backup_pod.add_nfs_volume(nfs_name, nfs_item)
        nfs_mount = V1VolumeMount(name=nfs_name,
                                  mount_path=NFS_DATA_PATH)
        backup_pod.set_volume_mount_path(volume_name=nfs_name, volume_mount=nfs_mount)
        return backup_pod

    @staticmethod
    def _add_nfs_volume(nfs_name, repo_key, backup_pod, _repo_dict):
        nfs_item = V1NFSVolumeSource(server=_repo_dict[repo_key].logical_ip_list[0],
                                     path=_repo_dict[repo_key].remote_path)
        backup_pod.add_nfs_volume(nfs_name, nfs_item)
        _path_dict = {
            DATA_REPO: NFS_DATA_PATH,
            META_REPO: NFS_META_PATH,
            CACHE_REPO: NFS_CACHE_PATH
        }
        nfs_mount = V1VolumeMount(name=nfs_name,
                                  mount_path=_path_dict[repo_key])
        backup_pod.set_volume_mount_path(volume_name=nfs_name, volume_mount=nfs_mount)
        return backup_pod

    @staticmethod
    def _check_pod_status(pod_info, pod_running_result, create_pod_info: CreatePodInfo, job_info: JobInfo):
        if pod_info.status.phase == "Running":
            pod_running_result[create_pod_info.pod_name] = True
        elif pod_info.status.phase == "Failed":
            raise ErrCodeException(err_code=ErrorCode.CREATE_POD_FAILED)
        else:
            event_manager = EventManager(cluster_authentication=job_info.resource.cluster_authentication,
                                         namespace=job_info.resource.namespace.name,
                                         task_id=job_info.task_id,
                                         )
            field_selector = f'involvedObject.name={create_pod_info.pod_name}'
            event_list = event_manager.list_namespace_event_by_label(field_selector)
            for event in event_list:
                if Event.NODE_HAS_NO_DRIVER in event['message']:
                    log.error(f'Node {create_pod_info.node_name} has not driver! Task id:{job_info.task_id}')
                    raise ErrCodeException(ErrorCode.NODE_HAS_NO_USE_CSI_DRIVER, create_pod_info.node_name)

    @staticmethod
    def _get_creat_pod_info_list(job_info: JobInfo, node_dict, logical_ip_nums):
        create_pod_info_list = []
        not_ready_node_list = []
        while logical_ip_nums > 0:
            # 防止死循环
            if len(not_ready_node_list) == len(node_dict.keys()):
                break
            for node_name in node_dict.keys():
                if not BackupPodManager._check_node_status(node_name, job_info):
                    not_ready_node_list.append(node_name)
                    continue
                pod_name = f'{PREFIX.LOGIC_CHECK_PREFIX}-{job_info.task_id}-{str(uuid.uuid4())}'
                labels = {
                    LabelConst.DPA_BACKUP_POD_COMMON_KEY: LabelConst.DPA_BACKUP_POD_COMMON_VALUE,
                    LabelConst.DPA_BACKUP_POD_TYPE_KEY: LabelConst.DPA_BACKUP_POD_TYPE_LOGIC_CHECK,
                    LabelConst.DPA_BACKUP_POD_TASK_KEY: job_info.task_id,
                    LabelConst.DPA_BACKUP_POD_NODE_KEY: node_name
                }
                create_pod_info = CreatePodInfo(node_name=node_name, pvc_name_list=[], pod_name=pod_name,
                                                command=["/bin/sh", "-c", "while true;do sleep 1;done"],
                                                pod_type=BackupPodType.OBTAIN_WHITE_LIST_TYPE, labels=labels)
                create_pod_info_list.append(create_pod_info)
                logical_ip_nums -= 1
        return create_pod_info_list

    @staticmethod
    def _check_node_status(node_name, job_info: JobInfo):
        cluster_authentication = job_info.resource.cluster_authentication
        rc_api = ResourceCoreApi(cluster_authentication)
        field_selector = f'metadata.name={node_name}'
        node_res = rc_api.list_cluster_resource_with_http_info('nodes', field_selector=field_selector)
        node = node_res.get('items', {})[0]
        for condition in node.get('status', {}).get('conditions', []):
            if condition.get('type', '') == 'Ready' and 'True' == condition.get('status', ''):
                log.info(f"Start check node status, node is {node_name}, status is Ready,"
                         f" task id is {job_info.task_id}")
                return True
        log.info(f"Start check node status, node is {node_name}, status is Not Ready,"
                 f" task id is {job_info.task_id}")
        return False

    @staticmethod
    def _pod_is_create(job_info: JobInfo, pod_info: CreatePodInfo):
        cluster_authentication = job_info.resource.cluster_authentication
        rc_api = ResourceCoreApi(cluster_authentication)
        field_selector = f'metadata.name={pod_info.pod_name}'
        pod_res = rc_api.list_namespaced_resource_with_http_info('pods', job_info.resource.namespace.name,
                                                                 field_selector=field_selector)
        if pod_res.get('items', []):
            return True
        else:
            return False

    @staticmethod
    def _pod_is_running(job_info: JobInfo, pod_name):
        cluster_authentication = job_info.resource.cluster_authentication
        rc_api = ResourceCoreApi(cluster_authentication)
        field_selector = f'metadata.name={pod_name}'
        pod_res = rc_api.list_namespaced_resource_with_http_info('pods', job_info.resource.namespace.name,
                                                                 field_selector=field_selector)
        if pod_res.get('items', [])[0].get('status', {}).get('phase', '') == PodPhase.RUNNING.value:
            return True
        else:
            return False

    @staticmethod
    def _get_all_logic_ip(job_info: JobInfo):
        logic_ip_list = set()
        logic_ip_list.update(job_info.data_repo.logical_ip_list)
        logic_ip_list.update(job_info.meta_repo.logical_ip_list)
        logic_ip_list.update(job_info.cache_repo.logical_ip_list)
        return logic_ip_list
