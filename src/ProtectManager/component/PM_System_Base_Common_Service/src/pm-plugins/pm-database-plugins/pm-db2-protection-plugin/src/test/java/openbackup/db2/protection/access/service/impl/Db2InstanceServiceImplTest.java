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
package openbackup.db2.protection.access.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.enums.Db2HadrRoleEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.RequestUriUtil;

import com.google.common.collect.ImmutableList;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link Db2InstanceServiceImpl} 测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class})
public class Db2InstanceServiceImplTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);


    private final AgentUnifiedRestApi agentUnifiedRestApi = Mockito.mock(AgentUnifiedRestApi.class);

    private Db2InstanceServiceImpl db2InstanceService = new Db2InstanceServiceImpl(resourceService, environmentService,
        agentUnifiedService);

    /**
     * 用例场景：校验db2单实例是否被注册
     * 前置条件：单实例已经被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_registered_when_check_single_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkSingleInstanceIsRegistered(mockResource()));
        Assert.assertEquals("This db2 single instance is registered.", legoCheckedException.getMessage());
        Assert.assertEquals(DatabaseErrorCode.INSTANCE_HAS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：校验db2单实例是否被注册
     * 前置条件：单实例未被注册
     * 检查点：无异常抛出
     */
    @Test
    public void check_single_instance_is_registered_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        db2InstanceService.checkSingleInstanceIsRegistered(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查db2单实例名称是否被修改
     * 前置条件：名称被修改
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_port_changed_when_check_single_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkSingleInstanceNameIsChanged(mockResource()));
        Assert.assertEquals("This db2 single instance name is changed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查db2单实例名称是否被修改
     * 前置条件：名称未被修改
     * 检查点：无异常抛出
     */
    @Test
    public void check_single_instance_port_is_changed_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        db2InstanceService.checkSingleInstanceNameIsChanged(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查集群实例
     * 前置条件：不是一个集群实例
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_is_not_cluster_instance_when_check_cluster_instance() {
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "{\"bodyErr\": 1677929488, 	\"code\": 200, 	\"message\": \"Login denied\" }"));
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockAgent());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkIsClusterInstance(mockResource()));
        Assert.assertEquals("Db2 cluster instance check fail.", legoCheckedException.getMessage());
        Assert.assertEquals(1677929488L, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群实例
     * 前置条件：属于一个集群实例
     * 检查点：无异常抛出
     */
    @Test
    public void check_is_cluster_instance_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockAgent());
        db2InstanceService.checkIsClusterInstance(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：校验集群实例是否被注册
     * 前置条件：集群实例已经被注册
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_registered_when_check_cluster_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(1));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkClusterInstanceIsRegistered(mockResource()));
        Assert.assertEquals("This cluster instance is registered.", legoCheckedException.getMessage());
        Assert.assertEquals(DatabaseErrorCode.INSTANCE_HAS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查集群实例名称是否被修改
     * 前置条件：名称被修改
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_name_changed_when_check_cluster_instance() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockInstanceNum(0));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkClusterInstanceNameIsChanged(mockResource()));
        Assert.assertEquals("This db2 cluster instance name is changed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：过滤集群实例
     * 前置条件：是dpf集群实例
     * 检查点：不过滤实例信息
     */
    @Test
    public void execute_filter_cluster_instance_when_cluster_type_is_not_power_ha() {
        ProtectedResource clusterInstance = mockClusterInstance(Db2ClusterTypeEnum.DPF.getType());
        db2InstanceService.filterClusterInstance(clusterInstance);
        Assert.assertEquals(IsmNumberConstant.TWO,
            clusterInstance.getDependencies().get(DatabaseConstants.CHILDREN).size());
    }

    /**
     * 用例场景：过滤集群实例
     * 前置条件：是powerHA集群实例
     * 检查点：过滤备节点实例信息
     */
    @Test
    public void execute_filter_cluster_instance_when_cluster_type_is_power_ha() {
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(mockAppEnv());
        ProtectedResource clusterInstance = mockClusterInstance(Db2ClusterTypeEnum.POWER_HA.getType());
        db2InstanceService.filterClusterInstance(clusterInstance);
        Assert.assertEquals(IsmNumberConstant.ONE,
            clusterInstance.getDependencies().get(DatabaseConstants.CHILDREN).size());
    }

    /**
     * 用例场景：检查hadr数据库版本
     * 前置条件：版本不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_version_inconsistent_when_checkHadrClusterInstance() {
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkHadrClusterInstance(mockClusterInstance(Db2ClusterTypeEnum.HADR.getType())));
        Assert.assertEquals(DatabaseErrorCode.DATABASE_VERSION_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查hadr数据库部署操作系统
     * 前置条件：系统不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_deployOs_inconsistent_when_checkHadrClusterInstance() {
        ProtectedResource clusterInstance = mockClusterInstance(Db2ClusterTypeEnum.HADR.getType());
        clusterInstance.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .get(0)
            .setVersion("11.1");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkHadrClusterInstance(clusterInstance));
        Assert.assertEquals(DatabaseErrorCode.DATABASE_OS_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：检查hadr数据库位数
     * 前置条件：数据库位数不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_databaseBits_inconsistent_when_checkHadrClusterInstance() {
        ProtectedResource clusterInstance = mockClusterInstance(Db2ClusterTypeEnum.HADR.getType());
        ProtectedResource instanceOne = clusterInstance.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .get(0);
        instanceOne.setVersion("11.1");
        instanceOne.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, "redhat");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> db2InstanceService.checkHadrClusterInstance(clusterInstance));
        Assert.assertEquals(DatabaseErrorCode.DATABASE_BITS_INCONSISTENT, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：扫描hadr数据库
     * 前置条件：返回的数据库不是HADR
     * 检查点：扫描数据库为空
     */
    @Test
    public void should_return_empty_database_if_database_not_hadr_when_scan_database() {
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(mockAgentBaseDto("0"));
        PowerMockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(mockDatabases());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockDatabaseList());
        List<ProtectedResource> resultDatabase = db2InstanceService.scanDatabase(
            mockClusterInstance(Db2ClusterTypeEnum.HADR.getType()), mockEnv());
        Assert.assertEquals(IsmNumberConstant.ZERO, resultDatabase.size());
    }

    /**
     * 用例场景：扫描dpf数据库
     * 前置条件：数据库正常
     * 检查点：扫描成功
     */
    @Test
    public void execute_scan_dpf_database_success() {
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(mockAgentBaseDto("0"));
        PowerMockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(mockDatabases());
        List<ProtectedResource> resultDatabase = db2InstanceService.scanDatabase(
            mockClusterInstance(Db2ClusterTypeEnum.DPF.getType()), mockEnv());
        Assert.assertEquals(IsmNumberConstant.ONE, resultDatabase.size());
    }

    /**
     * 用例场景：扫描hadr数据库
     * 前置条件：返回的数据库是hadr
     * 检查点：正常返回hadr数据库个数
     */
    @Test
    public void execute_scan_hadr_database_success() {
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(mockAgentBaseDto("0"));
        URI uriOne = RequestUriUtil.getRequestUri("127.0.0.1", 50000);
        AgentDetailDto databaseOne = mockDatabases();
        databaseOne.getResourceList()
            .get(0)
            .getExtendInfo()
            .put(Db2Constants.HADR_ROLE_KEY, Db2HadrRoleEnum.PRIMARY.getRole());
        PowerMockito.when(agentUnifiedRestApi.getDetail(ArgumentMatchers.eq(uriOne), any(), any()))
            .thenReturn(databaseOne);
        URI uriTwo = RequestUriUtil.getRequestUri("127.0.0.2", 50000);
        AgentDetailDto databaseTwo = mockDatabases();
        databaseTwo.getResourceList()
            .get(0)
            .getExtendInfo()
            .put(Db2Constants.HADR_ROLE_KEY, Db2HadrRoleEnum.STANDBY.getRole());
        PowerMockito.when(agentUnifiedRestApi.getDetail(ArgumentMatchers.eq(uriTwo), any(), any()))
            .thenReturn(databaseTwo);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockDatabaseList());
        List<ProtectedResource> resultDatabase = db2InstanceService.scanDatabase(
            mockClusterInstance(Db2ClusterTypeEnum.HADR.getType()), mockEnv());
        Assert.assertEquals(IsmNumberConstant.ONE, resultDatabase.size());
    }

    private PageListResponse<ProtectedResource> mockDatabaseList() {
        PageListResponse<ProtectedResource> databases = new PageListResponse<>();
        databases.setRecords(Collections.emptyList());
        return databases;
    }

    private PageListResponse<ProtectedResource> mockInstanceNum(int instanceNum) {
        PageListResponse<ProtectedResource> instances = new PageListResponse<>();
        instances.setTotalCount(instanceNum);
        return instances;
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setName("db2inst1");
        resource.setSubType(ResourceSubTypeEnum.DB2_INSTANCE.getType());
        resource.setExtendInfoByKey(DatabaseConstants.HOST_ID, UUID.randomUUID().toString());
        ProtectedResource subInstance = new ProtectedResource();
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("127.0.0.1");
        agent.setPort(50000);
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
        ProtectedEnvironment agentEnvOne = new ProtectedEnvironment();
        agentEnvOne.setUuid(UUID.randomUUID().toString());
        agentEnvOne.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.1");
        ProtectedEnvironment agentEnvTwo = new ProtectedEnvironment();
        agentEnvTwo.setUuid(UUID.randomUUID().toString());
        agentEnvTwo.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.2");
        agents.add(agentEnvOne);
        agents.add(agentEnvTwo);
        Map<String, List<ProtectedResource>> envDependencies = new HashMap<>();
        envDependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(envDependencies);
        return environment;
    }

    private ProtectedEnvironment mockAgent() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(UUID.randomUUID().toString());
        environment.setParentUuid(UUID.randomUUID().toString());
        environment.setEndpoint("127.0.0.1");
        environment.setPort(50000);
        return environment;
    }

    private ProtectedResource mockClusterInstance(String clusterType) {
        ProtectedResource instanceOne = mockInstanceOne();
        ProtectedResource instanceTwo = mockInstanceTwo();
        List<ProtectedResource> children = new ArrayList<>();
        children.add(instanceOne);
        children.add(instanceTwo);
        Map<String, List<ProtectedResource>> childrenMap = new HashMap<>();
        childrenMap.put(DatabaseConstants.CHILDREN, children);
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE, clusterType);
        clusterInstance.setDependencies(childrenMap);
        ProtectedEnvironment agentOne = new ProtectedEnvironment();
        agentOne.setUuid(UUID.randomUUID().toString());
        ProtectedEnvironment agentTwo = new ProtectedEnvironment();
        agentTwo.setUuid(UUID.randomUUID().toString());
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agentOne);
        agents.add(agentTwo);
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, agents);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(agentMap);
        clusterInstance.setEnvironment(environment);
        return clusterInstance;
    }

    private ProtectedResource mockInstanceOne() {
        ProtectedResource instanceOne = new ProtectedResource();
        instanceOne.setExtendInfoByKey(DatabaseConstants.ROLE, NodeType.MASTER.getNodeType());
        instanceOne.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, "suse");
        instanceOne.setExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY, "32");
        instanceOne.setExtendInfoByKey(DatabaseConstants.HOST_ID, UUID.randomUUID().toString());
        instanceOne.setVersion("10.5");
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("127.0.0.1");
        agent.setPort(50000);
        agent.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.1");
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(agent);
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, agents);
        instanceOne.setDependencies(agentMap);
        return instanceOne;
    }

    private ProtectedResource mockInstanceTwo() {
        ProtectedResource instanceTwo = new ProtectedResource();
        instanceTwo.setExtendInfoByKey(DatabaseConstants.ROLE, NodeType.SLAVE.getNodeType());
        instanceTwo.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, "redhat");
        instanceTwo.setExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY, "64");
        instanceTwo.setExtendInfoByKey(DatabaseConstants.HOST_ID, UUID.randomUUID().toString());
        instanceTwo.setVersion("11.1");
        ProtectedEnvironment agentTwo = new ProtectedEnvironment();
        agentTwo.setEndpoint("127.0.0.2");
        agentTwo.setPort(50000);
        agentTwo.setExtendInfoByKey(ResourceConstants.AGENT_IP_LIST, "127.0.0.2");
        List<ProtectedResource> agentsTwo = new ArrayList<>();
        agentsTwo.add(agentTwo);
        Map<String, List<ProtectedResource>> agentMapTwo = new HashMap<>();
        agentMapTwo.put(DatabaseConstants.AGENTS, agentsTwo);
        instanceTwo.setDependencies(agentMapTwo);
        return instanceTwo;
    }

    private AgentBaseDto mockAgentBaseDto(String errorCode) {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode(errorCode);
        return agentBaseDto;
    }

    private AgentDetailDto mockDatabases() {
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.VERSION, "V10.5.5.5");
        extendInfo.put(DatabaseConstants.DEPLOY_OS_KEY, "redhat");
        AppResource databaseOne = new AppResource();
        databaseOne.setExtendInfo(extendInfo);
        databaseOne.setName("testdb");
        databaseOne.setUuid(UUID.randomUUID().toString());
        AgentDetailDto result = new AgentDetailDto();
        result.setResourceList(Collections.singletonList(databaseOne));
        return result;
    }

    private AppEnvResponse mockAppEnv() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.ROLE, "1");
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(extendInfo);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setNodes(ImmutableList.of(nodeInfo));
        return appEnvResponse;
    }
}
