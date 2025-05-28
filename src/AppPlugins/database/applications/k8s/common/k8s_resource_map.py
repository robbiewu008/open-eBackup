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

from k8s.common.kubernetes_client.k8s_api_class_apps import ResourceStatefulSet
from k8s.common.kubernetes_client.k8s_api_class_apps.k8s_daemon_set_api import \
    ResourceDaemonSet
from k8s.common.kubernetes_client.k8s_api_class_apps.k8s_deployment_api import \
    ResourceDeployment

from k8s.common.kubernetes_client.struct import WorkLoadType

from k8s.common.kubernetes_client.k8s_api_class_version.k8s_query_cluster_version import ResourceCluster
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_namespace_api import ResourceNamespace
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_apps.k8s_workload_api import ResourceWorkLoad

K8SAPI = {
    "Cluster": ResourceCluster,
    "Pod": ResourcePod,
    "PersistentVolumeClaim": ResourcePersistentVolumeClaim,
    "Namespace": ResourceNamespace,
    "Workload": ResourceWorkLoad
}

WORKLOAD_API_MAP = {
    WorkLoadType.POD: ResourcePod,
    WorkLoadType.STATEFULSET: ResourceStatefulSet,
    WorkLoadType.DAEMONSET: ResourceDaemonSet,
    WorkLoadType.DEPLOYMENT: ResourceDeployment
}
