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
package openbackup.dameng.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * {@link DamengAgentProvider 测试类}
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {DamengAgentProvider.class, DataBaseAgentSelector.class})
public class DamengAgentProviderTest {
    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @MockBean
    private DamengService damengService;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private ResourceService resourceService;

    @Autowired
    private DamengAgentProvider agentProvider;

    /**
     * 用例场景：当为集群的时候，填充Agent成功
     * 前置条件：资源类型为达梦单节点（Dameng-cluster）
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success_when_subType_is_cluster() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(damengService.getEndpointList(anyString())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：当为单节点的时候，填充Agent成功
     * 前置条件：资源类型为达梦单节点（Dameng-singleNode）
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success_when_subType_is_single() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        ProtectedResource resource = agentSelectParam.getResource();
        resource.setSubType(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        agentSelectParam.setResource(resource);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        ProtectedResourceChecker checker = PowerMockito.mock(ProtectedResourceChecker.class);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(checker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setPort(22);
        environment.setUuid("uuid");
        protectedResourceMap.put(resource, Collections.singletonList(environment));
        PowerMockito.when(checker.collectConnectableResources(any())).thenReturn(protectedResourceMap);
        List<Endpoint> endpoints = agentProvider.getSelectedAgents(agentSelectParam);
        Assert.assertEquals("127.0.0.1", endpoints.get(0).getIp());
    }

    /**
     * 用例场景：Dameng资源类型匹配
     * 前置条件：资源类型为达梦集群（Dameng-cluster）
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setSubType(ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}
