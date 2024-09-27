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
package openbackup.tidb.resources.access.restore;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * TidbClusterRestoreInterceptorTest
 *
 * @author w00426202
 * @since 2023-07-27
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TidbUtil.class)
public class TidbClusterRestoreInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private CopyRestApi copyRestApi;

    private TidbClusterRestoreInterceptor tidbClusterRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbClusterRestoreInterceptor = new TidbClusterRestoreInterceptor(tidbService,
            new TidbAgentProvider(tidbService), copyRestApi, resourceService, defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbClusterRestoreInterceptor.applicable(ResourceSubTypeEnum.TIDB_CLUSTER.getType()));
    }

    @Test
    public void getLockResources_test() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("123123");
        restoreTask.setTargetObject(taskResource);
        tidbClusterRestoreInterceptor.getLockResources(restoreTask);
    }

    @Test
    public void intercept_test() throws Exception {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("123123");
        restoreTask.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(extendInfo);
        restoreTask.setTargetEnv(taskEnvironment);

        // setRestoreTaskEndpoint
        ProtectedResource protectedResource = new ProtectedResource();
        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(protectedResource);

        List<Endpoint> taskEndpoint = new ArrayList<>();
        Endpoint endpoint = new Endpoint("2we", "192.1.1.1", 22);
        taskEndpoint.add(endpoint);
        PowerMockito.when(tidbService.getTaskEndpoint(any())).thenReturn(taskEndpoint);

        // wrapTaskAfterInfo
        final Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        copy.setUuid("d4815037-7fcf-4d0d-8c56-e0ed0935c47d");
        copy.setAmount(0);
        copy.setGn(0);
        copy.setPrevCopyId("prevCopyId");
        copy.setNextCopyId("nextCopyId");
        copy.setPrevCopyGn(0);
        copy.setNextCopyGn(0);
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.mockStatic(TidbUtil.class);
        PowerMockito.doNothing().when(TidbUtil.class, "setTiupUuid", any(), anyString(), any(), any(), any());
        tidbClusterRestoreInterceptor.initialize(restoreTask);
    }

    @Test
    public void getRestoreFeature_test() {
        RestoreFeature restoreFeature = tidbClusterRestoreInterceptor.getRestoreFeature();
        Assert.assertNotNull(restoreFeature);
    }
}
