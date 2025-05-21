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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.k8s.protection.access.service.K8sCommonService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * K8sAgentProviderTest Test
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/4
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class K8sAgentProviderTest {
    private final K8sCommonService k8sCommonService = Mockito.mock(K8sCommonService.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final K8sAgentSelector k8sAgentSelector = new K8sAgentSelector(k8sCommonService, resourceService);

    @Before
    public void init() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("1");
        agent1.setPort(222);
        Mockito.when(k8sCommonService.getConnectiveInternalAgentByParams(any(), Mockito.anyBoolean()))
                .thenReturn(Collections.singletonList(agent1));
    }

    /**
     * 测试场景：applicable匹配成功
     * 前置条件：无
     * 检查点：返回True
     */
    @Test
    public void test_applicable() {
        Assert.assertTrue(k8sAgentSelector.applicable(ResourceTypeEnum.KUBERNETES_COMMON.getType()));
    }

    /**
     * 测试场景：校验选择环境agent成功 <br/>
     * 前置条件：参数补充正确 <br/>
     * 检查点：检查结果有返回且无异常
     */
    @Test
    @Ignore
    public void test_select_success() {
        ProtectedResource resource = MockFactory.mockProtectedResource();
        ProtectedEnvironment env = MockFactory.mockEnvironment();
        resource.setEnvironment(env);
        resource.setRootUuid("root");
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        Mockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(env));
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        List<Endpoint> endpoints = k8sAgentSelector.select(resource, new HashMap<>());
        Assert.assertEquals(1, endpoints.size());
    }
}
