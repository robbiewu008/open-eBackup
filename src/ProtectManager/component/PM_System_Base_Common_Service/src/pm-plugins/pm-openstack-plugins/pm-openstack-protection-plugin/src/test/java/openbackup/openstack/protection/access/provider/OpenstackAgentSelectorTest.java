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
package openbackup.openstack.protection.access.provider;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.auth.model.RoleBo;
import openbackup.system.base.sdk.auth.UserInnerResponse;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.*;

/**
 * 功能描述: test OpenstackAgentSelectorTest
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-12
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class OpenstackAgentSelectorTest {
    private static OpenstackAgentSelector openstackAgentSelector;
    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private static final DefaultProtectAgentSelector defaultSelector = Mockito.mock(DefaultProtectAgentSelector.class);
    @BeforeClass
    public static void init() {
        openstackAgentSelector = new OpenstackAgentSelector(resourceService, defaultSelector);
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为OpenStackContainer <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        boolean applicable = openstackAgentSelector.applicable(ResourceTypeEnum.OPEN_STACK.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 测试场景：校验选择环境agent成功 <br/>
     * 前置条件：参数补充正确 <br/>
     * 检查点：检查结果有返回且无异常
     */
    @Test
    public void test_select_success() {
        ProtectedResource resource = MockFactory.mockProtectedResource();
        ProtectedEnvironment env = MockFactory.mockEnvironment();
        resource.setEnvironment(env);
        resource.setRootUuid(null);
        ProtectedEnvironment agent = MockFactory.mockEnvironment();
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        agent.setEndpoint("agent_endpoint");
        agent.setPort(1);
        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        dependency.put(OpenstackConstant.AGENTS, Collections.singletonList(agent));
        env.setDependencies(dependency);
        Mockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
            .thenReturn(Optional.of(env));
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        List<Endpoint> endpoints = openstackAgentSelector.select(resource, new HashMap<>());
        Assert.assertEquals(1, endpoints.size());
        Assert.assertEquals(agent.getEndpoint(), endpoints.get(0).getIp());
    }

    /**
     * 测试场景：测试指定agent下发时选择成功 <br/>
     * 前置条件：参数补充正确 <br/>
     * 检查点：检查结果有返回且无异常
     */
    @Test
    public void test_select_when_agents_in_param() {
        ProtectedResource resource = MockFactory.mockProtectedResource();
        ProtectedEnvironment agent = MockFactory.mockEnvironment();
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        agent.setEndpoint("127.0.0.1");
        agent.setPort(8088);
        HashMap<String, String> parameters = new HashMap<>();
        parameters.put(AgentKeyConstant.AGENTS_KEY, "test_agent");
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(mockUserInnerResponse()));
        Mockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
            .thenReturn(Optional.of(agent));
        List<Endpoint> endpointList = mockEndPointList();
        PowerMockito.when(defaultSelector.selectByAgentParameter(any(), any())).thenReturn(endpointList);
        List<Endpoint> endpoints = openstackAgentSelector.select(resource, parameters);
        Assert.assertEquals(1, endpoints.size());
        Assert.assertEquals(agent.getEndpoint(), endpoints.get(0).getIp());
    }

    private List<Endpoint> mockEndPointList() {
        List<Endpoint> endpointList = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(8088);
        endpointList.add(endpoint);
        return endpointList;
    }

    private UserInnerResponse mockUserInnerResponse() {
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        RoleBo roleBo = new RoleBo();
        roleBo.setRoleName(Constants.Builtin.ROLE_SYS_ADMIN);
        userInnerResponse.setRolesSet(new HashSet<>(Collections.singletonList(roleBo)));
        userInnerResponse.setUserId("test_user");
        return userInnerResponse;
    }
}