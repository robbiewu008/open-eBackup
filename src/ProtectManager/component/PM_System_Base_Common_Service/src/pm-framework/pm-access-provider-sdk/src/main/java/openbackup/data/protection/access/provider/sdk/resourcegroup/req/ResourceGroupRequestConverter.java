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
package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;

import org.springframework.beans.BeanUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 类型转换
 *
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
