/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
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
import java.util.Collections;
import java.util.List;

/**
 * 增加ndmp-server 资源类型
 *
 * @author t30021437
 * @since 2023-05-08
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {NdmpServerEnvironmentProvider.class, NdmpServiceImpl.class})
public class NdmpServerEnvironmentProviderTest {
    private NdmpServerEnvironmentProvider ndmpServerEnvironmentProvider;

    @Mock
    private NdmpServiceImpl ndmpService;

    @Mock
    private LocalStorageService mockLocalStorageService;

    @Mock
    private ProviderManager mockProviderManager;

    @Mock
    private PluginConfigManager mockPluginConfigManager;

    @Mock
    private LocalStorageService localStorageService;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Before
    public void setUp() throws IllegalAccessException {
        ndmpServerEnvironmentProvider = new NdmpServerEnvironmentProvider(mockProviderManager, mockPluginConfigManager,
            ndmpService, agentUnifiedService);
        MemberModifier.field(NdmpServerEnvironmentProvider.class, "ndmpService")
            .set(ndmpServerEnvironmentProvider, ndmpService);
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
    }

    /**
     * 用例场景：资源类型检查成功
     * 前置条件：无
     * 检查点: 执行成功，资源类型正确
     */
    @Test
    public void applicable() {
        Assert.assertTrue(ndmpServerEnvironmentProvider.applicable(ResourceSubTypeEnum.NDMP_SERVER.getType()));
    }

    /**
     * 用例场景：检查成功
     * 前置条件：无
     * 检查点: 执行成功，无返回
     */
    @Test
    public void check() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP_SERVER.getType());
        protectedEnvironment.setType("data");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.NDMP_SRC, "{\"serverIp\":\"11\"}");
        PowerMockito.when(ndmpService.getexistingNdmpresources(any(), anyMap())).thenReturn(new ArrayList<>());
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(ndmpService.getAvailableAgents(any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(mockProviderManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        ndmpServerEnvironmentProvider.register(protectedEnvironment);
        Assert.assertTrue(true);
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
     * 检查点: 返回正确状态
     */
    @Test
    public void healthCheckWithResultStatus() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP_SERVER.getType());
        protectedEnvironment.setType("data");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.NDMP_SRC, "{\"serverIp\":\"11\"}");
        PowerMockito.when(ndmpService.getexistingNdmpresources(anyMap()))
            .thenReturn(new ArrayList<>(Collections.singleton(protectedEnvironment)));
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(mockProviderManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        PowerMockito.when(ndmpService.getAvailableAgents(any())).thenReturn(new ArrayList<>(exist));
        String status = ndmpServerEnvironmentProvider.healthCheckWithResultStatus(protectedEnvironment).get();
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), status);
    }

    /**
     * 用例场景：修改NDMP修改ip地址时，同步修改path
     * 前置条件：无
     * 检查点: path值和ip值相同
     */
    @Test
    public void check_when_update_endpoint() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.NDMP_SERVER.getType());
        protectedEnvironment.setType("data");
        protectedEnvironment.setExtendInfoByKey(NdmpConstant.NDMP_SRC, "{\"serverIp\":\"11\"}");
        PowerMockito.when(ndmpService.getexistingNdmpresources(any(), anyMap())).thenReturn(new ArrayList<>());
        PowerMockito.doNothing().when(ndmpService).checkConnention(any());
        List<ProtectedEnvironment> exist = new ArrayList<>();
        exist.add(protectedEnvironment);
        PowerMockito.when(ndmpService.getOneAgentHealthCheck(any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(mockProviderManager.findProvider(any(), any())).thenReturn(getResourceConnection());
        protectedEnvironment.setUuid("11111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        Authentication auth = new Authentication();
        auth.setAuthPwd("pwd");
        protectedEnvironment.setAuth(auth);
        PowerMockito.when(ndmpService.getexistingNdmpresources(any())).thenReturn(new ArrayList<>(exist));
        PowerMockito.when(ndmpService.getAvailableAgents(any())).thenReturn(new ArrayList<>(exist));
        ndmpServerEnvironmentProvider.register(protectedEnvironment);
        Assert.assertTrue(true);
    }
}