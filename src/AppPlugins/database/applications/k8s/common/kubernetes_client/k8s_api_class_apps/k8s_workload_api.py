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

from enum import Enum
from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler


class WorkloadApi:
    def __init__(self, api_type):
        self.workload_api = {}
        self.workload_api.update({'StatefulSet': f'{api_type}_namespaced_stateful_set'})
        self.workload_api.update({'Deployment': f'{api_type}_namespaced_deployment'})
        self.workload_api.update({'DaemonSet': f'{api_type}_namespaced_daemon_set'})
        self.workload_api.update({'ReplicaSet': f'{api_type}_namespaced_replica_set'})


class ApiType(str, Enum):
    LIST = "list"
    READ = "read"


class ResourceWorkLoad(ApiBase):
    def __init__(self, cluster_authentication, workload_type):
        super().__init__(cluster_authentication, self.AppsV1Api)
        self.type = workload_type

    @api_exception_handler
    def list(self, namespace, **kwargs):
        workload = WorkloadApi(ApiType.LIST)
        api_method = self.get_api_attr(workload.workload_api.get(self.type))
        return api_method(namespace, **kwargs)

    @api_exception_handler
    def limitlist(self, namespace, limit, field, label, **kwargs):
        workload = WorkloadApi(ApiType.LIST)
        api_method = self.get_api_attr(workload.workload_api.get(self.type))
        return api_method(namespace, limit=limit, field_selector=field, label_selector=label, **kwargs)

    @api_exception_handler
    def read(self, name, namespace, **kwargs):
        workload = WorkloadApi(ApiType.READ)
        api_method = self.get_api_attr(workload.workload_api.get(self.type))
        return api_method(name, namespace, **kwargs)
