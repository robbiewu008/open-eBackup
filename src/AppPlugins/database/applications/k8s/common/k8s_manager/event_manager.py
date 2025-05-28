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

from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.kubernetes_client.struct import ClusterAuthentication


class EventManager:
    def __init__(self, cluster_authentication: ClusterAuthentication, namespace, task_id):
        self._core_api = ResourceCoreApi(cluster_authentication)
        self.namespace = namespace
        self.task_id = task_id

    def list_namespace_event_by_label(self, field_selector):
        event_info = self._core_api.list_namespaced_resource_with_http_info(
            namespace=self.namespace, plural='events',
            field_selector=field_selector)['items']
        return event_info

    def list_cluster_event_by_label(self, field_selector):
        event_info = self._core_api.list_cluster_resource_with_http_info(plural='events',
                                                                         field_selector=field_selector)['items']
        return event_info
