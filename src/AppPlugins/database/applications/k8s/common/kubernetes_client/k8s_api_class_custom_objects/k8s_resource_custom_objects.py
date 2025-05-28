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

from k8s.common.kubernetes_client.struct import PatchInfo
from k8s.logger import log
from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler
from k8s.common.kubernetes_client.k8s_api_class_apiextensions.k8s_crd_api import ResourceCRD


class ResourceCustomObjects(ApiBase):
    def __init__(self, cluster_authentication):
        super().__init__(cluster_authentication, f'CustomObjects{self.POSTFIX}')
        self.__auth = cluster_authentication

    @api_exception_handler
    def create_with_namespace(
        self, group, version, namespace, plural, body, **kwargs
    ):
        api_method = self.get_api_attr('create_namespaced_custom_object')
        return api_method(group, version, namespace, plural, body, **kwargs)

    @api_exception_handler
    def create_without_namespace(
        self, group, version, plural, body, **kwargs
    ):
        api_method = self.get_api_attr('create_cluster_custom_object')
        return api_method(group, version, plural, body, **kwargs)

    @api_exception_handler
    def delete_with_namespace(self, group, version, namespace, plural, name, **kwargs):
        api_method = self.get_api_attr('delete_namespaced_custom_object')
        return api_method(group, version, namespace, plural, name, **kwargs)

    @api_exception_handler
    def delete_without_namespace(self, group, version, plural, name, **kwargs):
        api_method = self.get_api_attr('delete_cluster_custom_object')
        return api_method(group, version, plural, name, **kwargs)

    def list_without_namespace(self, group, version, plural, **kwargs):
        api_method = self.get_api_attr('list_cluster_custom_object')
        return api_method(group, version, plural, **kwargs)

    @api_exception_handler
    def list_cluster_custom_object(self, *args, **kwargs):
        api_method = self.get_api_attr('list_cluster_custom_object')
        ret = api_method(*args, **kwargs)
        return ret

    def list_namespaced_custom_object(self, group, version, namespace, plural, **kwargs):
        api_method = self.get_api_attr('list_namespaced_custom_object')
        return api_method(group, version, namespace, plural, **kwargs)

    @api_exception_handler
    def get_lists(self, namespace='', label_selector=''):
        crd_list = ResourceCRD(self.__auth).list_crd(label_selector=label_selector)
        crd_obj_list = []
        crd_obj_list_namespaced = []
        log.info(f"Start get crd items:{crd_list.items}")
        for resource in crd_list.items:
            try:
                log.debug(f"List crd resource:{resource}")
                plural = resource.spec.names.plural
                group = resource.spec.group
                ver = self.get_version(resource.spec.group)
                namespaced = True if resource.spec.scope == "Namespaced" else False
                if namespaced and namespace != '':
                    list_obj = self.list_namespaced_custom_object(
                        group=group,
                        version=ver,
                        namespace=namespace,
                        plural=plural,
                        label_selector=label_selector
                    )
                    crd_obj_list_namespaced.append(list_obj['items'] if list_obj['items'] else None)
                if not namespaced:
                    list_obj = self.list_cluster_custom_object(
                        group=group,
                        version=ver,
                        plural=plural,
                        label_selector=label_selector
                    )
                    crd_obj_list.append(list_obj['items'] if list_obj['items'] else None)
            except Exception as err:
                log.exception(f"Not find custom object for crd err:{err}")
                pass
        result = [crd_obj_list_namespaced, crd_obj_list]
        return result

    def get_version(self, group: str):
        if group.endswith(".k8s.io"):
            group = group[:-len(".k8s.io")]

        snapshot_version_pattern = f'{group.capitalize()}(.+?)Api'
        api_version = ''
        for version in self.k8s_client.get_prefer_versions():
            match = re.findall(snapshot_version_pattern, version)
            if match:
                api_version = match[0].lower()
                break

        return api_version

    def get_without_namespace(self, group, version, plural, name, **kwargs):
        api_method = self.get_api_attr('get_cluster_custom_object')
        return api_method(group, version, plural, name, **kwargs)

    def get_with_namespace(self, group, version, namespace, plural, name, **kwargs):
        api_method = self.get_api_attr('get_namespaced_custom_object')
        return api_method(group, version, namespace, plural, name, **kwargs)

    def patch_with_namespace(self, patch_info: PatchInfo, **kwargs):
        api_method = self.get_api_attr('patch_namespaced_custom_object')
        return api_method(patch_info.group, patch_info.version, patch_info.namespace, patch_info.plural,
                          patch_info.name, patch_info.body, **kwargs)

    def get_api_resources(self, group, version, **kwargs):
        api_method = self.get_api_attr('get_api_resources')
        return api_method(group, version, **kwargs)