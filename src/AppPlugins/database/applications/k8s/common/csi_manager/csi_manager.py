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

import re
import datetime

from common.common import report_job_details
from common.common_models import LogDetail, SubJobDetails
from common.const import DBLogLevel, SubJobStatusEnum
from k8s.common import const
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.label_const import LabelConst
from k8s.logger import log
from k8s.common.error_code import ErrorCode
from k8s.common.const import (PREFIX, SNAPSHOT_GROUP, SNAPSHOT_VERSION_PATTERN,
                              VOLUME_SNAPSHOT_CLASS_KIND,
                              VOLUME_SNAPSHOT_CLASS_PLURAL,
                              VOLUME_SNAPSHOT_KIND, VOLUME_SNAPSHOT_PLURAL,
                              K8SProtectError, SnapShotInfo)
from k8s.common.kubernetes_client.k8s_api import InitKubernetesApiError
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_custom_objects import \
    ResourceCustomObjects
from k8s.common.kubernetes_client.k8s_api_class_storage.k8s_resource_storage_class import \
    ResourceStorageClass
from k8s.common.kubernetes_client.struct import Resource
from k8s.common.utils import get_uuid_last_section
from pydantic import BaseModel


class PVCInfo(BaseModel):
    name: str
    uid: str
    storage_class_name: str
    csi_driver_name: str
    size: str
    access_mode: str


def is_pvc_can_backup(storage_class):
    for key, value in storage_class.parameters.items():
        if 'csi-driver-name' in key and value not in 'disk.csi.everest.io':
            return False
    return True


class CSIManager:

    def __init__(self, resource: Resource, kubernetes_back_info: const.JobInfo, pvc_list):
        self.resource = resource
        self.pvc_list = pvc_list
        self._kubernetes_back_info = kubernetes_back_info
        self.task_id = kubernetes_back_info.task_id
        # info format key: pvc_name value: PVCInfo
        self.pvc_info_map = dict()
        self.k8s_custom_objects_api = ResourceCustomObjects(resource.cluster_authentication)
        self.snapshot_infos = []
        self.driver_list = set()
        prefer_versions = self.k8s_custom_objects_api.k8s_client.get_prefer_versions()
        snapshot_api_is_exist = False
        log.info(f"CSIManger: prefer versions is {prefer_versions}")
        for version in prefer_versions:
            match = re.findall(SNAPSHOT_VERSION_PATTERN, version)
            if match:
                snapshot_api_is_exist = True
                self.snapshot_api_version = match[0].lower()
                break
        if not snapshot_api_is_exist:
            log.error('Cluster has not snapshot api.')
            raise InitKubernetesApiError

    def create_pvcs_snapshot(self):
        for pvc in self.pvc_list:
            try:
                if not self._get_csi_driver_from_pvc(pvc):
                    continue
                self._create_volume_snapshot_class(pvc)
                self._create_pvc_snapshot(pvc)
                log_info = const.K8sReportLabel.PVC_CREATE_SNAPSHOT_SUCCESS
                snapshot_time = datetime.datetime.now()
                log_detail = LogDetail(logInfo=log_info,
                                       logLevel=DBLogLevel.INFO.value,
                                       logInfoParam=[snapshot_time.strftime("%Y-%m-%d %H:%M:%S"), pvc])
                report_job_details(self._kubernetes_back_info.request_id,
                                   SubJobDetails(taskId=self.task_id, subTaskId=self._kubernetes_back_info.sub_id,
                                                 logDetail=[log_detail],
                                                 progress=const.Progress.PROGRESS_ZERO,
                                                 taskStatus=SubJobStatusEnum.RUNNING.value))
            except K8SProtectError as e:
                log.error(e.desc)
                self._delete_snapshot()
                return False, []
        return True, self.snapshot_infos

    def delete_pvc_snapshot(self, sp_name):
        try:
            self.k8s_custom_objects_api.delete_with_namespace(
                group=SNAPSHOT_GROUP,
                version=self.snapshot_api_version,
                namespace=self.resource.namespace.name,
                plural=VOLUME_SNAPSHOT_PLURAL,
                name=sp_name)
            return True
        except Exception as err:
            log.error(f'Delete volume snapshot:{sp_name} failed.{err}')
            return False

    def _delete_snapshot(self):
        for snapshot_info in self.snapshot_infos:
            if self.k8s_custom_objects_api.delete_with_namespace(
                group=SNAPSHOT_GROUP,
                version=self.snapshot_api_version,
                namespace=self.resource.namespace.name,
                plural=VOLUME_SNAPSHOT_PLURAL,
                name=snapshot_info.name
            ) is None:
                log.error(f'Delete volume snapshot:{snapshot_info.name} failed.')

    def _create_pvc_snapshot(self, pvc):
        log.info(f'Begin to create pvc snapshot. PVC:{pvc}')
        pvc_info = self.pvc_info_map.get(pvc)
        volume_snap_shot_class = f"{PREFIX.VOLUME_SNAPSHOT_CLASS_NAME_PREFIX}-{pvc_info.csi_driver_name}"
        volume_snap_shot = f"{PREFIX.VOLUME_SNAPSHOT_NAME_PREFIX}-{get_uuid_last_section(self.task_id)}-" \
                           f"{get_uuid_last_section(pvc_info.uid)}"
        log.info(f"Create snapshot name {volume_snap_shot}")
        volume_snapshot_resource = {
            "apiVersion": f"{SNAPSHOT_GROUP}/{self.snapshot_api_version}",
            "kind": VOLUME_SNAPSHOT_KIND,
            "metadata": {
                "name": volume_snap_shot,
                "labels": {LabelConst.DPA_BACKUP_POD_PVC_KEY: pvc,
                           LabelConst.DPA_BACKUP_POD_COMMON_KEY: LabelConst.DPA_BACKUP_POD_COMMON_VALUE,
                           LabelConst.DPA_BACKUP_POD_TASK_KEY: self.task_id}
            },
            "spec": {
                "volumeSnapshotClassName": f"{volume_snap_shot_class}",
                "source": {
                    "persistentVolumeClaimName": pvc
                }
            }
        }
        if self.k8s_custom_objects_api.create_with_namespace(
            group=SNAPSHOT_GROUP,
            version=self.snapshot_api_version,
            namespace=self.resource.namespace.name,
            plural=VOLUME_SNAPSHOT_PLURAL,
            body=volume_snapshot_resource
        ) is not None:
            log.info(f'Create volume snapshot success, pvc: {pvc}')
            self.snapshot_infos.append(
                SnapShotInfo(name=volume_snap_shot, storage_class_name=pvc_info.storage_class_name, size=pvc_info.size,
                             source_name=pvc, source_uid=pvc_info.uid, access_mode=pvc_info.access_mode)
            )
            return
        else:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, f'Create volume snapshot failed, pvc: {pvc}')

    def _create_volume_snapshot_class(self, pvc):
        volume_snap_shot_class = f"{PREFIX.VOLUME_SNAPSHOT_CLASS_NAME_PREFIX}-" \
                                 f"{self.pvc_info_map.get(pvc).csi_driver_name}"
        volume_snap_shot_class_list = self.k8s_custom_objects_api.list_cluster_custom_object(
            group=SNAPSHOT_GROUP,
            version=self.snapshot_api_version,
            plural=VOLUME_SNAPSHOT_CLASS_PLURAL,
            field_selector=f'metadata.name={volume_snap_shot_class}'
        )
        if volume_snap_shot_class_list is None:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'Find volume snapshot class failed.')
        if volume_snap_shot_class_list.get('items'):
            log.info(f'Volume snapshot class: {volume_snap_shot_class} already exists.')
            return
        volume_snapshot_class_resource = {
            "apiVersion": f"{SNAPSHOT_GROUP}/{self.snapshot_api_version}",
            "kind": VOLUME_SNAPSHOT_CLASS_KIND,
            "metadata": {
                "name": volume_snap_shot_class
            },
            "driver": self.pvc_info_map.get(pvc).csi_driver_name,
            "deletionPolicy": "Delete"
        }
        if self.k8s_custom_objects_api.create_without_namespace(
            group=SNAPSHOT_GROUP,
            version=self.snapshot_api_version,
            plural=VOLUME_SNAPSHOT_CLASS_PLURAL,
            body=volume_snapshot_class_resource
        ) is not None:
            log.info('Create volume snapshot class success.')
            return
        else:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'Create volume snapshot class failed.')

    def _get_csi_driver_from_pvc(self, pvc):
        try:
            k8s_pvc_api = ResourcePersistentVolumeClaim(self.resource.cluster_authentication)
            k8s_storage_class_api = ResourceStorageClass(self.resource.cluster_authentication)
        except InitKubernetesApiError as e:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'Init K8S client failed.') from e
        pvc_list = k8s_pvc_api.list(
            namespace=self.resource.namespace.name,
            field_selector=f'metadata.name={pvc}'
        )
        if pvc_list is None:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'List pvc failed.')
        if not pvc_list.items:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'Can not find target pvc.')

        storage_class_name = pvc_list.items[0].spec.storage_class_name
        access_mode = pvc_list.items[0].spec.access_modes[0]
        pvc_uid = pvc_list.items[0].metadata.uid
        pvc_size = pvc_list.items[0].status.capacity.get('storage')
        log.debug(f'PVC: {pvc}\'s storage class: {storage_class_name}')

        storage_class_list = k8s_storage_class_api.list(
            field_selector=f'metadata.name={storage_class_name}'
        )
        if not is_pvc_can_backup(storage_class_list.items[0]):
            log.info(f"Pvc:{pvc} not support snapshot, skip!")
            return False
        if storage_class_list is None:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'List storage class failed.')
        if not storage_class_list.items:
            raise K8SProtectError(ErrorCode.K8S_API_FAILED, 'Can not find csi driver of storage class.')
        csi_driver_name = self._get_csi_driver_name(storage_class_list.items[0], self.resource.cluster_authentication)
        self.pvc_info_map[pvc] = PVCInfo(
            name=pvc, uid=pvc_uid, storage_class_name=storage_class_name, csi_driver_name=csi_driver_name,
            size=pvc_size, access_mode=access_mode
        )
        log.debug(f'PVC: {pvc} uid: {pvc_uid} csi driver: {csi_driver_name}')
        return True

    def _get_csi_driver_name(self, storage_class, cluster_authentication):
        node_list = ResourceCoreApi(cluster_authentication) \
            .list_cluster_resource_with_http_info('nodes')
        rco = ResourceCustomObjects(cluster_authentication)
        # 获取集群上所有可用的csi-driver
        for node in node_list.get("items"):
            csi_node = rco.get_without_namespace(group='storage.k8s.io', version='v1', plural='csinodes',
                                      name=node['metadata']['name'])
            if csi_node.get("spec").get("drivers"):
                for driver in csi_node.get("spec").get("drivers"):
                    self.driver_list.add(driver.get("name"))
        log.info(f"Drivers is {self.driver_list}")
        # 分别从storage-class中的provisioner和parameter中获取csi-driver
        if storage_class.provisioner in self.driver_list:
            return storage_class.provisioner
        log.info(f"Storage class parameters is {storage_class.parameters}")
        for _, value in storage_class.parameters.items():
            if value in self.driver_list:
                return value
        raise K8SProtectError(ErrorCode.OPERATION_FAILED, 'Can not find csi driver of storage class.')
