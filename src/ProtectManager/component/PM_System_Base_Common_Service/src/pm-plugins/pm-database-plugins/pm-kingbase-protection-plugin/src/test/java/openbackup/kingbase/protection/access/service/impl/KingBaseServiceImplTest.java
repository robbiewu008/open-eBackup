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
package openbackup.kingbase.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
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
 * {@link KingBaseServiceImpl} 测试类
 *
 */
public class KingBaseServiceImplTest {
    private final static String endpoint = "8.40.99.125";

    private final static Integer port = 54321;

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final InstanceProtectionService instanceProtectionService = PowerMockito.mock(
        InstanceProtectionService.class);

    private final KingBaseServiceImpl kingBaseService = new KingBaseServiceImpl(resourceService,
        instanceProtectionService);

    /**
     * 用例场景：获取部署类型
     * 前置条件：资源类型
     * 检查点：是否返回正确的部署类型
     */
    @Test
    public void get_deploy_type_success() {
        String singleDeployType = kingBaseService.getDeployType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        String apDeployType = kingBaseService.getDeployType(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), singleDeployType);
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(), apDeployType);
    }

    /**
     * 用例场景：获取子实例信息
     * 前置条件：实例资源
     * 检查点：是否返回正确的子实例
     */
    @Test
    public void get_sub_instance_success() {
        List<TaskResource> singleSubInstances = kingBaseService.getSubInstances(mockSingleInstance());
        Assert.assertEquals(IsmNumberConstant.ZERO, singleSubInstances.size());
        List<TaskResource> clusterSubInstances = kingBaseService.getSubInstances(mockClusterInstance());
        Assert.assertEquals(IsmNumberConstant.TWO, clusterSubInstances.size());
    }

    /**
     * 用例场景：查询实例资源
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void get_resource_by_id_fail() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.empty());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> kingBaseService.getResourceById(any()));
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询实例资源
     * 前置条件：资源存在
     * 检查点：无异常抛出
     */
    @Test
    public void get_resource_by_id_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockSingleInstance()));
        ProtectedResource resource = kingBaseService.getResourceById(any());
        Assert.assertEquals(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType(), resource.getSubType());
    }

    /**
     * 用例场景：获取单实例环境node
     * 前置条件：单实例信息
     * 检查点：返回个数正确
     */
    @Test
    public void get_env_agents_by_single_instance_resource_success() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesBySingleInstance(any())).thenReturn(mockSingleNodes());
        List<Endpoint> singleInstanceAgents = kingBaseService.getAgentsByInstanceResource(mockSingleInstance());
        Assert.assertEquals(IsmNumberConstant.ONE, singleInstanceAgents.size());
    }

    /**
     * 用例场景：获取集群实例环境node
     * 前置条件：集群实例信息
     * 检查点：返回个数正确
     */
    @Test
    public void get_env_agents_by_cluster_instance_resource_success() {
        PowerMockito.when(instanceProtectionService.extractEnvNodesByClusterInstance(any())).thenReturn(mockClusterNodes());
        List<Endpoint> singleInstanceAgents = kingBaseService.getAgentsByInstanceResource(mockClusterInstance());
        Assert.assertEquals(IsmNumberConstant.TWO, singleInstanceAgents.size());
    }

    private ProtectedResource mockSingleInstance() {
        ProtectedResource singleInstance = new ProtectedResource();
        singleInstance.setSubType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        return singleInstance;
    }

    private ProtectedResource mockClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setSubType(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        List<ProtectedResource> children = new ArrayList<>();
        children.add(new ProtectedResource());
        children.add(new ProtectedResource());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        clusterInstance.setDependencies(dependencies);
        return clusterInstance;
    }

    private List<TaskEnvironment> mockSingleNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setEndpoint(endpoint);
        taskEnvironment.setPort(port);
        List<TaskEnvironment> singleNodes = new ArrayList<>();
        singleNodes.add(taskEnvironment);
        return singleNodes;
    }

    private List<TaskEnvironment> mockClusterNodes() {
        TaskEnvironment taskEnvironmentOne = new TaskEnvironment();
        taskEnvironmentOne.setUuid(UUID.randomUUID().toString());
        taskEnvironmentOne.setEndpoint(endpoint);
        taskEnvironmentOne.setPort(port);
        TaskEnvironment taskEnvironmentTwo = BeanTools.copy(taskEnvironmentOne, TaskEnvironment::new);
        List<TaskEnvironment> clusterNodes = new ArrayList<>();
        clusterNodes.add(taskEnvironmentOne);
        clusterNodes.add(taskEnvironmentTwo);
        return clusterNodes;
    }
}