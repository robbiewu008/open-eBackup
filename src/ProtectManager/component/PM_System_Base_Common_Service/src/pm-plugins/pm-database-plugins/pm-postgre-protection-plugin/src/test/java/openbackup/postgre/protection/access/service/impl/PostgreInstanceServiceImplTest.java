/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.postgre.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * postgre实例服务测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */
public class PostgreInstanceServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final InstanceProtectionService instanceProtectionService = PowerMockito.mock(
        InstanceProtectionService.class);

    private final PostgreInstanceServiceImpl serviceImpl = new PostgreInstanceServiceImpl(resourceService,
        instanceProtectionService);

    /**
     * 用例场景：根据资源ID查询资源信息
     * 前置条件：资源存在
     * 检查点：返回资源信息
     */
    @Test
    public void getResourceById_success() {
        String resourceUuid = UUID.randomUUID().toString();
        ProtectedResource mockResource = new ProtectedResource();
        mockResource.setUuid(resourceUuid);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(mockResource));
        ProtectedResource resource = serviceImpl.getResourceById(resourceUuid);
        Assert.assertEquals(resource.getUuid(), resourceUuid);
    }

    /**
     * 用例场景：获取单实例环境node
     * 前置条件：单实例信息
     * 检查点：返回个数正确
     */
    @Test
    public void should_return_agents_info_if_single_instance_when_get_agent() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any()))
            .thenReturn(mockSingleNodes());
        List<Endpoint> singleInstanceAgents = serviceImpl.getAgentsByInstanceResource(mockSingleInstance());
        Assert.assertEquals(IsmNumberConstant.ONE, singleInstanceAgents.size());
    }

    /**
     * 用例场景：获取集群实例环境node
     * 前置条件：集群实例信息
     * 检查点：返回个数正确
     */
    @Test
    public void should_return_agents_info_if_cluster_instance_when_get_agent() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesByClusterInstance(any()))
            .thenReturn(mockClusterNodes());
        List<Endpoint> singleInstanceAgents = serviceImpl.getAgentsByInstanceResource(mockClusterInstance());
        Assert.assertEquals(IsmNumberConstant.TWO, singleInstanceAgents.size());
    }

    private List<TaskEnvironment> mockSingleNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setEndpoint("127.0.0.1");
        taskEnvironment.setPort(59521);
        List<TaskEnvironment> singleNodes = new ArrayList<>();
        singleNodes.add(taskEnvironment);
        return singleNodes;
    }

    private List<TaskEnvironment> mockClusterNodes() {
        TaskEnvironment taskEnvironmentOne = new TaskEnvironment();
        taskEnvironmentOne.setUuid(UUID.randomUUID().toString());
        taskEnvironmentOne.setEndpoint("127.0.0.2");
        taskEnvironmentOne.setPort(59521);
        TaskEnvironment taskEnvironmentTwo = BeanTools.copy(taskEnvironmentOne, TaskEnvironment::new);
        List<TaskEnvironment> clusterNodes = new ArrayList<>();
        clusterNodes.add(taskEnvironmentOne);
        clusterNodes.add(taskEnvironmentTwo);
        return clusterNodes;
    }

    private ProtectedResource mockSingleInstance() {
        ProtectedResource singleInstance = new ProtectedResource();
        singleInstance.setSubType(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        return singleInstance;
    }

    private ProtectedResource mockClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        List<ProtectedResource> children = new ArrayList<>();
        children.add(new ProtectedResource());
        children.add(new ProtectedResource());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        clusterInstance.setDependencies(dependencies);
        return clusterInstance;
    }
}