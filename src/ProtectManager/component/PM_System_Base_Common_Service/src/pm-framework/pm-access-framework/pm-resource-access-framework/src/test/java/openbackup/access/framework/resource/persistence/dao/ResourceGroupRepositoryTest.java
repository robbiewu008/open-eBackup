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
package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.dao.ResourceGroupMapper;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupMemberMapper;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupRepositoryImpl;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;
import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetResourceService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * ResourceGroupRepositoryImplTest
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {ResourceGroupRepositoryImpl.class, ResourceGroupPo.class})
public class ResourceGroupRepositoryTest {
    @Autowired
    private ResourceGroupRepository resourceGroupRepository;

    @MockBean
    private ResourceGroupMapper resourceGroupMapper;

    @MockBean
    private ResourceGroupMemberMapper resourceGroupMemberMapper;

    @MockBean
    PageQueryService pageQueryService;

    @MockBean
    private SessionService sessionService;

    @MockBean
    private ResourceSetApi resourceSetApi;

    @MockBean
    private ProtectedObjectMapper protectedObjectMapper;

    @MockBean
    private ProtectObjectRestApi protectObjectRestApi;
    @MockBean
    private ResourceSetResourceService resourceSetResourceService;

    ResourceGroupDto getResourceGroupDto() {
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        resourceGroupDto.setName("test");
        resourceGroupDto.setSourceSubType(ResourceSubTypeEnum.MICROSOFT_VIRTUAL_MACHINE.getType());
        List<ResourceGroupMemberDto> resources = new ArrayList<>();
        ResourceGroupMemberDto resource = new ResourceGroupMemberDto();
        resource.setSourceId(UUID.randomUUID().toString());
        resource.setSourceSubType(ResourceSubTypeEnum.MICROSOFT_VIRTUAL_MACHINE.getType());
        resources.add(resource);
        resourceGroupDto.setResources(resources);
        return resourceGroupDto;
    }

    /**
     * 用例场景：创建资源组成功
     * 前置条件：不涉及
     * 检查点: 返回id
     */
    @Test
    public void create_resource_group_success() {
        Mockito.when(sessionService.getCurrentUser()).thenReturn(new TokenBo.UserBo() {
            {
                setId("user01");
            }
        });
        ResourceGroupDto resourceGroupDto = getResourceGroupDto();
        String id = resourceGroupRepository.save(resourceGroupDto);
        Assert.assertNotNull(id);
    }
}
