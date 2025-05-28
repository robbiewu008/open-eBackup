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

import shlex
from typing import List


from common.common_models import Copy, CopyInfoRepModel, ReportCopyInfoModel, LogDetail, SubJobDetails
from common.common import report_job_details
from common.const import RpcToolInterface
from common.exception.common_exception import ErrCodeException
from k8s.common import const
from k8s.common.error_code import ErrorCode
from k8s.common.k8s_manager.backup_pod_manager import BackupPodManager
from k8s.common.utils import prepare_kubernetes_meta_data_store_path
from k8s.common.k8s_manager.pod_white_handler import PodWhiteListHandler
from k8s.common.const import (PREFIX, SNAPSHOT_GROUP, HTTP_REQUEST_TIMEOUT, ConsistencyRulesWithAuthentication,
                              PVC_PREFIX_LENGTH, ConsistencyRuleType, SnapShotInfo, NFS_DATA_PATH, DATA_REPO,
                              META_REPO, CACHE_REPO, JobInfo)
from k8s.common.csi_manager import CSIManager
from k8s.common.k8s_manager.pvc_manager import PvcManager
from k8s.common.kubernetes_client.k8s_api import InitKubernetesApiError
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_snapshot_pvc_api import \
    SnapShotPVC
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import \
    ResourcePod
from k8s.common.kubernetes_client.struct import (SubRule, Namespace, Resource, BackTaskStatus)
from k8s.common.label_const import LabelConst
from k8s.common.utils import save_resource_to_yaml, exec_rc_tool_cmd
from k8s.logger import log
from k8s.services.backup.consistent_backup import ConsistentBackup
from k8s.services.backup.k8s_metadata_backup_handle import \
    K8sMetadataBackupHandle
from k8s.services.basic_job import BasicJob
from k8s.services.k8s_dispatcher import K8sDispatcher
from k8s.common.kubernetes_client.struct import ReportParam
from k8s.common.report.report_manager import ReportManger


LIMIT_MI_PVC_SIZE = 1024

PVC_SIZE_UNIT = 'Mi'


class BackupTask(BasicJob):
    def __init__(self, kubernetes_back_info: JobInfo, path_dict=None, ip_dict=None):
        super().__init__(kubernetes_back_info.task_id, kubernetes_back_info.resource.namespace.name,
                         kubernetes_back_info.image_name, kubernetes_back_info)
        self.data_size = None
        self._pvc_num_per_pod = 1
        self.pvc_list = set()
        self.pvc_name_and_size = {}
        self.kubernetes_back_info = kubernetes_back_info
        self.snapshot_infos = []
        self.path_dict = path_dict
        self.ip_dict = ip_dict
        self.repo_dict = {
            DATA_REPO: self.kubernetes_back_info.data_repo,
            META_REPO: self.kubernetes_back_info.meta_repo,
            CACHE_REPO: self.kubernetes_back_info.cache_repo
        }
        self.report_info = LogDetail()
        return

    @staticmethod
    def _execute_cmd_pod_with_container(k8s_pod_api: ResourcePod, pod, namespace: str, command: List[str],
                                        container_name: str):
        if not container_name:
            try:
                container_name = pod.spec.containers[0].name
            except Exception as e:
                log.exception(f'No valid container to execute cmd du to {e}.')
                return False
        log.info(
            f'Execute command {command} in namespace:{namespace} pod_name:{pod.metadata.name} '
            f'container:{container_name}.'
        )
        resp = k8s_pod_api.exec(
            pod.metadata.name, namespace, container=container_name, command=command, stderr=True,
            stdin=False, stdout=True, _preload_content=False, tty=False, _request_timeout=HTTP_REQUEST_TIMEOUT
        )
        time_out = 600
        while resp.is_open():
            resp.update(timeout=5)
            if resp.peek_stdout():
                log.warning(f"STDOUT: \n{resp.read_stdout()}")
            if resp.peek_stderr():
                log.error(f"STDERR: \n{resp.read_stderr()}, command:{command}")
                return False
            time_out -= 5
            if time_out == 0:
                log.error(f"TIME_OUT: \n{resp.read_stderr()}")
                return False
        log.debug(f'Execute cmd:{command} success!.')
        return True

    @staticmethod
    def _check_pvc_storage_size_and_return_new_size(storage_size):
        log.info(f"Start check pvc storage size:{storage_size}")
        if PVC_SIZE_UNIT in storage_size and LIMIT_MI_PVC_SIZE > int(storage_size[0:len(storage_size) - 2]):
            return str(LIMIT_MI_PVC_SIZE) + PVC_SIZE_UNIT
        return storage_size

    def prerequisite(self):
        log.info(f"WhiteList Pod start, task id:{self.task_id}")
        PodWhiteListHandler.create_temp_pod_and_add_ip_white_list(self.kubernetes_back_info)
        enable_consistent = self.kubernetes_back_info.advanced_params.get("is_consistent", "false")
        consistent_backup = None
        try:
            if enable_consistent.lower() == "true":
                log.info("execute pre consistent command.")
                consistent_backup = ConsistentBackup(self.kubernetes_back_info)
                if not consistent_backup.exec_consistent_rule(True):
                    log.error('Exec consistent script fail.')
                    return False
            if not self._create_snapshot_for_dataset(self.kubernetes_back_info.resource):
                log.error('Create snapshot fail can not create backup job')
                return False
            flag, pvc_name_list = self._create_pvc_for_snapshot(self.kubernetes_back_info.resource)
            if not flag:
                log.error('Create pvc for snapshot failed.')
                return False
            return True
        except Exception as err:
            log.error(f'=======Error!:{err}')
            log.exception(err)
            return False
        finally:
            if enable_consistent.lower() == "true":
                log.info("execute post consistent command.")
                consistent_backup.exec_consistent_rule(False)

    def backup_metadata(self, data_repo_path):
        if not self._backup_metadata_of_dataset(data_repo_path):
            log.error("Backup metadata failed")
            return False
        return True

    def backup_data(self):
        log.info("Start _backup_dataset")
        snap_shot_dict = self._get_snap_pvc_path()
        if not snap_shot_dict:
            node_dict = BackupPodManager.get_node_dict(self._job_info)
            on_use_logical_ip_list = BackupPodManager.get_logical_ip(self._job_info, node_dict)
            if not on_use_logical_ip_list:
                raise ErrCodeException(ErrorCode.NO_LOGICAL_PORTS_ARE_AVAILABLE, message='No useful logic ip')
            log.warning(f"No need to backup PVC, task id+{self.kubernetes_back_info.task_id}")
            return True
        log.warning(f"Back pod nums is:{self.kubernetes_back_info.pod_nums}")
        report_param = ReportParam(cache_path=self.kubernetes_back_info.cache_repo.local_path,
                                   pvc_name_list=list(snap_shot_dict.values()),
                                   req_id=self.kubernetes_back_info.request_id,
                                   job_id=self.kubernetes_back_info.task_id,
                                   sub_job_id=self.kubernetes_back_info.sub_id)
        report = ReportManger(report_param)
        status = BackTaskStatus(pod_num=0, total_pod_num=0, snap_shot_dict=snap_shot_dict, all_pvc_list=[])
        try:
            log.info(f"1===={self.kubernetes_back_info}")
            log.info(f"2===={snap_shot_dict}")
            k8s_main_action = K8sDispatcher(self.kubernetes_back_info, snap_shot_dict)
            k8s_main_action.start()
            log.info(f'AAction Backup succeeded!task id:{self.kubernetes_back_info.task_id}.')
            data_size, progress = report.statistics_of_backup_data()
            self.data_size = data_size
            self._report_copy_info()
            return True
        except ErrCodeException as err:
            self.report_info.log_detail = err.error_code
            self.report_info.log_detail_param = err.parameter_list
            return False
        except Exception as err:
            log.exception(f'Action Backup failed! task id:{self.kubernetes_back_info.task_id}.Err:{err}')
            return False

    def clean_task(self, cache_path):
        log.info(f'Start clean task! Task id:{self.kubernetes_back_info.task_id}')
        if not self._move_and_clean_cache(cache_path):
            log.error(f"Move_and_clean_cache failed! Task id:{self.kubernetes_back_info.task_id}")
            return False
        log.info(f'Move_and_clean_cache succeed! Task id:{self.kubernetes_back_info.task_id}')
        if not self._clean_pod():
            log.error(f"Clean_pod failed! Task id:{self.kubernetes_back_info.task_id}")
            return False
        if not self._clean_pvc():
            log.error(f"Clean_pvc failed! Task id:{self.kubernetes_back_info.task_id}")
            return False
        log.info(f'Clean_pod_and_pvc succeed! Task id:{self.kubernetes_back_info.task_id}')
        return True

    def _get_sts_list(self, sts_info, sts_name, pvc_num):
        pvc_name_list = []
        pvc_name = sts_info.metadata.name
        if pvc_name:
            if pvc_num == 1:
                if self._check_sts_pvc(pvc_name + "-" + sts_name):
                    pvc_name_list.append(pvc_name + "-" + sts_name)
                elif self._check_sts_pvc(pvc_name + "-" + sts_name + "-0"):
                    pvc_name_list.append(pvc_name + "-" + sts_name + "-0")
            else:
                sts_list = [(pvc_name + "-" + sts_name + "-" + str(i)) for i in range(pvc_num)]
                pvc_name_list.extend(sts_list)
        return pvc_name_list

    def _check_sts_pvc(self, pvc_name):
        pvc_api = ResourcePersistentVolumeClaim(self.kubernetes_back_info.resource.cluster_authentication)
        try:
            res = pvc_api.read(pvc_name, self.kubernetes_back_info.resource.namespace.name)
            log.debug(f"Get pvc {res}")
            return True
        except Exception as e:
            log.exception(f'Check PVC error! error: {e}.')
            return False

    def _get_snap_shot_path(self, pod):
        path_dict = dict()
        for container in pod.spec.containers:
            for mount in container.volume_mounts:
                if PREFIX.SNAPSHOT_PVC_NAME_PREFIX in mount.name:
                    path_dict.update({mount.name: mount.mount_path})
        log.info(f"Get snap shot path succeed! Task_id:{self.kubernetes_back_info.task_id}")
        return path_dict if path_dict else {}

    def _get_snap_pvc_path(self):
        path_dict = dict()
        pvc_api = ResourcePersistentVolumeClaim(self.kubernetes_back_info.resource.cluster_authentication)
        pvc_items = pvc_api.list(self.kubernetes_back_info.resource.namespace.name,
                                 label_selector=f'{LabelConst.DPA_BACKUP_POD_TASK_KEY}={self.task_id}').items
        for pvc in pvc_items:
            path_dict.update({pvc.metadata.name: pvc.metadata.labels.get(LabelConst.DPA_BACKUP_POD_PVC_KEY)})
            self.pvc_name_and_size.update({pvc.metadata.labels.get(LabelConst.DPA_BACKUP_POD_PVC_KEY)
                                           : pvc.metadata.labels.get(LabelConst.DPA_BACKUP_POD_PVC_SIZE_KEY)})
        log.info(f'Get path_dict: {path_dict}, pvc_name_and_size: {self.pvc_name_and_size}'
                 f', Task_id:{self.kubernetes_back_info.task_id}')
        return path_dict if path_dict else {}

    def _get_backup_command(self, pvc_name, snap_shot_path: str):
        back_type = str(self.kubernetes_back_info.resource.backup_type)
        pvc_path_cmd = ""
        nfs_path_cmd = f" -d {NFS_DATA_PATH}pvc/{pvc_name}"
        pvc_path_cmd += f"-s {snap_shot_path} "
        log.info(f"Path cmd is {pvc_path_cmd}")

        cmd = f"export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib64;cd /opt/podBackup/;./PodBackupApp -p -T 1 " \
              f"{snap_shot_path[PVC_PREFIX_LENGTH:]} -t {back_type} " + pvc_path_cmd + nfs_path_cmd
        command = ["/bin/sh", "-c", format(shlex.quote(cmd))]
        log.info(f"Backup command is {command}")
        return command

    def _backup_metadata_of_dataset(self, data_repo_path):
        metadata_backup_handler = K8sMetadataBackupHandle(
            self.kubernetes_back_info
        )
        metadata_backup_handler.backup_metadata()
        metadata_store_path = f"{data_repo_path}/meta-file/{self.kubernetes_back_info.copy_id}"
        if not prepare_kubernetes_meta_data_store_path(metadata_store_path):  # 保证一定存在对应仓库
            log.error(f"Dont find metadata_store_path {metadata_store_path}")
            return False
        log.info(f"Meta store path {metadata_store_path}")
        resource_list = metadata_backup_handler.get_all_resources()
        for resource in resource_list:
            if not resource:
                continue
            log.info(f'Start backup resource {resource.kind} metadata!')
            if not save_resource_to_yaml(metadata_store_path, resource):
                log.error(f"Backup metadata failed, kind:{resource.kind}")
                continue
            log.info(f"SUCCESS! Backup metadate for {resource.kind}")
        return True

    def _get_pvc_list(self, resource: Resource):
        log.info("Get pvc list by labels.")
        return self._get_pvc_by_labels(resource)

    def _get_pvc_by_labels(self, resource: Resource):
        try:
            pvc_manager = PvcManager(cluster_authentication=resource.cluster_authentication,
                                     namespace=resource.namespace.name, labels=resource.dataset.labels,
                                     task_id=self.kubernetes_back_info.task_id)
            self.pvc_list.update(
                [
                    pvc.get("metadata").get("name") for pvc in pvc_manager.get_dataset_by_label_and_group_by_type()
                ]
            )
        except Exception as err:
            log.exception(f"Get pvc failed, error is {err}")
            return False
        return True

    def _create_snapshot_for_dataset(self, resource: Resource):
        if self._get_pvc_list(resource) and self._create_snapshot_for_pvcs(resource):
            return True
        log.error('Create snapshot failed.')
        return False

    def _create_snapshot_for_pvcs(self, resource: Resource):
        log.info(f"PVC list:{self.pvc_list}")
        if not self.pvc_list:
            log.info('No pvcs need to create snapshot.')
            return True
        try:
            csi_manager = CSIManager(resource, self.kubernetes_back_info, self.pvc_list)
        except InitKubernetesApiError:
            log.error('Init K8S client failed.')
            return False
        ret, snapshot_infos = csi_manager.create_pvcs_snapshot()
        if not ret:
            log.error('Create pvc snapshot failed.')
            return False
        self.snapshot_infos = snapshot_infos
        return True

    def _create_pvc_for_snapshot(self, resource: Resource):
        log.info('Create pvc for snapshot.')
        pvc_snapshot_list = list()
        if not self.snapshot_infos:
            log.warning('Create pvc for snapshot failed snapshot name is empty')
            return True, pvc_snapshot_list
        for item in self.snapshot_infos:
            if not isinstance(item, SnapShotInfo):
                log.error(f'Create pvc for snapshot failed {item} type is not SnapShotInfo')
                continue
            try:
                pvc_annotations = self._get_pvc_annotations(item.source_name)
                pvc = SnapShotPVC(resource.cluster_authentication)
                pvc.set_pvc_name(f'{PREFIX.SNAPSHOT_PVC_NAME_PREFIX}-{self.kubernetes_back_info.task_id[-5:]}-'
                                 f'{item.source_uid}')
                log.warning(f'Pvc_name {PREFIX.SNAPSHOT_PVC_NAME_PREFIX}-{self.kubernetes_back_info.task_id[-5:]}'
                            f'-{item.source_name}')
                pvc.set_api_group(SNAPSHOT_GROUP)
                pvc.set_namespace(resource.namespace.name)
                pvc.set_snapshot_name(item.name)
                storage_size = self._check_pvc_storage_size_and_return_new_size(item.size)
                pvc.set_storage_size(storage_size)
                pvc.set_access_mode(item.access_mode)
                pvc.set_storage_class_name(item.storage_class_name)
                pvc.set_labels({LabelConst.DPA_BACKUP_POD_COMMON_KEY: LabelConst.DPA_BACKUP_POD_COMMON_VALUE})
                pvc.set_labels({LabelConst.DPA_BACKUP_POD_PVC_KEY: item.source_name})
                pvc.set_labels({LabelConst.DPA_BACKUP_POD_PVC_SIZE_KEY: item.size})
                pvc.set_labels({LabelConst.DPA_BACKUP_POD_TASK_KEY: self.kubernetes_back_info.task_id})
                log.info(f"Get pvc annotations:{pvc_annotations} success.")
                if pvc_annotations and 'everest.io/disk-volume-type' in pvc_annotations:
                    pvc.set_annotations({'everest.io/disk-volume-type': pvc_annotations['everest.io/disk-volume-type']})
                pvc.create_snapshot_pvc()
                pvc_snapshot_list.append(pvc)
            except Exception as e:
                log.exception(f'Create pvc for snapshot {item.name} failed:{e}.')
                del pvc
        log.info(f'Create pvc for snapshot success task id is {self.kubernetes_back_info.task_id}.')
        return True, pvc_snapshot_list

    def _get_pvc_annotations(self, source_name):
        pvc_api = ResourcePersistentVolumeClaim(self._job_info.resource.cluster_authentication)
        pvc_info = pvc_api.read(name=source_name, namespace=self._job_info.resource.namespace.name)
        return pvc_info.metadata.annotations

    def _clean_pvc(self):
        pvc_api = ResourcePersistentVolumeClaim(self._job_info.resource.cluster_authentication)
        sp_api = CSIManager(self._job_info.resource, self._job_info, [])
        pvc_list = pvc_api.list(
            self.ns_name, label_selector=f'{LabelConst.DPA_BACKUP_POD_TASK_KEY}={self.task_id}'
        ).items
        for pvc in pvc_list:
            if PREFIX.SNAPSHOT_PVC_NAME_PREFIX not in pvc.metadata.name:
                log.warning(f'PVC {pvc.metadata.name} has danger label! Delete the task id label!')
                continue
            # 删除快照pvc
            try:
                res = pvc_api.delete(pvc.metadata.name, self._job_info.resource.namespace.name)
                log.info(f"Delete pvc：{pvc.metadata.name} succeed!")
            except Exception as e:
                log.warning(f"Delete pvc：{pvc.metadata.name} failed! Error:{e}")
            # 删除快照
            try:
                sp_name = pvc.spec.data_source.name
                res = sp_api.delete_pvc_snapshot(sp_name)
            except Exception as e:
                log.warning(f"Delete sp：{pvc.metadata.name} failed! Error:{e}")
                continue
            if res:
                log.info(f"Delete snapshot：{sp_name} succeed!")
            else:
                log.error(f"Delete snapshot：{sp_name} failed!")
        return True

    def _execute_consistency_rule(self, consistency_rules: ConsistencyRulesWithAuthentication):
        try:
            k8s_pod_api = ResourcePod(consistency_rules.cluster_authentication)
        except Exception as e:
            log.exception(f'Init K8S client failed:{e}.')
            return False
        return self._execute_sub_rules(k8s_pod_api, consistency_rules.rule.sub_rule_list, consistency_rules.namespace)

    def _make_consistency_rule(self, back_info: JobInfo, rule_type: ConsistencyRuleType):
        consistency_rule = back_info.consistency_rules.post_rule
        if rule_type == ConsistencyRuleType.PRE_RULE:
            consistency_rule = back_info.consistency_rules.pre_rule
        consistency_rules_with_authentication = ConsistencyRulesWithAuthentication(
            rule=consistency_rule,
            cluster_authentication=back_info.resource.cluster_authentication,
            namespace=back_info.resource.namespace
        )
        return self._execute_consistency_rule(consistency_rules_with_authentication)

    def _execute_sub_rules(self, k8s_pod_api: ResourcePod, sub_rules: List[SubRule], backup_ns: Namespace):
        ret = True
        for sub_rule in sub_rules:
            # get all pods by pod_selector
            pod_label_selector = ','.join(sub_rule.pod_selector_list)
            log.debug(f'Pod_label_selector: {pod_label_selector}.')
            pod_list = k8s_pod_api.list(namespace=backup_ns.name, label_selector=pod_label_selector)
            if pod_list is None:
                log.error(f'Get pod by selector {pod_label_selector} failed.')
                ret = False
            # execute sub_rule for 'container' in every pods of pod_list
            for pod in pod_list.items:
                for action in sub_rule.action_list:
                    exec_result = self._execute_cmd_pod_with_container(k8s_pod_api, pod, backup_ns.name,
                                                                       shlex.split(action), sub_rule.container)
                    ret = False if not exec_result else ret
        return ret

    def _report_copy_info(self):
        log.info(f'Start report_copy_info! Task id: {self.task_id}')
        extend_info = CopyInfoRepModel(id=str(self.kubernetes_back_info.copy_id),
                                       repositoryType=self.kubernetes_back_info.resource.backup_type,
                                       remotePath=f'{self.ns_name}',
                                       isLocal=True,
                                       protocol="NFS",
                                       extendInfo={'backup_pvcs': self.pvc_name_and_size})
        data_rep_rsp = extend_info.dict(by_alias=True)
        copy_info = Copy(id=str(self.kubernetes_back_info.copy_id),
                         name=f'k8s-{self.task_id}',
                         format=self.kubernetes_back_info.resource.backup_type,
                         extendInfo=data_rep_rsp)
        report_copy_info = ReportCopyInfoModel(jobId=str(self.task_id),
                                               copy=copy_info.dict(by_alias=True))
        ret = exec_rc_tool_cmd(RpcToolInterface.REPORT_COPY_INFO, report_copy_info.dict(by_alias=True), self.task_id)
        if not ret:
            log.error(f"Fail to report copy info({copy_info})")
        log.info(f'Report copy info:{report_copy_info}, task id:{self.task_id}')
        return ret
