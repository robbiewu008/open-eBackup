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

import openbackup.access.framework.resource.persistence.dao.ResourceGroupMapper;
import openbackup.access.framework.resource.persistence.dao.ResourceGroupMemberMapper;
import openbackup.access.framework.resource.service.ResourceGroupRepository;
import openbackup.access.framework.resource.service.ResourceGroupServiceImpl;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.dao.ProtectedTaskMapper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupMemberDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupProtectedObjectLabelDto;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.CreateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ResourceGroupQueryParams;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.UpdateResourceGroupProtectedObjectRequest;
import openbackup.data.protection.access.provider.sdk.resourcegroup.resp.ResourceGroupDetailVo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;

/**
 * ResourceGroupServiceImplTest
 *
 */
@RunWith(MockitoJUnitRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {ResourceGroupServiceImpl.class})
public class ResourceGroupServiceImplTest {
    @Mock
    private ResourceGroupRepository resourceGroupRepository;

    @Mock
    private ResourceGroupMapper resourceGroupMapper;

    @Mock
    private ResourceGroupMemberMapper resourceGroupMemberMapper;

    @Mock
    private ProtectedObjectMapper protectedObjectMapper;

    @Mock
    private ProtectedTaskMapper protectedTaskMapper;

    @Mock
    private ProtectObjectRestApi protectObjectRestApi;

    @Mock
    private ResourceService resourceService;

    @Mock
    private JobService jobService;

    @InjectMocks
    private ResourceGroupServiceImpl resourceGroupService;

    @Before
    public void before() {
        Mockito.when(resourceGroupRepository.save(Mockito.any())).thenReturn("resource_group_id");
    }

    ResourceGroupDto getResourceGroupDto() {
        ResourceGroupDto resourceGroupVo = new ResourceGroupDto();
        resourceGroupVo.setName("test");
        resourceGroupVo.setUuid("test_resource_group");
        resourceGroupVo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        resourceGroupVo.setSourceSubType(ResourceSubTypeEnum.MICROSOFT_VIRTUAL_MACHINE.getType());
        List<ResourceGroupMemberDto> resources = new ArrayList<>();
        ResourceGroupMemberDto resource = new ResourceGroupMemberDto();
        resource.setSourceId("test_resource_id");
        resource.setSourceSubType(ResourceSubTypeEnum.MICROSOFT_VIRTUAL_MACHINE.getType());
        resources.add(resource);
        resourceGroupVo.setResources(resources);
        return resourceGroupVo;
    }

    PageListResponse<ProtectedResource> getResourceQueryResponse() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.MICROSOFT_VIRTUAL_MACHINE.getType());
        protectedResource.setUuid("test_resource_id");
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(protectedResource);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(protectedResources);
        pageListResponse.setTotalCount(1);
        return pageListResponse;
    }

    private BasePage<ResourceGroupDto> getBasepageResourceGroupDto() {
        BasePage<ResourceGroupDto> groupPoBasePage = new BasePage<>();
        groupPoBasePage.setPages(1);
        groupPoBasePage.setPageNo(0);
        groupPoBasePage.setTotal(1);
        groupPoBasePage.setPageSize(1);
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        resourceGroupDto.setUuid("test");
        resourceGroupDto.setName("test");
        resourceGroupDto.setPath("test");
        resourceGroupDto.setSourceSubType("test");
        resourceGroupDto.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        resourceGroupDto.setUserId("test");
        List<ResourceGroupDto> groupPos = new ArrayList<>();
        groupPos.add(resourceGroupDto);
        groupPoBasePage.setItems(groupPos);
        return groupPoBasePage;
    }

    /**
     * 测试用例：创建资源组成功(含一个组成员)
     * 前置条件：对数据库打桩，资源查询返回打桩
     * CHECK点：数据库表新增资源组数据记录
     */
    @Test
    public void should_create_resource_group_success_with_1_group_member() {
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(getResourceQueryResponse());
        String resourceGroupId = resourceGroupService.createResourceGroup(getResourceGroupDto());
        Assert.assertEquals("resource_group_id", resourceGroupId);
    }

    /**
     * 测试用例：创建资源组成功(含0个组成员)
     * 前置条件：对数据库打桩，资源查询返回打桩
     * CHECK点：数据库表新增资源组数据记录
     */
    @Test
    public void should_create_resource_group_success_with_0_group_member() {
        ResourceGroupDto resourceGroupDto = getResourceGroupDto();
        resourceGroupDto.setResources(new ArrayList<>());
        String resourceGroupId = resourceGroupService.createResourceGroup(resourceGroupDto);
        Assert.assertEquals("resource_group_id", resourceGroupId);
    }

    /**
     * 测试用例：创建资源组，资源不存在
     * 前置条件：对数据库打桩，对资源查询接口打桩，模拟资源不存在
     * CHECK点：创建失败，抛出错误码提示
     */
    @Test(expected = LegoCheckedException.class)
    public void create_resource_group_fail_when_resource_not_exist(){
        PageListResponse<ProtectedResource> pageListResponse = getResourceQueryResponse();
        pageListResponse.setTotalCount(0);
        pageListResponse.setRecords(new ArrayList<>());
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);
        resourceGroupService.createResourceGroup(getResourceGroupDto());
    }

    /**
     * 测试用例：创建资源组，资源类型和资源组类型不匹配
     * 前置条件：对数据库打桩，对资源查询接口打桩，模拟资源类型和组类型不一致
     * CHECK点：创建失败，抛出错误码提示
     */
    @Test(expected = LegoCheckedException.class)
    public void create_resource_group_fail_when_resource_sub_type_not_match(){
        PageListResponse<ProtectedResource> pageListResponse = getResourceQueryResponse();
        pageListResponse.getRecords().get(0).setSubType("other_type");
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);
        resourceGroupService.createResourceGroup(getResourceGroupDto());
    }

    /**
     * 测试用例：创建资源组，资源类型和资源组类型不匹配
     * 前置条件：对数据库打桩，对资源查询接口打桩，模拟资源类型和组类型不一致
     * CHECK点：创建失败，抛出错误码提示
     */
    @Test(expected = LegoCheckedException.class)
    public void create_resource_group_fail_when_resource_has_protected_obj(){
        PageListResponse<ProtectedResource> pageListResponse = getResourceQueryResponse();
        pageListResponse.getRecords().get(0).setProtectedObject(new ProtectedObject());
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);
        resourceGroupService.createResourceGroup(getResourceGroupDto());
    }

    /**
     * 测试用例：更新资源组成功
     * 前置条件：无
     * CHECK点：更新逻辑无问题
     */
    @Test
    public void update_resource_group_success() {
        ResourceGroupDto resourceGroupDto = getResourceGroupDto();
        Optional<ResourceGroupDto> optionalResourceGroupDto = Optional.of(resourceGroupDto);
        Mockito.when(resourceGroupRepository.selectById(Mockito.any())).thenReturn(optionalResourceGroupDto);
        Mockito.when(resourceGroupRepository.selectByScopeResourceIdAndName(Mockito.any(), Mockito.any())).thenReturn(optionalResourceGroupDto);
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(getResourceQueryResponse());
        resourceGroupService.updateResourceGroup(resourceGroupDto);
    }

    /**
     * 测试用例：查找资源组列表成功
     * 前置条件：无
     * CHECK点：查找逻辑无问题
     */
    @Test
    public void test_query_resource_groups_success() {
        Mockito.when(resourceGroupRepository.queryResourceGroups(any())).thenReturn(getBasepageResourceGroupDto());
        resourceGroupService.queryResourceGroups(new ResourceGroupQueryParams());
        Mockito.verify(resourceGroupRepository, Mockito.times(1)).queryResourceGroups(any());
    }

    /**
     * 测试用例：删除资源组成功
     * 前置条件：组成员不为空，repository 层打桩
     * CHECK点：组成员不为空时，查询无问题
     */
    @Test
    public void test_delete_resource_group_success() {
        Optional<ResourceGroupDto> optionalResourceGroupDto = Optional.of(getResourceGroupDto());
        Mockito.when(resourceGroupRepository.selectById(any())).thenReturn(optionalResourceGroupDto);
        resourceGroupService.deleteResourceGroup("resource_group_id");
        Mockito.verify(resourceGroupRepository, Mockito.times(1)).delete(any());
    }

    /**
     * 测试用例：查找资源组详情成功
     * 前置条件：组成员不为空，repository 层打桩
     * CHECK点：组成员不为空时，查询无问题
     */
    @Test
    public void test_query_resource_group_detail_success() {
        Optional<ResourceGroupDto> optionalResourceGroupDto = Optional.of(getResourceGroupDto());
        Mockito.when(resourceGroupRepository.selectById(any())).thenReturn(optionalResourceGroupDto);
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(getResourceQueryResponse());
        ResourceGroupDetailVo resourceGroupDetailVo =
                resourceGroupService.queryResourceGroupDetail("test_resource_group");
        Assert.assertEquals("test_resource_id",
                resourceGroupDetailVo.getResourceGroupMembers().get(0).getSourceId());
    }

    @Test
    public void test_get_resource_group_label_success() {
        CreateResourceGroupProtectedObjectRequest groupReq = new CreateResourceGroupProtectedObjectRequest();
        groupReq.setResourceGroupId("resource_group_id");
        groupReq.setSlaId("fake_sla_id");
        ResourceGroupProtectedObjectLabelDto resourceGroupLabel = resourceGroupService.getResourceGroupLabel(groupReq);
        Assert.assertEquals(resourceGroupLabel.getLabel(), "protection_without_backup_label");

        // postAction不为空时会设置手动备份标记
        groupReq.setPostAction("manual_backup");
        Assert.assertEquals(resourceGroupLabel.getLabel(), "protection_without_backup_label");
    }

    /**
     * 测试用例：保护前置检查保护对象已被保护
     * 前置条件：保护对象不为空
     * CHECK点：保护对象不为空时，抛出异常
     */
    @Test
    public void test_pre_check_before_protect_when_protect_object_not_empty_should_throw_exception() throws Exception {
        ProtectedObjectInfo protectObject = new ProtectedObjectInfo();
        protectObject.setResourceId("123");
        CreateResourceGroupProtectedObjectRequest createRequest = new CreateResourceGroupProtectedObjectRequest();
        createRequest.setResourceGroupId("resource_group_id");
        PowerMockito.when(protectObjectRestApi.getProtectObject(createRequest.getResourceGroupId()))
                .thenReturn(protectObject);
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        Class<ResourceGroupServiceImpl> serviceClass = ResourceGroupServiceImpl.class;
//        ResourceGroupServiceImpl resourceGroupService = new ResourceGroupServiceImpl();
        Method privateMethod = serviceClass.getDeclaredMethod("preCheckBeforeProtect",
                CreateResourceGroupProtectedObjectRequest.class, ResourceGroupDto.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
                () -> privateMethod.invoke(resourceGroupService, createRequest, resourceGroupDto));
    }

    /**
     * 测试用例：保护前置检查资源组为空
     * 前置条件：资源组为空
     * CHECK点：资源组为空时，抛出异常
     */
    @Test
    public void test_pre_check_before_protect_when_resource_group_empty_should_throw_exception() throws Exception {
        CreateResourceGroupProtectedObjectRequest createRequest = new CreateResourceGroupProtectedObjectRequest();
        createRequest.setResourceGroupId("resource_group_id");
        PowerMockito.when(protectObjectRestApi.getProtectObject(createRequest.getResourceGroupId()))
                .thenReturn(null);
        ResourceGroupDto resourceGroupDto = new ResourceGroupDto();
        Class<ResourceGroupServiceImpl> serviceClass = ResourceGroupServiceImpl.class;
        Method privateMethod = serviceClass.getDeclaredMethod("preCheckBeforeProtect",
                CreateResourceGroupProtectedObjectRequest.class, ResourceGroupDto.class);
        privateMethod.setAccessible(true);
        Assert.assertThrows(InvocationTargetException.class,
                () -> privateMethod.invoke(resourceGroupService, createRequest, resourceGroupDto));
    }

    /**
     * 测试用例：构造创建保护对象request成功
     * 前置条件：无
     * CHECK点：构造成功，返回保护对象request
     */
    @Test
    public void test_build_create_protected_object_request_should_return_protected_object_request() throws Exception {
        CreateResourceGroupProtectedObjectRequest createRequest = new CreateResourceGroupProtectedObjectRequest();
        createRequest.setResourceGroupId("123");
        Class<ResourceGroupServiceImpl> serviceClass = ResourceGroupServiceImpl.class;
        Method privateMethod = serviceClass.getDeclaredMethod("buildCreateProtectedObjectRequest",
                CreateResourceGroupProtectedObjectRequest.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(resourceGroupService, createRequest);
        Assert.assertEquals("123", createRequest.getResourceGroupId());
    }

    /**
     * 测试用例：构造更新保护对象request成功
     * 前置条件：无
     * CHECK点：构造成功，返回保护对象request
     */
    @Test
    public void test_build_update_protected_object_request_should_return_protected_object_request() throws Exception {
        UpdateResourceGroupProtectedObjectRequest updateRequest = new UpdateResourceGroupProtectedObjectRequest();
        updateRequest.setResourceGroupId("123");
        Class<ResourceGroupServiceImpl> serviceClass = ResourceGroupServiceImpl.class;
        Method privateMethod = serviceClass.getDeclaredMethod("buildUpdateProtectedObjectRequest",
                UpdateResourceGroupProtectedObjectRequest.class);
        privateMethod.setAccessible(true);
        privateMethod.invoke(resourceGroupService, updateRequest);
        Assert.assertEquals("123", updateRequest.getResourceGroupId());
    }
}
