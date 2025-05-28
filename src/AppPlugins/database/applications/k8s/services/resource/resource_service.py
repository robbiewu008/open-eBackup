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

from k8s.services.resource.resource_params import ResourceParam
from k8s.common.kubernetes_client.struct import WorkLoadType
from k8s.common.error_code import ErrorCode
from k8s.common.const import K8sJobKind, K8sType, K8sSubType, QueryClusterResponse
from common.const import ExecuteResultEnum
from common.exception.common_exception import ErrCodeException
from k8s.logger import log


class ResourceInfo:

    def __init__(self, pid, resource_param: ResourceParam):
        self.pid = pid
        self.param = resource_param

    @staticmethod
    def list_workload_pvc(namespace, pvc_name_list, pvc_resource):
        pvc_resource_items = []
        for pvc_name in pvc_name_list:
            try:
                pvc = pvc_resource.read(pvc_name, namespace)
                pvc_resource_items.append(pvc)
            except Exception:
                log.debug(f"Not found pvc:{pvc_name}!")
                pass
        return pvc_resource_items

    @staticmethod
    def generate_pvc_response(pvc_resource_items, total):
        log.debug("Start get_pvc_response!")
        response_pvc = {}
        resource_list = []
        for pvc_item in pvc_resource_items:
            resource_info = {}
            item = {
                "kind": "PersistentVolumeClaim",
                "capacity": pvc_item.status.capacity.get("storage")
            }
            resource_info.update({"name": pvc_item.metadata.name})
            resource_info.update({"extendInfo": item})
            resource_list.append(resource_info)
        response_pvc.update({"resourceList": resource_list})
        response_pvc.update({"total": total})
        response_pvc.update({"errorCode": ExecuteResultEnum.SUCCESS})
        return response_pvc

    @staticmethod
    def get_sts_list(sts_info, sts_name, pvc_num):
        pvc_name_list = []
        pvc_name = sts_info.metadata.name
        if pvc_name:
            if pvc_num == 1:
                pvc_name_list.append(pvc_name + "-" + sts_name)
                pvc_name_list.append(pvc_name + "-" + sts_name + "-0")
            else:
                sts_list = [(pvc_name + "-" + sts_name + "-" + str(i)) for i in range(pvc_num)]
                pvc_name_list.extend(sts_list)
        return pvc_name_list

    def do_check_cluster(self):
        client = self.param.get_resource_instance("Namespace")  # 可以使用ResourceNamespace来检查连通性
        if client is None:
            log.error(f"Get client failed! Req_id = {self.pid}")
            raise ErrCodeException(ErrorCode.AUTH_FAILED, message="Get client error!")
        api_list = []
        for api in client.k8s_client.get_prefer_versions():
            api_list.append(api)
        if len(api_list) < 2:  # 默认有CustomObjectsApi和VersionApi
            log.error(f"Get prefer_versions failed! Req_id = {self.pid}")
            raise ErrCodeException(ErrorCode.K8S_API_FAILED, message="Get prefer_versions error!")
        log.info(f"Action check_application Succeed! Req_id = {self.pid}")

    def do_query_cluster(self):
        """
        查询集群信息
        """
        log.info(f"Start do_query_cluster! ReqID:{self.pid}")
        resource_cluster = self.param.get_resource_instance("Cluster")
        query_response = QueryClusterResponse()
        query_response.cluster_type = K8sType.TYPE_CLUSTER
        query_response.sub_type = K8sSubType.SUBTYPE_CLUSTER
        query_response.extend_info = dict()
        version_info = resource_cluster.query_version()
        if not version_info.git_version:
            log.error("ERROR! Get no version info!")
            raise ErrCodeException(ErrorCode.K8S_API_FAILED, message="No version info")
        query_response.extend_info.update({"version": version_info.git_version})
        log.info(f"Action do_query_cluster succeed! Version={version_info.git_version}, req_id={self.pid}")
        return query_response

    def do_list_resource(self):
        log.info(f"Start do_list_resource, req_id={self.pid}")
        resource_kind = self.param.get_conditions_value().kind
        log.debug(f"Resource_kind={resource_kind}")
        if resource_kind == K8sJobKind.NAMESPACE:
            return self.list_all_namespace()
        elif resource_kind in K8sJobKind.WORKLOAD:
            return self.list_namespaced_workload()
        elif resource_kind == K8sJobKind.PVC:
            return self.list_pvc_resource()
        raise ErrCodeException(ErrorCode.PARAM_FAILED, message="Param Info Error!")

    def list_all_namespace(self):
        """
        查询集群命名空间信息
        """
        log.info(f"Start list_all_namespace! ReqID:{self.pid}")
        resource_namespace = self.param.get_resource_instance("Namespace")
        page_start, page_size, pages_no = self.param.get_pages_info()
        limit = page_size * (pages_no + 1)
        pre_num = limit - page_size
        field, label = self.param.get_selector_info()
        log.debug(f"Param is {limit},{field},{label}")
        namespace_info = resource_namespace.limitlist(limit, field, label)
        namespace_response = self.generate_response_from_info(namespace_info, page_start,
                                                              pre_num, K8sJobKind.NAMESPACE.value)
        log.info(f"Action list_all_namespace Succeed! req_id: {self.pid}")
        return namespace_response

    def list_namespaced_workload(self):
        """
        查询集群命名空间下Workload信息
        """
        log.info(f"Start list_namespaced_workload! ReqID:{self.pid}")
        conditions_value = self.param.get_conditions_value()
        workload_type = conditions_value.kind
        log.debug(f"WorkloadType: {workload_type}")
        if workload_type == WorkLoadType.POD.value:
            log.debug("Get Pod！")
            resource_workload = self.param.get_resource_instance("Pod")
        else:
            log.debug("Get Workload！")
            resource_workload = self.param.get_workload_resource_instance(conditions_value.kind)
        page_start, page_size, pages_no = self.param.get_pages_info()
        limit = page_size * (pages_no + 1)
        pre_num = limit - page_size
        field, label = self.param.get_selector_info()
        namespace = conditions_value.params_for_workload
        log.debug(f"namespace={namespace}")
        workload_info = resource_workload.limitlist(namespace, limit, field, label)
        workload_response = self.generate_response_from_info(workload_info, page_start, pre_num, workload_type)
        log.info(f"Action list_namespaced_workload end! req_id: {self.pid}")
        return workload_response

    def list_pvc_resource(self):
        """
        查询指定namespace或workload下的PVC信息
        """
        log.info(f"Start list_pvc_resource! ReqID:{self.pid}")
        pvc_resource = self.param.get_resource_instance("PersistentVolumeClaim")
        params_for_pvc = self.param.get_conditions_value().params_for_pvc
        page_start, page_size, pages_no = self.param.get_pages_info()
        limit = page_size * (pages_no + 1)
        pre_num = limit - page_size
        field, label = self.param.get_selector_info()
        namespace = params_for_pvc.get("namespace", "")
        log.debug(f"Namespace is {namespace}")
        if namespace == "":
            log.debug("Get all pvc!")
            pvc_info = pvc_resource.limitlist_for_all_namespace(limit, field, label)
            response_pvc = self.generate_response_from_info(pvc_info, page_start, pre_num, K8sJobKind.PVC.value)
        elif (field is not None) or (label is not None) or params_for_pvc.get("workload", "") == "":
            log.debug(f"Get namespace pvc for {namespace}")
            pvc_info = pvc_resource.limitlist(namespace, limit, field, label)
            response_pvc = self.generate_response_from_info(pvc_info, page_start, pre_num, K8sJobKind.PVC.value)
        else:
            workload = params_for_pvc.get("workload")
            kind = params_for_pvc.get("workload_kind")
            log.debug(f"Get workload:{workload}, kind:{kind}")
            pvc_name_list = self.get_workload_pvc(kind, workload, namespace)
            log.debug(f"Pvc_list:{pvc_name_list}")
            pvc_resource_items = self.list_workload_pvc(namespace, pvc_name_list, pvc_resource)
            total = len(pvc_resource_items)
            page_end = total if total < (page_start + page_size) else (page_start + page_size)
            response_pvc = self.generate_pvc_response(pvc_resource_items[page_start:page_end], total)
        log.info(f"list_pvc_resource end! req_id: {self.pid}")
        return response_pvc

    def get_workload_pvc(self, kind, workload, namespace):
        log.debug(f"Start get_workload_pvc! Workload:{workload}, namespace:{namespace}")
        if kind == WorkLoadType.POD.value:
            log.debug("Pod pvc list!")
            pvc_name_list = []
            resource_pod = self.param.get_resource_instance("Pod")
            pod_pvc = resource_pod.read(workload, namespace)
            for pvc in pod_pvc.spec.volumes:
                if pvc.persistent_volume_claim:
                    pvc_name_list.append(pvc.persistent_volume_claim.claim_name)
            return pvc_name_list
        resource_workload = self.param.get_workload_resource_instance(kind)
        workload_pvc = resource_workload.read(workload, namespace)
        if kind == WorkLoadType.STATEFULSET.value:
            if workload_pvc.spec.volume_claim_templates is not None:
                log.debug("Sts pv list!")
                pvc_name_list = []
                pvc_num = workload_pvc.status.current_replicas
                for pvc in workload_pvc.spec.volume_claim_templates:
                    pvc_name_list.extend(self.get_sts_list(pvc, workload, pvc_num))
            else:
                log.error(f"Sts has no pvc! Sts name:{workload_pvc.metadata.name}")
                pvc_name_list = []
        else:
            log.debug("Workload pvc list!")
            pvc_name_list = []
            for pvc in workload_pvc.spec.template.spec.volumes:
                if pvc.persistent_volume_claim is None:
                    log.error(f"Workload volumes have no pvc! Workload volumes name:{pvc.name}")
                    continue
                if pvc.persistent_volume_claim:
                    pvc_name_list.append(pvc.persistent_volume_claim.claim_name)
        return pvc_name_list

    def generate_response_from_info(self, info, start, pre_num, resource_type):
        log.info(f"Start get_response, req_id:{self.pid}")
        resource_list = []
        if info.items is None:
            log.error(f"Get response failed: No items! Pid={self.pid}")
            raise ErrCodeException(ErrorCode.K8S_API_FAILED, message="No items return!")
        num = len(info.items)
        if start > num:
            log.error(f"No items left! Items num = {num}, start at {start}")
            raise ErrCodeException(ErrorCode.OPERATION_FAILED, message="No more items!")
        if info.metadata.remaining_item_count is None:
            total = num + pre_num
        else:
            total = num + pre_num + info.metadata.remaining_item_count
        log.debug(f"Total:{total}")
        for resource_item in info.items[start:]:
            resource_info = {}
            if resource_item.metadata.uid == "" or resource_item.metadata.name == "":
                log.error(f"Get metadata failed! Req_id={self.pid}")
                raise ErrCodeException(ErrorCode.K8S_API_FAILED, message="Get metadata error!")
            item = {
                "kind": resource_type
            }
            if resource_type == K8sJobKind.PVC:
                item.update({"capacity": resource_item.status.capacity.get("storage")})
            resource_info.update({"name": resource_item.metadata.name})
            resource_info.update({"extendInfo": item})
            resource_list.append(resource_info)
        response = {"resourceList": resource_list, "total": total, "errorCode": ExecuteResultEnum.SUCCESS}
        log.debug(f"Response:{response}")
        return response
