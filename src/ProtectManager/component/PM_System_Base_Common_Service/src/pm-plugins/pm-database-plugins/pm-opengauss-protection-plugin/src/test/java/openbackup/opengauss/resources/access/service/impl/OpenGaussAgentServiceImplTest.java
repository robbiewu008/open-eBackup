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
package openbackup.opengauss.resources.access.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.provider.OpenGaussMockData;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * OpenGaussAgentServiceImpl测试类
 *
 */
public class OpenGaussAgentServiceImplTest {
    /**
     * 用例场景  获取agent信息成功
     * 前置条件：查询环境信息返回正常的值
     * 检查点: 查询agent信息成功
     */
    @Test
    public void get_agent_endpoint_success() {
        AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        OpenGaussAgentService openGaussAgentService = new OpenGaussAgentServiceImpl(agentUnifiedService,
            protectedEnvironmentService);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.mockProtectedEnvironment());
        List<Endpoint> agentEndpoint = openGaussAgentService.getAgentEndpoint("env_id");
        Assert.assertEquals(1,agentEndpoint.size() );
        Assert.assertEquals(8963,agentEndpoint.get(0).getPort() );
    }

    /**
     * 用例场景  检查集群信息是否包含状态值
     * 前置条件：查询集群接口返回正常集群信息
     * 检查点: 查询集群状态是否为Normal
     */
    @Test
    public void should_return_clusterState_normal_when_check_cluster_node_status() {
        AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        OpenGaussAgentService openGaussAgentService = new OpenGaussAgentServiceImpl(agentUnifiedService,
            protectedEnvironmentService);

        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any()))
            .thenReturn(OpenGaussMockData.buildAppEnvResponse());
        AppEnvResponse appEnvResponse = openGaussAgentService.getClusterNodeStatus(getClusterEnvironment());
        Assert.assertEquals("Normal", appEnvResponse.getExtendInfo().get(OpenGaussConstants.CLUSTER_STATE));
    }

    /**
     * 用例场景  补充环境中的nodes节点信息
     * 前置条件：环境信息查询ok，环境节点信息中有值
     * 检查点: 环境中的node中uuid值正确，角色信息被放到了node节点的扩展信息中
     */
    @Test
    public void should_update_environment_nodes_when_build_environment_nodes() {
        AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        OpenGaussAgentService openGaussAgentService = new OpenGaussAgentServiceImpl(agentUnifiedService,
            protectedEnvironmentService);

        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(any()))
            .thenReturn(OpenGaussMockData.mockProtectedEnvironment());

        TaskEnvironment newEnvNodes = openGaussAgentService.buildEnvironmentNodes(OpenGaussMockData.getTaskEnvironment());
        List<TaskEnvironment> nodes = newEnvNodes.getNodes();
        Assert.assertEquals(1, nodes.size());
        Assert.assertEquals("xxxxxxxxxxxxxxxxxxxx11", nodes.get(0).getUuid());
        Assert.assertEquals("1", nodes.get(0).getExtendInfo().get(OpenGaussConstants.CLUSTER_NODE_ROLE));
    }

    private ProtectedEnvironment getClusterEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        ProtectedResource protectedResource = new ProtectedResource();
        dependency.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedResource));
        protectedEnvironment.setDependencies(dependency);
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        return protectedEnvironment;
    }
}
