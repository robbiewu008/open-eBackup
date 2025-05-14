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
import json
from typing import List

from fastapi import Query, Path
from fastapi.params import Body

from app.common.auth.check_ath import CheckAuthModel
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.common.util.cleaner import clear
from app.resource.common.constants import VMWareStorageConstants
from app.resource.models.virtual_res_models import VirtualResourceTable  # noqa
from app.resource.schemas.env_schemas import EnvironmentSchema, ScanEnvSchema, StorageInfo, StorageIp
from app.resource.schemas.virtual_resource_schemas import ClusterConfigSchema, VirtualResourceSchema, \
    VirtualDiskDetailSchema, FolderSchema, VMwareSchema, RdmProductInitiatorDetailSchema
from app.resource.service.common import resource_service
from app.resource.service.vmware.service_instance_manager import service_instance_manager
from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
from app.common.concurrency import async_route

vmware_api = async_route()


@vmware_api.get("/virtual-machines/{vm_uuid}/disks",
                status_code=200,
                summary="查询VMware虚拟机的硬盘列表",
                description="根据VMware虚拟机ID查询VMware虚拟机的硬盘列表",
                response_model=List[VirtualDiskDetailSchema])
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:vm_uuid",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="vm_uuid")
)
def list_vm_disk(vm_uuid: str = Path(..., description="VMware虚拟机ID", min_length=1, max_length=64)):
    """
    查询虚拟机的磁盘列表
    :param vm_uuid: 虚拟机的UUID
    :return: 返回虚拟机的磁盘列表
    """
    res, root_rs = _get_resource_by_res_id(vm_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    disks = service.get_vm_disk(res)
    clear(env.password)
    return list(VirtualDiskDetailSchema(**disk) for disk in disks)


@vmware_api.get("/virtual-machines/{vm_uuid}",
                status_code=200,
                summary="查询VMware虚拟机的详细信息",
                description="根据VMware虚拟机ID查询VMware虚拟机的详细信息",
                response_model=VMwareSchema)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:vm_uuid",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="vm_uuid")
)
def get_vm_info(vm_uuid: str = Path(..., description="VMware虚拟机ID", min_length=1, max_length=64)):
    """
    查询VMware详细信息
    :param vm_uuid: 虚拟机的UUID
    :return: 返回VMware详细信息
    """
    return _get_vm_info(vm_uuid)


@vmware_api.get("/compute-resources/{compute_res_uuid}/networks",
                summary="查询计算资源的网络列表",
                description="根据计算资源ID查询计算资源的网络列表",
                status_code=200,
                response_model=List[VirtualResourceSchema])
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:compute_res_uuid",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="compute_res_uuid")
)
def list_compute_res_network(
        compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64)):
    """
    查询计算资源网络信息
    :param compute_res_uuid:计算资源的UUID
    :return: 返回计算资源的网络列表
    """
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    networks = service.get_networks_by_compute_res_name(res.name)
    clear(env.password)
    if not networks:
        return []
    else:
        return list(VirtualResourceSchema(**network) for network in networks)


@vmware_api.get("/compute-resources/{compute_res_uuid}/datastores",
                status_code=200,
                summary="查询计算资源的数据存储列表",
                description="根据计算资源ID查询计算资源的数据存储列表",
                response_model=List[VirtualResourceSchema])
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:compute_res_uuid",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="compute_res_uuid")
)
def list_compute_res_datastore(
        compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64)):
    """
    查询计算资源数据存储信息
    :param compute_res_uuid:计算资源的UUID
    :return: 返回计算资源的数据存储列表
    """
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    datastores = service.get_datastore_by_compute_res(res.name)
    clear(env.password)
    if not datastores:
        return []
    else:
        return list(VirtualResourceSchema(**ds) for ds in datastores)


@vmware_api.get("/compute-resources/clusters/{cluster_uuid}/config",
                status_code=200,
                summary="查询集群计算资源的配置",
                description="根据集群计算资源ID查询集群计算资源的配置",
                response_model=ClusterConfigSchema)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:cluster_uuid",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="cluster_uuid")
)
def get_cluster_config(
        cluster_uuid: str = Path(..., description="计算资源集群ID", min_length=1, max_length=64)):
    """
    查询指定集群计算资源的配置
    :param compute_res_uuid:集群计算资源的UUID
    :return: 返回配置信息
    """
    return _get_cluster_config_by_id(cluster_uuid)


@vmware_api.get("/free-effective-capacity",
                status_code=200,
                summary="查询RDM存储的剩余容量【内部接口】",
                description="查询RDM存储的剩余容量")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:env_id")
def get_free_effective_capacity(storage_ip: str = Query(None, description="存储ip"),
                                env_id: str = Query(None, description="vcenter的uuid")):
    extend_infos = resource_service.query_res_extend_info_by_resource_id(env_id)
    return VMwareDiscoveryService.get_free_effective_capacity(extend_infos, storage_ip)


@vmware_api.get("/v-center/register/storages/{env_id}",
                status_code=200,
                summary="查询v-center的注册存储信息",
                description="查询v-center的注册存储信息",
                response_model=List[StorageIp])
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:env_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="env_id")
)
def get_register_storages(env_id: str = Path(..., description="vcenter的uuid")):
    extend_infos = resource_service.query_res_extend_info_by_resource_id(env_id)
    storages = []
    if not extend_infos:
        return storages
    for extend_info in extend_infos:
        if extend_info.key == VMWareStorageConstants.STORAGES:
            storages = extend_info.value
    if not storages:
        return storages
    storages = json.loads(storages)
    ip_result = []
    for storage in storages:
        ip_list = storage.get("ip")
        for ip_info in ip_list:
            ip_result.append(StorageIp(ip=ip_info))
    return ip_result


@vmware_api.get("/internal/virtual-machines/{vm_uuid}",
                status_code=200,
                summary="查询VMware虚拟机的详细信息【内部接口】",
                response_model=VMwareSchema)
def internal_get_vm_info(vm_uuid: str = Path(..., description="VMware虚拟机ID", min_length=1, max_length=64)):
    return _get_vm_info(vm_uuid)


def _get_vm_info(vm_uuid):
    res, root_rs = _get_resource_by_res_id(vm_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    vm_info = service.get_vm_info(res)
    vm_info.update(uuid=vm_uuid)
    clear(env.password)
    return VMwareSchema(**vm_info)


@vmware_api.get("/internal/compute-resources/{compute_res_uuid}/folder",
                status_code=200,
                summary="查询计算资源的虚拟文件夹信息【内部接口】",
                description="根据计算资源ID查询计算资源的虚拟文件夹信息",
                response_model=FolderSchema)
def get_folder_info(compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64)):
    """
    查询虚拟机的虚拟文件夹信息
    :param vm_uuid: 虚拟机的UUID
    :return: 返回虚拟文件夹信息
    """
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    folder = service.get_folder_by_compute_res_name(res.name)
    clear(env.password)
    return FolderSchema(**folder)


@vmware_api.get("/internal/vmbackupagent/",
                status_code=200,
                summary="查询第一个VMware备份代理主机信息【内部接口】",
                description="查询第一个VMware备份代理主机信息",
                response_model=EnvironmentSchema)
def get_agent():
    """
    临时代码下个版本删除这个版本适配数据面查询vmware的agent并取第一个
    :return: 返回agent
    """
    result = resource_service.query_agent_info()
    return result


@vmware_api.get("/internal/virtual-machines/{vm_uuid}/disks",
                status_code=200,
                summary="查询VMware虚拟机的硬盘列表【内部接口】",
                description="根据VMware虚拟机ID查询VMware虚拟机的硬盘列表",
                response_model=List[VirtualDiskDetailSchema])
def list_vm_disk_details(
        vm_uuid: str = Path(..., description="VMware虚拟机ID")):
    """
    查询虚拟机的磁盘详情
    :param vm_uuid: 虚拟机的UUID
    :return: 返回虚拟机的磁盘详情
    """
    res, root_rs = _get_resource_by_res_id(vm_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    disks = service.get_vm_disk(res)
    # 校验是否包含共享类型为多写入器的磁盘（共享盘）
    service.check_disk_sharing_type_is_multi_writer(res)
    clear(env.password)
    return list(VirtualDiskDetailSchema(**disk) for disk in disks)


@vmware_api.get("/internal/virtual-machines/{vm_uuid}/config-file-datastores-name",
                status_code=200,
                summary="查询VMware虚拟机的配置文件所在存储的名称【内部接口】",
                description="根据VMware虚拟机ID查询虚拟机配置文件所在存储的名称",
                response_model=str)
def list_vm_config_file_datastore_name(
        vm_uuid: str = Path(..., description="VMware虚拟机ID")):
    """
    查询VMware虚拟机的配置文件所在存储的名称
    :param vm_uuid: 虚拟机的UUID
    :return: 返回存储所在名称
    """
    res, root_rs = _get_resource_by_res_id(vm_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    datastore_name = service.get_vm_config_file_datastore_name(res)
    clear(env.password)
    return datastore_name


@vmware_api.get(
    "/internal/virtual-machines/{compute_res_uuid}/product-initiator/{vm_uuid}/{datastore_name}/{host_mo_ref}",
    status_code=200,
    summary="查询VMware虚拟机的RDM硬盘Product Initiator列表【内部接口】",
    description="根据VMware虚拟机ID查询VMware虚拟机RDM硬盘的Product Initiator列表",
    response_model=List[RdmProductInitiatorDetailSchema])
def list_rdm_product_initiator_details(
        compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64),
        vm_uuid: str = Path(..., description="VMware虚拟机ID"),
        datastore_name: str = Path(..., description="datastore名称"),
        host_mo_ref: str = Path(..., description="host名称")):
    """
    查询虚拟机的RDM磁盘启动器信息详情
    :param vm_uuid: 已废弃参数
    :param compute_res_uuid: 计算资源的UUID
    :param datastore_name: datastore名称
    :param host_mo_ref: host名称
    :return: 返回虚拟机的RDM磁盘启动器信息详情
    """
    compute_res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        compute_res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    product_initiators = service.get_product_initiators(compute_res.name, datastore_name, host_mo_ref)
    list_pi = list(RdmProductInitiatorDetailSchema(**product_initiator) for product_initiator in product_initiators)
    clear(env.password)
    return list_pi


@vmware_api.get("/internal/virtual-machines/{compute_res_uuid}/product-initiator-new-location/{host_mo_ref}",
                status_code=200,
                summary="恢复到新位置时，查询VMware虚拟机的RDM硬盘Product Initiator列表【内部接口】",
                description="恢复到新位置时，查询VMware虚拟机RDM硬盘的Product Initiator列表",
                response_model=List[RdmProductInitiatorDetailSchema])
def list_rdm_product_initiator_details_for_new_location(
        compute_res_uuid: str = Path(..., description="host ID", min_length=1, max_length=64),
        host_mo_ref: str = Path(..., description="host名称")):
    """
    查询虚拟机的RDM磁盘启动器信息详情
    :param compute_res_uuid: 计算资源的UUID
    :param host_mo_ref: host名称
    :return: 返回虚拟机的RDM磁盘启动器信息详情
    """
    compute_res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(compute_res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    product_initiators = service.get_product_initiators_for_new_location(compute_res.name, host_mo_ref)
    list_pi = list(RdmProductInitiatorDetailSchema(**product_initiator) for product_initiator in product_initiators)
    clear(env.password)
    return list_pi


@vmware_api.get("/internal/compute-resources/{compute_res_uuid}/networks",
                summary="查询计算资源的网络信息列表【内部接口】",
                description="根据计算资源ID查询计算资源的网络信息列表",
                status_code=200,
                response_model=List[VirtualResourceSchema])
def list_compute_res_network_details(
        compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64)):
    """
    查询计算资源网络信息
    :param compute_res_uuid:计算资源的UUID
    :return: 返回计算资源的网络列表
    """
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    networks = service.get_networks_by_compute_res_name(res.name)
    clear(env.password)
    if not networks:
        return []
    else:
        return list(VirtualResourceSchema(**network) for network in networks)


@vmware_api.get("/internal/compute-resources/{compute_res_uuid}/datastores",
                status_code=200,
                summary="查询计算资源的数据存储列表【内部接口】",
                description="根据计算资源ID查询计算资源的数据存储列表",
                response_model=List[VirtualResourceSchema])
def list_compute_res_datastore_details(
        compute_res_uuid: str = Path(..., description="计算资源ID", min_length=1, max_length=64)):
    """
    查询计算资源数据存储信息
    :param compute_res_uuid:计算资源的UUID
    :return: 返回计算资源的数据存储列表
    """
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    datastores = service.get_datastore_by_compute_res(res.name)
    clear(env.password)
    if not datastores:
        return []
    else:
        return list(VirtualResourceSchema(**ds) for ds in datastores)


@vmware_api.get("/internal/compute-resources/clusters/{cluster_uuid}/config",
                status_code=200,
                summary="查询集群计算资源的配置【内部接口】",
                description="根据集群计算资源ID查询集群计算资源的配置",
                response_model=ClusterConfigSchema)
def internal_get_cluster_config(
        cluster_uuid: str = Path(..., description="计算资源集群ID", min_length=1, max_length=64)):
    """
    查询指定集群计算资源的配置
    :param compute_res_uuid:集群计算资源的UUID
    :return: 返回配置信息
    """
    return _get_cluster_config_by_id(cluster_uuid)


@vmware_api.put("/internal/compute-resources/{compute_res_uuid}/action/refresh",
                status_code=200,
                summary="刷新计算资源下的虚拟机信息【内部接口】",
                description="刷新计算资源下的虚拟机信息")
def refresh_compute_res(
        compute_res_uuid: str = Path(..., description="计算资源ID"),
        vm_name: str = Query(None, description="虚拟机名称")):
    res, root_rs = _get_resource_by_res_id(compute_res_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    service.scan_vm_under_compute_resource(res, vm_name)
    clear(env.password)


@vmware_api.delete("/internal/virtual-machines/{vm_uuid}",
                   status_code=200,
                   summary="删除虚拟机信息【内部接口】",
                   description="删除虚拟机信息")
def delete_vm_res(vm_uuid: str = Path(..., description="资源ID")):
    res, root_rs = _get_resource_by_res_id(vm_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    service.delete_vm(res)
    clear(env.password)


@vmware_api.get("/internal/free-effective-capacity",
                status_code=200,
                summary="查询RDM存储的剩余容量【内部接口】",
                description="查询RDM存储的剩余容量")
def get_free_effective_capacity(storage_ip: str = Query(None, description="存储ip"),
                                env_id: str = Query(None, description="vcenter的uuid")):
    extend_infos = resource_service.query_res_extend_info_by_resource_id(env_id)
    return VMwareDiscoveryService.get_free_effective_capacity(extend_infos, storage_ip)


@vmware_api.post("/internal/check-wwn/{wwn}",
                 status_code=200,
                 summary="检查RDM是否属于指定存储【内部接口】",
                 description="检查RDM是否属于指定存储")
def is_wwn_in_storage(storage: StorageInfo = Body(None, description="存储信息"),
                      wwn: str = Path(..., description="RDM盘的wwn号")):
    return VMwareDiscoveryService.is_wwn_in_storage(wwn, storage)


@vmware_api.post("/internal/check-remote-host/{remote_host}",
                 status_code=200,
                 summary="检查nas类型的remoteHost是否属于指定存储【内部接口】",
                 description="检查nas类型的remoteHost是否属于指定存储")
def is_remote_host_in_storage(storage: StorageInfo = Body(None, description="存储信息"),
                              remote_host: str = Path(..., description="nas类型的remoteHost")):
    return VMwareDiscoveryService.is_remote_host_in_storage(remote_host, storage)


@vmware_api.post("/internal/check-remote-host-name/{remote_host_name}",
                 status_code=200,
                 summary="检查nas类型的remoteHostName是否存在于Netapp类型存储的IP列表中【内部接口】",
                 description="检查nas类型的remoteHostName是否存在于Netapp类型存储的IP列表中")
def is_remote_host_name_in_netapp_storage_ip_address(storage: StorageInfo = Body(None, description="存储信息"),
                                                     remote_host_name: str = Path(..., description="remoteHostName")):
    return VMwareDiscoveryService.is_remote_host_name_in_netapp_storage_ip_address(remote_host_name, storage)


def _get_resource_by_res_id(res_id):
    res = resource_service.query_resource_by_id(res_id)
    if res is None:
        raise EmeiStorBizException(
            error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])

    root_res = resource_service.query_environment({"uuid": res.root_uuid})
    if root_res is None:
        raise EmeiStorBizException(
            error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    return res, root_res[0]


def _get_cluster_config_by_id(cluster_uuid):
    res, root_rs = _get_resource_by_res_id(cluster_uuid)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(
        res.root_uuid)
    service = VMwareDiscoveryService(service_instance, env)
    cluster_config = service.get_cluster_config_by_name(res.name)
    clear(env.password)
    return ClusterConfigSchema(**cluster_config)
