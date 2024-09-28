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
package openbackup.tpops.protection.access.provider;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.service.impl.TpopsGaussDBServiceImpl;

import lombok.SneakyThrows;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 */
@RunWith(MockitoJUnitRunner.class)
@PrepareForTest(TpopsGaussDBResourceProvider.class)
public class TpopsGaussDBResourceProviderTest {
    private TpopsGaussDBResourceProvider tpopsGaussDBResourceProvider;

    private ProviderManager providerManager;

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private LocalStorageService localStorageService;

    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private TpopsGaussDBServiceImpl gaussDbService;

    private TaskRepositoryManager taskRepositoryManager;

    private AgentUnifiedService agentUnifiedService;

    @Before
    public void init() throws IllegalAccessException {
        providerManager = Mockito.mock(ProviderManager.class);
        agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        clusterIntegrityChecker = Mockito.mock(UnifiedClusterResourceIntegrityChecker.class);
        localStorageService = Mockito.mock(LocalStorageService.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
        tpopsGaussDBResourceProvider = new TpopsGaussDBResourceProvider();
        gaussDbService = new TpopsGaussDBServiceImpl(resourceService, providerManager, resourceConnectionCheckProvider,
            clusterIntegrityChecker, taskRepositoryManager);
        MemberModifier.field(TpopsGaussDBResourceProvider.class, "tpopsGaussDbService")
            .set(tpopsGaussDBResourceProvider, gaussDbService);
    }

    /**
     * 用例场景：检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        Assert.assertTrue(tpopsGaussDBResourceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：设置路径
     * 前置条件：复制校验
     * 检查点：复制校验路径设置成功
     */
    @SneakyThrows
    @Test
    public void beforeCreate() {
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put("agents", resources);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setDependencies(dependencies);
        protectedEnvironment.setEndpoint("1.2.3.4");
        tpopsGaussDBResourceProvider.beforeCreate(protectedEnvironment);
        Assert.assertEquals("1.2.3.4", protectedEnvironment.getEndpoint());
    }

    /**
     * 用例场景：调用更新资源接口
     * 前置条件：无
     * 检查点：资源更新接口，不会对参数做任何操作
     */
    @Test
    public void beforeUpdate() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        tpopsGaussDBResourceProvider.beforeUpdate(protectedResource);
        Assert.assertEquals(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType(), protectedResource.getSubType());
    }

    /**
     * 将ResourceFeature对象的isSupportedLanFree属性置为false表示不支持lanfree并返回
     *
     * @return 资源是否支持lanfree
     */
    @Test
    public void getResourceFeature() {
        Assert.assertFalse(tpopsGaussDBResourceProvider.getResourceFeature().isSupportedLanFree());
    }
}
