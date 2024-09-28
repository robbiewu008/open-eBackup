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
package openbackup.clickhouse.plugin.provider;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;

import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseAgentProviderTest {
    private ClickHouseAgentProvider clickHouseAgentProvider;

    private ResourceService resourceService;

    ;

    @Before
    public void init() {
        resourceService = PowerMockito.mock(ResourceService.class);
        clickHouseAgentProvider = new ClickHouseAgentProvider(resourceService);
    }

    /**
     * 用例场景：备份场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void backup_select_success() {
        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setUuid("agentUuid1");
        agentEnv1.setEndpoint("192.168.0.1");
        agentEnv1.setPort(22);
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv1)));

        ProtectedEnvironment agentEnv2 = new ProtectedEnvironment();
        agentEnv2.setUuid("agentUuid2");
        agentEnv2.setEndpoint("192.168.0.2");
        agentEnv2.setPort(22);
        ProtectedResource nodeResource2 = new ProtectedResource();
        nodeResource2.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv2)));

        ProtectedEnvironment clusterEnv = new ProtectedEnvironment();
        clusterEnv.setDependencies(
            ImmutableMap.of(ResourceConstants.CHILDREN, Lists.newArrayList(nodeResource1, nodeResource2)));

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(clusterEnv));

        ProtectedEnvironment databaseEnv = new ProtectedEnvironment();
        databaseEnv.setRootUuid("1111");

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(databaseEnv)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        List<Endpoint> endpointList = clickHouseAgentProvider.getSelectedAgents(agentSelectParam);

        List<Endpoint> list = new ArrayList<>();
        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId("agentUuid2");
        endpoint2.setIp("192.168.0.2");
        endpoint2.setPort(22);
        list.add(endpoint2);
        Endpoint endpoint = new Endpoint();
        endpoint.setId("agentUuid1");
        endpoint.setIp("192.168.0.1");
        endpoint.setPort(22);
        list.add(endpoint);

        assertThat(endpointList).usingRecursiveComparison().isEqualTo(list);
    }

    /**
     * 用例场景：扫描场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void scan_select_success() {
        ProtectedEnvironment agentEnv1 = new ProtectedEnvironment();
        agentEnv1.setUuid("agentUuid1");
        agentEnv1.setEndpoint("192.168.0.1");
        agentEnv1.setPort(22);

        ProtectedEnvironment agentEnv2 = new ProtectedEnvironment();
        agentEnv2.setUuid("agentUuid2");
        agentEnv2.setEndpoint("192.168.0.2");
        agentEnv2.setPort(22);
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setDependencies(
            ImmutableMap.of(DatabaseConstants.AGENTS, Lists.newArrayList(agentEnv1, agentEnv2)));

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(nodeResource1)
            .jobType(JobTypeEnum.RESOURCE_SCAN.getValue())
            .build();

        List<Endpoint> endpointList = clickHouseAgentProvider.getSelectedAgents(agentSelectParam);

        List<Endpoint> list = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("agentUuid1");
        endpoint.setIp("192.168.0.1");
        endpoint.setPort(22);
        list.add(endpoint);
        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId("agentUuid2");
        endpoint2.setIp("192.168.0.2");
        endpoint2.setPort(22);
        list.add(endpoint2);

        assertThat(endpointList).usingRecursiveComparison().isEqualTo(list);
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(nodeResource1).build();
        boolean applicable = clickHouseAgentProvider.applicable(agentSelectParam);
        Assert.assertTrue(applicable);
    }
}
