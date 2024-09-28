/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.VmwareFeignConfiguration;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.model.DatasourceEntity;
import openbackup.system.base.sdk.resource.model.Storage;
import openbackup.system.base.sdk.resource.model.VMFolder;
import openbackup.system.base.sdk.resource.model.VMWareDetailInfo;
import openbackup.system.base.sdk.resource.model.VirtualResourceSchema;
import openbackup.system.base.sdk.resource.model.vmwaremodel.ProductInitiator;
import openbackup.system.base.sdk.resource.model.vmwaremodel.VirtualDiskDetailSchema;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * 虚拟机资源api
 *
 */
@FeignClient(
        name = "VMWareRestApi",
        url = "${pm-resource-manager.url}/v1",
        configuration = VmwareFeignConfiguration.class)
public interface VMWareRestApi {
    /**
     * 查询虚拟机的磁盘详情
     *
     * @param vmUuid 虚拟机的UUID
     * @return 返回虚拟机的磁盘详情
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{vm_uuid}/disks")
    @ResponseBody
    List<VirtualDiskDetailSchema> queryVMDisk(@PathVariable("vm_uuid") String vmUuid);

    /**
     * 查询指定计算资源的的网络列表
     *
     * @param vmUuid 主机id
     * @return 网络列表
     */
    @ExterAttack
    @GetMapping("/internal/compute-resources/{computeResUuid}/networks")
    @ResponseBody
    List<VirtualResourceSchema> queryComputeNetwork(@PathVariable("computeResUuid") String vmUuid);

    /**
     * 查询指定计算资源的数据存储列表
     *
     * @param computeResUuid 主机id
     * @return 数据存储列表
     */
    @ExterAttack
    @GetMapping("/internal/compute-resources/{computeResUuid}/datastores")
    @ResponseBody
    List<VirtualResourceSchema> queryComputeDataStores(@PathVariable("computeResUuid") String computeResUuid);

    /**
     * 根据虚拟机uuid查询VMware虚拟机的配置文件所在存储的名称
     *
     * @param vmUuid 虚拟机的UUID
     * @return 数据存储列表
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{vm_uuid}/config-file-datastores-name")
    @ResponseBody
    String queryComputeDataStoresNameByVmId(@PathVariable("vm_uuid") String vmUuid);

    /**
     * 查询虚拟机的磁盘详情
     *
     * @param vmUuid 虚拟机的UUID
     * @return 返回虚拟机的磁盘详情
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{vm_uuid}/disks")
    @ResponseBody
    List<VirtualDiskDetailSchema> queryVMDiskInternal(@PathVariable("vm_uuid") String vmUuid);

    /**
     * 查询虚拟机rdm磁盘的ProductInitiator详情
     *
     * @param vmUuid 虚拟机的UUID
     * @param computeResUuid 主机id
     * @param datastoreName datastore名称
     * @param hostMoRef host名称
     * @return 返回虚拟机的磁盘详情
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{computeResUuid}/product-initiator/{vm_uuid}/{datastoreName}/{hostMoRef}")
    @ResponseBody
    List<ProductInitiator> queryVmRdmDiskProductInitiator(
        @PathVariable("computeResUuid") String computeResUuid, @PathVariable("vm_uuid") String vmUuid,
        @PathVariable("datastoreName") String datastoreName, @PathVariable("hostMoRef") String hostMoRef);

    /**
     * 恢复到新位置时，查询虚拟机rdm磁盘的ProductInitiator详情
     *
     * @param computeResUuid 主机id
     * @param hostMoRef host名称
     * @return 返回虚拟机的磁盘详情
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{computeResUuid}/product-initiator-new-location/{hostMoRef}")
    @ResponseBody
    List<ProductInitiator> queryVmRdmDiskProductInitiatorForNewLocation(
        @PathVariable("computeResUuid") String computeResUuid, @PathVariable("hostMoRef") String hostMoRef);

    /**
     * 查询虚拟机的folderName
     *
     * @param computeResUuid 虚拟机的UUID
     * @return 返回虚拟机的磁盘详情
     */
    @ExterAttack
    @GetMapping("/internal/compute-resources/{computeResUuid}/folder")
    @ResponseBody
    VMFolder queryFolderInternal(@PathVariable("computeResUuid") String computeResUuid);

    /**
     * query resources
     *
     * @param page      page
     * @param size      size
     * @param orders    orders
     * @param condition condition
     * @return resources
     */
    @ExterAttack
    @GetMapping("/internal/virtual-resource")
    BasePage<DatasourceEntity> queryResources(
            @RequestParam("page_no") int page,
            @RequestParam("page_size") int size,
            @RequestParam("conditions") String condition,
            @RequestParam("orders") List<String> orders);

    /**
     * 刷新资源
     *
     * @param computeResUuid 虚拟机的UUID
     * @param vmName 刷新指定虚拟机名称
     * @return refresh status
     */
    @ExterAttack
    @PutMapping("/internal/compute-resources/{computeResUuid}/action/refresh")
    @ResponseBody
    Object refreshResources(
            @PathVariable("computeResUuid") String computeResUuid, @RequestParam("vm_name") String vmName);

    /**
     * 获取虚拟机详细信息
     *
     * @param resourceId 资源ID
     * @return FileSetEntity
     */
    @ExterAttack
    @GetMapping("/internal/virtual-machines/{resource_id}")
    @ResponseBody
    VMWareDetailInfo queryVMWareDetailInfo(@PathVariable("resource_id") String resourceId);

    /**
     * 删除资源
     *
     * @param vmUuid 虚拟机的UUID
     */
    @ExterAttack
    @DeleteMapping("/internal/virtual-machines/{vmUuid}")
    void deleteVMResource(@PathVariable("vmUuid") String vmUuid);

    /**
     * 检查RDM是否属于指定存储
     *
     * @param storage 存储信息
     * @param wwn     RDM磁盘的wwn
     * @return 检查RDM是否属于指定存储
     */
    @ExterAttack
    @PostMapping("/internal/check-wwn/{wwn}")
    boolean isWwnInStorage(@RequestBody Storage storage, @PathVariable("wwn") String wwn);

    /**
     * 检查nas类型的remoteHost是否属于指定存储
     *
     * @param storage 存储信息
     * @param remoteHost nas类型的remoteHost
     * @return 检查nas类型的remoteHost是否属于指定存储
     */
    @ExterAttack
    @PostMapping("/internal/check-remote-host/{remote_host}")
    boolean isRemoteHostInStorage(@RequestBody Storage storage, @PathVariable("remote_host") String remoteHost);

    /**
     * 检查nas类型的remoteHostName是否存在于Netapp类型存储的IP列表中
     *
     * @param storage 存储信息
     * @param remoteHostName nas类型的remoteHostName
     * @return 检查nas类型的remoteHostName是否存在于Netapp类型存储的IP列表中
     */
    @ExterAttack
    @PostMapping("/internal/check-remote-host-name/{remote_host_name}")
    boolean isRemoteHostNameInNetappStorageIpAddress(@RequestBody Storage storage,
        @PathVariable("remote_host_name") String remoteHostName);

    /**
     * 查询存储的剩余容量
     *
     * @param storageIp 存储ip
     * @param envId vcenter的uuid
     * @return 返回存储的剩余容量
     */
    @ExterAttack
    @GetMapping("/internal/free-effective-capacity")
    String getFreeEffectiveCapacity(@RequestParam("storage_ip") String storageIp,
                                    @RequestParam("env_id") String envId);
}
