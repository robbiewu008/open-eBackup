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
package openbackup.openstack.protection.access.common;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.provider.MockFactory;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: OpenstackAgentServiceTest
 *
 */
public class OpenstackAgentServiceTest {
    private static OpenstackAgentService agentService;

    private static final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private static final ProtectedEnvironmentRetrievalsService envRetrievalsService =
        Mockito.mock(ProtectedEnvironmentRetrievalsService.class);

    @BeforeClass
    public static void init() {
        agentService = new OpenstackAgentService(envRetrievalsService, agentUnifiedService);
    }

    /**
     * 用例场景：测试获取所有agent列表成功 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果与预期相同
     */
    @Test
    public void test_get_all_agents_success() {
        Map<ProtectedResource, List<ProtectedEnvironment>> agentMap = new HashMap<>();
        List<ProtectedEnvironment> agents = Collections.singletonList(MockFactory.mockAgentResource());
        agentMap.put(null, agents);
        PowerMockito.when(envRetrievalsService.collectConnectableResources(any(ProtectedResource.class)))
            .thenReturn(agentMap);
        List<ProtectedEnvironment> allAgents = agentService.getAllAgents(MockFactory.mockEnvironment());
        Assert.assertEquals(agents, allAgents);
    }

    /**
     * 用例场景：获取环境信息时失败 <br/>
     * 前置条件：调用agent接口获取环境信息抛出异常 <br/>
     * 检查点：抛出指定异常
     */
    @Test
    public void should_raise_exception_if_query_agent_cluster_info_failed_when_queryClusterInfo() {
        Map<ProtectedResource, List<ProtectedEnvironment>> agentMap = new HashMap<>();
        List<ProtectedEnvironment> agents = Collections.singletonList(MockFactory.mockAgentResource());
        agentMap.put(null, agents);
        PowerMockito.when(envRetrievalsService.collectConnectableResources(any(ProtectedResource.class)))
            .thenReturn(agentMap);
        Mockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenThrow(new LegoCheckedException("test"));
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        Assert.assertThrows(LegoCheckedException.class, () -> agentService.queryClusterInfo(environment));
        Mockito.reset(agentUnifiedService);
    }

    /**
     * 用例场景：检查连通性成功 <br/>
     * 前置条件：agent接口返回非空，错误码为成功 <br/>
     * 检查点：无异常
     */
    @Test
    public void test_check_connectivity_success() {
        Map<ProtectedResource, List<ProtectedEnvironment>> agentMap = new HashMap<>();
        List<ProtectedEnvironment> agents = Collections.singletonList(MockFactory.mockAgentResource());
        agentMap.put(null, agents);
        PowerMockito.when(envRetrievalsService.collectConnectableResources(any(ProtectedResource.class)))
            .thenReturn(agentMap);
        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode(OpenstackConstant.SUCCESS_CODE);
        Mockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(response);
        agentService.checkConnectivity(MockFactory.mockEnvironment());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：检查连通性失败 <br/>
     * 前置条件：agent接口返回错误 <br/>
     * 检查点：无异常
     */
    @Test
    public void should_raise_exception_if_check_application_failed_when_checkConnectivity() {
        String errorCode = "-1";
        String errorMsg = "errorMsg";
        Map<ProtectedResource, List<ProtectedEnvironment>> agentMap = new HashMap<>();
        List<ProtectedEnvironment> agents = Collections.singletonList(MockFactory.mockAgentResource());
        agentMap.put(null, agents);
        PowerMockito.when(envRetrievalsService.collectConnectableResources(any(ProtectedResource.class)))
            .thenReturn(agentMap);
        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode(errorCode);
        ActionResult actionResult = new ActionResult();
        actionResult.setBodyErr(errorCode);
        actionResult.setMessage(errorMsg);
        response.setErrorMessage(JSONObject.writeValueAsString(actionResult));
        Mockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(response);
        Assert.assertThrows(errorMsg, LegoCheckedException.class, () ->
            agentService.checkConnectivity(MockFactory.mockEnvironment()));
    }

}
