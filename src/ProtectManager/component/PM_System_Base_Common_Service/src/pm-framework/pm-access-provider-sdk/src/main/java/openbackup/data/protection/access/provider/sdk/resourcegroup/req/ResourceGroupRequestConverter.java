/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;

import org.springframework.beans.BeanUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 类型转换
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-23
 */
public class ResourceGroupRequestConverter {
    /**
     * createResourceRequest 转 resourceGroupDto
     *
     * @param createResourceGroupRequest 创建资源组请求体
     * @return resourceGroupDto
     */
    public static ResourceGroupDto resourceGroupCreateReqToDto(CreateResourceGroupRequest createResourceGroupRequest) {
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        BeanUtils.copyProperties(createResourceGroupRequest, resourceGroupDto);
        return getResourceGroupDto(createResourceGroupRequest.getResourceIds(), resourceGroupDto);
    }

    private static ResourceGroupDto getResourceGroupDto(List<String> resourcesIds, ResourceGroupDto resourceGroupDto) {
        if (resourcesIds == null) {
            return resourceGroupDto;
        }
        List<ResourceGroupMemberDto> resourceGroupMemberDtoList = new ArrayList<>();
        for (String resourceId : resourcesIds) {
            ResourceGroupMemberDto resourceGroupMemberDto = new ResourceGroupMemberDto();
            resourceGroupMemberDto.setSourceId(resourceId);
            resourceGroupMemberDtoList.add(resourceGroupMemberDto);
        }
        resourceGroupDto.setResources(resourceGroupMemberDtoList);
        return resourceGroupDto;
    }

    /**
     * resourceGroupUpdateReqToDto
     *
     * @param updateReq updateReq
     * @return ResourceGroupDto
     */
    public static ResourceGroupDto resourceGroupUpdateReqToDto(UpdateResourceGroupRequest updateReq) {
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        BeanUtils.copyProperties(updateReq, resourceGroupDto);
        return getResourceGroupDto(updateReq.getResourceIds(), resourceGroupDto);
    }
}