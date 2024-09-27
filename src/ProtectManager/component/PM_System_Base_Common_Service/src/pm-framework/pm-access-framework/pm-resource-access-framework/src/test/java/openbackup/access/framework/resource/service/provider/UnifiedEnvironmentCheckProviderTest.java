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
package openbackup.access.framework.resource.service.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentParamProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentParamProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * 功能描述: UnifiedEnvironmentCheckProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({UnifiedEnvironmentCheckProvider.class, EnvironmentLinkStatusHelper.class})
public class UnifiedEnvironmentCheckProviderTest {
    private static ResourceService resourceService;
    private static AgentUnifiedService agentService;
    private static ProviderManager providerManager;
    private static UnifiedEnvironmentCheckProvider environmentCheckProvider;
    private static UnifiedEnvironmentParamProvider paramProvider;

    private static final String AGENT_ID = "d75480905d844090970dc72f297a2b3b";

    @BeforeClass
    public static void init() {
        resourceService = PowerMockito.mock(ResourceService.class);
        agentService = PowerMockito.mock(AgentUnifiedService.class);
        providerManager = PowerMockito.mock(ProviderManager.class);
        PluginConfigManager configManager = new DefaultPluginConfigManager();
        configManager.init();
        paramProvider = new UnifiedEnvironmentParamProvider(resourceService);
        environmentCheckProvider = new UnifiedEnvironmentCheckProvider(resourceService, agentService,
                configManager, providerManager, paramProvider);
    }

    @Before
    public void mock() {
        PowerMockito.when(resourceService.getResourceById(AGENT_ID)).thenReturn(Optional.of(mockAgentEnvironment()));
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(0);
        pageListResponse.setRecords(Collections.emptyList());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode("0");
        PowerMockito.when(agentService.checkApplication(any(), any())).thenReturn(response);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：注册环境时，对环境的参数和连通性检查成功，环境注册成功
     * 前置条件：受保护环境参数正确
     * 检查点：受保护环境状态为ONLINE
     */
    @Test
    public void test_register_check_success() {
        ProtectedEnvironment environment = mockEnvironment();
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：更新受保护环境时，对环境的参数和连通性检查成功，受保护环境更新成功
     * 前置条件：受保护环境参数正确
     * 检查点：受保护环境状态为ONLINE
     */
    @Test
    public void test_update_check_success() {
        ProtectedEnvironment environment = mockEnvironment();
        String uuid = UUID.randomUUID().toString();
        environment.setUuid(uuid);
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
        Assert.assertEquals(uuid, environment.getUuid());
    }

    /**
     * 用例场景：注册环境时，由于agent列表为空，导致环境注册失败
     * 前置条件：受保护环境的agent列表为空
     * 检查点：抛出异常，环境注册失败
     */
    @Test(expected = LegoCheckedException.class)
    public void test_register_check_failed_when_agents_is_empty() {
        ProtectedEnvironment environment = mockEnvironment();
        PowerMockito.when(resourceService.getResourceById(AGENT_ID)).thenReturn(Optional.empty());
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
    }

    /**
     * 用例场景：注册环境时，由于连通性检查失败，环境注册失败
     * 前置条件：受保护环境的参数正确
     * 检查点：抛出异常，环境注册失败
     */
    @Test(expected = LegoCheckedException.class)
    public void test_register_check_failed_get_error_code_from_body_err() {
        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode("200");
        ActionResult actionResult = new ActionResult();
        actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        response.setErrorMessage(JsonUtil.json(actionResult));
        PowerMockito.when(agentService.checkApplication(any(), any())).thenReturn(response);
        ProtectedEnvironment environment = mockEnvironment();
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
    }

    /**
     * 用例场景：注册环境时，由于已注册的同类型受保护环境已达到最大规格，环境注册失败
     * 前置条件：已注册的同类型受保护环境已达到最大规格
     * 检查点：抛出异常，环境注册失败
     */
    @Test(expected = LegoCheckedException.class)
    public void test_register_check_failed_when_spec_over_limit() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(4);
        ProtectedEnvironment environment = mockEnvironment();
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", environment.getType());
        filter.put("subType", environment.getSubType());
        PowerMockito.when(resourceService.query(0, 1, filter)).thenReturn(pageListResponse);
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
    }

    /**
     * 用例场景：注册环境时，由于该受保护环境已注册，环境注册失败
     * 前置条件：该受保护环境已注册
     * 检查点：抛出异常，环境注册失败
     */
    @Test(expected = LegoCheckedException.class)
    public void test_register_check_failed_when_env_repeat_registered() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(pageListResponse);
        ProtectedEnvironment environment = mockEnvironment();
        PowerMockito.when(providerManager.findProviderOrDefault(
                EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        environmentCheckProvider.check(environment);
    }

    /**
     * 用例场景：注册环境时，由于agent状态为离线，导致环境注册失败
     * 前置条件：受保护环境的agent离线
     * 检查点：抛出异常，环境注册失败
     */
    @Test(expected = LegoCheckedException.class)
    public void test_register_check_failed_when_agents_is_offline() {
        ProtectedEnvironment environment = mockEnvironment();
        ProtectedEnvironment agentEnvironment = mockAgentEnvironment();
        agentEnvironment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.when(resourceService.getResourceById(AGENT_ID)).thenReturn(Optional.of(agentEnvironment));
        PowerMockito.when(providerManager.findProviderOrDefault(
            EnvironmentParamProvider.class, environment, paramProvider)).thenReturn(paramProvider);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        environmentCheckProvider.check(environment);
    }

    private static ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("Test-FC-Env");
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8888);
        environment.setSubType(ResourceSubTypeEnum.KUBERNETES.getType());
        environment.setDependencies(mockDependencies());
        environment.setAuth(new Authentication());
        return environment;
    }

    private static Map<String, List<ProtectedResource>> mockDependencies() {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid(AGENT_ID);
        List<ProtectedResource> protectedResources = Collections.singletonList(agent);
        dependencies.put("agents", protectedResources);
        return dependencies;
    }

    private static ProtectedEnvironment mockAgentEnvironment() {
        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setName("Test-Agent-Env");
        agentEnv.setEndpoint("127.0.0.1");
        agentEnv.setPort(8888);
        agentEnv.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return agentEnv;
    }
}