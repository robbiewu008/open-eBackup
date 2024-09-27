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
package openbackup.ndmp.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.impl.NdmpServiceImpl;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author t30021437
 * @since 2023-05-08
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {NdmpEnvironmentProvider.class, NdmpServiceImpl.class})
public class NdmpEnvironmentProviderTest {
    private NdmpEnvironmentProvider ndmpEnvironmentProvider;

    @Mock
    private NdmpServiceImpl ndmpService;

    @Mock
    private LocalStorageService localStorageService;

    @Mock
    private ProviderManager providerManager;

    @Mock
    private PluginConfigManager pluginConfigManager;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Before
    public void setUp() throws IllegalAccessException {
        ndmpEnvironmentProvider = new NdmpEnvironmentProvider(providerManager, pluginConfigManager, ndmpService,
            localStorageService, agentUnifiedService);
        MemberModifier.field(NdmpEnvironmentProvider.class, "ndmpService").set(ndmpEnvironmentProvider, ndmpService);
        MemberModifier.field(NdmpEnvironmentProvider.class, "localStorageService")
            .set(ndmpEnvironmentProvider, localStorageService);
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
    }

    @Test
    public void applicable() {
        Assert.assertTrue(ndmpEnvironmentProvider.applicable(ResourceSubTypeEnum.NDMP.getType()));
    }

    /**
     * 用例场景：检查成功
     * 前置条件：无
     * 检查点: 注册逻辑，检查成功
     */
    @Test
    public void check() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedEnvironment.setName("ndmp");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP, "1");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.VERSION, "{\"version\":\"1\"}");
        Authentication auth = new Authentication();
        auth.setAuthKey("test");
        auth.setAuthPwd("testPwd");
        protectedEnvironment.setAuth(auth);
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());

        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        PowerMockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);

        PowerMockito.when(ndmpService.getexistingNdmpresources(anyString(), any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(new ArrayList<>(exist));
        ProtectedEnvironment prv = getProtectedEnvironment();
        ndmpEnvironmentProvider.register(prv);
        Assert.assertEquals("1", prv.getExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP));
    }

    /**
     * 用例场景：检查成功
     * 前置条件：无
     * 检查点: 更新逻辑，检查成功
     */
    @Test
    public void checkUuidNull() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedEnvironment.setName("ndmp");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP, "1");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.VERSION, "{\"version\":\"1\"}");
        Authentication auth = new Authentication();
        auth.setAuthKey("test");
        auth.setAuthPwd("testPwd");
        protectedEnvironment.setAuth(auth);
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());

        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        PowerMockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);

        PowerMockito.when(ndmpService.getexistingNdmpresources(anyString(), any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(new ArrayList<>(exist));
        ProtectedEnvironment prv = getProtectedEnvironment();
        prv.setUuid("");
        ndmpEnvironmentProvider.register(prv);
        Assert.assertEquals("1", prv.getExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP));
    }

    private ResourceConnectionCheckProvider getResourceConnection() {
        return new ResourceConnectionCheckProvider() {
            @Override
            public ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource,
                ProtectedResourceChecker protectedResourceChecker) {
                return null;
            }

            @Override
            public boolean applicable(ProtectedResource object) {
                return false;
            }

            @Override
            public ResourceCheckContext checkConnection(ProtectedResource object) {
                return new ResourceCheckContext();
            }
        };
    }

    /**
     * 用例场景：检查成功
     * 前置条件：无
     * 检查点: 执行失败，返回离线
     */
    @Test
    public void healthCheckWithResultStatus() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedEnvironment.setName("ndmp");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP, "1");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.VERSION, "{\"version\":\"1\"}");
        Authentication auth = new Authentication();
        auth.setAuthKey("test");
        auth.setAuthPwd("testPwd");
        protectedEnvironment.setAuth(auth);
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());

        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        PowerMockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);

        PowerMockito.when(ndmpService.getexistingNdmpresources(anyString(), any())).thenReturn(new ArrayList<>(exist));
        ProtectedEnvironment prv = getProtectedEnvironment();
        Optional<String> status = ndmpEnvironmentProvider.healthCheckWithResultStatus(getProtectedEnvironment());
        Assert.assertEquals(LinkStatusEnum.OFFLINE.getStatus().toString(), status.get());
    }

    /**
     * 用例场景：浏览资源
     * 前置条件：无
     * 检查点: 浏览资源返回正确
     */
    @Test
    public void scan() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedEnvironment.setName("ndmp");
        protectedEnvironment.setUuid("123445");
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());
        PowerMockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Lists.newArrayList(protectedEnvironment));
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any())).thenReturn(response);
        List<ProtectedResource> agentResources = new ArrayList<>();
        agentResources.add(protectedEnvironment);
        PowerMockito.when(ndmpService.getInterAgents()).thenReturn(agentResources);
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(agentResources);
        List<ProtectedResource> result = ndmpEnvironmentProvider.scan(getProtectedEnvironment());
        Assert.assertEquals(1, result.size());
    }

    /**
     * 用例场景：浏览资源
     * 前置条件：无
     * 检查点: 浏览资源返回正确
     */
    @Test
    public void browse() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        protectedEnvironment.setName("ndmp");
        protectedEnvironment.setUuid("123445");
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());
        PowerMockito.when(ndmpService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        PowerMockito.when(agentUnifiedService.getDetailPageList(any(), any(), any(), any())).thenReturn(response);
        List<ProtectedResource> agentResources = new ArrayList<>();
        agentResources.add(protectedEnvironment);
        PowerMockito.when(ndmpService.getInterAgents()).thenReturn(agentResources);
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(agentResources);
        PageListResponse<ProtectedResource> result = ndmpEnvironmentProvider.browse(getProtectedEnvironment(),
            new BrowseEnvironmentResourceConditions());
        Assert.assertEquals(1, result.getTotalCount());
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("123");
        protectedEnvironment.setName("register");
        protectedEnvironment.setType("dataBase");

        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        Map<String, String> extendInfo = new HashMap<>();
        protectedEnvironment.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11111111");
        protectedResource.setExtendInfoByKey("version", "3.0.0");
        resources.add(protectedResource);
        resourceMap.put(NdmpConstant.AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);

        Authentication authentication = new Authentication();
        protectedEnvironment.setAuth(authentication);
        return protectedEnvironment;
    }
}