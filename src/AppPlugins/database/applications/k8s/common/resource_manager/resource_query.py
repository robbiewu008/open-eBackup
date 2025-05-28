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

from abc import ABC, abstractmethod


class ResourceQueryStrategy(ABC):
    """
    查询资源的策略接口
    """

    @abstractmethod
    def query(self, k8s_api, namespace, limit, label_selector, _continue):
        pass


class PVCQueryStrategy(ResourceQueryStrategy):
    """
    查询 PVC 的具体策略
    """

    def query(self, k8s_api, namespace, limit, label_selector, _continue):
        return k8s_api.limitlist(namespace, limit=limit, _continue=_continue, field='', label=label_selector)


class PodQueryStrategy(ResourceQueryStrategy):
    """
    查询 Pod 的具体策略
    """

    def query(self, k8s_api, namespace, limit, label_selector, _continue):
        return k8s_api.list_namespaced_pod(namespace, limit=limit, _continue=_continue, field='',
                                           label=label_selector)


class NamespaceQueryStrategy(ResourceQueryStrategy):
    """
    查询 Namespace 的具体策略
    """

    def query(self, k8s_api, namespace, limit, label_selector, _continue):
        return k8s_api.list_namespace(limit=limit, _continue=_continue)


class ResourceQueryContext:
    """
    资源查询的上下文类，根据资源类型选择不同的查询策略
    """

    def __init__(self, strategy: ResourceQueryStrategy):
        self._strategy = strategy

    def set_strategy(self, strategy: ResourceQueryStrategy):
        self._strategy = strategy

    def query(self, k8s_api, namespace, limit, label_selector, _continue):
        return self._strategy.query(k8s_api, namespace, limit, label_selector, _continue)


class ResourceQuery:
    def __init__(self, k8s_api=None, resource_type=None, namespace=None, _continue=None,
                 limit=10, label_selector=None):
        self.k8s_api = k8s_api
        self.resource_type = resource_type
        self.namespace = namespace
        self.limit = limit
        self.label_selector = label_selector
        self._continue = _continue

    def get_k8s_resource_info(self):
        """
        通用函数，分页查询 Kubernetes 资源（如 PVC、Pod、Namespace 等），使用策略模式。
        :return: 返回所有查询到的资源信息列表
        """
        # 创建查询策略上下文
        if self.resource_type == 'pvc':
            strategy = PVCQueryStrategy()
        elif self.resource_type == 'pod':
            strategy = PodQueryStrategy()
        elif self.resource_type == 'namespace':
            strategy = NamespaceQueryStrategy()
        else:
            raise ValueError(f"Unsupported resource type: {self.resource_type}")

        context = ResourceQueryContext(strategy)

        all_resource_info = []

        # 执行查询
        resource_info = context.query(self.k8s_api, self.namespace, self.limit, self.label_selector, self._continue)

        # 将当前页的数据添加到结果集中
        all_resource_info.extend(resource_info.items)

        result = {
            'all_resource_info': all_resource_info,
            # 没有记录时，会返回None
            '_continue': resource_info.metadata._continue
        }
        return result
