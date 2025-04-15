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
package openbackup.sqlserver.protection.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * sqlserver公共基础服务测试类
 *
 */
public class SqlServerBaseServiceTest {
    private SqlServerBaseService sqlServerBaseService;

    private ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;


    private final RepositoryStrategyManager repositoryStrategyManager = Mockito.mock(RepositoryStrategyManager.class);

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        this.sqlServerBaseService = new SqlServerBaseService(resourceService, agentUnifiedService,
            repositoryStrategyManager);
    }

    @Test
    public void test_check_instance_exist_success() {
        ProtectedResource protectedResource = mockProtectResource();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        response.setRecords(resources);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(response);
        sqlServerBaseService.checkInstanceExist(protectedResource);
    }

    @Test
    public void test_check_instance_exist_error() {
        ProtectedResource protectedResource = mockProtectResource();
        protectedResource.setName("TEST");
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setName("TEST");
        resources.add(resource);
        response.setRecords(resources);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(response);
        Assert.assertThrows(DataProtectionAccessException.class, () -> {
            sqlServerBaseService.checkInstanceExist(protectedResource);
        });
    }

    private ProtectedResource mockProtectResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        ProtectedResource agentResource = new ProtectedResource();
        agentResource.setUuid("testUUID");
        Map<String, List<ProtectedResource>> agentMap = new HashMap<>();
        agentMap.put(DatabaseConstants.AGENTS, Lists.newArrayList(agentResource));
        protectedResource.setDependencies(agentMap);
        return protectedResource;
    }

    @Test
    public void test_find_associated_resources_to_set_next_full_success() {
        ProtectedResource protectedResource = mockProtectResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource));
        RestoreTask restoreTask = mock_restore_task();
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        sqlServerBaseService.findAssociatedResourcesToSetNextFull(restoreTask);
        restoreTask.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        sqlServerBaseService.findAssociatedResourcesToSetNextFull(restoreTask);
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        sqlServerBaseService.findAssociatedResourcesToSetNextFull(restoreTask);
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        response.setRecords(resources);
        PowerMockito.when(resourceService.query(any(ResourceQueryParams.class))).thenReturn(response);
        sqlServerBaseService.findAssociatedResourcesToSetNextFull(restoreTask);
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
        sqlServerBaseService.findAssociatedResourcesToSetNextFull(restoreTask);

    }

    private RestoreTask mock_restore_task() {
        RestoreTask task = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("UUID");
        task.setTargetObject(taskResource);
        return task;
    }

    /**
     * 用例场景：校验恢复任务相关参数
     * 前置条件：1、新位置恢复；2、目标路径符合要求
     * 检查点：校验成功
     */
    @Test
    public void check_restore_task_param_success() {
        RestoreTask restoreTask = new RestoreTask();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH, "c:\\administrator");
        restoreTask.setAdvanceParams(extendInfo);
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        System.out.println(extendInfo.get(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH));
        sqlServerBaseService.checkRestoreTaskParam(restoreTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：校验恢复任务相关参数
     * 前置条件：1、新位置恢复；2、目标路径不符合要求
     * 检查点：校验为参数异常，抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_restore_task_param_with_wrong_path() {
        RestoreTask restoreTask = new RestoreTask();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SqlServerConstants.KEY_RESTORE_NEW_LOCATION_PATH, "/opt/OceanProtect/");
        restoreTask.setAdvanceParams(extendInfo);
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        Assert.assertThrows(LegoCheckedException.class, () -> sqlServerBaseService.checkRestoreTaskParam(restoreTask));
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 存在一个Agent主机信息
     * 检 查 点：1. 获取到Agent主机信息和预期一样
     */
    @Test
    public void get_agent_by_single_instance_uuid_success() {
        String singleInstanceUuid = "123";
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agentResources = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        agentResources.add(agentEnv);
        dependencies.put("agents", agentResources);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setDependencies(dependencies);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
            .thenReturn(Optional.of(protectedResource));
        ProtectedEnvironment agentReturnEnv = sqlServerBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid);
        Assert.assertEquals(agentReturnEnv, agentEnv);
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 没有Agent主机信息
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_null() {
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        dependencies.put("agents", null);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
            .thenReturn(Optional.of(protectedResource));
        Assert.assertThrows("single instance dependency agent is not one.", LegoCheckedException.class,
            () -> sqlServerBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid));
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. 存在多个Agent主机信息
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_not_one() {
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        List<ProtectedResource> agentResources = new ArrayList<>();
        agentResources.add(agentEnv);
        agentResources.add(agentEnv);
        dependencies.put("agents", agentResources);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
            .thenReturn(Optional.of(protectedResource));
        Assert.assertThrows("single instance dependency agent is not one.", LegoCheckedException.class,
            () -> sqlServerBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid));
    }

    /**
     * 用例场景：针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     * 前置条件：1. Agent主机信息不是环境
     * 检 查 点：1. 报错，且错误信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_if_get_agent_when_agent_is_not_env() {
        String singleInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        List<ProtectedResource> agentResources = new ArrayList<>();
        ProtectedResource agentRes = new ProtectedResource();
        agentRes.setUuid(UUID.randomUUID().toString());
        agentResources.add(agentRes);
        dependencies.put("agents", agentResources);
        PowerMockito.when(resourceService.getResourceById(singleInstanceUuid))
            .thenReturn(Optional.of(protectedResource));
        Assert.assertThrows("sqlserver agent resource is not env.", LegoCheckedException.class,
            () -> sqlServerBaseService.getAgentBySingleInstanceUuid(singleInstanceUuid));
    }

    /**
     * 用例场景：针对集群实例uuid，从dependency里，获取集群实例下面的所有子实例
     * 前置条件：1. 存在子实例信息
     * 检 查 点：1. 不报错且获取到子实例信息和预期一样
     */
    @Test
    public void get_single_instance_by_cluster_instance_success() {
        String clusterInstanceUuid = "123";
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        List<ProtectedResource> singleInstances = new ArrayList<>();
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setUuid(UUID.randomUUID().toString());
        singleInstances.add(agentEnv);
        dependencies.put("children", singleInstances);
        PowerMockito.when(resourceService.getResourceById(clusterInstanceUuid))
            .thenReturn(Optional.of(protectedResource));
        List<ProtectedResource> returnSingleInstances = sqlServerBaseService.getSingleInstanceByClusterInstance(
            clusterInstanceUuid);
        Assert.assertEquals(singleInstances.size(), returnSingleInstances.size());
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：1. ip和port不为空
     * 检 查 点：1. 不报错且获取到endpoint信息和预期一样
     */
    @Test
    public void get_agent_endpoint_success() {
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setEndpoint("8.40.99.101");
        env.setPort(2181);
        env.setUuid("1111");
        Endpoint agentEndpoint = sqlServerBaseService.getAgentEndpoint(env);
        Assert.assertEquals(agentEndpoint.getId(), env.getUuid());
        Assert.assertEquals(agentEndpoint.getIp(), env.getEndpoint());
        Assert.assertEquals(agentEndpoint.getPort(), (int) env.getPort());
    }

    /**
     * 用例场景：根据Agent主机信息，获取Agent主机的Endpoint
     * 前置条件：1. ip和port不为空
     * 检 查 点：1. 不报错且获取到endpoint信息和预期一样
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_agent_endpoint() {
        ProtectedEnvironment env = new ProtectedEnvironment();
        Assert.assertThrows("sqlserver agent env lack require msg.", LegoCheckedException.class,
            () -> sqlServerBaseService.getAgentEndpoint(env));
    }

    /**
     * 用例场景：获取资源失败
     * 前置条件：资源不存在
     * 检 查 点：获取资源失败抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_getResource() {
        PowerMockito.when(resourceService.getResourceById("111111")).thenReturn(Optional.empty());
        Assert.assertThrows(LegoCheckedException.class, () -> sqlServerBaseService.getResourceByUuid("111111"));
    }

    /**
     * 用例场景：填充备份任务nodes信息
     * 前置条件：实例注册正常，备份任务可以正常下发到PM插件
     * 检 查 点：成功填充nodes信息
     */
    @Test
    public void supplyNodes_success() {
        Endpoint end = new Endpoint();
        end.setIp("127.0.0.1");
        end.setPort(1);
        RestoreTask task = new RestoreTask();
        task.setAgents(Collections.singletonList(end));
        HostDto hostdto = new HostDto();
        hostdto.setEndpoint("127.0.0.1");
        hostdto.setPort(1);
        PowerMockito.when(agentUnifiedService.getHost(any(), any())).thenReturn(hostdto);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        task.setTargetEnv(taskEnvironment);
        sqlServerBaseService.supplyNodes(task);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：下发agent(数据库类型）
     * 前置条件：能够正常下发任务，集群状态正常
     * 检查点：下发agent成功
     */
    @Test
    public void convert_node_list_to_agents_success_when_resource_type_is_database() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        protectedResource.setUuid("this_resource_id");
        protectedResource.setParentUuid("parent_uuid");
        ProtectedResource parentResource = BeanTools.copy(protectedResource, ProtectedResource::new);
        ProtectedEnvironment environment = generateProtectedEnvironment("parent_uuid");
        Map<String, List<ProtectedResource>> map = new HashMap<>();
        map.put(DatabaseConstants.AGENTS, Collections.singletonList(environment));
        parentResource.setDependencies(map);
        PowerMockito.when(resourceService.getResourceById("this_resource_id"))
            .thenReturn(Optional.of(protectedResource));
        PowerMockito.when(resourceService.getResourceById("parent_uuid")).thenReturn(Optional.of(parentResource));
        assetQueryNodeListSuccess();
    }

    private void assetQueryNodeListSuccess() {
        List<Endpoint> agents = sqlServerBaseService.convertNodeListToAgents("this_resource_id");
        Assert.assertEquals(1, agents.size());
        Assert.assertEquals("127.0.0.1", agents.get(0).getIp());
    }

    /**
     * 用例场景：下发agent(实例类型）
     * 前置条件：能够正常下发任务，集群状态正常
     * 检查点：下发agent成功
     */
    @Test
    public void convert_node_list_to_agents_success_when_resource_type_is_cluster_instance() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        protectedResource.setUuid("this_resource_id");
        ProtectedEnvironment environment = generateProtectedEnvironment("this_resource_id");
        Map<String, List<ProtectedResource>> map = new HashMap<>();
        map.put(DatabaseConstants.AGENTS, Collections.singletonList(environment));
        protectedResource.setDependencies(map);
        PowerMockito.when(resourceService.getResourceById("this_resource_id"))
            .thenReturn(Optional.of(protectedResource));
        assetQueryNodeListSuccess();
    }

    /**
     * 用例场景：下发agent(可用性组类型）
     * 前置条件：能够正常下发任务，集群状态正常
     * 检查点：下发agent成功
     */
    @Test
    public void convert_node_list_to_agents_success_when_resource_type_is_always_on() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
        protectedResource.setUuid("this_resource_id");
        ProtectedEnvironment environment = generateProtectedEnvironment("this_resource_id");
        Map<String, List<ProtectedResource>> map = new HashMap<>();
        map.put(DatabaseConstants.INSTANCE, Collections.singletonList(environment));
        map.put(DatabaseConstants.AGENTS, Collections.singletonList(environment));
        protectedResource.setDependencies(map);
        PowerMockito.when(resourceService.getResourceById("this_resource_id"))
            .thenReturn(Optional.of(protectedResource));
        assetQueryNodeListSuccess();
    }

    /**
     * 用例场景：查询可用性组下面数据库成功
     * 前置条件：可用性组注册成功
     * 检 查 点：成功获取可用性组下面的数据库
     */
    @Test
    public void queryDatabasesInAlwaysOn_success() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(0);
        pageListResponse.setRecords(new ArrayList<>());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        PageListResponse<ProtectedResource> resourcePageListResponse
            = sqlServerBaseService.queryDatabasesInAlwaysOnOrInstance("121212");
        Assert.assertEquals(0, resourcePageListResponse.getTotalCount());
    }

    private ProtectedEnvironment generateProtectedEnvironment(String uuid) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setPort(222);
        environment.setUuid(uuid);
        return environment;
    }
}