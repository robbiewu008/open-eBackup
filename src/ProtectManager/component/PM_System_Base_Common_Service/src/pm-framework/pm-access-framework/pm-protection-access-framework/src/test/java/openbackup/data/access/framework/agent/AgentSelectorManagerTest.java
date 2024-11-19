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

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.TokenMocker;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;


@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class})
public class AgentSelectorManagerTest {
    private final ProviderManager providerManager = mock(ProviderManager.class);
    private final UserService userService = mock(UserService.class);
    private DataBaseAgentSelector dataBaseAgentSelector =
            mock(DataBaseAgentSelector.class);
    private DefaultProtectAgentSelector defaultSelector =
        mock(DefaultProtectAgentSelector.class);
    private final AgentSelectorManager agentSelectorManager = new AgentSelectorManager(providerManager,
        userService, dataBaseAgentSelector,defaultSelector);

    /**
     * 用例名称：验证根据副本获取agent列表时，正确返回<br/>
     * 前置条件：无<br/>
     * check点：agent列表和数量都符合期望<br/>
     */
    @Test
    public void should_return_one_when_selectAgentsByCopy_given_one_resource() {
        // given
        List<Endpoint> endpoints = Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999));
        ProtectAgentSelector mockSelector = Mockito.mock(ProtectAgentSelector.class);
        given(providerManager.findProvider(eq(ProtectAgentSelector.class), any(), any())).willReturn(mockSelector);
        given(mockSelector.select(any(), any())).willReturn(endpoints);
        TokenBo tokenBo = TokenMocker.getMockedTokenBo();
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        given(userService.getUserInfoByUserId(any())).willReturn(userInnerResponse);
        // when
        List<Endpoint> returnEndPoints = agentSelectorManager.selectAgentsByCopy(CopyMocker.mockHcsCopy());
        // then
        Assert.assertEquals(returnEndPoints.size(), 1);
        Assert.assertEquals(returnEndPoints, endpoints);
    }
    /**
     * 用例名称：数据库插件，根据资源获取agent列表时，正确返回<br/>
     * 前置条件：无<br/>
     * check点：agent列表和数量都符合期望<br/>
     */
    @Test
    public void should_return_one_when_database_selectAgentByResource_given_one_resource() {
        // given
        List<Endpoint> endpoints = Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999));
        AgentSelector mockSelector = Mockito.mock(AgentSelector.class);
        given(providerManager.findProvider(eq(AgentSelector.class), any(), any())).willReturn(mockSelector);
        given(mockSelector.getSelectedAgents(any())).willReturn(endpoints);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("testuuid");
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        // when
        List<Endpoint> returnEndPoints = agentSelectorManager.selectAgentByResource(protectedResource,
                JobTypeEnum.BACKUP.getValue(), new HashMap<>());
        // then
        Assert.assertEquals(returnEndPoints.size(), 1);
        Assert.assertEquals(returnEndPoints, endpoints);
    }
    /**
     * 用例名称：非数据库插件，验证根据资源获取agent列表时，正确返回<br/>
     * 前置条件：无<br/>
     * check点：agent列表和数量都符合期望<br/>
     */
    @Test
    public void should_return_one_when_not_database_selectAgentByResource_given_one_resource() {
        // given
        List<Endpoint> endpoints = Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999));
        given(providerManager.findProvider(eq(AgentSelector.class), any(), any())).willReturn(null);
        ProtectAgentSelector mockSelector = Mockito.mock(ProtectAgentSelector.class);
        given(providerManager.findProviderOrDefault(eq(ProtectAgentSelector.class), any(), any())).willReturn(mockSelector);
        given(mockSelector.select(any(), any())).willReturn(endpoints);
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("testuuid");
        protectedResource.setType(ResourceTypeEnum.HCS.getType());
        // when
        List<Endpoint> returnEndPoints = agentSelectorManager.selectAgentByResource(protectedResource,
                JobTypeEnum.BACKUP.getValue(), new HashMap<>());
        // then
        Assert.assertEquals(returnEndPoints.size(), 1);
        Assert.assertEquals(returnEndPoints, endpoints);
    }
}