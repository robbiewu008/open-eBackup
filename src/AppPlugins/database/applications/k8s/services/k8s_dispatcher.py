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
from enum import Enum
from typing import Optional

from pydantic import BaseModel, Field

from common.common import report_job_details
from common.common_models import SubJobDetails
from common.const import SubJobStatusEnum
from common.exception.common_exception import ErrCodeException
from k8s.common.k8s_manager.backup_pod_manager import BackupPodManager, CreatePodInfo
from k8s.common.const import NFS_DATA_PATH, ActionType, BackupPodType, JobInfo, K8SProtectError, \
    TARGET_PREFIX, RestoreCommandParam, Event
from k8s.common.error_code import ErrorCode
from k8s.common.k8s_manager.event_manager import EventManager
from k8s.common.k8s_manager.workload_manager import WorkloadManager
from k8s.common.kubernetes_client.k8s_api_class_apps.k8s_workload_api import ResourceWorkLoad
from k8s.common.kubernetes_client.k8s_api_class_batch.k8s_job_api import ResourceJob
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.kubernetes_client.struct import PodPhase, WorkloadInfo
from k8s.common.label_const import LabelConst
from k8s.common.utils import validate_cmd_strs
from k8s.logger import log


class NodeStatus(BaseModel):
    uid: str
    total_nums: int = Field(0, description='total_nums')
    pod_nums: int = Field(0, description='pod_nums')
    prefer_pvc = []
    status: bool


class PvcStatus(str, Enum):
    # pvc完成状态，同对应的pod状态
    SUCCEEDED = "Succeeded"
    RUNNING = 'Running'
    IDLE = "idle"


class PvcInfo(BaseModel):
    pvc_status: str
    pvc_name: Optional[str]
    node_name: Optional[str]
    start_times: int
    running_time: int


class PodStatus(BaseModel):
    pvc_name: str
    phase: str
    node_name: str
    pod_name: str


class K8sDispatcher:

    def __init__(self, job_info: JobInfo, pvc_dict):
        """
        :param task_id:
        :param job_info: JobInfo
        :param pvc_list: Backup {snapshot_pvc:origin_pvc}
                         Restore {target_pvc:origin_pvc（副本里面的）}
        :param auth:
        """
        self._job_info = job_info
        self._pvc_dict = pvc_dict
        self._pod = []
        self._pod_api = ResourcePod(self._job_info.resource.cluster_authentication)
        self._pvc_api = ResourcePersistentVolumeClaim(self._job_info.resource.cluster_authentication)
        self._core_api = ResourceCoreApi(self._job_info.resource.cluster_authentication)
        self._replica_set_api = ResourceWorkLoad(self._job_info.resource.cluster_authentication, 'ReplicaSet')
        self._job_api = ResourceJob(self._job_info.resource.cluster_authentication)
        # 单节点pod数量限制
        self._pod_nums = job_info.pod_nums
        # 节点管理 {NodeName: NodeStatus(uid, pod_nums, total_nums)}
        self._node_status = {}
        # 查询待备份pvc节点相关信息, {pvc_name:node_name}
        self._pvc_node_map = {}
        # 查询pvc相关workload信息，{workload_name:[WorkloadInfo]}
        self._workload_pvc_map = {}
        # 根据accessMode对pvc分类，value pvc name
        self._many_pvc = []

        self._backup_pod_mgr = BackupPodManager()
        self._workload_mgr = WorkloadManager()

        # pvc对应状态  {pvc_name: PvcInfo()}
        self._pvc_status_dict = {}
        self._task_timeout = job_info.task_timeout

    @staticmethod
    def _get_pod_name(pvc):
        time.sleep(1)
        time_n = round(time.time())
        return f"dpa-pvc-data-move-pod-{time_n}"

    def start(self):
        self._init_values()
        self._init_pvc_status()
        try:
            if self._job_info.action_type == ActionType.RESTORE:
                self._init_workloads()
            self._parameter_verification()
            node_dict = BackupPodManager.get_node_dict(self._job_info)
            on_use_logical_ip_list = BackupPodManager.get_logical_ip(self._job_info, node_dict)
            if not on_use_logical_ip_list:
                raise ErrCodeException(ErrorCode.NO_LOGICAL_PORTS_ARE_AVAILABLE, message='No useful logic ip')
            self._job_info.data_repo.logical_ip_list = self._job_info.meta_repo.logical_ip_list = \
                self._job_info.cache_repo.logical_ip_list = on_use_logical_ip_list

            while not self._check_pvc_all_completed():
                self._check_pvc_and_node_status()
                create_pod_list = []
                create_pod_list = self._add_many_pvc(create_pod_list)
                log.info(f'Create_pod_list:{create_pod_list}, task id:{self._job_info.task_id}')
                res = BackupPodManager.create_backup_pods(self._job_info, create_pod_list)
                log.info(f'Create pod! Pod result:{res}. Task id:{self._job_info.task_id}')
                self._report_process()
                time.sleep(10)
        finally:
            self._post_action()
        return True

    def _check_node_status_and_move_pod(self):
        # 检查节点状态
        self._check_node_status()
        self._move_pod()

    def _check_all_node_of_node_selector_is_faults(self, node_selector):
        if not node_selector:
            return
        for node_status in self._node_status.values():
            if node_status.status:
                return
        raise Exception(f"Node is faults, which is selected by node selector {self._job_info.node_selector}")

    def _move_pod(self):
        # 删故障节点上的pod
        for node_name, node_status in self._node_status.items():
            if not node_status.status:
                pod_label = f'{LabelConst.DPA_BACKUP_POD_NODE_KEY}={node_name}'
                pod_items = self._pod_api.list(self._job_info.resource.namespace.name, label_selector=pod_label).items
                for pod in pod_items:
                    self._pod_api.delete(pod.metadata.name, self._job_info.resource.namespace.name,
                                         grace_period_seconds=0)
                log.info(f"Delete pod success, node name is {node_name}, node status is {node_status.status}, "
                         f"pod num is {len(pod_items)} ,namespace is {self._job_info.resource.namespace.name}, "
                         f"task id {self._job_info.task_id}")
        # 更新pvc的状态,将故障节点上的pvc初始化为idle
        pvc_name_list = []
        for pvc_name, pvc_info in self._pvc_status_dict.items():
            if self._node_status.get(pvc_info.node_name) and not self._node_status.get(pvc_info.node_name).status:
                self._init_pvc(pvc_name)
                pvc_name_list.append(pvc_name)

        log.info(f"Init pvc success, pvc_name_list is {pvc_name_list}, _pvc_status_dict is {self._pvc_status_dict}"
                 f" node_status is {self._node_status}, task is {self._job_info.task_id}")

    def _init_pvc(self, pvc):
        self._pvc_status_dict.get(pvc).pvc_status = PvcStatus.IDLE
        self._pvc_status_dict.get(pvc).running_time = 0

    def _check_node_status(self):
        cluster_authentication = self._job_info.resource.cluster_authentication
        rc_api = ResourceCoreApi(cluster_authentication)
        node_selector = "" if self._job_info.node_selector is None else self._job_info.node_selector
        node_res = rc_api.list_cluster_resource_with_http_info('nodes', label_selector=node_selector)
        log.info(f"Start check node status, node_selector is {node_selector},"
                 f" task id is {self._job_info.task_id}")
        if not node_res['items']:
            log.error(f"Node num is {len(node_res['items'])}, task id {self._job_info.task_id}")
        for node in node_res['items']:
            if not self._node_status.get(node['metadata']['name']):
                continue
            for condition in node['status']['conditions']:
                if (condition['type']) == 'Ready' and 'True' == (condition['status']):
                    self._node_status.get(node['metadata']['name']).status = True
                    break
            else:
                self._node_status.get(node['metadata']['name']).status = False
        self._check_all_node_of_node_selector_is_faults(node_selector)
        log.info(f"Node status is {self._node_status}, task id {self._job_info.task_id}")

    def _generate_labels(self, pvc_name, node_name):
        if self._job_info.action_type.value == ActionType.BACKUP.value:
            action = LabelConst.DPA_BACKUP_POD_ACTION_BACKUP
        else:
            action = LabelConst.DPA_BACKUP_POD_ACTION_RESTORE
        return {
            LabelConst.DPA_BACKUP_POD_COMMON_KEY: LabelConst.DPA_BACKUP_POD_COMMON_VALUE,
            LabelConst.DPA_BACKUP_POD_PVC_KEY: pvc_name,
            LabelConst.DPA_BACKUP_POD_NODE_KEY: node_name,
            LabelConst.DPA_BACKUP_POD_TYPE_KEY: LabelConst.DPA_BACKUP_POD_TYPE_DATA_MOVE,
            LabelConst.DPA_BACKUP_POD_TASK_KEY: self._job_info.task_id,
            LabelConst.DPA_BACKUP_POD_ACTION_KEY: action
        }

    def _init_values(self):
        self._update_node_status()
        self._query_pvc_node_map()
        self._categorize_pvc(self._pvc_dict)
        self._remove_unrelated_workload()

    def _init_pvc_status(self):
        for pvc in self._many_pvc:
            self._pvc_status_dict.update({pvc: PvcInfo(pvc_status=PvcStatus.IDLE.value,
                                                       start_times=0,
                                                       running_time=0)})

    def _check_pvc_all_completed(self):
        for _, pvc_info in self._pvc_status_dict.items():
            if pvc_info.pvc_status != PvcStatus.SUCCEEDED.value:
                return False
        return True

    def _check_pvc_completed_or_running(self, pvc_name):
        log.info(f'Pvc:{pvc_name} no need to create')
        return self._pvc_status_dict.get(pvc_name).pvc_status == PvcStatus.SUCCEEDED.value or \
            self._pvc_status_dict.get(pvc_name).pvc_status == PvcStatus.RUNNING.value

    def _check_pvc_and_node_status(self):
        # 根据label查询出所有的备份pod
        pvc_pod_status_dict = {}
        pod_label = f'{LabelConst.DPA_BACKUP_POD_COMMON_KEY}={LabelConst.DPA_BACKUP_POD_COMMON_VALUE},' \
                    f'{LabelConst.DPA_BACKUP_POD_TYPE_KEY}={LabelConst.DPA_BACKUP_POD_TYPE_DATA_MOVE}'
        pod_items = self._pod_api.list(self._job_info.resource.namespace.name, label_selector=pod_label).items
        for item in pod_items:
            pvc_name = item.metadata.labels.get(LabelConst.DPA_BACKUP_POD_PVC_KEY)
            node_name = item.metadata.labels.get(LabelConst.DPA_BACKUP_POD_NODE_KEY)
            log.info(f'Item info is: pvc name {pvc_name}, node name {node_name}')
            pod_status = PodStatus(phase=item.status.phase, pvc_name=pvc_name, node_name=node_name,
                                   pod_name=item.metadata.name)
            pvc_pod_status_dict.update({pvc_name: pod_status})
        log.info(f'====Pvc_pod_status_dict{pvc_pod_status_dict}, task id:{self._job_info.task_id}')
        # 一、检查pvc是否完成, 更新_pvc_status_dict
        for pvc in self._pvc_status_dict.keys():
            if pvc not in pvc_pod_status_dict.keys():
                log.warning(f'Pod loss! Pvc name:{pvc}, task id:{self._job_info.task_id}')
                self._update_pvc_status(pvc, PvcStatus.IDLE.value)
            else:
                pod_status_obj = pvc_pod_status_dict[pvc]
                if pod_status_obj.phase == PodPhase.SUCCEEDED.value:
                    self._update_pvc_status(pvc, PvcStatus.SUCCEEDED.value)
                    self._pod_api.delete(pod_status_obj.pod_name, self._job_info.resource.namespace.name)
                    log.info(f'Pvc finished! Pvc name:{pvc}, task id:{self._job_info.task_id}')
                elif pod_status_obj.phase == PodPhase.RUNNING.value:
                    self._update_pvc_status(pvc, PvcStatus.RUNNING.value)
                elif pod_status_obj.phase == PodPhase.PENDING.value:
                    self._check_pending_pod_pvc(pod_status_obj, pvc)
                    self._update_pvc_status(pvc, PvcStatus.RUNNING.value)
                else:
                    self._update_pvc_status(pvc, PvcStatus.IDLE.value)
                    self._pod_api.delete(pod_status_obj.pod_name, self._job_info.resource.namespace.name)

        # 二、 更新node状态， 更新_node_status
        node_temp_dict = {}
        for node_name in self._node_status.keys():
            node_temp_dict.update({node_name: 0})
        for pvc in pvc_pod_status_dict.keys():
            if pvc_pod_status_dict.get(pvc).phase == PvcStatus.RUNNING.value or \
                    pvc_pod_status_dict.get(pvc).phase == PodPhase.PENDING.value:
                if not node_temp_dict.get(pvc_pod_status_dict.get(pvc).node_name):
                    node_temp_dict.update({pvc_pod_status_dict.get(pvc).node_name: 1})
                else:
                    node_temp_dict[pvc_pod_status_dict.get(pvc).node_name] += 1
        for node_name in node_temp_dict:
            self._node_status.get(node_name).pod_nums = node_temp_dict.get(node_name)
        log.info(f'Node status:{self._node_status}, node_temp_dict is {node_temp_dict}, '
                 f'pvc_pod_status_dict is {pvc_pod_status_dict}, task id:{self._job_info.task_id}')
        # 根据查询出来的所有备份pod，汇总node
        # 实时检查node的状态
        self._check_node_status_and_move_pod()
        return

    def _update_pvc_status(self, pvc, status):
        if self._pvc_status_dict.get(pvc).pvc_status != PvcStatus.SUCCEEDED.value:
            self._pvc_status_dict.get(pvc).pvc_status = status
            self._pvc_status_dict.get(pvc).running_time += 10
            return True
        return False

    def _add_many_pvc(self, create_pod_list):
        for pvc in self._many_pvc:
            if self._check_pvc_completed_or_running(pvc):
                continue
            for node, status in self._node_status.items():
                if status.status and status.pod_nums < self._pod_nums:
                    pod_name = self._get_pod_name(pvc)
                    command = self._generate_command([pvc])
                    create_pod_info = CreatePodInfo(node_name=node, pvc_name_list=[pvc],
                                                    pod_name=pod_name, command=command,
                                                    pod_type=BackupPodType.BACKUP_PVC_TYPE,
                                                    labels=self._generate_labels(pvc, node))
                    create_pod_list.append(create_pod_info)
                    self._pod.append(pod_name)
                    self._node_status.get(node).pod_nums += 1
                    self._node_status.get(node).total_nums += 1
                    self._add_and_check_pvc_start_times(pvc, node)
                    break
        return create_pod_list

    def _add_and_check_pvc_start_times(self, pvc, node):
        log.info(f'Pvc status dict:{self._pvc_status_dict}')
        if self._pvc_status_dict.get(pvc):
            self._pvc_status_dict.get(pvc).start_times += 1
            self._pvc_status_dict.get(pvc).node_name = node
            log.info(f"pvc: {pvc} start_times is {self._pvc_status_dict.get(pvc).start_times}")
        if self._pvc_status_dict.get(pvc).start_times > 5:
            log.error(f'pvc: {pvc} start_times exceed limit.')
            raise K8SProtectError(ErrorCode.OPERATION_FAILED, "pvc start_times exceed limit.")

    def _report_process(self):
        report_job_details(self._job_info.request_id,
                           SubJobDetails(taskId=self._job_info.task_id,
                                         subTaskId=self._job_info.sub_id,
                                         progress=0,
                                         logDetail=[],
                                         taskStatus=SubJobStatusEnum.RUNNING.value))

    def _post_action(self):
        log.info(f'Backup Pod Action Start! Task id:{self._job_info.task_id}')
        label_selector = LabelConst.DPA_BACKUP_POD_COMMON_KEY + "=" + LabelConst.DPA_BACKUP_POD_COMMON_VALUE
        pod_list = []
        for pod in self._pod_api.list(self._job_info.resource.namespace.name,
                                      label_selector=label_selector).items:
            pod_list.append(pod.metadata.name)
        BackupPodManager.del_backup_pods(self._job_info.resource.cluster_authentication,
                                         self._job_info.resource.namespace.name, pod_list)
        if self._job_info.action_type == ActionType.RESTORE:
            patch_fail_workload_list = self._workload_mgr.restore_replicas(self._job_info, self._workload_pvc_map)
            # 增加工作负载恢复部分伸缩成功事件
        log.info(f'Backup Pod Action Success! Task id:{self._job_info.task_id}')

    def _generate_command(self, pvc_list):
        command = ["/bin/sh", "-c"]
        if self._job_info.action_type == ActionType.BACKUP:
            action_param = f'-t {self._job_info.job_type} '
        elif self._job_info.action_type == ActionType.RESTORE:
            action_param = f'-c {self._job_info.job_type} '
            if self._job_info.job_type == RestoreCommandParam.OVERWRITE.value:
                action_param = action_param + ' -r'
        else:
            log.error(f'Wrong action type! Task id:{self._job_info.task_id}')
            raise ErrCodeException(err_code=ErrorCode.PARAM_FAILED)
        param = ''
        for pvc in pvc_list:
            # backup : origin  restore: origin(副本里面的)
            if pvc not in self._pvc_dict:
                log.error(f'Abnormal pvc name:{pvc}, task id:{self._job_info.task_id}')
                raise ErrCodeException(err_code=ErrorCode.PARAM_FAILED)
            target_pvc = self._pvc_dict.get(pvc)
            source_pvc = pvc
            target_path_cmd = f"{NFS_DATA_PATH}pvc/{target_pvc} "
            source_path_cmd = f"{TARGET_PREFIX}{source_pvc} "
            cmd = f'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64;cd /opt/podBackup/;./PodBackupApp ' \
                  f'-T {self._job_info.action_type.value} ' \
                  f'-p {target_pvc} ' \
                  f'-s {source_path_cmd} ' \
                  f'-d {target_path_cmd} ' \
                  f'{action_param}'
            if self._job_info.action_type == ActionType.BACKUP:
                meta_file_dir = f"{NFS_DATA_PATH}meta-file/{self._job_info.copy_id}/root_file_{target_pvc}"
                log.info("Begin to backup power")
                cmd += f";ls -ld {source_path_cmd} > {meta_file_dir}"
            if self._job_info.action_type == ActionType.RESTORE:
                log.info("Begin to get power")
                power_id = self._get_power_from_file(target_pvc)
                cmd += f';chmod {power_id} {source_path_cmd}'
            if param:
                param += ';'
            param += cmd
        command.append(param)
        log.info(f"Path cmd is {command}")
        log.info(f"Pod command is {command}, pvc list is {pvc_list}")
        return command

    def _get_power_from_file(self, target_pvc):
        log.info("Begin to get local power file")
        data_repo_path = self._job_info.data_repo.local_path
        metadata_store_path = f"{data_repo_path}/meta-file/{self._job_info.copy_id}"
        log.info(f"local path is {metadata_store_path}")
        file_path = f"{metadata_store_path}/root_file_{target_pvc}"
        with open(file_path, "r") as f:
            text = f.read()
        parts = text.split()
        permission_str = parts[0]
        permission_dict = {'r': 4, 'w': 2, 'x': 1, '-': 0}
        permission_part = permission_str[1:]
        result = 0
        try:
            for i in range(3):
                permission_number = sum([permission_dict[char] for char in permission_part[i * 3:i * 3 + 3]])
                result += permission_number * (10 ** (2 - i))
        except KeyError as err:
            raise ErrCodeException from err
        return result

    def _query_pvc_node_map(self):
        pod_list = self._pod_api.list(self._job_info.resource.namespace.name).items
        for pod in pod_list:
            node = pod.spec.node_name
            for volume in pod.spec.volumes:
                if volume.persistent_volume_claim:
                    self._pvc_node_map.update({volume.persistent_volume_claim.claim_name: node})
                else:
                    continue
                if pod.metadata.owner_references:
                    self._get_workload_info_to_map(pod.metadata.owner_references,
                                                   volume.persistent_volume_claim.claim_name)

    def _categorize_pvc(self, pvc_dict: dict):
        for pvc in pvc_dict.keys():
            res = self._pvc_api.read(pvc, self._job_info.resource.namespace.name)
            if 'ReadOnlyMany' in res.spec.access_modes and self._job_info.action_type == ActionType.RESTORE.value:
                continue
            if pvc not in self._many_pvc:
                self._many_pvc.append(pvc)

    def _update_node_status(self):
        node_dict = BackupPodManager.get_node_dict(self._job_info)
        if not node_dict:
            raise ErrCodeException(err_code=ErrorCode.OPERATION_FAILED)
        for node_name, _ in node_dict.items():
            self._node_status.update({node_name: NodeStatus(
                uid=node_name.split('-')[-1], pod_nums=0, total_nums=0, prefer_pvc=[], status=True)})
        return

    def _check_pending_pod_pvc(self, pod_status_obj, pvc):
        log.info(f'Checking pvc {pod_status_obj}, task id:{self._job_info.task_id}')
        self._check_running_timeout(pvc)
        if self._job_info.action_type == ActionType.RESTORE.value:
            try:
                field_selector = f'involvedObject.name={pod_status_obj.pod_name}'
                event_manager = EventManager(cluster_authentication=self._job_info.resource.cluster_authentication,
                                             namespace=self._job_info.resource.namespace.name,
                                             task_id=self._job_info.task_id)
                event_info = event_manager.list_namespace_event_by_label(field_selector)
                self._check_recovery_pod_event(event_info, pod_status_obj)
                return True
            except ErrCodeException as err:
                raise ErrCodeException from err
            except Exception as err:
                log.error(f'Pvc status error!{err} Task id:{self._job_info.task_id}')
                raise Exception from err
        return True

    def _check_recovery_pod_event(self, event_info, pod_status_obj):
        for event in event_info:
            if Event.PVC_IS_DELETED in event['message']:
                log.error(f'PVC {pod_status_obj.pvc_name} is being deleted! Task id:{self._job_info.task_id}')
                raise ErrCodeException(err_code=ErrorCode.OPERATION_FAILED)

    def _check_running_timeout(self, pvc):
        if self._pvc_status_dict.get(pvc).running_time >= self._task_timeout:
            log.error(f'Pvc {pvc} pending timeout! Task id:{self._job_info.task_id}')
            raise ErrCodeException(err_code=ErrorCode.OPERATION_FAILED)
        return True

    def _get_workload_info_to_map(self, owner_references, claim_name):
        for owner in owner_references:
            if owner.controller:
                self._fill_workload_info(owner, claim_name)

    def _get_deployment_config_info_to_map(self, owner_references: list, claim_name):
        for owner in owner_references:
            if owner.get('controller', False):
                self._fill_deployment_config_info(owner, claim_name)

    def _fill_deployment_config_info(self, owner: dict, claim_name):
        name = owner.get('name', '')
        if name in self._workload_pvc_map:
            self._workload_pvc_map[name].pvc.append(claim_name)
        else:
            log.info(f'Add workload {name} to map, {owner} task id:{self._job_info.task_id}')
            workload_info = WorkloadInfo(pvc=[claim_name], kind=owner.get('kind', ''), name=name,
                                         api_version=owner.get('apiVersion', ''))
            self._workload_pvc_map.update({name: workload_info})

    def _fill_workload_info(self, owner, pvc_name):
        namespace = self._job_info.resource.namespace.name
        if owner.kind == 'ReplicaSet':
            field_selector = f'metadata.name={owner.name}'
            replica_set = self._replica_set_api.list(namespace, field_selector=field_selector).items[0]
            if replica_set.metadata.owner_references:
                self._get_workload_info_to_map(replica_set.metadata.owner_references, pvc_name)
                return
        if owner.kind == 'ReplicationController':
            field_selector = f'metadata.name={owner.name}'
            rc = self._core_api.list_namespaced_resource_with_http_info(plural='replicationcontrollers',
                                                                        namespace=namespace,
                                                                        field_selector=field_selector)['items'][0]
            if rc.get('metadata', {}).get('ownerReferences', {}):
                self._get_deployment_config_info_to_map(rc.get('metadata', {}).get('ownerReferences', {}), pvc_name)
                return
        if owner.kind == 'Job':
            field_selector = f'metadata.name={owner.name}'
            job = self._job_api.list(namespace,
                                     field_selector=field_selector).items[0]
            if job.metadata.owner_references:
                self._get_workload_info_to_map(job.metadata.owner_references, pvc_name)
        if owner.name in self._workload_pvc_map:
            self._workload_pvc_map[owner.name].pvc.append(pvc_name)
        else:
            log.info(f'Add workload {owner.name} to map, {owner.to_dict()} task id:{self._job_info.task_id}')
            workload_info = WorkloadInfo(pvc=[pvc_name], kind=owner.kind, name=owner.name,
                                         api_version=owner.api_version)
            self._workload_pvc_map.update({owner.name: workload_info})

    def _remove_unrelated_workload(self):
        for workload in list(self._workload_pvc_map.keys()):
            flag = False
            for pvc in self._workload_pvc_map.get(workload).pvc:
                if pvc in self._many_pvc:
                    flag = True
                    break
            if not flag:
                log.info(f'Remove workload {workload}, task id:{self._job_info.task_id}')
                self._workload_pvc_map.pop(workload)

    def _init_workloads(self):
        log.info(f'Start init workloads info and shut down replicas, task id:{self._job_info.task_id}')
        self._workload_mgr.update_workload_info(self._job_info, self._workload_pvc_map)
        log.info(f'Start shut down replicas, task id:{self._job_info.task_id}')
        patch_fail_workload_list = self._workload_mgr.shut_down_replicas(self._job_info, self._workload_pvc_map)
        if patch_fail_workload_list:
            # 需要申请错误码
            raise ErrCodeException(ErrorCode.OPERATION_FAILED, *patch_fail_workload_list)

    def _parameter_verification(self):
        to_verified_params = self._init_params_need_verify()
        for param in to_verified_params:
            if not validate_cmd_strs(param):
                raise ErrCodeException(err_code=ErrorCode.PARAM_FAILED)
        log.info(f'Param verification succeeded! Task id:{self._job_info.task_id}')

    def _init_params_need_verify(self):
        to_verified_params = [self._job_info.job_type, self._job_info.action_type.value]
        for pvc_key, pvc_value in self._pvc_dict.items():
            to_verified_params.append(pvc_key)
            to_verified_params.append(pvc_value)
        log.debug(f'To verified params:{to_verified_params}')
        return to_verified_params
