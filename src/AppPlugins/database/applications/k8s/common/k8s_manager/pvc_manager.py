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

from k8s.common.kubernetes_client.k8s_api_class_apps.k8s_workload_api import ResourceWorkLoad
from k8s.common.kubernetes_client.k8s_api_class_batch.k8s_beta_cron_job_api import ResourceCronJob
from k8s.common.kubernetes_client.k8s_api_class_batch.k8s_job_api import ResourceJob
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.kubernetes_client.struct import ClusterAuthentication, WorkLoadType
from k8s.common.label_const import LabelConst


class PvcManager:
    def __init__(self, cluster_authentication: ClusterAuthentication, namespace, labels, task_id):
        self.cluster_authentication = cluster_authentication
        self.namespace = namespace
        self.labels = labels
        self.task_id = task_id

    def get_dataset_by_label_and_group_by_type(self):
        pvc_list = []
        k8s_pvc_api = ResourcePersistentVolumeClaim(self.cluster_authentication)
        rc = ResourceCoreApi(self.cluster_authentication)
        if not self.labels:
            exclude_label = f"{LabelConst.DPA_BACKUP_POD_COMMON_KEY}!={LabelConst.DPA_BACKUP_POD_COMMON_VALUE}"
            pvcs = k8s_pvc_api.list(namespace=self.namespace, label_selector=exclude_label)
            pvc_list.extend(
                [
                    rc.get_namespaced_resource_with_http_info(name=pvc.metadata.name, namespace=self.namespace,
                                                              plural='persistentvolumeclaims')
                    for pvc in pvcs.items
                ]
            )
            return pvc_list

        workload_list = [
            WorkLoadType.REPLICASET, WorkLoadType.STATEFULSET, WorkLoadType.DAEMONSET,
            WorkLoadType.DEPLOYMENT
        ]
        for workload_type in workload_list:
            if workload_type == WorkLoadType.JOB:
                workload_api = ResourceJob(self.cluster_authentication)
                workload_items = workload_api.list(self.namespace,
                                                   label_selector=self.labels).items
                # 根据每种数据集查询其管理的pod。数据集的选择器就是pod的标签
                self.get_pod_dispatched_by_dataset(workload_items=workload_items, pvc_list=pvc_list)
                continue
            elif workload_type == WorkLoadType.CRONJOB:
                workload_api = ResourceCronJob(self.cluster_authentication)
                workload_items = workload_api.list(self.namespace,
                                                   label_selector=self.labels).items
                # 根据每种数据集查询其管理的pod。数据集的选择器就是pod的标签
                self.get_pod_dispatched_by_cronjob(workload_items=workload_items, pvc_list=pvc_list)
                continue
            workload_api = ResourceWorkLoad(self.cluster_authentication, workload_type.value)
            workload_items = workload_api.list(self.namespace,
                                               label_selector=self.labels).items
            # 根据每种数据集查询其管理的pod。数据集的选择器就是pod的标签
            self.get_pod_dispatched_by_dataset(workload_items=workload_items, pvc_list=pvc_list)
        return pvc_list

    def get_pod_dispatched_by_dataset(self, workload_items, pvc_list):
        for workload in workload_items:
            selector_dict = workload.spec.selector.match_labels
            # 拼接pod的label_selector,例子:'test=test,open=open'
            label_selector_list = []
            for key, value in selector_dict.items():
                label_selector_list.append(key + '=' + value)
            label_selector = ','.join(str(label) for label in label_selector_list)
            # 查询每个pod下绑定的pvc
            self.get_pvc_info_bound_by_pod(pvc_list=pvc_list, label_selector=label_selector)

    def get_pod_dispatched_by_cronjob(self, workload_items, pvc_list):
        for workload in workload_items:
            selector_dict = workload.spec.job_template.spec.selector.match_labels
            # 拼接pod的label_selector,例子:'test=test,open=open'
            label_selector_list = []
            for key, value in selector_dict.items():
                label_selector_list.append(key + '=' + value)
            label_selector = ','.join(str(label) for label in label_selector_list)
            # 查询每个pod下绑定的pvc
            self.get_pvc_info_bound_by_pod(label_selector=label_selector, pvc_list=pvc_list)

    def get_pvc_info_bound_by_pod(self, label_selector, pvc_list):
        pod_api = ResourcePod(self.cluster_authentication)
        k8s_pvc_api = ResourceCoreApi(self.cluster_authentication)
        pod_res = pod_api.list(self.namespace, label_selector=label_selector)
        for pod in pod_res.items:
            for volume in pod.spec.volumes:
                if not volume.persistent_volume_claim:
                    continue
                pvc_name = volume.persistent_volume_claim.claim_name
                # 根据命名空间和属性选择器查询唯一的pvc
                pvc_info = k8s_pvc_api.get_namespaced_resource_with_http_info(name=pvc_name, namespace=self.namespace,
                                                                              plural='persistentvolumeclaims')
                # 过滤重复的pvc
                if pvc_info not in pvc_list:
                    pvc_list.append(pvc_info)