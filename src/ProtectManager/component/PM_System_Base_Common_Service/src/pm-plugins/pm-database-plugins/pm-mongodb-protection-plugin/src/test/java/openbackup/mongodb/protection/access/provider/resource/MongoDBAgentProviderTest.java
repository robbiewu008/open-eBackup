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
package openbackup.mongodb.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

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

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * {@link MongoDBAgentProvider 测试类}
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {MongoDBAgentProvider.class, DataBaseAgentSelector.class})
public class MongoDBAgentProviderTest {
    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private ResourceService resourceService;

    @Autowired
    private MongoDBAgentProvider agentProvider;

    /**
     * 用例场景：MongoDB填充Agent成功
     * 前置条件：资源类型为MongoDB-cluster
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        ProtectedResource resource = agentSelectParam.getResource();
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
     * 用例场景：MongoDB资源过滤
     * 前置条件：资源类型为MongoDB-cluster
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        resource.setUuid("uuid");
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}
