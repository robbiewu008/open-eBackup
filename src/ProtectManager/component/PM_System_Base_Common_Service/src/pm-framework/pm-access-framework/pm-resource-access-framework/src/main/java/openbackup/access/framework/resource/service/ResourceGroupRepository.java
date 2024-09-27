/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.system.base.sdk.copy.model.BasePage;

import java.util.List;
import java.util.Optional;

/**
 * 资源组Repository
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-20
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
     * @return String 资源组id
     */
    String update(ResourceGroupDto resourceGroupDto);

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
}