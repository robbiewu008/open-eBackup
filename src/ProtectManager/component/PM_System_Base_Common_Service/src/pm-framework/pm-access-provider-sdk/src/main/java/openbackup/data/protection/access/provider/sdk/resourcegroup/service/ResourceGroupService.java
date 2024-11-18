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
package openbackup.data.protection.access.provider.sdk.resourcegroup.service;

import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupResultDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupDetailVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupVo;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;

import java.util.List;
import java.util.Optional;

/**
 * 资源组服务
 *
 */
public interface ResourceGroupService {
    /**
     * 创建资源组
     *
     * @param resourceGroupDto 资源组信息
     * @return resourceGroupId
     */
    String createResourceGroup(ResourceGroupDto resourceGroupDto);

    /**
     * updateResourceGroup
     *
     * @param resourceGroupDto resourceGroupDto
     * @return String
     */
    String updateResourceGroup(ResourceGroupDto resourceGroupDto);

    /**
     * 资源组列表查询
     *
     * @param resourceGroupQueryParams 查询参数对象
     * @return ResourceGroupVo 列表
     */
    PageListResponse<ResourceGroupVo> queryResourceGroups(ResourceGroupQueryParams resourceGroupQueryParams);

    /**
     * deleteResourceGroup
     *
     * @param resourceGroupId resourceGroupId
     * @return String
     */
    String deleteResourceGroup(String resourceGroupId);

    /**
     * queryResourceGroupDetail
     *
     * @param resourceGroupId resourceGroupId
     * @return ResourceGroupDetailVo
     */
    ResourceGroupDetailVo queryResourceGroupDetail(String resourceGroupId);

    /**
     * queryResourceGroupDTO
     *
     * @param resourceGroupId resourceGroupId
     * @return queryResourceGroupDTO
     */
    Optional<ResourceGroupDto> queryResourceGroupDto(String resourceGroupId);

    /**
     * createProtectedObject
     *
     * @param createRequest createRequest
     * @return String
     */
    List<ResourceGroupResultDto> createProtectedObject(CreateResourceGroupProtectedObjectRequest createRequest);

    /**
     * updateProtectedObject
     *
     * @param createRequest createRequest
     * @return String
     */
    List<ResourceGroupResultDto> updateProtectedObject(UpdateResourceGroupProtectedObjectRequest createRequest);

    /**
     * deleteProtectedObject
     *
     * @param resourceGroupId resourceGroupId
     * @return String
     */
    String deleteProtectedObject(String resourceGroupId);

    /**
     * 资源组批量激活保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    void activateResourceGroup(ProtectionBatchOperationReq batchOperationReq);

    /**
     * 资源组批量禁用保护
     *
     * @param batchOperationReq 保护批处理操作请求体
     */
    void deactivateResourceGroup(ProtectionBatchOperationReq batchOperationReq);

    /**
     * 根据资源子类型获取所有的资源组列表
     *
     * @param subTypeList 子类型列表
     * @return 资源组列表
     */
    List<ResourceGroupDto> getAllResourceGroupList(List<String> subTypeList);
}