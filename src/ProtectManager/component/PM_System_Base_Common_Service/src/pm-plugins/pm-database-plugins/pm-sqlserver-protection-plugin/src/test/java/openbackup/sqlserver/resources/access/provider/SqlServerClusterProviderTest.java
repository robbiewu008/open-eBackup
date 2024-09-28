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
package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * SQL Server集群扫描测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {EnvironmentLinkStatusHelper.class})
public class SqlServerClusterProviderTest {
    private SqlServerClusterProvider sqlServerClusterProvider;
    private ResourceService resourceService;
    private ProtectedEnvironmentService protectedEnvironmentService;
    private SqlServerBaseService sqlServerBaseService;
    private ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);
        this.sqlServerBaseService = Mockito.mock(SqlServerBaseService.class);
        this.sqlServerClusterProvider = new SqlServerClusterProvider(Mockito.mock(ProviderManager.class),
            Mockito.mock(PluginConfigManager.class), resourceService, protectedEnvironmentService,
            sqlServerBaseService);
        protectedEnvironment.setName("cluster");
        protectedEnvironment.setUuid("1");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(1433);
        protectedEnvironment.setDependencies(getDependencyByAgentNum(1));
    }

    /**
     * 用例场景：SQL Server集群环境扫描类provider过滤
     * 前置条件：资源类型为SQL Server单实例
     * 检查点：类过滤检查针对正确和错误的类型，分别返回成功和失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(sqlServerClusterProvider.applicable(ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType()));
        Assert.assertFalse(sqlServerClusterProvider.applicable(ResourceSubTypeEnum.SQL_SERVER.getType()));
    }

    /**
     * 用例场景：SQL Server集群资源扫描
     * 前置条件：集群下的的agents主机仅一个
     * 检查点：抛出主机数量错误异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_scan_cluster_with_only_one_agent() {
        protectedEnvironment.setDependencies(null);
        Assert.assertThrows("[SQL Server] cluster node number error.", LegoCheckedException.class,
            () -> sqlServerClusterProvider.scan(protectedEnvironment));
    }

    /**
     * 用例场景：SQL Server集群资源扫描
     * 前置条件：集群下无dependencies数据
     * 检查点：抛出主机数量错误异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_scan_cluster_with_no_dependencies() {
        protectedEnvironment.setDependencies(getDependencyByAgentNum(1));
        Assert.assertThrows("[SQL Server] cluster node number error.", LegoCheckedException.class,
            () -> sqlServerClusterProvider.scan(protectedEnvironment));
    }

    /**
     * 用例场景：SQL Server集群健康检查
     * 前置条件：集群下无dependencies数据
     * 检查点：抛出主机数量错误异常
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment node1 = BeanTools.clone(protectedEnvironment);
        node1.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        ProtectedEnvironment node2 = BeanTools.clone(protectedEnvironment);
        node2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        Mockito.when(sqlServerBaseService.getProtectedEnvironmentByResourceList(any())).thenReturn(
            Arrays.asList(node1, node2));
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setDependencies(null);
        Assert.assertThrows(LegoCheckedException.class,
            () -> sqlServerClusterProvider.validate(protectedEnvironment));
    }

    /**
     * 用例场景：SQL Server集群资源扫描
     * 前置条件：传入的agents主机存在，且扫描出两个资源
     * 检查点：集群资源、集群实例加扫描资源共四个
     */
    @Test
    public void scan_database_success_with_no_instance_resource() {
        int scanSize = 2;
        protectedEnvironment.setDependencies(getDependencyByAgentNum(scanSize));
        Mockito.when(protectedEnvironmentService.getEnvironmentById(any())).thenReturn(protectedEnvironment);
        ProtectedEnvironment node = BeanTools.copy(getProtectedResource(ResourceSubTypeEnum.HOST_SYSTEM.getType()),
            ProtectedEnvironment::new);
        node.setEndpoint("127.0.0.7");
        node.setPort(1433);
        Mockito.when(sqlServerBaseService.getProtectedEnvironmentByResourceList(any())).thenReturn(
            Arrays.asList(protectedEnvironment, protectedEnvironment)).thenReturn(Collections.singletonList(node));
        PageListResponse<ProtectedResource> resourcePageListResponse = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType())));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(resourcePageListResponse);
        Mockito.when(sqlServerBaseService.getResourceOfClusterByType(any(), any(), anyBoolean()))
            .thenReturn(Collections.singletonList(
                getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType())));
        Mockito.when(sqlServerBaseService.getDatabaseInfoByAgent(any(), any(), any(), any(), anyString())).thenReturn(
            Arrays.asList(getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType()),
                getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType())));

        List<ProtectedResource> resources = sqlServerClusterProvider.scan(protectedEnvironment);
        Assert.assertEquals(resources.size(), 4);
    }

    /**
     * 用例场景：SQL Server集群资源状态检查
     * 前置条件：传入的agents主机存在，且扫描出一个资源
     * 检查点：集群资源加扫描无异常抛出
     */
    @Test
    public void cluster_check_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(getDependencyByAgentNum(0));
        Mockito.when(protectedEnvironmentService.getEnvironmentById(any())).thenReturn(environment);
        this.protectedEnvironment.setDependencies(getDependencyByAgentNum(2));
        PageListResponse<ProtectedResource> resourcePageListResponse = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType())));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resourcePageListResponse);
        sqlServerClusterProvider.register(this.protectedEnvironment);
        Assert.assertTrue(true);
    }

    private Map<String, List<ProtectedResource>> getDependencyByAgentNum(int num) {
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        List<ProtectedResource> agents = new ArrayList<>(num);
        for (int i = 0; i < num; i++) {
            agents.add(getProtectedResource(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType()));
        }
        dependency.put(DatabaseConstants.AGENTS, agents);
        return dependency;
    }

    private ProtectedResource getProtectedResource(String subType) {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(subType);
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "1433");
        extendInfo.put(DatabaseConstants.DATABASE_ID, "2");
        extendInfo.put(DatabaseConstants.END_POINT, "windows-1");
        extendInfo.put(SqlServerConstants.AG_ID, "654321");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123456");
        protectedResource.setName("windows-1");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedResource));
        List<ProtectedResource> protectedResources = new ArrayList<>();
        protectedResources.add(protectedResource);
        dependencies.put(SqlServerConstants.INSTANCE, protectedResources);
        resource.setDependencies(dependencies);
        resource.setExtendInfo(extendInfo);
        resource.setUuid("654321");
        resource.setName("windows-1");
        return resource;
    }
}
