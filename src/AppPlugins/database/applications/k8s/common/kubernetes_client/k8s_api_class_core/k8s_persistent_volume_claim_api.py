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


class ResourcePersistentVolumeClaim(ApiBase):
    def __init__(self, cluster_authentication):
        super().__init__(cluster_authentication, f'Core{self.POSTFIX}')

    @api_exception_handler
    def list(self, namespace, **kwargs):
        api_method = self.get_api_attr('list_namespaced_persistent_volume_claim')
        return api_method(namespace, **kwargs)

    @api_exception_handler
    def limitlist(self, namespace, limit, field, label, **kwargs):
        api_method = self.get_api_attr('list_namespaced_persistent_volume_claim')
        return api_method(namespace, limit=limit, field_selector=field, label_selector=label, **kwargs)

    @api_exception_handler
    def list_for_all_namespace(self, **kwargs):
        api_method = self.get_api_attr('list_persistent_volume_claim_for_all_namespaces')
        return api_method(**kwargs)

    @api_exception_handler
    def limitlist_for_all_namespace(self, limit, field, label, **kwargs):
        api_method = self.get_api_attr('list_persistent_volume_claim_for_all_namespaces')
        return api_method(limit=limit, field_selector=field, label_selector=label, **kwargs)

    @api_exception_handler
    def read(self, name, namespace, **kwargs):
        api_method = self.get_api_attr('read_namespaced_persistent_volume_claim')
        return api_method(name, namespace, **kwargs)

    @api_exception_handler
    def create(self, namespace, body, **kwargs):
        api_method = self.get_api_attr('create_namespaced_persistent_volume_claim')
        return api_method(namespace, body, **kwargs)

    @api_exception_handler
    def delete(self, name, namespace, **kwargs):
        api_method = self.get_api_attr('delete_namespaced_persistent_volume_claim')
        return api_method(name, namespace, **kwargs)
