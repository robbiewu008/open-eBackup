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

from common.exception.common_exception import ErrCodeException
from k8s.common.error_code import ErrorCode
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.resource_manager.resource_query import ResourceQuery
from k8s.logger import log


class ClusterChecker:
    def __init__(self, kubernetes_back_info=None):
        self.kubernetes_back_info = kubernetes_back_info

    def before_backup_check(self):
        try:
            # 检查node selector标签选择的node是否异常,根据node的type为Ready,并且status不为True,则node异常,返回False;
            # 如果node selector标签未找到任何node,检查不通过。返回False
            if not self.check_node_status():
                return ErrCodeException(ErrorCode.NODE_STATUS_IS_ABNORMAL, 'Backup', "Backup pre check failed")
        except Exception as err:
            log.exception(f'Before backup check failed! task id:{self.kubernetes_back_info.task_id}.Err:{err}')
            return ErrCodeException(ErrorCode.NODE_STATUS_IS_ABNORMAL, 'Backup', "Backup pre check failed")
        try:
            # 如果pvc的访问模式为ReadWriteOnce，并且pvc绑定的pod不在node selector选中的节点中，检查失败,返回False
            # 没有node_selector则不检查,返回True
            if not self.check_read_write_once_pvc_bound_pod_is_on_selector_node():
                return ErrCodeException(ErrorCode.READ_WRITE_ONCE_PVC_BOUND_POD_IS_NOT_ON_SELECTOR_NODE, 'Backup',
                                        "Backup pre check failed")
        except Exception as err:
            log.exception(f'Before backup check failed! task id:{self.kubernetes_back_info.task_id}.Err:{err}')
            return ErrCodeException(ErrorCode.READ_WRITE_ONCE_PVC_BOUND_POD_IS_NOT_ON_SELECTOR_NODE, 'Backup',
                                    "Backup pre check failed")
        try:
            # 检查pvc的状态，若状态不为bound，检查失败返回false
            return self.check_pvc_status()
        except Exception as err:
            log.exception(f'Before backup check failed! task id:{self.kubernetes_back_info.task_id}.Err:{err}')
            return ErrCodeException(ErrorCode.PVC_STATUS_INVALID, '', message="Backup pre check failed")

    def check_node_status(self):
        cluster_authentication = self.kubernetes_back_info.resource.cluster_authentication
        rc_api = ResourceCoreApi(cluster_authentication)
        node_selector = self.kubernetes_back_info.node_selector
        node_res = rc_api.list_cluster_resource_with_http_info('nodes', label_selector=node_selector)
        log.info(f"Start check node status, node_selector is {node_selector},"
                 f" task id is {self.kubernetes_back_info.task_id}")
        if not node_res['items']:
            log.error(f"Node num is {len(node_res['items'])}, task id {self.kubernetes_back_info.task_id}")
            return False
        for node in node_res['items']:
            for condition in node['status']['conditions']:
                if (condition['type']) == 'Ready' and 'True' != (condition['status']):
                    log.error(f"Node is {node['metadata']['name']}, task id {self.kubernetes_back_info.task_id}")
                    return False
        return True

    def check_read_write_once_pvc_bound_pod_is_on_selector_node(self):
        cluster_authentication = self.kubernetes_back_info.resource.cluster_authentication
        pod_api = ResourcePod(cluster_authentication)
        node_selector = self.kubernetes_back_info.node_selector
        namespace = self.kubernetes_back_info.resource.namespace.name
        log.info(
            f"Start check ReadWriteOnce pvc bound pod is on selector select node , node_selector is {node_selector},"
            f"task id is {self.kubernetes_back_info.task_id}")
        if node_selector == '':
            return True
        pod_res = pod_api.list(namespace)
        for pod in pod_res.items:
            if not self.check_pvc_in_pod_status(pod=pod):
                return False
        return True

    def check_pvc_in_pod_status(self, pod):
        cluster_authentication = self.kubernetes_back_info.resource.cluster_authentication
        k8s_pvc_api = ResourcePersistentVolumeClaim(cluster_authentication)
        namespace = self.kubernetes_back_info.resource.namespace.name
        node_selector = self.kubernetes_back_info.node_selector
        rc_api = ResourceCoreApi(cluster_authentication)
        node_res = rc_api.list_cluster_resource_with_http_info('nodes', label_selector=node_selector)
        node_name_list = []
        for node in node_res['items']:
            node_name_list.append(node['metadata']['name'])
        for volume in pod.spec.volumes:
            if volume.persistent_volume_claim:
                pvc_name = volume.persistent_volume_claim.claim_name
                # 根据命名空间和属性选择器查询唯一的pvc
                pvc_info = k8s_pvc_api.read(name=pvc_name, namespace=namespace)
                access_mode = pvc_info.spec.access_modes[0]
                # 如果pvc的访问模式为ReadWriteOnce，并且pvc绑定的pod不在node selector选中的节点中，检查失败,返回False
                if access_mode == 'ReadWriteOnce' and pod.spec.node_name not in node_name_list:
                    log.error(f"Pvc name is {pvc_name}, pod is {pod.metadata.name}, node list is {node_name_list}, "
                              f"task id is {self.kubernetes_back_info.task_id}")
                    return False
        return True

    def check_pvc_status(self):
        cluster_authentication = self.kubernetes_back_info.resource.cluster_authentication
        k8s_pvc_api = ResourcePersistentVolumeClaim(cluster_authentication)
        namespace = self.kubernetes_back_info.resource.namespace.name
        if not self.kubernetes_back_info.resource.dataset.labels:
            label_selector = ''
        else:
            label_selector = self.kubernetes_back_info.resource.dataset.labels
        error_pvc_list = get_all_pvc_info(k8s_pvc_api, namespace, 10, label_selector)
        if error_pvc_list:
            log.exception(f'Before backup check failed! task id:{self.kubernetes_back_info.task_id}. '
                          f'error_pvc_list:{error_pvc_list}')
            return ErrCodeException(ErrorCode.PVC_STATUS_INVALID, ",".join(map(str, error_pvc_list)),
                                    message="Backup pre check failed")
        return ErrCodeException(ErrorCode.SUCCESS)


def get_all_pvc_info(k8s_pvc_api, namespace, limit=10, label_selector=None):
    _continue = ''
    result = []
    while True:
        # 查询当前页的 PVC 信息
        query = ResourceQuery(k8s_pvc_api, resource_type='pvc', namespace=namespace, _continue=_continue,
                              limit=limit, label_selector=label_selector)
        pvc_info_dict = query.get_k8s_resource_info()
        pvc_list = pvc_info_dict.get('all_resource_info')
        _continue = pvc_info_dict.get('_continue')
        for pvc in pvc_list:
            if pvc.status and pvc.status.phase != 'Bound':
                result.append(pvc.metadata.name)
        if not _continue:
            break
    return result
