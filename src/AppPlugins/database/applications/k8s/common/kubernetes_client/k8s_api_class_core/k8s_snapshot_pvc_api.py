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
from kubernetes.client import V1PersistentVolumeClaim, V1PersistentVolumeClaimSpec, V1ObjectMeta, \
                              V1TypedLocalObjectReference, V1ResourceRequirements

from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler, InitKubernetesApiError
from k8s.common.kubernetes_client.struct import ClusterAuthentication
from k8s.logger import log

POSTFIX = "Core(.+?)Api"


class SnapShotPVC(ApiBase):
    """[summary]

    Args:
        cluster_authentication: ClusterAuthentication [description]
        ClusterAuthentication para
        auth_type: AuthType conf or toke
        token: Optional[Token]
        kube_config: Optional[str]
    """
    def __init__(self, cluster_authentication: ClusterAuthentication):
        super().__init__(cluster_authentication, 'Core')
        self._pvc_attr = V1PersistentVolumeClaim()
        api_version = False
        for version in self.k8s_client.get_prefer_versions():
            match = re.findall(POSTFIX, version)
            if match:
                self._pvc_attr.api_version = f'{match[0].lower()}'
                api_version = True
                break
        if not api_version:
            log.error("get api version fail")
            raise InitKubernetesApiError
        self._pvc_attr.kind = 'PersistentVolumeClaim'
        self._pvc_attr.metadata = V1ObjectMeta()
        self._pvc_attr.metadata.labels = dict()
        self._pvc_attr.metadata.annotations = dict()
        self._pvc_attr.spec = V1PersistentVolumeClaimSpec()
        self._pvc_attr.spec.access_modes = []
        self._access_mode = ""
        self._pvc_attr.spec.data_source = V1TypedLocalObjectReference(
            kind='VolumeSnapshot', name='')
        self._pvc_attr.spec.resources = V1ResourceRequirements()
        self._namespace = ""

    def set_labels(self, label: dict):
        self._pvc_attr.metadata.labels.update(label)

    def set_annotations(self, annotation: dict):
        self._pvc_attr.metadata.annotations.update(annotation)

    def set_snapshot_name(self, name: str):
        self._pvc_attr.spec.data_source.name = name

    def set_pvc_name(self, name: str):
        self._pvc_attr.metadata.name = name

    def set_pvc_metadata(self, meta_data: V1ObjectMeta):
        self._pvc_attr.metadata = meta_data

    def set_api_group(self, api_group: str):
        self._pvc_attr.spec.data_source.api_group = api_group

    def set_storage_class_name(self, classname: str):
        self._pvc_attr.spec.storage_class_name = classname

    def set_access_mode(self, mode: str):
        """[summary]
        set pvc access mode
        Args:
            mode (str): [description]
            ReadWriteOnce -- the volume can be mounted as read-write by a single node
            ReadOnlyMany -- the volume can be mounted read-only by many nodes
            ReadWriteMany -- the volume can be mounted as read-write by many nodes
        """
        if mode is None:
            log.error("pvc access mode must not None")
            raise InitKubernetesApiError
        self._access_mode = mode

    def set_storage_size(self, size: str):
        """[summary]
        set storage size must same as snap volume size
        Args:
            size (str): [description] size formant is Mi or Gi
        """
        size_dic = dict()
        size_dic['storage'] = size
        self._pvc_attr.spec.resources.requests = size_dic

    def set_namespace(self, namespace: str):
        self._namespace = namespace
        self._pvc_attr.metadata.namespace = namespace

    def get_name(self) -> str:
        return self._pvc_attr.metadata.name

    @api_exception_handler
    def create_snapshot_pvc(self):
        create_pvc_attr = self.get_api_attr('create_namespaced_persistent_volume_claim')
        if self._pvc_attr.metadata.name is None:
            log.error('Create snapshot pvc name must not none')
            raise InitKubernetesApiError
        if self._namespace == "":
            log.warning(f'Create snapshot pvc {self._pvc_attr.metadata.name} namespace is default')
            self._namespace = 'default'
            self._pvc_attr.metadata.namespace = 'default'
        if not self._pvc_attr.spec.storage_class_name:
            log.error(f'create snapshot pvc {self._pvc_attr.metadata.name} storage_class_name must not none')
            raise InitKubernetesApiError
        if self._pvc_attr.spec.data_source.name == '':
            log.error(f'create snapshot pvc {self._pvc_attr.metadata.name} snapshot name must not none')
            raise InitKubernetesApiError
        self._pvc_attr.spec.access_modes.append(self._access_mode)
        log.info(f"Create pvc attr is {self._pvc_attr}")
        ret = create_pvc_attr(self._namespace, self._pvc_attr)
        log.info(f"Create snapshot pvc snapshot {self._pvc_attr.metadata.name} result {ret}")
        return ret
