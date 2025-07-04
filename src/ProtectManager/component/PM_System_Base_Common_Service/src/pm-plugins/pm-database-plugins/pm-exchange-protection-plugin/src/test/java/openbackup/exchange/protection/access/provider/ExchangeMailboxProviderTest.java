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
package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.impl.ExchangeServiceImpl;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ExchangeMailboxProvider Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ExchangeMailboxProvider.class, ExchangeServiceImpl.class})
public class ExchangeMailboxProviderTest {
    private final ExchangeMailboxProvider provider = new ExchangeMailboxProvider();

    /**
     * 用例场景：测试检查受保护资源
     * 前置条件：无
     * 检查点：path设置成功
     */
    @Test
    public void testBeforeCreate() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("endpoint");
        ExchangeServiceImpl exchangeService = Mockito.mock(ExchangeServiceImpl.class);
        Mockito.when(exchangeService.getEnvironmentById(ArgumentMatchers.any())).thenReturn(environment);
        ReflectionTestUtils.setField(provider, "exchangeService", exchangeService);
        ProtectedResource resource = new ProtectedResource();
        ProtectedResource dependency = new ProtectedResource();
        dependency.setUuid("1");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(ExchangeConstant.EXCHANGE_AGENTS, Collections.singletonList(dependency));
        resource.setDependencies(dependencies);
        provider.beforeCreate(resource);
        Assert.assertEquals("endpoint", resource.getPath());
    }

    /**
     * 用例场景：测试设置不支持lanfree
     * 前置条件：无
     * 检查点：设置成功
     */
    @Test
    public void testGetResourceFeature() {
        ResourceFeature resourceFeature = provider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isSupportedLanFree());
    }
}