/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.dameng.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.dameng.protection.access.DamengTestDataUtil;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

/**
 * dameng单节点注册测试类
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@RunWith(PowerMockRunner.class)
public class DamengSingleNodeProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private final DamengService damengService = PowerMockito.mock(DamengService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider =
        PowerMockito.mock(ResourceConnectionCheckProvider.class);

    private final DamengSingleNodeProvider damengSingleNodeProvider = new DamengSingleNodeProvider(providerManager,
        pluginConfigManager, damengService, resourceService);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengSingleNodeProvider.applicable(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType()));
    }

    /**
     * 用例场景：dameng单节点创建
     * 前置条件：联通性检查成功、且未创建过
     * 检查点：dameng单节点创建成功
     */
    @Test
    public void register_check_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.checkConnection(any()))
            .thenReturn(buildResourceCheckContext());
        damengSingleNodeProvider.register(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：dameng单机修改
     * 前置条件：联通性检查成功、且已经注册
     * 检查点：dameng单机修改成功
     */
    @Test
    public void update_check_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.checkConnection(any())).thenReturn(buildResourceCheckContext());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        damengSingleNodeProvider.register(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        ActionResult[] actionResults = Collections.singletonList(actionResult).toArray(new ActionResult[0]);
        PowerMockito.when(resourceService.check(any())).thenReturn(actionResults);
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        PowerMockito.when(damengService.getNodeInfoFromNodes(any())).thenReturn(DamengTestDataUtil.buildNodeInfo());
        Optional<String> status = damengSingleNodeProvider.healthCheckWithResultStatus(environment);
        Assert.assertEquals(status.get(), LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查失败
     */
    @Test
    public void health_check_failed() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(-1L);
        ActionResult[] actionResults = Collections.singletonList(actionResult).toArray(new ActionResult[0]);
        PowerMockito.when(resourceService.check(any())).thenReturn(actionResults);
        PowerMockito.when(damengService.getNodeInfoFromNodes(any())).thenReturn(DamengTestDataUtil.buildNodeInfo());
        Optional<String> status = damengSingleNodeProvider.healthCheckWithResultStatus(environment);
        Assert.assertEquals(status.get(), LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    /**
     * 用例场景：dameng单机注册
     * 前置条件：已经注册过该节点
     * 检查点：dameng单机注册失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_register_check() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        Set<String> uuidAndPortSet = new HashSet<>();
        uuidAndPortSet.add("uuid_8080");
        PowerMockito.when(damengService.getExistingUuidAndPort(any())).thenReturn(uuidAndPortSet);
        PowerMockito.when(damengService.connectUuidAndPort(any(), any())).thenReturn("uuid_8080");
        damengSingleNodeProvider.register(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    private ResourceCheckContext buildResourceCheckContext() {
        ResourceCheckContext context = new ResourceCheckContext();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setMessage("{\"dbPath\":\"/path\",\"dbName\":\"dameng\",\"version\":\"V8\"}");
        context.setActionResults(Collections.singletonList(actionResult));
        return context;
    }
}
