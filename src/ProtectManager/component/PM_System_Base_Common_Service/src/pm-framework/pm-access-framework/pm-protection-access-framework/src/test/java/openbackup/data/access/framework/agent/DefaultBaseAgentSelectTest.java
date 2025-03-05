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
package openbackup.data.access.framework.agent;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.mocks.ResourceMocker;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.TokenBo;
import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;

/**
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class})
public class DefaultBaseAgentSelectTest {
    private static ProviderManager providerManager = mock(ProviderManager.class);
    private static ResourceService resourceService = mock(ResourceService.class);
    private static ProtectedResourceChecker protectedResourceChecker = mock(ProtectedResourceChecker.class);
    private static DataBaseAgentSelector dataBaseAgentSelector = new DataBaseAgentSelector();

    @BeforeClass
    public static void init() {
        Whitebox.setInternalState(dataBaseAgentSelector, "providerManager", providerManager);
        Whitebox.setInternalState(dataBaseAgentSelector, "resourceService", resourceService);
        Whitebox.setInternalState(dataBaseAgentSelector, "protectedResourceChecker", protectedResourceChecker);
    }
    /**
     * 用例名称：验证根据参数获取agent列表时，正确返回<br/>
     * 前置条件：无<br/>
     * check点：agent数量都符合期望<br/>
     */
    @Test
    public void should_return_one_when_select_given_agentSelectParam() {
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(new ProtectedResource()).build();
        // given
        List<Endpoint> endpoints = Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999));
        ProtectedResourceChecker mockSelector = Mockito.mock(ProtectedResourceChecker.class);
        given(providerManager.findProviderOrDefault(eq(ProtectedResourceChecker.class), any(), any())).willReturn(mockSelector);
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap  = mockCollectConnectableResources();
        given(mockSelector.collectConnectableResources(any())).willReturn(resourceListMap);
        given(resourceService.getResourceById(any())).willReturn(Optional.of(ResourceMocker.mockResource()));
        // when
        List<Endpoint> returnEndPoints = dataBaseAgentSelector.getSelectedAgents(agentSelectParam);
         // then
        Assert.assertEquals(returnEndPoints.size(), 1);
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> mockCollectConnectableResources() {
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        List<ProtectedEnvironment> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("1");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        list.add(protectedEnvironment);
        map.put(ResourceMocker.mockResource(), list);
        return map;
    }

}