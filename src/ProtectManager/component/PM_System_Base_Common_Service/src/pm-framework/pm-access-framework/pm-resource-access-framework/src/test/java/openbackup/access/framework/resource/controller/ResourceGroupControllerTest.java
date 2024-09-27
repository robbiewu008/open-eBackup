/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.controller.ResourceGroupController;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupDetailVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupVo;
import openbackup.data.protection.access.provider.sdk.resourcegroup.service.ResourceGroupService;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

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
import java.util.Collections;
import java.util.List;
import java.util.UUID;

/**
 * 功能描述 资源组 controller测试
 *
 * @author w30044259
 * @since 2023-11-20
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {ResourceGroupController.class})
public class ResourceGroupControllerTest {
    @Autowired
    private ResourceGroupController resourceGroupController;

    @MockBean
    private ResourceGroupService resourceGroupService;

    @Test
    public void test_create_resource_group() {
        Mockito.when(resourceGroupService.createResourceGroup(Mockito.any())).thenReturn("uuid");
        UuidObject resourceGroup = resourceGroupController.createResourceGroup(getCreateExerciseRequest());
        Assert.assertEquals("uuid", resourceGroup.getUuid());
    }

    CreateResourceGroupRequest getCreateExerciseRequest(){
        CreateResourceGroupRequest createResourceGroupRequest = new CreateResourceGroupRequest();
        createResourceGroupRequest.setName("test");
        createResourceGroupRequest.setSourceSubType(ResourceSubTypeEnum.HYPER_V_VM.getType());
        createResourceGroupRequest.setScopeResourceId(UUID.randomUUID().toString());
        createResourceGroupRequest.setResourceIds(Collections.singletonList("test_resource_id"));
        return createResourceGroupRequest;
    }

    @Test
    public void test_update_resource_group_success() {
        Mockito.when(resourceGroupService.updateResourceGroup(Mockito.any())).thenReturn("uuid");
        UpdateResourceGroupRequest updateResourceGroupReq = getUpdateResourceGroupReq();
        UuidObject resourceGroup = resourceGroupController.updateResourceGroup("uuid", updateResourceGroupReq);
        Assert.assertEquals("uuid", resourceGroup.getUuid());
    }

    UpdateResourceGroupRequest getUpdateResourceGroupReq() {
        UpdateResourceGroupRequest updateResourceGroupReq = new UpdateResourceGroupRequest();
        updateResourceGroupReq.setName("name");
        List<String> resourceIds = new ArrayList<>();
        resourceIds.add("test");
        updateResourceGroupReq.setResourceIds(resourceIds);
        return updateResourceGroupReq;
    }

    @Test
    public void test_query_resource_groups() {
        PageListResponse pageListResponse = Mockito.mock(PageListResponse.class);
        Mockito.when(resourceGroupService.queryResourceGroups(
                Mockito.any(ResourceGroupQueryParams.class))).thenReturn(pageListResponse);
        PageListResponse<ResourceGroupVo> groupVoPageListResponse = resourceGroupController.queryResourceGroups("", 1
                , 10, "-created_time");
        Assert.assertNotNull(groupVoPageListResponse);
    }

    @Test
    public void test_delete_resource_group_success() {
        Mockito.when(resourceGroupService.deleteResourceGroup(Mockito.anyString())).thenReturn("uuid");
        UuidObject uuid = resourceGroupController.deleteResourceGroup("uuid");
        Assert.assertEquals(uuid.getUuid(), "uuid");
    }

    ResourceGroupDetailVo getResourceGroupDetailVo() {
        ResourceGroupDetailVo resourceGroupDetailVo = new ResourceGroupDetailVo();
        resourceGroupDetailVo.setUuid("test_resource_group_id");
        return resourceGroupDetailVo;
    }

    @Test
    public void test_query_resource_group_detail() {
        Mockito.when(resourceGroupService.queryResourceGroupDetail(Mockito.any())).thenReturn(
                getResourceGroupDetailVo());
        ResourceGroupDetailVo resourceGroupDetailVo = resourceGroupController.queryResourceGroupDetail(
                "test_resource_group_id");
        Assert.assertEquals("test_resource_group_id", resourceGroupDetailVo.getUuid());
    }
}