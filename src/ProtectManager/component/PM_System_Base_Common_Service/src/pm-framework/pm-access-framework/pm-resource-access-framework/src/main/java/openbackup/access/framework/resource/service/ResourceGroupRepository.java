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
package openbackup.access.framework.resource.service;

import com.huawei.oceanprotect.report.enums.ProtectStatusEnum;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.system.base.sdk.copy.model.BasePage;

import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * 资源组Repository
 *
 */
public interface ResourceGroupRepository {
    /**
     * 保存资源组
     *
     * @param resourceGroupDto 资源组Dto
     * @return 资源组id
     */
    String save(ResourceGroupDto resourceGroupDto);

    /**
     * selectByNameAndPath
     *
     * @param scopeResourceId scopeResourceId
     * @param resourceGroupName resourceGroupName
     * @return Optional<ResourceGroupDto>
     */
    Optional<ResourceGroupDto> selectByScopeResourceIdAndName(
            String scopeResourceId, String resourceGroupName);

    /**
     * selectById
     *
     * @param resourceGroupId resourceGroupId
     * @return Optional<ResourceGroupDto>
     */
    Optional<ResourceGroupDto> selectById(String resourceGroupId);

    /**
     * getResourceGroupCount
     *
     * @param sourceSubType sourceSubType
     * @return int
     */
    int getResourceGroupCount(String sourceSubType);

    /**
     * selectMemberBySourceId
     *
     * @param resourceId resourceId
     * @return Optional<ResourceGroupMemberDto>
     */
    Optional<ResourceGroupMemberDto> selectMemberBySourceId(String resourceId);

    /**
     * queryResourceGroups
     *
     * @param queryResourceGroupReq queryResourceGroupReq
     * @return BasePage<ResourceGroupPo>
     */
    BasePage<ResourceGroupDto> queryResourceGroups(ResourceGroupQueryParams queryResourceGroupReq);

    /**
     * selectMemberBySourceIds
     *
     * @param resourceIds resourceIds
     * @return List<ResourceGroupMemberDto>
     */
    List<ResourceGroupMemberDto> selectMemberBySourceIds(List<String> resourceIds);

    /**
     * update
     *
     * @param resourceGroupDto resourceGroupDto
     * @param toDeleteResourceIdSet 待删除的子资源
     * @param toAddResourceIdSet 待新增的子资源
     * @return String 资源组id
     */
    String update(ResourceGroupDto resourceGroupDto, Set<String> toDeleteResourceIdSet, Set<String> toAddResourceIdSet);

    /**
     * deleteByScopeResourceId
     *
     * @param scopeResourceId resource group 所属环境的 id
     */
    void deleteByScopeResourceId(String scopeResourceId);

    /**
     * delete
     *
     * @param resourceGroupId resourceGroupId
     */
    void delete(String resourceGroupId);

    /**
     * selectProtectedObjectById
     *
     * @param resourceGroupId resourceGroupId
     * @return Optional<ProtectedObjectPo>
     */
    Optional<ProtectedObjectPo> selectProtectedObjectById(String resourceGroupId);

    /**
     * updateResourceGroup
     *
     * @param resourceGroupDto resourceGroupDto
     * @return String
     */
    String updateResourceGroup(ResourceGroupDto resourceGroupDto);

    /**
     * 获取所有的资源组列表
     *
     * @param subTypeList 子类型列表
     * @return 资源组列表
     */
    List<ResourceGroupDto> getAllResourceGroupList(List<String> subTypeList);

    /**
     * countByGroupType
     *
     * @param groupType groupType
     * @return int
     */
    int countByGroupType(String groupType);

    /**
     * 更新保护状态
     *
     * @param uuid id
     * @param protectStatus 保护状态
     * @return 更新结果
     */
    int updateStatusById(String uuid, ProtectStatusEnum protectStatus);
}