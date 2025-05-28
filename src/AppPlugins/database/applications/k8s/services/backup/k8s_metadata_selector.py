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

from kubernetes.client import V1APIResource

from k8s.common.const import PREFIX, VOLUME_SNAPSHOT_CLASS_PLURAL
from k8s.common.k8s_manager.pvc_manager import PvcManager
from k8s.common.label_const import LabelConst
from k8s.logger import log
from k8s.common.kubernetes_client.k8s_api import ClientCenter
from k8s.common.kubernetes_client.k8s_core_api.resource_core_api import ResourceCoreApi
from k8s.common.kubernetes_client.k8s_api_class_custom_objects import \
    ResourceCustomObjects
from k8s.common.kubernetes_client.struct import ClusterAuthentication, ResourceInfo
from kubernetes import client
from kubernetes.client.rest import ApiException

ITEMS = 'items'

PATTERN = re.compile(r"(?<!^)(?=[A-Z])")


def get_resources_for_api(prefer_version, k8s_client):
    api_resources = None
    api_base_class = None
    try:
        api_base_class = getattr(client, prefer_version)
        api_resources = api_base_class(api_client=k8s_client).get_api_resources()
    except Exception as e:
        log.exception(
            f"Failed to call get_api_resources for: {prefer_version}, exception: {str(e)}"
            "This error may occur for new APIs and Custom Resources APIs."
        )
    return api_resources, api_base_class


def get_crd_def_resources(api_base_class, label_filter, k8s_client):
    v1beta1_crd_items = []
    method_name = "list_custom_resource_definition"
    try:
        list_cluster_crd = getattr(api_base_class(api_client=k8s_client), method_name)
        cluster_crd_items = list_cluster_crd(label_selector=label_filter).items

        if isinstance(api_base_class(api_client=k8s_client), client.ApiextensionsV1Api):
            # collect crds also in beta1 format due to schema changes between the v1 & beta1 api versions:
            # https://github.com/kubernetes/kubernetes/issues/87231
            # https://github.com/vmware-tanzu/velero/issues/2249
            log.info(
                "adding to backup the v1beta1 format of the custom resource definitions"
            )
            list_v1beta1_crd = getattr(
                client.ApiextensionsV1beta1Api(api_client=k8s_client), method_name
            )
            v1beta1_crd_items = list_v1beta1_crd(label_selector=label_filter).items
    except (ApiException, AttributeError) as e:
        log.exception(
            f"Failed to get list_custom_resource_definition of: {type(api_base_class(api_client=k8s_client))},"
            f" exception: {str(e)}"
        )
        if 'cluster_crd_items' in locals():
            return cluster_crd_items, []
        return [], []
    return cluster_crd_items, v1beta1_crd_items


def verify_resource_verbs_for_backup_and_restore(resource):
    if ("list" not in resource.verbs) or ("create" not in resource.verbs):
        log.error(
            f"Resource verbs not correct: {resource.name} {resource.verbs}"
        )
        return False
    return True


def remove_controlled_items(item_list):
    return [
        local_item
        for local_item in item_list
        if not local_item['metadata'].get('ownerReferences', []) or not
        local_item['metadata'].get('ownerReferences', [])[0].get('controller', False)
    ]


def handle_cluster_crd_resource(list_obj, plural):
    if plural == VOLUME_SNAPSHOT_CLASS_PLURAL:
        for _ in range(len(list_obj['items'])):
            item = list_obj['items'].pop(0)
            if PREFIX.VOLUME_SNAPSHOT_CLASS_NAME_PREFIX not in item.get('metadata').get('name'):
                list_obj['items'].append(item)
    if plural == 'volumesnapshotcontents':
        for _ in range(len(list_obj['items'])):
            item = list_obj['items'].pop(0)
            if PREFIX.VOLUME_SNAPSHOT_NAME_PREFIX not in item.get('spec').get('volumeSnapshotRef').get('name'):
                list_obj['items'].append(item)
    if 'items' not in list_obj:
        list_obj['items'] = []
    return list_obj


def remove_satus_of_resource(items):
    return [
        {**local_item, 'status': {}}  # 在循环体内赋值
        for local_item in items
    ]


def get_custom_object(api_instance, api_ver_list, resource: dict, label_filter, ns_to_backup):
    res = V1APIResource(kind='', name='', namespaced=False, singular_name='', verbs=[])
    plural = resource.get('resource', res).name
    api = resource.get('group', '')
    ver = api_ver_list[api]
    kind = resource.get('resource', res).kind
    namespaced = resource.get('resource', res).namespaced
    if not verify_resource_verbs_for_backup_and_restore(resource.get('resource', res)):
        return ResourceInfo(namespace='', groups='', version='v1', plural=plural, kind=kind,
                            items=[])
    exclude_label = f"{LabelConst.DPA_BACKUP_POD_COMMON_KEY}!={LabelConst.DPA_BACKUP_POD_COMMON_VALUE}"
    if label_filter:
        label_filter += ','
    label_filter += exclude_label
    log.debug(f"{api} {ver} {plural} {namespaced}")
    try:
        if namespaced and ns_to_backup != '':
            list_obj = api_instance.list_namespaced_custom_object(api, ver, ns_to_backup, plural,
                                                                  label_selector=label_filter)
            single_results = ResourceInfo(namespace=ns_to_backup, groups=api, version=ver, plural=plural,
                                          kind=kind, items=list_obj["items"])
        else:
            list_obj = api_instance.list_cluster_custom_object(api, ver, plural)
            list_obj = handle_cluster_crd_resource(list_obj, plural)
            single_results = ResourceInfo(namespace='', groups=api, version=ver, plural=plural,
                                          kind=kind, items=list_obj["items"])
        if not list_obj["items"]:
            log.warning(f'Found nothing with {api}, {ver}, {ns_to_backup}, {plural}, {label_filter}')
        single_results.items = remove_controlled_items(single_results.items)
        single_results.items = remove_satus_of_resource(single_results.items)
        return single_results
    except ApiException as e:
        log.exception(f"No namespace level crd for:{api} {ver} {ns_to_backup} {plural} {label_filter} {str(e)}")
        return ResourceInfo(namespace='', groups=api, version=ver, plural=plural, kind=kind, items=[])


def get_crd_objects(resource_def_list, api_ver_list, ns_to_backup, label_filter, k8s_client):
    api_instance = client.CustomObjectsApi(api_client=k8s_client)
    crd_obj_list = []
    for resource in resource_def_list:
        obj = get_custom_object(api_instance, api_ver_list, resource, label_filter, ns_to_backup)
        if obj.items:
            crd_obj_list.append(obj)
    return crd_obj_list


def get_non_crd_objects(resource, resource_api, backuped_namespace, label_selector, cluster_authentication):
    api_instance = ResourceCustomObjects(cluster_authentication)
    api = resource_api[0]
    ver = resource_api[1]
    plural = resource.name
    namespaced = resource.namespaced
    if not verify_resource_verbs_for_backup_and_restore(resource):
        return ResourceInfo(namespace='', groups=api, version=ver, plural=plural, kind=resource.kind, items=[])
    log.debug(f"{api} {ver} {plural} {namespaced} {backuped_namespace}")
    try:
        if namespaced and backuped_namespace:
            list_obj = api_instance.list_namespaced_custom_object(api, ver, backuped_namespace, plural,
                                                                  label_selector=label_selector)
            single_results = ResourceInfo(namespace=backuped_namespace, groups=api, version=ver, plural=plural,
                                          kind=resource.kind, items=list_obj["items"])
        else:
            list_obj = api_instance.list_cluster_custom_object(api, ver, plural)
            single_results = ResourceInfo(namespace='', groups=api, version=ver, plural=plural,
                                          kind=resource.kind, items=list_obj["items"])
        if not list_obj["items"]:
            log.warning(f'Found nothing with {api}, {ver}, {backuped_namespace}, {plural}, {label_selector}')
        single_results.items = remove_controlled_items(single_results.items)
        single_results.items = remove_satus_of_resource(single_results.items)
        return single_results
    except ApiException as e:
        log.exception(f"No namespace level crd for:{api} {ver} {backuped_namespace} {plural} {label_selector} {str(e)}")
        return ResourceInfo(namespace='', groups=api, version=ver, plural=plural, kind=resource.kind, items=[])


def get_core_objects(resource, backuped_namespace, label_selector, k8s_auth_info):
    core_dap_api = ResourceCoreApi(k8s_auth_info)
    if not verify_resource_verbs_for_backup_and_restore(resource):
        return ResourceInfo(namespace='', groups='', version='v1', plural=resource.name, kind=resource.kind,
                            items=[])
    databackup_core_resource = ['pods']
    cluster_core_resource = ['namespaces', 'nodes', 'persistentvolumes']
    try:
        if resource.name == 'persistentvolumeclaims':
            return ResourceInfo(namespace=backuped_namespace, groups='', version='v1', plural=resource.name,
                                kind=resource.kind, items=[])
        elif resource.name in cluster_core_resource:
            list_obj = core_dap_api.list_cluster_resource_with_http_info(resource.name)
            list_obj[ITEMS] = remove_satus_of_resource(list_obj[ITEMS])
            return ResourceInfo(namespace='', groups='', version='v1', plural=resource.name,
                                kind=resource.kind, items=list_obj[ITEMS])
        elif resource.name in databackup_core_resource:
            if label_selector:
                label_selector += ','
            exclude_label = f"{LabelConst.DPA_BACKUP_POD_COMMON_KEY}!={LabelConst.DPA_BACKUP_POD_COMMON_VALUE}"
            label_selector += exclude_label
            list_obj = core_dap_api.list_namespaced_resource_with_http_info(resource.name, backuped_namespace,
                                                                            label_selector=label_selector)
        else:
            list_obj = core_dap_api.list_namespaced_resource_with_http_info(resource.name, backuped_namespace,
                                                                            label_selector=label_selector)
        list_obj[ITEMS] = remove_controlled_items(list_obj[ITEMS])
        list_obj[ITEMS] = remove_satus_of_resource(list_obj[ITEMS])
        return ResourceInfo(namespace=backuped_namespace, groups='', version='v1', plural=resource.name,
                            kind=resource.kind, items=list_obj[ITEMS])
    except Exception as e:
        log.warning(f'Not found resource:{resource.name}, response body:{e}')
        return ResourceInfo(namespace=backuped_namespace, groups='', version='v1', plural=resource.name,
                            kind=resource.kind, items=[])


def get_all_resources_with_label(k8s_auth_info: ClusterAuthentication, label_selector: str, backuped_namespace: str,
                                 task_id: str):
    log.info(f'{label_selector} {backuped_namespace}')
    k8s_client = ClientCenter().get_client(k8s_auth_info)
    preferred_api_versions = k8s_client.get_prefer_versions()
    crd_api_resources = k8s_client.get_custom_api_name_map()
    api_groups_maps = k8s_client.get_api_groups_map()
    non_crd_resources = []
    crd_resources = []
    log.debug(f'Get prefer version:{preferred_api_versions}')
    # 查询数据集的pvc信息
    pvc_manager = PvcManager(cluster_authentication=k8s_auth_info,
                             namespace=backuped_namespace, labels=label_selector,
                             task_id=task_id)
    pvc_list = pvc_manager.get_dataset_by_label_and_group_by_type()
    log.info(f"Get pvc info success, namespace is {backuped_namespace}, pvc num is {len(pvc_list)}")
    pvc_results = ResourceInfo(namespace=backuped_namespace, groups='', version='v1',
                               plural='persistentvolumeclaims',
                               kind='PersistentVolumeClaim', items=pvc_list)
    non_crd_resources.append(pvc_results if pvc_results.items != [] else None)
    for prefer_version in api_groups_maps.keys():
        log.debug(f"Get_resources_for_api {prefer_version}")
        api_resources, api_base_class = get_resources_for_api(prefer_version, k8s_client.get_client())
        # 查找core资源（无group资源）
        if prefer_version == "CoreV1Api":
            for resource in api_resources.resources:
                log.debug(f"Resource name:{resource.name}")
                core_info = get_core_objects(resource, backuped_namespace, label_selector, k8s_auth_info)
                non_crd_resources.append(core_info if core_info.items != [] else None)
            continue
        if (not api_resources) or (not api_base_class):
            log.warning(f'{prefer_version} {api_base_class} abnormal')
            continue
    crd_resources_list = get_all_objects(k8s_auth_info=k8s_auth_info, namespace=backuped_namespace)
    # 查询CRD类型资源
    if crd_api_resources and crd_resources_list:
        crd_resources = get_crd_objects(crd_resources_list, crd_api_resources, backuped_namespace, label_selector,
                                        k8s_client.get_client())
    non_crd_resources = [x for x in non_crd_resources if x is not None]
    return crd_resources, non_crd_resources


def get_all_objects(k8s_auth_info: ClusterAuthentication, namespace):
    k8s_client = ClientCenter().get_client(k8s_auth_info)
    crd_api_resources = k8s_client.get_custom_api_name_map()
    rco = ResourceCustomObjects(k8s_auth_info)
    result = []
    for group, ver in crd_api_resources.items():
        try:
            resources = rco.get_api_resources(group, ver).resources
            base_param = {
                'group': group,
                'ver': ver,
                'namespace': namespace
            }
            append_exist_resource(base_param, rco, resources, result)
        except ApiException as ex:
            if ex.status == 404:
                log.info(f"Resources of group{group}:ver{ver} does not exist, continue.")
        except ValueError as ex:
            if ex.args and ex.args[0] == 'Invalid value for `group_version`, must not be `None`':
                log.info(f"Resources of group{group}:ver{ver} does not exist, continue.")
    return result


def append_exist_resource(base_param: dict, rco: ResourceCustomObjects, resources: [], result: []):
    group = base_param.get('group', '')
    namespace = base_param.get('namespace', '')
    ver = base_param.get('ver', '')
    for resource in resources:
        plural = resource.name
        namespaced = resource.namespaced
        try:
            if namespaced:
                rco.list_namespaced_custom_object(group=group, version=ver, namespace=namespace, plural=plural)
            else:
                rco.list_without_namespace(group=group, version=ver, plural=plural)
        except ApiException as ex:
            if ex.status == 404:
                log.info(f"Resources of group{group}:ver{ver}:plural{plural} does not exist, continue.")
                continue
        resource.version = ver
        res = {
            'resource': resource,
            'group': group
        }
        result.append(res)
