/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.saphana.protection.access.service.impl;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * {@link SapHanaResourceServiceImpl Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class SapHanaResourceServiceImplTest {
    private final ResourceConnectionCheckProvider connectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final SapHanaResourceServiceImpl hanaResourceService = new SapHanaResourceServiceImpl(
        connectionCheckProvider, environmentService, resourceService);

    /**
     * 用例场景：根据包含uuid信息的ProtectedResource列表查询ProtectedEnvironment列表
     * 前置条件：主机都存在
     * 检查点：返回ProtectedEnvironment列表
     */
    @Test
    public void should_return_list_if_hosts_exists_when_queryEnvironments() {
        ProtectedResource firstResource = new ProtectedResource();
        firstResource.setUuid("eb1db427bd9243ad9e3acfe8a0744b7b");
        ProtectedEnvironment firstEnv = new ProtectedEnvironment();
        firstEnv.setUuid("eb1db427bd9243ad9e3acfe8a0744b7b");
        firstEnv.setEndpoint("10.10.10.11");
        ProtectedResource secondResource = new ProtectedResource();
        secondResource.setUuid("b7c1ad7c6f9448a7a3ee4980ccac4c37");
        ProtectedEnvironment secondEnv = new ProtectedEnvironment();
        secondEnv.setUuid("b7c1ad7c6f9448a7a3ee4980ccac4c37");
        secondEnv.setEndpoint("10.10.10.12");
        PowerMockito.when(environmentService.getEnvironmentById("eb1db427bd9243ad9e3acfe8a0744b7b"))
            .thenReturn(firstEnv);
        PowerMockito.when(environmentService.getEnvironmentById("b7c1ad7c6f9448a7a3ee4980ccac4c37"))
            .thenReturn(secondEnv);
        List<ProtectedResource> agents = Arrays.asList(firstResource, secondResource);
        List<ProtectedEnvironment> envs = hanaResourceService.queryEnvironments(agents);
        Assert.assertEquals(envs.size(), 2);
    }

    /**
     * 用例场景：根据条件查询资源信息
     * 前置条件：存在符合条件的资源信息
     * 检查点：返回ProtectedResource列表
     */
    @Test
    public void should_return_list_if_query_success_when_listResourcesByConditions() {
        ProtectedResource firstResource = new ProtectedResource();
        firstResource.setUuid("eb1db427bd9243ad9e3acfe8a0744b7b");
        ProtectedResource secondResource = new ProtectedResource();
        secondResource.setUuid("b7c1ad7c6f9448a7a3ee4980ccac4c37");
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(2);
        queryResult.setRecords(Arrays.asList(firstResource, secondResource));
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        List<ProtectedResource> resources = hanaResourceService.listResourcesByConditions(conditions);
        Assert.assertEquals(resources.size(), 2);
    }

    /**
     * 用例场景：根据资源uuid查询资源信息
     * 前置条件：资源不存在
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_ex_if_resource_not_exists_when_getResourceById() {
        String resourceId = "eb1db427bd9243ad9e3acfe8a0744b7b";
        ProtectedResource tmpResource = new ProtectedResource();
        tmpResource.setUuid(resourceId);
        PowerMockito.when(resourceService.getResourceById(resourceId)).thenReturn(Optional.empty());
        hanaResourceService.getResourceById(resourceId);
    }

    /**
     * 用例场景：根据资源uuid查询资源信息
     * 前置条件：资源存在
     * 检查点：返回资源信息
     */
    @Test
    public void should_return_ProtectedResource_if_resource_exists_when_getResourceById() {
        String resourceId = "eb1db427bd9243ad9e3acfe8a0744b7b";
        ProtectedResource tmpResource = new ProtectedResource();
        tmpResource.setUuid(resourceId);
        PowerMockito.when(resourceService.getResourceById(resourceId)).thenReturn(Optional.of(tmpResource));
        Assert.assertNotNull(hanaResourceService.getResourceById(resourceId));
    }

    /**
     * 用例场景：检查已注册实例数目
     * 前置条件：已注册实例数目大于等于上限
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_ex_if_instances_num_exceed_limit_when_checkInstanceNumber() {
        List<ProtectedResource> resources = new ArrayList<>();
        for (int i = 0; i < SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT; i++) {
            ProtectedResource tmpResource = new ProtectedResource();
            tmpResource.setUuid("eb1db427bd9243ad9e3acfe8a0744b7b");
            resources.add(tmpResource);
        }
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT);
        queryResult.setRecords(resources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkInstanceNumber();
    }

    /**
     * 用例场景：检查已注册实例数目
     * 前置条件：已注册实例数目小于上限
     * 检查点：不抛错
     */
    @Test
    public void should_not_throw_ex_if_instances_num_meet_limit_when_checkInstanceNumber() {
        int tmpInstNum = SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT - 1;
        List<ProtectedResource> resources = new ArrayList<>();
        for (int i = 0; i < tmpInstNum; i++) {
            ProtectedResource tmpResource = new ProtectedResource();
            tmpResource.setUuid("eb1db427bd9243ad9e3acfe8a0744b7b");
            resources.add(tmpResource);
        }
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(tmpInstNum);
        queryResult.setRecords(resources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkInstanceNumber();
    }

    /**
     * 用例场景：检查实例资源是否已注册
     * 前置条件：不存在和待注册实例systemId相同的资源
     * 检查点：不抛错
     */
    @Test
    public void should_not_throw_ex_if_no_same_sid_inst_when_checkInstanceIsRegistered() {
        ProtectedEnvironment newInstance = new ProtectedEnvironment();
        newInstance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "a00");
        newInstance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        newInstance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        List<ProtectedResource> resources = new ArrayList<>();
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(0);
        queryResult.setRecords(resources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkInstanceIsRegistered(newInstance);
    }

    /**
     * 用例场景：检查实例资源是否已注册
     * 前置条件：存在和待注册实例systemId相同的资源，且有主机已被使用
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_ex_if_exists_same_sid_inst_and_host_has_used_when_checkInstanceIsRegistered() {
        ProtectedEnvironment newInstance = new ProtectedEnvironment();
        newInstance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "a00");
        newInstance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        newInstance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        ProtectedResource secondAgent = new ProtectedResource();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        List<ProtectedResource> agents = Arrays.asList(firstAgent, secondAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        newInstance.setDependencies(dependencies);
        ProtectedResource existedInstance = new ProtectedResource();
        existedInstance.setUuid("801d0200-6131-4fab-883c-dff7bb1bac0b ");
        String nodesStr =
            "[{\"uuid\":\"d0de521f-1475-413e-a902-ec9fd7b5e42a\",\"type\":\"Host\",\"subType\":\"UBackupAgent\"},"
                + "{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"type\":\"Host\",\"subType\":\"UBackupAgent\"}]";
        existedInstance.setExtendInfoByKey(SapHanaConstants.NODES, nodesStr);
        List<ProtectedResource> existedResources = new ArrayList<>();
        existedResources.add(existedInstance);
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(1);
        queryResult.setRecords(existedResources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkInstanceIsRegistered(newInstance);
    }

    /**
     * 用例场景：检查数据库资源是否已注册
     * 前置条件：存在和待注册实例systemId相同的资源，且实例的主机都未使用
     * 检查点：不抛错
     */
    @Test
    public void should_not_throw_ex_if_exists_same_sid_inst_and_hosts_no_used_when_checkInstanceIsRegistered() {
        ProtectedEnvironment newInstance = new ProtectedEnvironment();
        newInstance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "a00");
        newInstance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        newInstance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        ProtectedResource secondAgent = new ProtectedResource();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        List<ProtectedResource> agents = Arrays.asList(firstAgent, secondAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        newInstance.setDependencies(dependencies);
        ProtectedResource existedInstance = new ProtectedResource();
        existedInstance.setUuid("801d0200-6131-4fab-883c-dff7bb1bac0b ");
        String nodesStr =
            "[{\"uuid\":\"d0de521f-1475-413e-a902-ec9fd7b5e42a\",\"type\":\"Host\",\"subType\":\"UBackupAgent\"},"
                + "{\"uuid\":\"6413d987-3c27-4e3d-92d0-aa18bdcf4a4c\",\"type\":\"Host\",\"subType\":\"UBackupAgent\"}]";
        existedInstance.setExtendInfoByKey(SapHanaConstants.NODES, nodesStr);
        List<ProtectedResource> existedResources = new ArrayList<>();
        existedResources.add(existedInstance);
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(1);
        queryResult.setRecords(existedResources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkInstanceIsRegistered(newInstance);
    }

    /**
     * 用例场景：检查数据库资源是否已注册
     * 前置条件：该数据库资源已注册
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_ex_if_db_is_registered_exceed_limit_when_checkDbIsRegistered() {
        String instanceId = "eb1db427bd9243ad9e3acfe8a0744b7b";
        ProtectedResource newDbResource = new ProtectedResource();
        newDbResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        newDbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        newDbResource.setParentUuid(instanceId);
        newDbResource.setName("SYSTEMDB");
        String resourceId = "5ded83e5f6364678a26f08bd8cccd2a9";
        ProtectedResource existedDbResource = BeanTools.clone(newDbResource);
        existedDbResource.setUuid(resourceId);
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(existedDbResource);
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(1);
        queryResult.setRecords(resources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkDbIsRegistered(newDbResource);
    }

    /**
     * 用例场景：检查数据库资源是否已注册
     * 前置条件：该数据库资源未注册
     * 检查点：不抛错
     */
    @Test
    public void should_not_throw_ex_if_db_not_registered_when_checkDbIsRegistered() {
        String instanceId = "eb1db427bd9243ad9e3acfe8a0744b7b";
        ProtectedResource newDbResource = new ProtectedResource();
        newDbResource.setSourceType(ResourceTypeEnum.DATABASE.getType());
        newDbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        newDbResource.setParentUuid(instanceId);
        newDbResource.setName("SYSTEMDB");
        List<ProtectedResource> resources = new ArrayList<>();
        PageListResponse<ProtectedResource> queryResult = new PageListResponse<>();
        queryResult.setTotalCount(0);
        queryResult.setRecords(resources);
        PowerMockito.when(
            resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.any()))
            .thenReturn(queryResult);
        hanaResourceService.checkDbIsRegistered(newDbResource);
    }
}
