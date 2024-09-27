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
package openbackup.database.base.plugin.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link InstanceResourceServiceImpl} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class InstanceResourceServiceImplTest {
    private static final String DMR_PROXY_IP = "8.40.99.187";

    private static final Integer DME_PROXY_PORT = 8090;

    private static final String PRIMARY_ROLE = "1";

    private final AgentUnifiedRestApi agentApi = Mockito.mock(AgentUnifiedRestApi.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final CommonAlarmService commonAlarmService = PowerMockito.mock(CommonAlarmService.class);

    private final MemberClusterService memberClusterService = PowerMockito.mock(MemberClusterService.class);

    private InstanceResourceServiceImpl instanceResourceService = new InstanceResourceServiceImpl(resourceService,
        agentUnifiedService, commonAlarmService, memberClusterService);

    /**
     * 用例场景：校验集群实例是否被注册
     * 前置条件：集群实例已经被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_registered_when_check_cluster_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> instanceResourceService.checkClusterInstanceIsRegistered(mockResource()));
        Assert.assertEquals("This cluster instance is registered.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验集群实例是否被注册
     * 前置条件：集群实例未注册
     * 检查点：无异常抛出
     */
    @Test
    public void check_cluster_instance_is_registered_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        instanceResourceService.checkClusterInstanceIsRegistered(mockResource());
        Mockito.verify(resourceService, Mockito.times(1)).query(anyInt(), anyInt(), any());
    }

    /**
     * 用例场景：检查集群实例
     * 前置条件：不是一个集群实例
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_is_not_cluster_instance_when_check_cluster_instance() {
        PowerMockito.when(agentApi.check(any(), any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "{\"bodyErr\": 1677929488, 	\"code\": 200, 	\"message\": \"Login denied\" }"));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> instanceResourceService.checkIsClusterInstance(mockResource()));
        Assert.assertEquals("Cluster instance check fail.", legoCheckedException.getMessage());
        Assert.assertEquals(1677929488L, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群实例
     * 前置条件：属于一个集群实例
     * 检查点：无异常抛出
     */
    @Test
    public void check_is_cluster_instance_success() {
        instanceResourceService.checkIsClusterInstance(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查集群实例端口是否被修改
     * 前置条件：端口被修改
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_port_changed_when_check_cluster_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> instanceResourceService.checkClusterInstancePortIsChanged(mockResource()));
        Assert.assertEquals("This cluster instance port is changed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.RESOURCE_PORT_IS_MODIFIED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群实例端口是否被修改
     * 前置条件：端口没有修改
     * 检查点：无异常抛出
     */
    @Test
    public void check_cluster_instance_port_is_changed_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        instanceResourceService.checkClusterInstancePortIsChanged(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：设置节点角色
     * 前置条件：参数正常
     * 检查点：节点角色是否正确
     */
    @Test
    public void set_cluster_instance_node_role_success() {
        Mockito.when(agentApi.queryCluster(any(), any(), any())).thenReturn(mockAppEnvResponse());
        ProtectedResource resource = mockResource();
        instanceResourceService.setClusterInstanceNodeRole(resource);
        String actualRole = resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .get(IsmNumberConstant.ZERO)
            .getExtendInfo()
            .get(DatabaseConstants.ROLE);
        Assert.assertEquals(PRIMARY_ROLE, actualRole);
    }

    /**
     * 用例场景：校验单实例是否被注册
     * 前置条件：单实例已经被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_registered_when_check_signal_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> instanceResourceService.checkSignalInstanceIsRegistered(mockResource()));
        Assert.assertEquals("This signal instance is registered.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验单实例是否被注册
     * 前置条件：单实例未被注册
     * 检查点：无异常抛出
     */
    @Test
    public void check_signal_instance_is_registered_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        instanceResourceService.checkSignalInstanceIsRegistered(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查单实例端口是否被修改
     * 前置条件：端口被修改
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_port_changed_when_check_signal_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> instanceResourceService.checkSignalInstancePortIsChanged(mockResource()));
        Assert.assertEquals("This signal instance port is changed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.RESOURCE_PORT_IS_MODIFIED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查单实例端口是否被修改
     * 前置条件：端口未被修改
     * 检查点：无异常抛出
     */
    @Test
    public void check_signal_instance_port_is_changed_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        instanceResourceService.checkSignalInstancePortIsChanged(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查单实例状态
     * 前置条件：认证信息正常
     * 检查点：状态为在线
     */
    @Test
    public void check_single_instance_online_status() {
        ProtectedResource resource = mockSignalInstance();
        instanceResourceService.healthCheckSingleInstance(resource);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：检查单实例状态
     * 前置条件：认证信息异常
     * 检查点：状态为离线
     */
    @Test
    public void check_single_instance_offline_status() {
        PowerMockito.when(agentApi.check(any(), any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "{\"bodyErr\": 1677929488, 	\"code\": 200, 	\"message\": \"Login denied\" }"));
        ProtectedResource resource = mockSignalInstance();
        instanceResourceService.healthCheckSingleInstance(resource);
        Assert.assertEquals(LinkStatusEnum.OFFLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：检查集群环境下的实例状态，只要任意节点的实例在线
     * 前置条件：认证信息正常
     * 检查点：无异常抛出
     */
    @Test
    public void any_node_instance_online_when_check_cluster_instance_of_environment() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstance());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        instanceResourceService.healthCheckClusterInstanceOfEnvironment(mockEnvironment(),
            ClusterInstanceOnlinePolicy.ANY_NODE_ONLINE);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查集群环境下的实例状态，所有节点的实例在线
     * 前置条件：状态正常
     * 检查点：无异常抛出
     */
    @Test
    public void all_nodes_instance_online_when_check_cluster_instance_of_environment() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstance());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        instanceResourceService.healthCheckClusterInstanceOfEnvironment(mockEnvironment(),
            ClusterInstanceOnlinePolicy.ALL_NODES_ONLINE);
        Assert.assertTrue(true);
    }

    private PageListResponse<ProtectedResource> mockInstanceNum(int instanceNum) {
        PageListResponse<ProtectedResource> instances = new PageListResponse<>();
        instances.setTotalCount(instanceNum);
        return instances;
    }

    private PageListResponse<ProtectedResource> mockInstance() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setAuth(new Authentication());
        List<ProtectedResource> dataList = new ArrayList<>();
        dataList.add(protectedResource);
        PageListResponse<ProtectedResource> instances = new PageListResponse<>();
        instances.setRecords(dataList);
        return instances;
    }

    private ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setParentUuid(UUID.randomUUID().toString());
        return environment;
    }

    private ProtectedResource mockSignalInstance() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(DMR_PROXY_IP);
        environment.setPort(DME_PROXY_PORT);
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(environment);
        resource.setAuth(new Authentication());
        return resource;
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid(UUID.randomUUID().toString());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "5432");
        resource.setExtendInfo(extendInfo);
        ProtectedResource subInstance = new ProtectedResource();
        subInstance.setExtendInfoByKey(DatabaseConstants.HOST_ID, UUID.randomUUID().toString());
        subInstance.setExtendInfoByKey(DatabaseConstants.SERVICE_IP, "127.0.0.1");
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(DMR_PROXY_IP);
        environment.setPort(DME_PROXY_PORT);
        subInstance.setEnvironment(environment);
        subInstance.setAuth(new Authentication());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("d9c6a90e-e86d-473b-9d1d-793982b1c6c1");
        agent.setEndpoint(DMR_PROXY_IP);
        agent.setPort(DME_PROXY_PORT);
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> subDependencies = new HashMap();
        subDependencies.put(DatabaseConstants.AGENTS, agents);
        subInstance.setDependencies(subDependencies);
        List<ProtectedResource> children = new ArrayList<>();
        children.add(subInstance);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        resource.setDependencies(dependencies);
        resource.setEnvironment(mockEnv());
        return resource;
    }

    private ProtectedEnvironment mockEnv() {
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid("d9c6a90e-e86d-473b-9d1d-793982b1c6c1");
        agentEnv.setEndpoint(DMR_PROXY_IP);
        agentEnv.setPort(DME_PROXY_PORT);
        agents.add(agentEnv);
        Map<String, List<ProtectedResource>> envDependencies = new HashMap<>();
        envDependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(envDependencies);
        return environment;
    }

    private AppEnvResponse mockAppEnvResponse() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.ROLE, PRIMARY_ROLE);
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(extendInfo);
        nodeInfo.setEndpoint("127.0.0.1");
        List<NodeInfo> nodeInfos = new ArrayList<>();
        nodeInfos.add(nodeInfo);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(nodeInfos);
        return appEnvResponse;
    }
}