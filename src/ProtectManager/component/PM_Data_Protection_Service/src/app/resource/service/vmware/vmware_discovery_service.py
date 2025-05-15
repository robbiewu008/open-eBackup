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
import os
import re
import ssl
import stat
import urllib.parse
import uuid
from enum import Enum
from ipaddress import ip_address
from socket import socket, AF_INET, AF_INET6
from ssl import CERT_REQUIRED, create_default_context
from typing import Optional

from pyVim import connect
from pyVim.connect import VimSessionOrientedStub
from pyVmomi import vim
from pydantic import BaseModel

from app.backup.client.rbac_client import RBACClient
from app.base.consts import BYTE_SIZE_CONST
from app.base.db_base import database
from app.common.clients.system_base_client import SystemBaseClient
from app.common.enums.os_type import OS
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum, LinkStatusEnum
from app.common.event_messages.Resource.resource import ResourceDeletedRequest, ResourceAddedRequest
from app.common.events import producer
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.common.rpc.system_base_rpc import decrypt
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo
from app.common.util.cleaner import clear
from app.common.util.retry.retryer import retry
from app.resource.common.constants import VMwareRetryConstants, VMWareCertConstants, DmaProxy, VMWareStorageConstants
from app.resource.models.resource_models import EnvironmentTable
from app.resource.models.resource_models import ResourceTable
from app.resource.models.virtual_res_models import VirtualResourceTable
from app.resource.schemas.env_schemas import ScanEnvSchema, StorageInfo
from app.resource.service.common import resource_service
from app.resource.service.common.domain_resource_object_service import get_domain_id_list
from app.resource.service.common.resource_service import comment_event_message
from app.resource.service.vmware import service_instance_manager as src_instance
from app.resource.service.vmware.storage_cache_manager import get_storage_service
from app.resource.service.vmware.storage_netapp_service import StorageNetAppService
from app.resource.service.vmware.vmware_tag_service import VMTagReader

log = get_logger(__name__)
# SOCKET Time Out is 5 seconds
TIMEOUT = 5

IDE_CONTROLLER_KEY = ["200", "201"]
SCSI_CONTROLLER_KEY = ["1000", "1001", "1002", "1003"]
SATA_CONTROLLER_KEY = ["15000", "15001", "15002", "15003"]
NVME_CONTROLLER_KEY = ["31000", "31001", "31002", "31003"]

CONTROLLER_KEY_MAP = {
    "1000": "SCSI(0",
    "1001": "SCSI(1",
    "1002": "SCSI(2",
    "1003": "SCSI(3",
    "200": "IDE(0",
    "201": "IDE(1",
    "15000": "SATA(0",
    "15001": "SATA(1",
    "15002": "SATA(2",
    "15003": "SATA(3",
    "31000": "NVME(0",
    "31001": "NVME(1",
    "31002": "NVME(2",
    "31003": "NVME(3"
}

# match datastore name
DATASTORE_PATTERN = "\[([^\[\]]+)\]"

# default vmware resource
DEFAULT_VMWARE_RESOURCE = None

TAG_SCAN_MIN_VERSION = 6.5


class MachineParameters(BaseModel):
    name: str = ""
    path: str = "/"
    type: str = ""
    parent_name: str = ""
    parent_uuid: str = ""
    vm_ip: str = ""
    env_ip: Optional[str] = ""
    sub_type: str = ""
    uuid: str = ""
    instance_id: str = ""
    children: int = None
    capacity: int = 0
    free_space: int = 0
    uncommitted: int = 0
    root_uuid: Optional[str] = ""
    is_template: bool = False
    mo_id: str = ""
    link_status: int = None
    alias_type: str = ""
    alias_value: str = ""
    version: str = ""
    os_type: str = ""
    tags: str = ""
    firmware: str = ""
    user_id: Optional[str] = ""
    authorized_user: Optional[str] = ""


class EntityType(str, Enum):
    NotDefined = "undefined"
    VMware = "VMware"
    VMWareVCenter = "VirtualCenter"
    VMWareDataCenter = "Datacenter"
    VMWareVm = "VM"
    VMWareDataStore = "Datastore"
    VMWareEsx = "ESX"
    VMWareEsxi = "ESXI"
    VMWareNetwork = "Network"
    VMWareServer = "Server"
    FusionCompute = "fusion_compute"
    VMWareCluster = "Cluster"
    VMWareResourcePool = "ResourcePool"
    VMWareFolder = "Folder"
    VMwareVirtualApp = "VirtualApp"
    VMWareVmTemplate = "VmTemplate"


class ScanNode:
    obj = None
    parent_path: str = ''
    parent_name: str = ''
    parent_uuid: str = ''
    root_folder: str = ''
    root_uuid: str = ''
    vm_ip: str = ''
    env_ip: str = ''
    link_status: int
    user_id: str = ''
    authorized_user: str = ''

    def __init__(self, obj, parent=None):
        self.obj = obj
        if parent is not None:
            self.parent_path = parent.parent_path
            self.parent_name = parent.parent_name
            self.parent_uuid = parent.parent_uuid
            self.root_folder = parent.root_folder
            self.root_uuid = parent.root_uuid
            self.env_ip = parent.env_ip
            self.user_id = parent.user_id
            self.authorized_user = parent.authorized_user

    def get_machine_params(self):
        m_params = {
            'path': self.parent_path,
            'parent_name': self.parent_name,
            'parent_uuid': self.parent_uuid,
            'root_uuid': self.root_uuid,
            'env_ip': self.env_ip,
            'user_id': self.user_id,
            'authorized_user': self.authorized_user
        }
        return MachineParameters(**m_params)


def _get_status(runtime):
    if runtime.powerState == "poweredOn" and runtime.connectionState == "connected":
        return 1
    else:
        return 0


def get_vir_domain_id_list(resource_id: str, domain_id: str = None):
    log.debug(f'get_vir_domain_id_list, resource_id:{resource_id}, domain_id:{domain_id}')
    if not resource_id:
        if domain_id:
            return [domain_id]
        else:
            return []
    return get_domain_id_list(resource_id, domain_id=domain_id)


class VMwareDiscoveryService:
    def __init__(self, service_instance, env: ScanEnvSchema):
        self.service_instance = service_instance
        self.env = env
        self.vsphere = None

    def _scan_datacenter(self, results: [], node: ScanNode):
        datacenter = node.obj
        log.info(f"[VMWare Scan] Scan datacenter[{datacenter._moId}] start.")
        child_node = ScanNode(None, node)
        if datacenter.__class__.__name__ == "vim.Datacenter":
            dc_mp = node.get_machine_params()
            dc_mp.name = urllib.parse.unquote(datacenter.name)
            dc_mp.path += os.sep + dc_mp.name
            dc_mp.type = EntityType.VMWareDataCenter.value
            dc_mp.sub_type = "vim.Datacenter"
            dc_mp.instance_id = datacenter._serverGuid
            dc_mp.uuid = self.gen_uuid(datacenter, self.env.uuid)
            dc_mp.user_id = node.user_id
            dc_mp.authorized_user = node.authorized_user
            results.append(dc_mp)

            child_node.parent_path = dc_mp.path
            child_node.parent_name = dc_mp.name
            child_node.parent_uuid = dc_mp.uuid
            child_node.parent_name = datacenter.name
            child_node.parent_path = dc_mp.path
        # These special root folders of a DataCenter are not shown to the user (are hidden)
        self._scan_folder(results, ScanNode(
            datacenter.hostFolder, child_node), hidden=True)
        # For VMs and Templates only (includes ALL VMs and VirtualApps):
        self._scan_folder(results, ScanNode(
            datacenter.vmFolder, child_node), hidden=True)

    def _get_host(self, node: ScanNode, is_cluster):
        host = node.obj
        log.debug(f"[VMWare Scan] Scan host[{host._moId}].")
        summary = host.summary
        cfg_summary = summary.config

        host_mp = node.get_machine_params()
        host_mp.name = urllib.parse.unquote(cfg_summary.name)
        host_mp.type = "Host"
        host_mp.link_status = _get_status(host.runtime)
        host_mp.sub_type = host.__class__.__name__
        if is_cluster:
            host_mp.path += os.sep + host_mp.name

        host_mp.alias_type = host.parent.resourcePool.__class__.__name__
        host_mp.alias_value = host.parent.resourcePool._moId

        # the path is actually that of the parent ComputeResource!
        host_mp.uuid = None

        # If management_ip is not None then this ESX is managed by a VCenter
        host_mp.instance_id = summary.hardware.uuid
        host_mp.mo_id = host._moId
        host_mp.uuid = self.gen_uuid(host, self.env.uuid)
        host_mp.user_id = node.user_id
        host_mp.authorized_user = node.authorized_user
        if host.parent.__class__.__name__ == "vim.ClusterComputeResource":
            host_mp.children = 0
        elif host.parent.__class__.__name__ == "vim.ComputeResource":
            host_mp.children = self._get_sub_vm_num(host.parent.resourcePool)
        host_mp.version = cfg_summary.product.version
        return host_mp

    def _get_vm(self, node: ScanNode):
        vm = node.obj
        log.debug(f"[VMWare Scan] Get vm {vm._moId}.")
        cfg_summary = vm.summary.config

        vm_mp = node.get_machine_params()
        vm_mp.path = vm_mp.path + os.sep
        vm_mp.type = "VM"
        vm_mp.name = urllib.parse.unquote(cfg_summary.name)
        vm_mp.is_template = cfg_summary.template
        vm_mp.sub_type = (
            EntityType.VMWareVmTemplate if vm_mp.is_template else vm.__class__.__name__
        )
        vm_mp.instance_id = cfg_summary.instanceUuid
        vm_mp.firmware = vm.config.firmware if vm.config and vm.config.firmware else ""
        vm_mp.uuid = self.gen_uuid(vm, self.env.uuid)
        vm_mp.os_type = self.get_os_type(vm.summary.config.guestFullName)
        vm_mp.mo_id = vm._moId
        vm_mp.link_status = (1 if vm.runtime.powerState == "poweredOn" else 0)
        vm_mp.user_id = node.user_id
        vm_mp.authorized_user = node.authorized_user
        if vm.guest.net:
            vm_ips = []
            for nets in vm.guest.net:
                vm_ips.extend(nets.ipAddress)
            vm_mp.vm_ip = ",".join(vm_ips)
        else:
            vm_mp.vm_ip = vm.guest.ipAddress
        # Just adding first used network for now

        return vm_mp

    def _scan_resource_pool(self, results: list, node: ScanNode):
        """
        扫描某个几点下资源池
        :param results: 扫描的结果
        :param node: 节点对象
        :return:
        """
        pool = node.obj
        log.info(f"[VMWare Scan] Scan resource pool[{pool._moId}].")
        child_node = ScanNode(None, node)
        is_root_pool = pool.parent == pool.owner
        if not is_root_pool:
            pool_mp = node.get_machine_params()
            pool_mp.name = urllib.parse.unquote(pool.name)
            pool_mp.path += os.sep + pool_mp.name
            pool_mp.mo_id = pool._moId
            pool_mp.type = "ResourcePool"
            pool_mp.sub_type = pool.__class__.__name__
            pool_mp.uuid = self.gen_uuid(pool, self.env.uuid)
            pool_mp.alias_type = pool.__class__.__name__
            pool_mp.alias_value = pool._moId
            pool_mp.user_id = node.user_id
            pool_mp.authorized_user = node.authorized_user
            results.append(pool_mp)

            child_node.parent_path = pool_mp.path
            child_node.parent_name = pool_mp.name
            child_node.parent_uuid = pool_mp.uuid

        children = pool.resourcePool
        if children is not None:
            for child in children:
                self._scan_resource_pool(results, ScanNode(child, child_node))

        vms = pool.vm
        if vms is not None:
            log.info(f"[VMWare Scan] Scan count[{len(vms)}] in resource pool {pool._moId}.")
            for vm in vms:
                results.append(self._get_vm(ScanNode(vm, child_node)))

    def _scan_compute_resource(self, results: list, node: ScanNode, is_cluster=False):
        resource = node.obj

        log.info(f"[VMWare Scan] Scan compute resource[{resource._moId}] start.")
        # If is_cluster is False, the ComputeResource is not visible to the user and there should only
        # be a single host under it. All VMs in the ResourcePool hierarchy have
        # a path under this ComputeResource but the user will see them as children of this host.
        # If is_cluster is True, the ClusterComputeResource is visible to the user and there can be 0 or more
        # hosts in this cluster, and all VMs in the ResourcePool hierarchy are children of this cluster.
        child_node = ScanNode(None, node)
        if is_cluster:
            cluster_mp = node.get_machine_params()
            cluster_mp.name = urllib.parse.unquote(resource.name)
            cluster_mp.path += os.sep + cluster_mp.name
            cluster_mp.type = "Cluster"
            cluster_mp.sub_type = resource.__class__.__name__
            cluster_mp.mo_id = resource._moId
            cluster_mp.uuid = self.gen_uuid(resource, self.env.uuid)
            cluster_mp.alias_type = resource.resourcePool.__class__.__name__
            cluster_mp.alias_value = resource.resourcePool._moId
            # vms in cluster
            cluster_mp.children = self._get_sub_vm_num(resource.resourcePool)
            cluster_mp.user_id = node.user_id
            cluster_mp.authorized_user = node.authorized_user
            results.append(cluster_mp)

            child_node.parent_path = cluster_mp.path
            child_node.parent_name = cluster_mp.name
            child_node.parent_uuid = cluster_mp.uuid

        else:
            child_node.parent_path += os.sep + resource.name
        cr_host_name = None
        cr_host_uuid = None
        hosts = resource.host
        if hosts is not None:
            for host in hosts:
                m_node = self._get_host(ScanNode(host, child_node), is_cluster)
                if not is_cluster and cr_host_uuid is None and m_node is not None:
                    cr_host_name = m_node.name
                    cr_host_uuid = m_node.uuid
                results.append(m_node)

        if resource.resourcePool is not None:
            # if is_cluster is False, there should be a single host which we set as the parent for these
            # VMs (but the path is still that of the ResourcePool).
            if not is_cluster and cr_host_uuid is not None:
                child_node.parent_name = cr_host_name
                child_node.parent_uuid = cr_host_uuid
            self._scan_resource_pool(results, ScanNode(resource.resourcePool, child_node))

    def _get_sub_vm_num(self, resource_pool):
        total = 0
        if not resource_pool:
            return total
        total += len(resource_pool.vm)
        for pool in resource_pool.resourcePool:
            total += self._get_sub_vm_num(pool)
        return total

    def _get_network(self, node: ScanNode):
        # 获取网络信息扫描
        network = node.obj

        net_mp = node.get_machine_params()
        net_mp.name = network.name
        net_mp.path += os.sep + net_mp.name
        net_mp.mo_id = network._moId
        net_mp.type = network.__class__.__name__
        net_mp.uuid = self.gen_uuid(network, self.env.uuid)
        return net_mp

    def _get_datastore(self, node: ScanNode):
        # 扫描计算资源
        datastore = node.obj
        summary = datastore.summary

        ds_mp = node.get_machine_params()
        ds_mp.name = summary.name
        ds_mp.type = datastore.__class__.__name__
        ds_mp.path = None
        ds_mp.uuid = self.gen_uuid(datastore, self.env.uuid)
        ds_mp.mo_id = datastore._moId
        ds_mp.capacity = summary.capacity
        ds_mp.free_space = summary.freeSpace
        ds_mp.uncommitted = 0
        if summary.accessible and summary.uncommitted is not None:
            ds_mp.uncommitted = summary.uncommitted

        if datastore.host is not None and len(datastore.host) > 0:
            ds_mp.path = datastore.host[0].mountInfo.path
        if ds_mp.path is None:
            url = datastore.summary.url
            if url.startswith('ds://'):
                ds_mp.path = url[5:]
        if ds_mp.path is None:
            ds_mp.path = ''
        else:
            if not ds_mp.path.endswith('/'):
                ds_mp.path += os.sep
        ds_mp.path += ds_mp.name

        return ds_mp

    def _scan_folder(self, results: list, node: ScanNode, hidden=False, is_root=False):
        folder = node.obj
        log.info(f"[VMWare Scan] Scan folder[{folder._moId}] start.")
        child_node = ScanNode(None, node)
        if not hidden:
            folder_mp = node.get_machine_params()
            folder_mp.name = urllib.parse.unquote(folder.name)
            folder_mp.path += os.sep + folder_mp.name
            folder_mp.type = "Folder"
            folder_mp.sub_type = folder.__class__.__name__
            folder_mp.uuid = self.gen_uuid(folder, self.env.uuid)
            folder_mp.user_id = node.user_id
            folder_mp.authorized_user = node.authorized_user
            results.append(folder_mp)

            child_node.parent_path = folder_mp.path
            child_node.parent_name = folder_mp.name
            child_node.parent_uuid = folder_mp.uuid
        elif not is_root:
            child_node.root_folder = folder.name
        children = folder.childEntity
        if children is not None:
            for child in children:
                child_type = child.__class__.__name__
                child_node.obj = child
                if 'vim.Datacenter' == child_type:
                    self._scan_datacenter(results, child_node)
                elif 'vim.ComputeResource' == child_type:
                    self._scan_compute_resource(results, child_node)
                elif 'vim.ClusterComputeResource' == child_type:
                    self._scan_compute_resource(
                        results, child_node, is_cluster=True)
                elif 'vim.Folder' == child_type and (
                        "ComputeResource" in child.childType or "Datacenter" in child.childType):
                    self._scan_folder(results, child_node)

    def scan_vsphere(self, vsphere, env_param: ScanEnvSchema):
        log.info(f"[VMWare Scan] Scan vsphere start.")
        content = self.service_instance.RetrieveContent()
        root_folder_node = ScanNode(content.rootFolder)

        results = []

        root_folder_node.parent_path = vsphere.get("path")
        root_folder_node.parent_name = vsphere.get("name")
        root_folder_node.parent_uuid = vsphere.get("uuid")
        root_folder_node.root_uuid = vsphere.get("uuid")
        root_folder_node.env_ip = vsphere.get("endpoint")
        root_folder_node.user_id = vsphere.get("user_id")
        root_folder_node.authorized_user = vsphere.get("authorized_user")

        # The root folder ("Datacenters") is never shown (is hidden).
        self._scan_folder(results, root_folder_node, hidden=True, is_root=True)
        self._add_vm_tags(vsphere, env_param, results)
        return results

    @staticmethod
    def _add_vm_tags(vsphere, env_param, results):
        version = vsphere.get('version')
        version_2_digit = float('.'.join(version.split('.')[0:2]))
        log.debug(f"[VMWare Scan] API Version: "
                  f"version_2_digit:{version_2_digit}, "
                  f"version:{version}.")
        if version_2_digit < TAG_SCAN_MIN_VERSION:
            log.warning(f"[VMWare Scan] Tag infos are not supported on vCenters below {TAG_SCAN_MIN_VERSION}. "
                        f"version_2_digit:{version_2_digit}, "
                        f"version:{version}.")
            return
        sub_type = vsphere.get('sub_type')
        if not sub_type or sub_type != ResourceSubTypeEnum.vCenter:
            log.warning(f"vsphere is not vCenter, no tags info. sub_type:{sub_type}")
            return
        log.info(f"[VMWare Scan] Scan tag start.")
        with VMTagReader(env_param.endpoint, env_param.port, env_param.user_name, env_param.password) as tag_reader:
            for resource in results:
                if resource.type == ResourceTypeEnum.VM:
                    log.debug(f"[VMWare Scan] Get {resource.mo_id} tag.")
                    resource.tags = tag_reader.get_vm_tags(resource.mo_id)

    def find_path(self, path, vm):
        if vm is not None:
            path = self.search_root(path, vm.resourcePool.parent)
        return path

    def search_root(self, path, node):
        if node is None:
            return path
        if hasattr(node, "parent") and node.parent:
            # Folder层级过滤不展示
            node_type = node.__class__.__name__
            if 'vim.Folder' != node_type:
                path = node.name + os.sep + path
            return self.search_root(path, node.parent)
        else:
            return path

    def get_vm_by_name(self, vm_name):
        get_vm_res = None
        content = self.service_instance.RetrieveContent()
        vm_view = content.viewManager.CreateContainerView(
            content.rootFolder, [vim.VirtualMachine], True).view
        for vm in vm_view:
            if urllib.parse.unquote(vm.name) == vm_name:
                return vm
        return get_vm_res

    def get_vm_by_id(self, vm_uuid):
        vm = None
        content = self.service_instance.RetrieveContent()
        search_index = content.searchIndex
        if vm_uuid is not None:
            vm = search_index.FindByUuid(None, vm_uuid, True, True)
        return vm

    def get_vm_info(self, res):
        # 获取虚拟机信息
        def get_hardware(virtual_machine):
            """
            查询虚拟机硬件信息 cpu
            :param virtual_machine: 虚拟机
            :return:
            """
            return {
                "num_cpu": virtual_machine.config.hardware.numCPU,
                "num_cores_per_socket": virtual_machine.config.hardware.numCoresPerSocket,
                "memory": virtual_machine.config.hardware.memoryMB,
                "controller": list(self._get_vm_controller(virtual_machine))
            }

        def get_vm_config_file_ds(virtual_machine):
            """
            查询虚拟机配置文件所在的数据存储
            :param virtual_machine: 虚拟机
            :return:
            """
            get_vm_config_ds_res = None
            ds_name = virtual_machine.config.files.vmPathName
            search_name = re.search(DATASTORE_PATTERN, ds_name)
            if search_name:
                ds_name = search_name.groups()[0]
            ds = list(
                datastore for datastore in virtual_machine.datastore if datastore.name == ds_name)
            if ds:
                ds = ds[0]
                return {
                    "uuid": ds._moId + ":" + (ds.info.vmfs.uuid if hasattr(ds.info, "vmfs") else ds.info.name),
                    "name": ds.info.name,
                    "mo_id": ds._moId
                }
            return get_vm_config_ds_res

        def get_vm_runtime(virtual_machine):
            """
            查询虚拟机的运行环境
            :param virtual_machine:虚拟机
            :return:
            """
            host = virtual_machine.runtime.host
            host_summary = host.summary
            host_uuid = self.gen_uuid(host, self.env.uuid)
            return {
                "host": {
                    "instance_id": host_summary.hardware.uuid,
                    "uuid": host_uuid,
                    "name": urllib.parse.unquote(host.name),
                    "mo_id": host._moId,
                    "version": host_summary.config.product.version
                }
            }

        vm = self.get_vm(res)

        return {
            "hardware": get_hardware(vm),
            "vmx_datastore": get_vm_config_file_ds(vm),
            "runtime": get_vm_runtime(vm),
            "firmware": vm.config.firmware if vm.config and vm.config.firmware else "",
            "vm_parent_location": vm.parent._moId
        }

    def get_vm_disk(self, res):
        """
        根据虚拟机的名称查询虚拟机的磁盘信息
        :param vm_name: 虚拟机名称
        :return: 返回虚拟机的磁盘列表
        """
        vm = self.get_vm(res)
        return self._get_vm_disk(vm)

    def get_vm(self, res):
        vm = self.get_vm_by_id(res.instance_id)
        if vm is None:
            vm = self.get_vm_by_name(res.name)
        if vm is None:
            log.error(f"Virtual machine is None! Please check whether the VM exists.")
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, error_message="Get None value of VM!")
        else:
            self.check_datastore_state_of_vm(vm)
            return vm

    def get_networks_by_compute_res_name(self, compute_name):
        """
        根据资源资源的名称查询可用的网络
        :param compute_name: 计算资源名称
        :return: 网络列表
        """
        content = self.service_instance.RetrieveContent()
        compute_res = self._get_compute_resource(content, compute_name)
        if compute_res is not None:
            return self._get_networks_by_compute_res(compute_res)
        return []

    @staticmethod
    def _get_vm_controller(vm):
        # 查看控制器的key类型判断虚拟机支持控制器
        log.info(f'vm={vm}')
        results = set()
        for dev in vm.config.hardware.device:
            if dev.__class__.__name__ == "vim.vm.device.VirtualDisk":
                continue
            if dev.key is None:
                continue
            if str(dev.key) in IDE_CONTROLLER_KEY:
                results.add("IDE")
            elif str(dev.key) in SCSI_CONTROLLER_KEY:
                results.add("SCSI")
            elif str(dev.key) in SATA_CONTROLLER_KEY:
                results.add("SATA")
            elif str(dev.key) in NVME_CONTROLLER_KEY:
                results.add("NVME")
        return results

    def _get_vm_disk(self, vm):
        """
        根据资源资源的名称查询可用的磁盘
        :param vm: 虚拟机资源
        :return: 磁盘列表
        """
        results = []
        if not vm.config or not vm.config.hardware or not vm.config.hardware.device:
            log.error(f"vm disk info not exist.")
            return results
        for dev in vm.config.hardware.device:
            if dev.__class__.__name__ == "vim.vm.device.VirtualDisk" and CONTROLLER_KEY_MAP.get(
                    str(dev.controllerKey)):
                disk = {}
                controller_key = dev.controllerKey
                slot = CONTROLLER_KEY_MAP.get(
                    str(controller_key)) + ":" + str(dev.unitNumber) + ")"
                disk["type"] = "VirtualDisk"
                disk["sub_type"] = "vim.vm.device.VirtualDisk"
                disk["uuid"] = self.get_disk_uuid(dev)
                disk["slot"] = slot
                disk["name"] = dev.deviceInfo.label
                disk["path"] = dev.backing.fileName
                disk["parent_name"] = urllib.parse.unquote(vm.name)
                disk["parent_uuid"] = vm.summary.config.instanceUuid
                disk["root_uuid"] = self.env.uuid
                disk["capacity"] = dev.capacityInKB
                disk = self._get_vm_disk_data_store(disk, dev)
                results.append(disk)
        return results

    @staticmethod
    def _get_vm_disk_data_store(disk, dev):
        disk_lun = None
        if hasattr(dev.backing, "lunUuid") and dev.backing.lunUuid and \
                len(dev.backing.lunUuid) > 23:
            disk_lun = "naa." + dev.backing.lunUuid[10:-12]
        datastore = dev.backing.datastore.info
        VMwareDiscoveryService.get_data_store(disk, datastore, dev, disk_lun)
        if dev.backing is not None and hasattr(dev.backing, "compatibilityMode"):
            if dev.backing.compatibilityMode is not None:
                disk["disk_lun"] = disk_lun
                disk["disk_type"] = "rdm"
        else:
            disk["disk_type"] = "normal"
        return disk

    def _get_folder_by_compute_res(self, compute_res):
        """
        查询计算资源所在的Folder
        :param compute_res:
        :return:
        """
        if compute_res is not None:
            if hasattr(compute_res, "vmFolder"):
                return {
                    "name": urllib.parse.unquote(compute_res.vmFolder.name),
                    "mo_id": compute_res.vmFolder._moId
                }
            else:
                return self._get_folder_by_compute_res(compute_res.parent)
        return DEFAULT_VMWARE_RESOURCE

    def _get_compute_resource(self, content, compute_name):
        """
        根据计算资源的名称和类型查询对应的计算资源
        :param content: ServiceInstance目录
        :param compute_name: 计算资源的名称
        :return: 返回对应的计算资源
        """
        # 查询content中计算资源和资源池
        compute_res_view = content.viewManager.CreateContainerView(
            content.rootFolder, [vim.ComputeResource, vim.ResourcePool], True).view
        # 选择ESX主机时datastore
        if content.about.name == ResourceSubTypeEnum.ESXi.value or \
                content.about.name == ResourceSubTypeEnum.ESX.value:
            if compute_res_view[0].__class__.__name__ == 'vim.ComputeResource' and hasattr(compute_res_view[0],
                                                                                           "datastore"):
                return compute_res_view[0]
        # 选择资源池或Vapp时候的datastore
        for res in compute_res_view:
            if urllib.parse.unquote(res.name) == compute_name and isinstance(res, vim.ResourcePool):
                return self._get_resource_pool_compute_resource(res.parent)
        # 选择集群时的datastore过滤
        for res in compute_res_view:
            if urllib.parse.unquote(res.name) == compute_name and hasattr(res, "datastore"):
                return res
            if res.__class__.__name__ == 'vim.ClusterComputeResource':
                host = self._get_host_system_from_cluster_compute_resource(
                    res, compute_name)
                if host:
                    return host
        return DEFAULT_VMWARE_RESOURCE

    @staticmethod
    def _get_nearest_layer_compute_resource(content, compute_name):
        """
        根据计算资源的名称和类型查询最近一层的计算资源
        :param content: ServiceInstance目录
        :param compute_name: 计算资源的名称
        :return: 返回对应的计算资源
        """
        # 查询content中计算资源和资源池
        compute_res_view = content.viewManager.CreateContainerView(
            content.rootFolder, [vim.ComputeResource, vim.ResourcePool], True).view
        # 选择ESX主机时
        if content.about.name == ResourceSubTypeEnum.ESXi.value or \
                content.about.name == ResourceSubTypeEnum.ESX.value:
            if compute_res_view[0].__class__.__name__ == 'vim.ComputeResource' and hasattr(compute_res_view[0],
                                                                                           "datastore"):
                return compute_res_view[0]
        # 选择资计算资源，资源池或集群
        for res in compute_res_view:
            if urllib.parse.unquote(res.name) == compute_name and isinstance(res, (
                    vim.ResourcePool, vim.ClusterComputeResource, vim.ComputeResource)):
                return res
        return DEFAULT_VMWARE_RESOURCE

    @staticmethod
    def _get_host_system_from_cluster_compute_resource(cluster_compute_res, compute_res_name):
        # 根据集群计算资源获取主机系统
        hosts = cluster_compute_res.host
        if len(hosts) > 0:
            for host in hosts:
                if urllib.parse.unquote(host.name) == compute_res_name and hasattr(host, "datastore"):
                    return host
        return DEFAULT_VMWARE_RESOURCE

    def _get_resource_pool_compute_resource(self, node):
        if isinstance(node, (vim.ComputeResource, vim.ClusterComputeResource)):
            return node
        if hasattr(node, "parent") and node.parent:
            return self._get_resource_pool_compute_resource(node.parent)
        return DEFAULT_VMWARE_RESOURCE

    def get_folder_by_compute_res_name(self, compute_name):
        """
        根据资源资源的名称查询可用的folder
        :param compute_name: 计算资源名称
        :return: 网络列表
        """
        content = self.service_instance.RetrieveContent()
        compute_res = self._get_compute_resource(content, compute_name)
        if compute_res is not None:
            return self._get_folder_by_compute_res(compute_res)
        return DEFAULT_VMWARE_RESOURCE

    def _get_networks_by_compute_res(self, compute_res):
        """
        查询计算资源可用的网络
        :param compute_res:
        :return:
        """
        network_list = []
        for network in compute_res.network:
            if hasattr(network, "config") and network.config is not None and network.config.uplink == True:
                continue
            list_item = {
                "uuid": self.gen_uuid(network, self.env.uuid),
                "name": urllib.parse.unquote(network.name),
                "mo_id": network._moId,
                "type": "Network",
                "sub_type": network.__class__.__name__,
                "paren_name": urllib.parse.unquote(network.parent.name),
                "parent_uuid": self.gen_uuid(network.parent, self.env.uuid),
                "root_uuid": self.gen_uuid(network, self.env.uuid),
            }
            network_list.append(list_item)
        return network_list

    @staticmethod
    def get_service_instance_with_proxy(host, port, user_name, password, ssl_context=None):
        smart_stub = connect.SmartStubAdapter(host=host, port=port, httpProxyHost=DmaProxy.host,
                                              httpProxyPort=DmaProxy.port,
                                              sslContext=ssl_context)
        session_stub = VimSessionOrientedStub(smart_stub,
                                              VimSessionOrientedStub.makeUserLoginMethod(username=user_name,
                                                                                         password=password))
        return vim.ServiceInstance('ServiceInstance', session_stub)

    @staticmethod
    def _get_compute_res_id(ds):
        # 获取计算资源id
        if hasattr(ds.info, "vmfs"):
            return ds.info.vmfs.uuid
        elif hasattr(ds.info, "nas"):
            return ds.info.nas.name
        elif hasattr(ds.info, "vvolDS"):
            return ds.info.vvolDS.scId
        return DEFAULT_VMWARE_RESOURCE

    @staticmethod
    def _get_vm_ip(vm):
        # 获取vm_ip
        if vm.guest.net:
            vm_ips = []
            for nets in vm.guest.net:
                vm_ips.extend(nets.ipAddress)
            return ",".join(vm_ips)
        else:
            return vm.guest.ipAddress

    def get_cluster_config_by_name(self, compute_res_name):
        content = self.service_instance.RetrieveContent()
        drs_enabled = True
        compute_res = self._get_compute_resource(content, compute_res_name)
        if isinstance(compute_res, vim.ClusterComputeResource):
            drs_enabled = compute_res.configuration.drsConfig.enabled
        return {
            "drs_enabled": drs_enabled
        }

    @staticmethod
    def get_vm_disks(resource_id: str):
        res = resource_service.query_resource_by_id(resource_id)
        if res is None:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST)
        root_rs = resource_service.query_environment({"uuid": res.root_uuid})
        env = ScanEnvSchema(**root_rs[0])
        service_instance = src_instance.service_instance_manager.get_service_instance(res.root_uuid)
        service = VMwareDiscoveryService(service_instance, env)
        disks = service.get_vm_disk(res)
        clear(env.password)
        return disks

    @retry(exceptions=Exception, tries=VMwareRetryConstants.RETRY_TIMES, wait=VMwareRetryConstants.WAIT_TIME,
           backoff=VMwareRetryConstants.BACKOFF, logger=log)
    def get_datastore_by_compute_res(self, compute_res_name):
        """
        查询计算资源的的数据存储
        :param compute_res_name:
        :return: DataStore list
        """
        content = self.service_instance.RetrieveContent()
        compute_res = self._get_compute_resource(content, compute_res_name)
        if compute_res is not None:
            return list({
                "type": "Datastore",
                "sub_type": "vim.Datastore",
                "uuid": ds._moId + ":" + self._get_compute_res_id(ds) if self._get_compute_res_id(ds) else ds._moId,
                "name": ds.info.name,
                "path": ds.info.url,
                "mo_id": ds._moId,
                "root_uuid": self.env.uuid,
                "parent_uuid": None,
                "parent_name": None,
                "capacity": ds.summary.capacity / BYTE_SIZE_CONST if ds.summary.capacity else 0,
                "free_space": ds.summary.freeSpace / BYTE_SIZE_CONST if ds.summary.freeSpace else 0,
                "partitions": list(
                    extent.diskName for extent in ds.info.vmfs.extent) if hasattr(ds.info, "vmfs") else None
            } for ds in compute_res.datastore if
                not ds.info.name.startswith("TemporaryDsForIR") and ds.summary and ds.summary.accessible)
        return DEFAULT_VMWARE_RESOURCE

    @staticmethod
    def get_vsphere_info(service_instance, params):
        """
        获取vsphere信息
        :param service_instance: service instance
        :param params: 发现参数
        :return: vsphere基本信息
        """
        content = service_instance.RetrieveContent()
        name = content.about.name
        instance_uuid = content.about.instanceUuid
        if not instance_uuid:
            instance_uuid = str(uuid.uuid4()) + "OceanProtect"

        # vCenter
        vsphere = {
            "uuid": instance_uuid,
            "type": params.type,
            "name": params.name,
            "sub_type": name,
            "path": params.endpoint,
            "endpoint": params.endpoint,
            "port": params.port
        }
        return vsphere

    @staticmethod
    def get_host_info(service_instance, env_uuid):

        def build_host_info(host):
            host_info = {}
            host_info["mo_id"] = host._moId
            host_info["uuid"] = VMwareDiscoveryService.gen_uuid(
                host, env_uuid)
            return host_info

        content = service_instance.RetrieveContent()
        # 查询被vCenter或ESX管理的主机系统，如果只有一个，且管理服务器的IP地址不为空，则表示该ESX/ESXi已经备份vCenter管理
        host_system_view = content.viewManager.CreateContainerView(
            content.rootFolder, [vim.HostSystem], True).view
        return list(build_host_info(host) for host in host_system_view)

    @staticmethod
    def is_managed_by_vcenter(service_instance):
        """
        检查service instance是否已经被vCenter管理
        :param service_instance: service instance
        :return: 已经管理返回true，否则返回false
        """
        content = service_instance.RetrieveContent()

        # 查询被vCenter或ESX管理的主机系统，如果只有一个，且管理服务器的IP地址不为空，则表示该ESX/ESXi已经备份vCenter管理
        host_system_view = content.viewManager.CreateContainerView(
            content.rootFolder, [vim.HostSystem], True).view
        if host_system_view is not None and hasattr(host_system_view[0].summary, "managementServerIp"):
            vcenter_ip = host_system_view[0].summary.managementServerIp
            if vcenter_ip is not None:
                return True, vcenter_ip

        return False, ""

    @staticmethod
    def get_os_type(os_name: str):
        """
        获取VM操作系统类型
        vSphere版本7.0.0.10000、8.0.0.10000中不支持的操作系统：
        (具体可通过查阅VMware Compatibility Guide兼容性指南了解详情或访问http://kb.vmware.com/kb/2015161)
        Windows: Microsoft Windows NT,Microsoft Windows 98,Microsoft Windows 95,Microsoft Windows 3.1,Microsoft MS-DOS
        Linux: Red Hat Enterprise Linux 2.1,VMware CRX Pod 1,SUSE Linux Enterprise 8/9,
               Debian GNU/Linux 4/5/6 (6版本7.0.0.10000不完全支持、8.0.0.10000版本不再支持),SUSE openSUSE,Red Hat Fedora
        :param os_name: 操作系统名称
        :return: os_type操作系统类型
        """
        linux_list = ["Linux", "CentOS", "VMware Photon", "Asianux"]
        if os_name is None:
            os_type = OS.OTHER.value
        elif True in list(linux in os_name for linux in linux_list):
            os_type = OS.LINUX.value
        elif OS.WINDOWS.value in os_name:
            os_type = OS.WINDOWS.value
        else:
            os_type = OS.OTHER.value
        return os_type

    @retry(exceptions=Exception, tries=VMwareRetryConstants.RETRY_TIMES, wait=VMwareRetryConstants.WAIT_TIME,
           backoff=VMwareRetryConstants.BACKOFF, logger=log)
    def scan_vm_under_compute_resource(self, compute_res, vm_name):
        log.info(f"scan vm {vm_name} under compute resource {compute_res.uuid}")
        content = self.service_instance.RetrieveContent()
        compute_resource = self._get_nearest_layer_compute_resource(
            content, compute_res.name)
        vms = []

        if not compute_resource:
            return
        if hasattr(compute_resource, "vm"):
            vms = compute_resource.vm
        elif hasattr(compute_resource, "resourcePool"):
            vms = compute_resource.resourcePool.vm
        if not vms:
            return
        env = self.get_env_params(compute_res.root_uuid)
        with VMTagReader(env.get("host"), env.get("port"), env.get("user_name"), env.get("password")) as tag_reader:
            if float('.'.join(content.about.version.split('.')[0:2])) < TAG_SCAN_MIN_VERSION:
                tag_reader = None
            if env.get("not_vcenter"):
                tag_reader = None
            if vm_name is None:
                self.__refresh_all_vm_resource_under_computer_res(vms, compute_res, tag_reader)
            else:
                self.__refresh_vm_under_computer_res_by_name(vms, compute_res, vm_name, tag_reader)

    @staticmethod
    def get_env_params(env_uuid):
        with database.session() as db:
            env = db.query(EnvironmentTable).filter(EnvironmentTable.uuid == env_uuid).first().as_dict()
            env["password"] = SystemBaseClient.decrypt(env.get("password")).get("plaintext")
            env["host"] = env.get("endpoint")
            # 查询是否为vCenter环境
            res = resource_service.query_resource_by_id(env_uuid)
            if res and res.sub_type != ResourceSubTypeEnum.vCenter:
                env["not_vcenter"] = True
        return env

    def __refresh_all_vm_resource_under_computer_res(self, vm_list, computer_res, tag_reader: VMTagReader):
        vm_info_list = []
        for vm in vm_list:
            result = self.get_refresh_vm_info(computer_res, vm, tag_reader)
            vm_info_list.append(result)
        self.merge_resource(vm_info_list, computer_res.uuid)

    def __refresh_vm_under_computer_res_by_name(self, vm_list, computer_res, vm_name,
                                                tag_reader: VMTagReader):
        vm_info_list = []
        for vm in vm_list:
            if urllib.parse.unquote(vm.name) == vm_name:
                result = self.get_refresh_vm_info(computer_res, vm, tag_reader)
                vm_info_list.append(result)
                break
        with database.session() as db:
            # 新增虚拟机
            for vm_info in vm_info_list:
                db.merge(VirtualResourceTable(**vm_info))
                add_msg = ResourceAddedRequest(
                    request_id=str(uuid.uuid4()), resource_id=vm_info.get("uuid"))
                domain_id_list = get_vir_domain_id_list(vm_info.get("parent_uuid"))
                resource_set_relation_info = ResourceSetRelationInfo(
                    resource_object_id=vm_info.get("uuid"),
                    resource_set_type=ResourceSetTypeEnum.RESOURCE.value,
                    scope_module=ResourceSetScopeModuleEnum.VMWARE.value,
                    domain_id_list=domain_id_list,
                    parent_uuid=vm_info.get("parent_uuid"),
                    sub_type=ResourceSubTypeEnum.VirtualMachine.value
                )
                RBACClient.add_resource_set_relation(resource_set_relation_info)
                producer.produce(add_msg)

    def get_refresh_vm_info(self, res, vm, tag_reader):
        vm_info = {
            "uuid": self.gen_uuid(vm, self.env.uuid),
            "instance_id": vm.summary.config.instanceUuid,
            "mo_id": vm._moId,
            "name": urllib.parse.unquote(vm.name),
            "type": "VM",
            "vm_ip": self._get_vm_ip(vm),
            "env_ip": res.env_ip,
            "sub_type": vm.__class__.__name__,
            "capacity": 0,
            "free_space": 0,
            "uncommitted": 0,
            "alias_type": "",
            "alias_value": "",
            "parent_name": res.name,
            "firmware": vm.config.firmware if vm.config and vm.config.firmware else "",
            "parent_uuid": res.uuid,
            "root_uuid": res.root_uuid,
            "path": res.env_ip + os.sep + self.find_path("", vm) + os.sep,
            "link_status": (1 if vm.runtime.powerState == "poweredOn" else 0),
            "os_type": self.get_os_type(vm.summary.config.guestFullName),
            "tags": tag_reader.get_vm_tags(vm._moId) if tag_reader else "",
            "user_id": res.user_id,
            "authorized_user": res.authorized_user
        }
        return vm_info

    def delete_vm(self, vm_res):
        log.info(f"start delete vm. {vm_res.uuid}")
        instance_vm = self.get_vm_by_id(vm_res.instance_id)
        if instance_vm is not None:
            log.warning("the vm exists and does not need to be deleted.")
            return
        with database.session() as db:
            filters = {ResourceTable.uuid == vm_res.uuid}
            db.query(ResourceTable).filter(*filters).delete(synchronize_session=False)
            comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(
                uuid.uuid4()), resource_id=vm_res.uuid)

    @staticmethod
    def merge_resource(resources, parent_uuid):
        # 本次扫描的资源id
        current_res_id_list = list(res.get("uuid") for res in resources)
        with database.session() as db:
            filters = {VirtualResourceTable.parent_uuid == parent_uuid,
                       VirtualResourceTable.sub_type == ResourceSubTypeEnum.VirtualMachine}
            exist_res_list = db.query(
                VirtualResourceTable).filter(*filters).all()
            exist_res_id_list = list(res.uuid for res in exist_res_list)
            # 数据库存在，而新扫描上的不存在，则表示资源已经被删除
            deleted_res_id_list = list(
                res.uuid for res in exist_res_list if res.uuid not in current_res_id_list)
            delete_filters = {
                ResourceTable.uuid.in_(deleted_res_id_list)}
            db.query(ResourceTable).filter(
                *delete_filters).delete(synchronize_session=False)
            RBACClient.delete_resource_set_relation(deleted_res_id_list, ResourceSetTypeEnum.RESOURCE.value)
            # 数据库中不存在，但是新扫描上来的资源存在，则表示资源时新增加的
            added_res_id_list = list(
                res_id for res_id in current_res_id_list if res_id not in exist_res_id_list)
            added_res = list(VirtualResourceTable(
                **res) for res in resources if res.get("uuid") in added_res_id_list)
            db.add_all(added_res)
            for vm_uuid in added_res_id_list:
                resource_set_relation_info = ResourceSetRelationInfo(
                    resource_object_id=vm_uuid,
                    resource_set_type=ResourceSetTypeEnum.RESOURCE.value,
                    scope_module=ResourceSetScopeModuleEnum.VMWARE.value,
                    domain_id_list=get_vir_domain_id_list(parent_uuid),
                    parent_uuid=parent_uuid,
                    sub_type=ResourceSubTypeEnum.VirtualMachine.value
                )
                RBACClient.add_resource_set_relation(resource_set_relation_info)
            # 数据库中存在，且新扫描上的也存在，表示需要更新
            updated_res_id_list = list(
                res_id for res_id in current_res_id_list if res_id in exist_res_id_list)
            updated_res = list(
                res for res in resources if res.get("uuid") in updated_res_id_list)
            db.bulk_update_mappings(VirtualResourceTable, updated_res)

        for deleted_id in deleted_res_id_list:
            # 删除的资源发送kafka消息
            delete_msg = ResourceDeletedRequest(
                request_id=str(uuid.uuid4()), resource_id=deleted_id)
            producer.produce(delete_msg)
        for add_id in added_res_id_list:
            # 新增的资源发送kafka消息
            add_msg = ResourceAddedRequest(
                request_id=str(uuid.uuid4()), resource_id=add_id)
            producer.produce(add_msg)
        pass

    @staticmethod
    def login(env: ScanEnvSchema):
        """
        登录vCenter/ESX/ESXi
        :param env: vCenter/ESX/ESXi环境参数
        :return: 如果登录成功返回vCenter/ESX/ESXi的服务实例，否则抛出异常
        """
        ip = ip_address(env.endpoint)
        host = str(ip)
        cert_file_name = None
        try:
            if env.verify_cert == 1:
                service_instance, cert_file_name = VMwareDiscoveryService.login_with_ssl(host, env)
            else:
                service_instance = VMwareDiscoveryService.login_with_no_ssl(env, host)
            service_instance.RetrieveContent()
            VMwareDiscoveryService.clear_alarm_if_env_exist(env.uuid)
            return service_instance, cert_file_name
        except Exception as e:
            log.exception("connect vcenter/esx error.")
            if isinstance(e, EmeiStorBizException):
                raise e
            class_name = e.__class__.__name__
            if class_name == 'vim.fault.InvalidLogin' or class_name == 'vim.fault.NoPermission':
                VMwareDiscoveryService.send_alarm_if_env_exist(env.uuid)
                raise EmeiStorBizException(CommonErrorCodes.USER_OR_PASSWORD_IS_INVALID,
                                           message="The user name or password is incorrect.", retryable=True) from e
            if class_name == 'timeout' or class_name == 'TypeError' or class_name == 'OSError':
                VMwareDiscoveryService.send_alarm_if_env_exist(env.uuid)
                raise EmeiStorBizException(ResourceErrorCodes.NETWORK_CONNECTION_TIMEDOUT,
                                           message="Network connection timed out.", retryable=True) from e
            VMwareDiscoveryService.send_alarm_if_env_exist(env.uuid)
            raise EmeiStorBizException(ResourceErrorCodes.VMWARE_CONNECTION_FAILED,
                                       message="The VMware connection failed.", retryable=True) from e
        finally:
            pass

    @staticmethod
    def login_with_no_ssl(env, host):
        if VMwareDiscoveryService.is_connectable(host, env.port):
            service_instance = connect.SmartConnect(
                host=host, port=env.port, user=env.user_name, pwd=env.password, disableSslCertValidation=True)
        else:
            service_instance = VMwareDiscoveryService.get_service_instance_with_proxy(
                host, env.port, env.user_name, env.password, ssl.SSLContext(ssl.PROTOCOL_TLS))
        return service_instance

    @staticmethod
    def login_with_ssl(host, env: ScanEnvSchema):
        dir_path = '/opt/OceanProtect/protectmanager/cert/external/VMWARE/'
        ca_data = env.extend_context.get(VMWareCertConstants.CERTIFICATION)
        crl_data = env.extend_context.get(VMWareCertConstants.REVOCATION_LIST)
        cert_file_name = env.extend_context.get(VMWareCertConstants.CERT_NAME)
        tls_compatible = env.extend_context.get(VMWareCertConstants.TLS_COMPATIBLE)
        crl_file_path = dir_path + env.name + ".crl"
        if ca_data is None:
            raise EmeiStorBizException(ResourceErrorCodes.REGISTER_VMWARE_CERT_FAILURE,
                                       message="there is no cert can be use to login.")
        VMwareDiscoveryService.check_file_size(ca_data, crl_data)
        try:
            ssl_context = create_default_context(cadata=ca_data)
            ssl_context.check_hostname = False
            ssl_context.verify_mode = CERT_REQUIRED
            # TLS版本不满足会报OSError
            if not tls_compatible or tls_compatible == str(False):
                ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
                ssl_context.set_ciphers(VMWareCertConstants.CIPHERS)
            # 通过临时文件加载吊销列表
            if crl_data:
                if not os.path.exists(dir_path):
                    os.mkdir(dir_path)
                flags = os.O_WRONLY | os.O_CREAT
                mode = stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP
                with os.fdopen(os.open(crl_file_path, flags, mode), 'w') as crl_file:
                    crl_file.write(crl_data)
                ssl_context.verify_flags = ssl.VERIFY_CRL_CHECK_LEAF
                ssl_context.load_verify_locations(cafile=crl_file_path)
                log.info("load vSphere crl file success.")
            if VMwareDiscoveryService.is_connectable(host, env.port):
                service_instance = connect.SmartConnect(
                    host=host, port=env.port, user=env.user_name, pwd=env.password, sslContext=ssl_context)
            else:
                service_instance = VMwareDiscoveryService.get_service_instance_with_proxy(
                    host, env.port, env.user_name, env.password, ssl_context=ssl_context)
        except Exception as e:
            if e.__class__.__name__ == "SSLCertVerificationError":
                raise VMwareDiscoveryService.get_ssl_exception(e) from e
            else:
                raise e
        finally:
            if os.path.exists(crl_file_path):
                os.remove(crl_file_path)
        return service_instance, cert_file_name

    @staticmethod
    def clear_alarm_if_env_exist(env_uuid):
        env_info = resource_service.query_resource_by_id(env_uuid)
        if env_info:
            src_instance.EnvLinkStatusChecker.clear_alarm(env_id=env_uuid)
            src_instance.EnvLinkStatusChecker.delete_alarm_flag(env_id=env_uuid)
        resource_service.update_link_status(env_id_list=[env_uuid], link_status=LinkStatusEnum.Online)

    @staticmethod
    def send_alarm_if_env_exist(env_uuid):
        env_info = resource_service.query_resource_by_id(env_uuid)
        if env_info:
            user_id = env_info.user_id if env_info.user_id else None
            src_instance.EnvLinkStatusChecker.send_alarm(env_id=env_uuid, env_ip=env_info.endpoint,
                                                         env_name=env_info.name, user_id=user_id)
        resource_service.update_link_status(env_id_list=[env_uuid], link_status=LinkStatusEnum.Offline)

    @staticmethod
    def check_file_size(ca_data, crl_data):
        max_cert_size = '1MB'
        max_crl_size = '0.005MB'
        if crl_data and len(crl_data.encode()) > VMWareCertConstants.CRL_MAX_SIZE:
            raise EmeiStorBizException(ResourceErrorCodes.FILE_SIZE_IS_OVER_LIMIT, *[max_crl_size],
                                       message="revocation list size is over 5KB.")
        if len(ca_data.encode()) > VMWareCertConstants.CERT_MAX_SIZE:
            raise EmeiStorBizException(ResourceErrorCodes.FILE_SIZE_IS_OVER_LIMIT, *[max_cert_size],
                                       message="certification size is over 1MB.")

    @staticmethod
    def get_ssl_exception(exception):
        verify_code = exception.verify_code
        log.info(f"error is {exception.__class__.__name__}, verify_code is :{verify_code},"
                 f" verify_message is :{exception.verify_message}")
        if verify_code == VMWareCertConstants.REVOCATION_CODE:
            return EmeiStorBizException(ResourceErrorCodes.ENVIRONMENT_CERT_IS_REVOKED,
                                        message='certificate of the vsphere has been revoked.')
        elif verify_code == VMWareCertConstants.EXPIRED_CRL_CODE:
            return EmeiStorBizException(ResourceErrorCodes.CRL_HAS_EXPIRED,
                                        message='The revocation list has expired.')
        elif verify_code == VMWareCertConstants.INVALID_CRL_CODE:
            return EmeiStorBizException(ResourceErrorCodes.CRL_IS_INVALID,
                                        message='The revocation list does not match the certification.')
        else:
            return EmeiStorBizException(ResourceErrorCodes.REGISTER_VMWARE_CERT_FAILURE,
                                        message=exception.__cause__)

    @staticmethod
    def is_connectable(host: str, port):
        host_ip = ip_address(host)
        timeout_seconds = 1
        sock = socket(AF_INET6 if host_ip.version == 6 else AF_INET)
        sock.settimeout(timeout_seconds)
        result = sock.connect_ex((host, int(port)))
        log.info("Host: {}, Port: {}, Is connectable: {}".format(host, port, result == 0))
        sock.close()
        return result == 0

    @staticmethod
    def gen_uuid(obj, env_uuid: str):
        """
        生成对象的uuid
        :param obj: 对象实例
        :param env_uuid: 环境的UUID
        :return: 返回对象的uuid
        """
        return str(
            uuid.uuid5(
                uuid.NAMESPACE_URL, env_uuid + "+" + obj.__class__.__name__ + ":" + obj._moId,
            )
        )

    @staticmethod
    def disconnect(service_instance):
        connect.Disconnect(service_instance)

    @staticmethod
    def check_datastore_state_of_vm(vm):
        """
        查询虚拟机上的存储状态是否都正常
        :param vm: 虚拟机资源
        :return: 状态正常则返回，状态不正常则抛出对应的异常
        """
        log.info(f"start to check datastore state of VM: {vm.name}.")
        if not vm.datastore:
            log.error(f"Can not get datastore information of VM: {vm.name}.")
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       error_message="Failed to get VM datastore information.")
        for ds in vm.datastore:
            if not ds.summary or not ds.summary.accessible:
                log.error(f"Can not get VM info! Please check VM state and datastore: {ds.name} state in vCenter.")
                raise EmeiStorBizException(error=ResourceErrorCodes.VIRTUAL_MACHINE_INFO_FAILED,
                                           error_message="Failed to get virtual machine information.")

    @staticmethod
    def get_ds_name(dev, ds_name):
        # rdm盘的uuid用lunUuid的值
        if dev.__class__.__name__ == "vim.vm.device.VirtualDisk" and CONTROLLER_KEY_MAP.get(
                str(dev.controllerKey)):
            # rdm盘的uuid用lunUuid的值
            if dev.backing is not None and hasattr(dev.backing, "compatibilityMode"):
                if dev.backing.compatibilityMode is not None:
                    ds_name = dev.backing.datastore.name
                    return ds_name
        return ds_name

    @staticmethod
    def get_esxi_moref(compute_res, host_mo_ref):
        in_em_it_list = []
        if compute_res.__class__.__name__ == "vim.HostSystem":
            return VMwareDiscoveryService.get_initiators_name_and_esxi_moref_and_initiators_type(compute_res)
        for host in compute_res.host:
            if host_mo_ref == host._moId:
                in_em_it_list = VMwareDiscoveryService.get_initiators_name_and_esxi_moref_and_initiators_type(host)
        return in_em_it_list

    @staticmethod
    def get_initiators_name_and_esxi_moref_and_initiators_type(host):
        in_em_it_list = []
        initiators_name_list = []
        for host_bus_adapter in host.configManager.storageSystem.storageDeviceInfo.hostBusAdapter:
            if host_bus_adapter.__class__.__name__ == "vim.host.FibreChannelHba":
                in_em_it_list, initiators_name_list = VMwareDiscoveryService.get_in_em_it_list_when_fc(
                    host, host_bus_adapter, initiators_name_list, in_em_it_list)
            if host_bus_adapter.__class__.__name__ == "vim.host.InternetScsiHba":
                in_em_it_list, initiators_name_list = VMwareDiscoveryService.get_in_em_it_list_when_iscsi(
                    host, host_bus_adapter, initiators_name_list, in_em_it_list)
        return in_em_it_list

    @staticmethod
    def get_in_em_it_list_when_fc(host, host_bus_adapter, initiators_name_list, in_em_it_list):
        if host_bus_adapter.portWorldWideName is not None and host_bus_adapter.status == "online":
            # 获取光纤通道类型存储适配器中的WWNP的值，并将该字符串转换为16进制，移除开头的0x
            initiators_name = str(hex(int(host_bus_adapter.portWorldWideName)))[2:]
            esxi_moref = host.name
            # FibreChannel:0 InternetScsi:1
            initiators_type = 0
            # 需要对initiators_name进行去重处理
            if initiators_name not in initiators_name_list:
                initiators_name_list.append(initiators_name)
                in_em_it_list.append([initiators_name, esxi_moref, initiators_type])
                return in_em_it_list, initiators_name_list
        return in_em_it_list, initiators_name_list

    @staticmethod
    def get_in_em_it_list_when_iscsi(host, host_bus_adapter, initiators_name_list, in_em_it_list):
        if host_bus_adapter.iScsiName is not None and host_bus_adapter.status == "online":
            initiators_name = host_bus_adapter.iScsiName
            esxi_moref = host.name
            # FibreChannel:0 InternetScsi:1
            initiators_type = 1
            # 需要对initiators_name进行去重处理
            if initiators_name not in initiators_name_list:
                initiators_name_list.append(initiators_name)
                in_em_it_list.append([initiators_name, esxi_moref, initiators_type])
                return in_em_it_list, initiators_name_list
        return in_em_it_list, initiators_name_list

    @staticmethod
    def get_product_initiator_list(ds, datastore_name, compute_res_name, in_em_it_list, result_list):
        if not ds.info.name.startswith("TemporaryDsForIR") and ds.summary and ds.summary.accessible:
            if datastore_name == ds.name:
                for in_em_it in in_em_it_list:
                    initiators_name = in_em_it[0]
                    esxi_moref = in_em_it[1]
                    initiators_type = in_em_it[2]
                    initiators_result = [
                        {
                            "esxi_name": compute_res_name,
                            "esxi_moref": esxi_moref,
                            "initiators": {
                                "type": initiators_type,
                                "name": initiators_name
                            }
                        }
                    ]
                    result_list.extend(initiators_result)
        return result_list

    @staticmethod
    def get_product_initiator_list_for_new_location(ds, compute_res_name, in_em_it_list, result_list):
        if not ds.info.name.startswith("TemporaryDsForIR") and ds.summary and ds.summary.accessible:
            for in_em_it in in_em_it_list:
                initiators_name = in_em_it[0]
                esxi_moref = in_em_it[1]
                initiators_type = in_em_it[2]
                initiators_result = [
                    {
                        "esxi_name": compute_res_name,
                        "esxi_moref": esxi_moref,
                        "initiators": {
                            "type": initiators_type,
                            "name": initiators_name
                        }
                    }
                ]
                result_list.extend(initiators_result)
        return result_list

    @staticmethod
    def get_disk_uuid(dev):
        disk_uuid = dev.backing.uuid
        # rdm盘的uuid用lunUuid的值
        if dev.backing is not None and hasattr(dev.backing, "compatibilityMode"):
            if dev.backing.compatibilityMode is not None:
                disk_uuid = dev.backing.lunUuid
        return disk_uuid

    @staticmethod
    def get_data_store(disk, datastore, dev, disk_lun):
        if hasattr(datastore, "vmfs"):
            disk["is_nas"] = False
            disk["datastore"] = {
                "mo_id": dev.backing.datastore._moId,
                "uuid": datastore.vmfs.uuid,
                "url": datastore.url,
                "name": datastore.vmfs.name,
                "type": datastore.vmfs.type,
                "partitions": list(
                    extent.diskName for extent in datastore.vmfs.extent)
            }
            if disk_lun is not None:
                disk["datastore"]["partitions"].append(disk_lun)
        elif hasattr(datastore, "nas"):
            VMwareDiscoveryService.set_disk_info_when_nas(datastore, disk, dev)
        elif hasattr(datastore, "vvolDS"):
            disk["is_nas"] = False
            disk["datastore"] = {
                "mo_id": dev.backing.datastore._moId,
                "uuid": datastore.vvolDS.scId,
                "url": datastore.url,
                "name": datastore.vvolDS.name,
                "type": datastore.vvolDS.type
            }
        elif hasattr(datastore, "containerId"):
            disk["is_nas"] = False
            disk["datastore"] = {
                "mo_id": dev.backing.datastore._moId,
                "uuid": datastore.containerId,
                "url": datastore.url,
                "name": datastore.name,
                "type": datastore.dynamicType
            }
        else:
            disk["is_nas"] = False
            disk["datastore"] = {
                "mo_id": dev.backing.datastore._moId,
                "uuid": str(uuid.uuid4()),
                "url": datastore.url,
                "name": datastore.name,
                "type": datastore.dynamicType
            }

    @staticmethod
    def set_disk_info_when_nas(datastore, disk, dev):
        disk["is_nas"] = True
        disk["nas_info"] = {
            "name": datastore.nas.name,
            "type": datastore.nas.type,
            "remote_host": datastore.nas.remoteHost,
            "remote_host_names": list(
                remote_host_name for remote_host_name in datastore.nas.remoteHostNames
            ),
            "remote_path": datastore.nas.remotePath
        }
        disk["datastore"] = {
            "uuid": str(uuid.uuid4()),
            "mo_id": dev.backing.datastore._moId,
            "url": datastore.url,
            "name": datastore.nas.name,
            "type": datastore.nas.type,
        }

    @staticmethod
    def get_free_effective_capacity(extend_infos, storage_ip: str):
        storages = ''
        storage_dict = {}
        if not extend_infos:
            log.info(f"Extend_infos not exist")
            return 0
        for extend_info in extend_infos:
            if extend_info.key == VMWareStorageConstants.STORAGES:
                storages = extend_info.value
        if not storages:
            log.info(f"Storages not exist")
            return 0
        storages = json.loads(storages)
        for storage in storages:
            ip_list = storage.get('ip', '')
            for ip in ip_list:
                if storage_ip == ip:
                    storage_dict = storage
        port = storage_dict.get('port', 8088)
        username = storage_dict.get('username', '')
        password = decrypt(storage_dict.get('password', ''))
        storage_type = storage_dict.get('type', '')
        if storage_type == ResourceSubTypeEnum.NET_APP.value:
            log.info(f"Choose NetApp storage:{storage_ip}, capacity cannot be obtained, select another storage device.")
            return 0
        storage_service = get_storage_service(storage_ip, username, password, port)
        capacity = storage_service.get_free_effective_capacity_from_storage()
        log.info(f"Get capacity from storage success, storage ip:{storage_ip}, port:{port}, capacity:{capacity}")
        return capacity

    @staticmethod
    def is_wwn_in_storage(wwn: str, storage: StorageInfo):
        storage_service = get_storage_service(storage.ip, storage.username, storage.password, storage.port)
        return storage_service.is_wwn_in_storage(wwn)

    @staticmethod
    def is_remote_host_in_storage(remote_host: str, storage: StorageInfo):
        storage_service = get_storage_service(storage.ip, storage.username, storage.password, storage.port)
        return storage_service.is_remote_host_in_storage(remote_host)

    @staticmethod
    def is_remote_host_name_in_netapp_storage_ip_address(remote_host_name: str, storage: StorageInfo):
        storage_service = StorageNetAppService(storage.ip, storage.port, storage.username, storage.password)
        return storage_service.is_remote_host_name_in_netapp_storage_ip_address(remote_host_name)

    @staticmethod
    def _get_vm_config_file_datastore_name(vm):
        """
        根据资源资源的名称查询虚拟机的配置文件所在存储的名称
        :param vm: 虚拟机资源
        :return: 虚拟机的配置文件所在存储的名称
        """
        results = []
        if not vm.summary or not vm.summary.config or not vm.summary.config.vmPathName:
            log.error(f"Get vmPathName failed.")
            return results
        vm_path_name = vm.summary.config.vmPathName
        datastore_name = vm_path_name.split("[")[1].split("]")[0]
        return datastore_name

    def get_vm_config_file_datastore_name(self, res):
        """
        根据虚拟机的名称查询VMware虚拟机的配置文件所在存储的名称
        :param vm_name: 虚拟机名称
        :return: 虚拟机的配置文件所在存储的名称
        """
        vm = self.get_vm(res)
        return self._get_vm_config_file_datastore_name(vm)

    @retry(exceptions=Exception, tries=VMwareRetryConstants.RETRY_TIMES, wait=VMwareRetryConstants.WAIT_TIME,
           backoff=VMwareRetryConstants.BACKOFF, logger=log)
    def get_product_initiators(self, compute_res_name, datastore_name, host_mo_ref):
        """
        根据虚拟机的名称查询虚拟机的RDM磁盘启动器信息
        :param compute_res_name: 主机资源名称
        :param datastore_name: datastore名称
        :param host_mo_ref: host moref名称
        :return: 返回虚拟机的RDM磁盘启动器信息
        """
        content = self.service_instance.RetrieveContent()
        compute_res = self._get_compute_resource(content, compute_res_name)
        if not compute_res:
            return []
        try:
            in_em_it_list = self.get_esxi_moref(compute_res, host_mo_ref)
        except Exception as error:
            log.info(f"An error occurred:", error)
            return []
        result_list = []
        for ds in compute_res.datastore:
            result_list = self.get_product_initiator_list(
                ds, datastore_name, compute_res_name, in_em_it_list, result_list)
            if len(result_list) > 0:
                return result_list
        return []

    @retry(exceptions=Exception, tries=VMwareRetryConstants.RETRY_TIMES, wait=VMwareRetryConstants.WAIT_TIME,
           backoff=VMwareRetryConstants.BACKOFF, logger=log)
    def get_product_initiators_for_new_location(self, compute_res_name, host_mo_ref):
        """
        根据虚拟机的名称查询虚拟机的RDM磁盘启动器信息
        :param compute_res_name: 主机资源名称
        :return: 返回虚拟机的RDM磁盘启动器信息
        """
        content = self.service_instance.RetrieveContent()
        compute_res = self._get_compute_resource(content, compute_res_name)
        if not compute_res:
            return []
        try:
            in_em_it_list = self.get_esxi_moref(compute_res, host_mo_ref)
        except Exception as error:
            log.info(f"An error occurred:", error)
            return []
        result_list = []
        for ds in compute_res.datastore:
            result_list = self.get_product_initiator_list_for_new_location(
                ds, compute_res_name, in_em_it_list, result_list)
            if len(result_list) > 0:
                return result_list
        return []

    def check_disk_sharing_type_is_multi_writer(self, res):
        """
        校验虚拟机中是否包含分享类型为多写入器的磁盘（共享盘）
        :param res: 资源名称
        :return: 若不包含则返回True，否则抛出异常
        """
        multi_writer_disk_list = self.get_disk_sharing_type_is_multi_writer(res)
        if len(multi_writer_disk_list) == 0:
            return True
        else:
            log.error(f"Disk Sharing Type is sharingMultiWriter!")
            raise EmeiStorBizException(error=ResourceErrorCodes.HAS_MULTI_WRITER_SHARING_DISK,
                                       error_message="Disk Sharing Type is sharingMultiWriter!")

    def get_disk_sharing_type_is_multi_writer(self, res):
        """
        获取虚拟机中分享类型为多写入器的磁盘（共享盘）列表
        :param res: 资源名称
        :return: 分享类型为多写入器的磁盘列表
        """
        vm = self.get_vm(res)
        return self.get_vm_disk_if_sharing_type_is_multi_writer(vm)

    def get_vm_disk_if_sharing_type_is_multi_writer(self, vm):
        """
        根据资源资源的名称查询共享类型为多写入器的磁盘（共享盘）
        :param vm: 虚拟机资源
        :return: 磁盘列表
        """
        results = []
        if not vm.config or not vm.config.hardware or not vm.config.hardware.device:
            log.error(f"vm disk info not exist.")
            return results
        for dev in vm.config.hardware.device:
            if dev.__class__.__name__ == "vim.vm.device.VirtualDisk" and CONTROLLER_KEY_MAP.get(str(dev.controllerKey)):
                if dev.backing is not None and hasattr(dev.backing, "sharing"):
                    self.set_disk_info_if_sharing_type_is_multi_writer(dev, vm, results)
        return results

    def set_disk_info_if_sharing_type_is_multi_writer(self, dev, vm, results):
        if dev.backing.sharing is not None and dev.backing.sharing == "sharingMultiWriter":
            disk = {}
            controller_key = dev.controllerKey
            slot = CONTROLLER_KEY_MAP.get(str(controller_key)) + ":" + str(dev.unitNumber) + ")"
            disk["type"] = "VirtualDisk"
            disk["sub_type"] = "vim.vm.device.VirtualDisk"
            disk["uuid"] = dev.backing.uuid
            disk["slot"] = slot
            disk["name"] = dev.deviceInfo.label
            disk["path"] = dev.backing.fileName
            disk["parent_name"] = urllib.parse.unquote(vm.name)
            disk["parent_uuid"] = vm.summary.config.instanceUuid
            disk["root_uuid"] = self.env.uuid
            disk["capacity"] = dev.capacityInKB
            disk = self._get_vm_disk_data_store(disk, dev)
            results.append(disk)
