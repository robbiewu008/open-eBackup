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
from kubernetes.client import V1Pod, V1PodSpec, V1ObjectMeta, \
    V1PersistentVolumeClaimVolumeSource, V1NFSVolumeSource, \
    V1Container, V1ContainerPort, V1VolumeMount, V1Volume, V1PodStatus, V1SecurityContext, V1ResourceRequirements

from k8s.common.const import UserIdentity, ResourceRequirements
from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler, InitKubernetesApiError
from k8s.common.kubernetes_client.struct import ClusterAuthentication
from k8s.logger import log

POSTFIX = "Core(.+?)Api"


class BackupPod(ApiBase):
    """create Backup job """

    def __init__(self, cluster_authentication: ClusterAuthentication):
        super().__init__(cluster_authentication, f'Core{self.POSTFIX}')
        self._pod_body = V1Pod()
        self._pod_body.api_version = 'v1'
        self._pod_body.kind = 'Pod'
        self._pod_body.spec = V1PodSpec(containers=[])
        self._pod_body.metadata = V1ObjectMeta()
        self._pod_body.metadata.labels = {}
        self._pod_body.metadata.labels.update({'backup-pod': 'agent'})
        self._init_pod_spec_para()
        self._pod_spec.containers = []
        self._pod_spec.volumes = []
        self._init_container_para()
        self._volume_dic = {}
        self._volume_path_dic = {}
        self._container_ports = {}

    def set_node_name(self, node_name: str):
        self._pod_spec.node_name = node_name

    def set_job_name(self, job_name: str):
        if not job_name or ' ' in job_name:
            log.error('Job name is empty or contain space')
            raise InitKubernetesApiError
        self._pod_body.metadata.name = job_name

    def add_nfs_volume(self, nfs_name: str, volume_object: V1NFSVolumeSource):
        """[summary]
        add nfs volume to pod
        Args:
            nfs_name (str): [description] nfs name and make sure the name is the only one in volumes
            volume_object (V1NFSVolumeSource): [description] NFS object
        """
        if self._volume_dic.get(nfs_name) is None:
            self._volume_dic[nfs_name] = volume_object
        else:
            log.error(f"Job nfs volume name {nfs_name} have repeats")
            raise InitKubernetesApiError

    def add_snapshot_pvc_volume(self, pvc_name: str, pvc_object: V1PersistentVolumeClaimVolumeSource):
        """[summary]
        add snap shot pvc to pod
        Args:
            pvc_name (str): [description] pvc_name is same as V1PersistentVolumeClaimVolumeSource's claim name
            pvc_object (V1PersistentVolumeClaimVolumeSource): [description] pvc object
        """
        if self._volume_dic.get(pvc_name) is None:
            self._volume_dic[pvc_name] = pvc_object
        else:
            log.error(f"Job snapshot_pvc name {pvc_name} have repeats")
            raise InitKubernetesApiError

    def set_volume_mount_path(self, volume_name: str, volume_mount: V1VolumeMount):
        """[summary]
        set volume in container mount point  volume must have in pod
        Args:
            volume_name (str): [description] volume name must not None
            volume_mount (V1VolumeMount): [description] volume in container mount point
        """
        if self._volume_dic.get(volume_name) is None:
            log.error(f"Volume name {volume_name} is no exist")
            raise InitKubernetesApiError
        else:
            self._volume_path_dic[volume_name] = volume_mount

    def set_job_metadata(self, metadata: V1ObjectMeta):
        self._pod_body.metadata = metadata

    def set_labels(self, label: dict):
        if label:
            self._pod_body.metadata.labels.update(label)

    def set_active_deadline_seconds(self, seconds: int):
        self._pod_body.spec.active_deadline_seconds = seconds

    def set_backoff_limit(self, limit: int):
        self._pod_body.spec.backoff_limit = limit

    def set_ttl_seconds_after_finished(self, seconds: int):
        self._pod_body.spec.ttl_seconds_after_finished = seconds

    def set_container_image(self, image_name: str):
        if not image_name or ' ' in image_name:
            log.error('Image name is empty or contain space')
            raise InitKubernetesApiError
        self._agent_container.image = image_name

    def set_container_command(self, command: list):
        if command:
            self._agent_container.command = command

    def add_container_port(self, port: V1ContainerPort):
        self._agent_container.ports.append(port)

    def set_job_namespace(self, namespace: str):
        if not namespace or not namespace.strip():
            log.error("Namespace is None or only contain space")
            raise InitKubernetesApiError
        self._pod_body.metadata.namespace = namespace

    @api_exception_handler
    def create_backup_pod(self):
        create_job_attr = self.get_api_attr('create_namespaced_pod')
        if not self._pod_body.metadata.namespace:
            self._pod_body.metadata.namespace = 'default'

        for item in self._volume_path_dic.values():
            self._agent_container.volume_mounts.append(item)

        self._pod_spec.containers.append(self._agent_container)

        for name, volume_object in self._volume_dic.items():
            volume_item = V1Volume(name=name)
            if type(volume_object) == V1NFSVolumeSource:
                volume_item.nfs = volume_object
            elif type(volume_object) == V1PersistentVolumeClaimVolumeSource:
                volume_item.persistent_volume_claim = volume_object
            self._pod_spec.volumes.append(volume_item)
        self._pod_body.spec = self._pod_spec
        return create_job_attr(self._pod_body.metadata.namespace, self._pod_body)

    @api_exception_handler
    def delete_backup_pod(self):
        delete_job_attr = self.get_api_attr('delete_namespaced_pod')
        return delete_job_attr(self._pod_body.metadata.name, self._pod_body.metadata.namespace)

    @api_exception_handler
    def get_backup_pod(self) -> V1PodStatus:
        get_job_attr = self.get_api_attr('read_namespaced_pod_status')
        ret = get_job_attr(self._pod_body.metadata.name, self._pod_body.metadata.namespace)
        return ret.status

    def set_resource_requirement(self, resource_requirements=V1ResourceRequirements()):
        self._agent_container.resources = resource_requirements

    def _init_container_para(self):
        # some container para not sure
        security_context = V1SecurityContext(run_as_user=UserIdentity.ROOT.value)
        resource_requirements = V1ResourceRequirements(limits=ResourceRequirements.LIMITS,
                                                       requests=ResourceRequirements.REQUESTS)
        self._agent_container = V1Container(name='agent', image_pull_policy='IfNotPresent',
                                            security_context=security_context, resources=resource_requirements)
        self._agent_container.image = ''
        self._agent_container.ports = []
        self._agent_container.volume_mounts = []
        self._agent_container.command = ["/bin/sh", "-c", "while true;do sleep 1;done"]

    def _init_pod_spec_para(self):
        # some pod para not sure
        self._pod_spec = V1PodSpec(containers=[])
        self._pod_spec.host_network = True
        self._pod_spec.volumes = []
        self._pod_spec.restart_policy = "Never"
