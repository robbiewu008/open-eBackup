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

from k8s.common.const import JobInfo
from k8s.logger import log
from k8s.common.kubernetes_client.struct import ClusterAuthentication
from k8s.services.backup.k8s_metadata_selector import get_all_resources_with_label


class K8sMetadataBackupHandle:
    def __init__(self, kubernetes_back_info: JobInfo):
        self._kubernetes_back_info = kubernetes_back_info
        self._all_resources = []

    @staticmethod
    def _print_single_resource_list(resources_items):
        for single_resource in resources_items:
            rc_name = single_resource["metadata"]["name"]
            log.debug(f"Resource_type---resource name---{rc_name}")

    def get_all_resources(self):
        return self._all_resources

    def backup_metadata(self):
        backuped_namespace = self._kubernetes_back_info.resource.namespace.name
        k8s_auth_info = self._kubernetes_back_info.resource.cluster_authentication
        if not self._kubernetes_back_info.resource.dataset.labels:
            label_selector = ''
        else:
            label_selector = self._kubernetes_back_info.resource.dataset.labels
        log.info(f"Backup by label, label selector : {label_selector} .")
        self._backup_by_label(k8s_auth_info, label_selector, backuped_namespace)
        self._print_resources_count()
        return

    def _print_resources_count(self):
        log.info(f"All_resources type count: {len(self._all_resources)}")
        for resources_list in self._all_resources:
            if not resources_list:
                log.warning("Nothing been backed up.")
                continue
            if isinstance(resources_list.items[0], dict):
                if "namespace" in resources_list.items[0]["metadata"] and \
                        resources_list.items[0]["metadata"]["namespace"]:
                    namespace = resources_list.items[0]["metadata"]["namespace"]
                else:
                    namespace = "-"
            else:
                namespace = resources_list.items[0].metadata.namespace
            log.warning(
                f"Resource_type--namespace--{namespace}--len---{len(resources_list.items)}"
            )
            self._print_single_resource_list(resources_list.items)

    def _backup_by_label(self, k8s_auth_info: ClusterAuthentication, label_selector: str, backuped_namespace: str):
        crd_resources_list, not_crd_resources = get_all_resources_with_label(k8s_auth_info, label_selector,
                                                                             backuped_namespace,
                                                                             self._kubernetes_back_info.task_id)
        if crd_resources_list:
            self._all_resources.extend(crd_resources_list)
        if not_crd_resources:
            self._all_resources.extend(not_crd_resources)
        return
