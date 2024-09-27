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
package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.controller.CommonResourceAccessController;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.access.framework.resource.service.lock.ResourceDistributedLockService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.query.SessionService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.Optional;

/**
 * CommonResourceAccessController测试
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-19
 */
public class CommonResourceAccessControllerTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class, Mockito.RETURNS_DEEP_STUBS);

    private final SessionService sessionService = Mockito.mock(SessionService.class, Mockito.RETURNS_DEEP_STUBS);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class, Mockito.RETURNS_DEEP_STUBS);

    private final ResourceExtendInfoService resourceExtendInfoService = Mockito.mock(ResourceExtendInfoService.class,
        Mockito.RETURNS_DEEP_STUBS);

    private final ResourceDistributedLockService distributedLockService = Mockito.mock(ResourceDistributedLockService.class, Mockito.RETURNS_DEEP_STUBS);

    private final ProtectedResourceMonitorService protectedResourceMonitorService = Mockito.mock(
        ProtectedResourceMonitorService.class);
    private final JobService jobService = Mockito.mock(JobService.class, Mockito.RETURNS_DEEP_STUBS);

    /**
     * 用例名称：验证资源注册接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：不重名注册成功。重名失败<br/>
     */
    @Test
    public void create_resource() {
        ProtectedResource resource = new ProtectedResource();
        CommonResourceAccessController controller = createCommonResourceAccessController();

        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class, Mockito.RETURNS_DEEP_STUBS);
        Mockito.when(providerManager.findProvider(ResourceProvider.class, resource, null)).thenReturn(resourceProvider);
        Mockito.when(resourceProvider.getResourceFeature().isShouldCheckResourceNameDuplicate()).thenReturn(true);
        PageListResponse pageListResponse = Mockito.mock(PageListResponse.class, Mockito.RETURNS_DEEP_STUBS);
        Mockito.when(sessionService.call(Mockito.any(), Mockito.eq(Constants.Builtin.ROLE_SYS_ADMIN)))
            .thenReturn(pageListResponse);
        Mockito.when(distributedLockService.tryLockAndGet(Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(new String[] {"11"});
        // 重名
        Mockito.when(pageListResponse.getRecords().size()).thenReturn(1).thenReturn(0);
        Assert.assertThrows(LegoCheckedException.class, () -> controller.createResource(resource));
        // 不重名
        Mockito.when(resourceService.create(Mockito.any(), Mockito.eq(true))).thenReturn(new String[] {"11"});
        Assert.assertEquals(controller.createResource(resource).getUuid(), "11");
    }

    /**
     * 用例名称：验证资源更新接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：不重名更新成功。重名失败<br/>
     */
    @Test
    public void update_resource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("aa");
        CommonResourceAccessController controller = createCommonResourceAccessController();

        Mockito.when(resourceService.query(Mockito.eq(0), Mockito.eq(1), Mockito.any()).getRecords())
            .thenReturn(Collections.singletonList(new ProtectedResource()));

        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class, Mockito.RETURNS_DEEP_STUBS);
        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(resourceProvider);
        Mockito.when(resourceProvider.getResourceFeature().isShouldCheckResourceNameDuplicate()).thenReturn(true);
        PageListResponse pageListResponse = Mockito.mock(PageListResponse.class, Mockito.RETURNS_DEEP_STUBS);
        Mockito.when(sessionService.call(Mockito.any(), Mockito.eq(Constants.Builtin.ROLE_SYS_ADMIN)))
            .thenReturn(pageListResponse);
        // 重名
        Mockito.when(pageListResponse.getRecords().size()).thenReturn(1).thenReturn(0);
        Assert.assertThrows(LegoCheckedException.class, () -> controller.updateResource("11", resource));
        // 不重名
        controller.updateResource("11", resource);
        Mockito.verify(distributedLockService, Mockito.times(1))
            .tryLockAndRun(Mockito.any(), Mockito.any(), Mockito.any());
    }

    /**
     * 用例名称：验证资源查询接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：查询符合预期<br/>
     */
    @Test
    public void query_resource_page() {
        PageListResponse pageListResponse = Mockito.mock(PageListResponse.class, Mockito.RETURNS_DEEP_STUBS);
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);
        Mockito.when(pageListResponse.getRecords()).thenReturn(Collections.singletonList(new ProtectedResource()));
        PageListResponse<ProtectedResource> response = createCommonResourceAccessController().queryResources(0, 1, null,
            false);
        Assert.assertEquals(response.getRecords().size(), 1);
    }

    /**
     * 用例名称：验证资源扫描接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：未有扫描则正常运行，有则抛错。<br/>
     */
    @Test
    public void scan_protected_resource_success() {
        Mockito.when(resourceService.checkEnvScanTaskIsRunning(Mockito.any()))
            .thenReturn(false)
            .thenReturn(true);
        CommonResourceAccessController controller = createCommonResourceAccessController();
        Mockito.when(resourceService.getBasicResourceById(Mockito.any())).thenReturn(Optional.of(new ProtectedResource()));
        controller.scanProtectedResource("11");
        Assert.assertThrows(LegoCheckedException.class, () -> controller.scanProtectedResource("22"));
    }

    private CommonResourceAccessController createCommonResourceAccessController() {
        return new CommonResourceAccessController(resourceService, sessionService, providerManager, distributedLockService,
            resourceExtendInfoService);
    }
}
