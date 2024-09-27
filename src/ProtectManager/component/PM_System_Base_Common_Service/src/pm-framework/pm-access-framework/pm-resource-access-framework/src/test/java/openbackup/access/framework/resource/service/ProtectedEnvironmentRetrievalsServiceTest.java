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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * ProtectedEnvironmentRetrievalsService测试类
 *
 * @author h30027154
 * @since 2022-07-05
 */
public class ProtectedEnvironmentRetrievalsServiceTest {
    /**
     * 用例名称：能正常的解析出环境。<br/>
     * 前置条件：配置文件已配置。<br/>
     * check点：根据传入resource结构解析出环境信息
     */
    @Test
    public void can_success_solve_environment() {
        ProtectedEnvironmentRetrievalsService protectedEnvironmentRetrievalsService
            = createProtectedEnvironmentRetrievalsService();

        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap
            = protectedEnvironmentRetrievalsService.collectConnectableResources(createInputResource());
        resourceListMap.forEach(((resource, protectedEnvironments) -> {
            if (Objects.equals(resource.getUuid(), "db-01")) {
                Assert.assertEquals(protectedEnvironments.size(), 1);
                Assert.assertEquals(protectedEnvironments.get(0).getEndpoint(), "1.1.1.1");
            }
        }));
    }

    private ProtectedResource createInputResource() {
        ProtectedResource root = new ProtectedResource();
        root.setName("root-name");
        root.setDependencies(new HashMap<>());
        ProtectedResource child = new ProtectedResource();
        child.setUuid("db-01");
        root.getDependencies().put("children", Arrays.asList(child));

        child.setDependencies(new HashMap<>());
        ProtectedEnvironment host = new ProtectedEnvironment();
        host.setUuid("host-uuid");
        host.setEndpoint("1.1.1.1");
        child.getDependencies().put("host", Arrays.asList(host));

        return root;
    }

    private ProtectedEnvironmentRetrievalsService createProtectedEnvironmentRetrievalsService() {
        ResourceExtensionManager resourceExtensionManager = Mockito.mock(ResourceExtensionManager.class);
        CollectableConfig collectableConfig = new CollectableConfig();
        collectableConfig.setResource("children");
        collectableConfig.setEnvironment("host");
        Mockito.when(resourceExtensionManager.invoke(Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(Arrays.asList(collectableConfig));

        ResourceService resourceService = Mockito.mock(ResourceService.class);
        ProtectedResource resource01 = new ProtectedResource();
        resource01.setUuid("db-01");
        resource01.setPath("dbPath");
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(Collections.singletonList(resource01));
        Mockito.when(resourceService.query(Mockito.anyInt(), Mockito.anyInt(), Mockito.any()))
            .thenReturn(pageListResponse);

        ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);
        ProtectedEnvironmentRetrievalsService protectedEnvironmentRetrievalsService
            = new ProtectedEnvironmentRetrievalsService(resourceExtensionManager, resourceService,
            protectedEnvironmentService);
        return protectedEnvironmentRetrievalsService;
    }
}
