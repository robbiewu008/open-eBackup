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
package openbackup.oracle.provider;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class OracleClusterProviderTest {
    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);
    private final PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final OracleClusterDatabaseProvider databaseProvider = Mockito.mock(OracleClusterDatabaseProvider.class);

    private final OracleClusterProvider oracleClusterProvider = new OracleClusterProvider(providerManager,
            pluginConfigManager, oracleBaseService, resourceService, databaseProvider);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void before() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    /**
     * 用例场景：oracle集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(oracleClusterProvider.applicable(ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.getType()));
    }

    /**
     * 用例场景：oracle集群环境检查
     * 前置条件：无
     * 检查点：检查成功
     */
    @Test
    public void check_success() {
        ProtectedEnvironment environment = mockEnvironment();
        environment.setName("RAC");
        List<ProtectedResource> resources = environment.getDependencies().get(DatabaseConstants.AGENTS);

        ProtectedEnvironment firstAgent = new ProtectedEnvironment();
        firstAgent.setUuid("1eb50f4d-975f-43be-898b-e97cf411ed14");
        firstAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        firstAgent.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        firstAgent.setOsType("linux");
        ProtectedEnvironment secondAgent = new ProtectedEnvironment();
        secondAgent.setUuid("fc2d213a-482c-4e3b-b72d-9c166bc8f952");
        secondAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        secondAgent.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        secondAgent.setOsType("linux");
        resources.add(firstAgent);
        resources.add(secondAgent);

        Mockito.when(oracleBaseService.getEnvironmentById("1eb50f4d-975f-43be-898b-e97cf411ed14")).thenReturn(firstAgent);
        Mockito.when(oracleBaseService.getEnvironmentById("fc2d213a-482c-4e3b-b72d-9c166bc8f952")).thenReturn(secondAgent);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockEmptyResponse());
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
                .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        oracleClusterProvider.register(environment);
        Assert.assertNotNull(environment.getUuid());
        Assert.assertEquals(environment.getOsType(), "linux");
    }
    /**
     * 用例场景：oracle集群注册检查
     * 前置条件：所选节点数量少于2个
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_number_less_than_two() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("cluster node number error.");
        ProtectedEnvironment environment = mockEnvironment();
        oracleClusterProvider.register(environment);
    }

    /**
     * 用例场景：oracle集群注册检查
     * 前置条件：所选节点当中某个主机离线
     * 检查点：抛出异常
     */
    @Test(expected = DataProtectionAccessException.class)
    public void should_throw_DataProtectionAccessException_if_exist_agent_offline() {
        expectedException.expect(DataProtectionAccessException.class);
        expectedException.expectMessage("Protected environment is offLine!");

        ProtectedEnvironment environment = mockEnvironment();
        List<ProtectedResource> resources = environment.getDependencies().get(DatabaseConstants.AGENTS);

        ProtectedEnvironment agent = new ProtectedEnvironment();
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
                .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        agent.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        ProtectedEnvironment environment2 = new ProtectedEnvironment();
        environment1.setUuid("1eb50f4d-975f-43be-898b-e97cf411ed14");
        environment2.setUuid("fc2d213a-482c-4e3b-b72d-9c166bc8f952");
        resources.add(environment1);
        resources.add(environment2);

        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(agent);
        oracleClusterProvider.register(environment);
    }

    /**
     * 用例场景：oracle集群健康检查
     * 前置条件：至少一个主机在线
     * 检查点：检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment environment = mockEnvironment();
        List<ProtectedResource> resources = environment.getDependencies().get(DatabaseConstants.AGENTS);

        ProtectedEnvironment online = new ProtectedEnvironment();
        online.setUuid("123");
        online.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());

        ProtectedEnvironment offline = new ProtectedEnvironment();
        offline.setUuid("321");
        offline.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        resources.add(online);
        resources.add(offline);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
                .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());

        Mockito.when(oracleBaseService.getEnvironmentById("123")).thenReturn(online);
        Mockito.when(oracleBaseService.getEnvironmentById("321")).thenReturn(offline);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockEmptyResponse());
        oracleClusterProvider.validate(environment);
        Mockito.verify(oracleBaseService, Mockito.times(1)).getEnvironmentById(any());
    }

    /**
     * 用例场景：oracle集群健康检查
     * 前置条件：没有主机在线
     * 检查点：抛出异常
     */
    @Test(expected = DataProtectionAccessException.class)
    public void should_throw_DataProtectionAccessException_if_no_host_online() {
        expectedException.expect(DataProtectionAccessException.class);
        expectedException.expectMessage("Oracle cluster is offLine!");
        ProtectedEnvironment environment = mockEnvironment();
        List<ProtectedResource> resources = environment.getDependencies().get(DatabaseConstants.AGENTS);

        ProtectedEnvironment online = new ProtectedEnvironment();
        online.setUuid("1eb50f4d-975f-43be-898b-e97cf411ed14");
        online.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());

        ProtectedEnvironment offline = new ProtectedEnvironment();
        offline.setUuid("fc2d213a-482c-4e3b-b72d-9c166bc8f952");
        offline.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        resources.add(online);
        resources.add(offline);

        Mockito.when(oracleBaseService.getEnvironmentById("1eb50f4d-975f-43be-898b-e97cf411ed14")).thenReturn(online);
        Mockito.when(oracleBaseService.getEnvironmentById("fc2d213a-482c-4e3b-b72d-9c166bc8f952")).thenReturn(offline);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(mockEmptyResponse());
        oracleClusterProvider.validate(environment);
    }

    /**
     * 用例场景：oracle集群删除检查
     * 前置条件：无
     * 检查点：有集群数据库的场景下删除失败
     */
    @Test
    public void should_throw_LegoCheckedException_if_exist_database() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("oracle cluster delete failed");
        ProtectedEnvironment environment = mockEnvironment();
        PageListResponse<ProtectedResource> response = mockEmptyResponse();
        response.getRecords().add(new ProtectedResource());
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        oracleClusterProvider.remove(environment);
    }

    private ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(new HashMap<>());
        environment.getDependencies().put(DatabaseConstants.AGENTS, new ArrayList<>());
        return environment;
    }

    private PageListResponse<ProtectedResource> mockEmptyResponse() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        response.setRecords(new ArrayList<>());
        return response;
    }
}