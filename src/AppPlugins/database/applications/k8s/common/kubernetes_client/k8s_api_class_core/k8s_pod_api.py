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

from kubernetes.client import ApiException
from kubernetes.stream import stream

from k8s.common.const import HTTP_REQUEST_TIMEOUT
from k8s.common.kubernetes_client.k8s_api import ApiBase, api_exception_handler


class ResourcePod(ApiBase):
    def __init__(self, cluster_authentication):
        super().__init__(cluster_authentication, f'Core{self.POSTFIX}')

    @api_exception_handler
    def list(self, namespace, **kwargs):
        api_method = self.get_api_attr('list_namespaced_pod')
        return api_method(namespace, **kwargs)

    @api_exception_handler
    def limitlist(self, namespace, limit, field, label, **kwargs):
        api_method = self.get_api_attr('list_namespaced_pod')
        return api_method(namespace, limit=limit, field_selector=field, label_selector=label, **kwargs)

    @api_exception_handler
    def read(self, name, namespace, **kwargs):
        api_method = self.get_api_attr('read_namespaced_pod')
        return api_method(name, namespace, **kwargs)

    @api_exception_handler
    def exec(self, name, namespace, **kwargs):
        api_method = self.get_api_attr('connect_get_namespaced_pod_exec')
        return stream(api_method, name, namespace, **kwargs)

    @api_exception_handler
    def delete(self, name, namespace, **kwargs):
        api_method = self.get_api_attr('delete_namespaced_pod')
        try:
            api_method(name, namespace, **kwargs)
        except ApiException as ex:
            if ex.status == 404:
                return
            raise ex


    @api_exception_handler
    def async_pod_exec(self, pod_name, namespace, command, container, ignore_res=False):
        api_method = self.get_api_attr('connect_get_namespaced_pod_exec')
        resp = stream(api_method,
                      pod_name,
                      namespace,
                      command=command,
                      stderr=True, stdin=False,
                      stdout=True, tty=False,
                      container=container,
                      _preload_content=False,
                      _request_timeout=HTTP_REQUEST_TIMEOUT)
        if ignore_res:
            return resp
        time_out = 600
        while resp.is_open():
            resp.update(timeout=5)
            if resp.peek_stdout():
                return resp.read_stdout()
            if resp.peek_stderr():
                return resp.read_stderr()
            time_out -= 5
            if time_out == 0:
                break
        resp.close()
        raise AttributeError()

    @api_exception_handler
    def sync_pod_exec(self, pod_name, namespace, command, container):
        api_method = self.get_api_attr('connect_get_namespaced_pod_exec')
        resp = stream(api_method,
                      pod_name,
                      namespace,
                      command=command,
                      stderr=True, stdin=False,
                      stdout=True, tty=False,
                      container=container,
                      _preload_content=False,
                      _request_timeout=HTTP_REQUEST_TIMEOUT)
        return resp
