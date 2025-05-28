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

from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler


class ResourceDaemonSet(ApiBase):
    def __init__(self, cluster_authentication):
        super().__init__(cluster_authentication, self.AppsV1Api)

    @api_exception_handler
    def list(self, namespace, **kwargs):
        api_method = self.get_api_attr('list_namespaced_daemon_set')
        return api_method(namespace, **kwargs)
