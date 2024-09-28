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
package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.provider.OpenstackProjectResourceProvider;
import openbackup.openstack.protection.access.provider.OpenstackResourceScanProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: test OpenstackProjectResourceProvider
 *
 */
public class OpenstackProjectResourceProviderTest {
    private static OpenstackProjectResourceProvider openstackProjectResourceProvider;

    private static final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private static final ResourceExtendInfoService resourceExtendInfoService = PowerMockito.mock(
            ResourceExtendInfoService.class);

    private static final OpenstackResourceScanProvider openstackResourceScanProvider = PowerMockito.mock(
            OpenstackResourceScanProvider.class);

    @BeforeClass
    public static void init() {
        openstackProjectResourceProvider = new OpenstackProjectResourceProvider(openstackResourceScanProvider,
                resourceService, resourceExtendInfoService);
    }

    /**
     * 用例名称：验证项目扫描成功。<br/>
     * 前置条件：参数正确。<br/>
     * check点：无异常，返回扫描结果。<br/>
     */
    @Test
    public void test_scan_success() {
        ProtectedResource projectResource = mockProjectResource();
        List<ProtectedResource> scannedRes = new ArrayList<>();
        ProtectedResource scannedResource = mockProjectResource();
        scannedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        scannedRes.add(scannedResource);
        Mockito.when(openstackResourceScanProvider.scanByAgent(any(), any())).thenReturn(scannedRes);
        List<ProtectedResource> resources = openstackProjectResourceProvider.scan(projectResource);
        Assert.assertEquals(1, resources.size());
    }

    /**
     * 用例场景：OpenStack项目资源插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.OPENSTACK_PROJECT.getType());
        openstackProjectResourceProvider.beforeCreate(resource);
        openstackProjectResourceProvider.beforeUpdate(resource);
        boolean isOpenstack = openstackProjectResourceProvider.applicable(resource);
        Assert.assertTrue(isOpenstack);
    }

    private ProtectedResource mockProjectResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("project_mock_uuid");
        protectedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_PROJECT.getType());
        Map<String, String> extendInfo = new HashMap<>();
        protectedResource.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        auth.setExtendInfo(extendInfo);
        protectedResource.setAuth(auth);
        return protectedResource;
    }

}
