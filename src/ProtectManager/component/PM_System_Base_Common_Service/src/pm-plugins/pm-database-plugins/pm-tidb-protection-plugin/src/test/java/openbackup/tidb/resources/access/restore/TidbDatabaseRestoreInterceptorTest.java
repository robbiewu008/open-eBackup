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

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * TidbClusterRestoreInterceptorTest
 *
 * @author w00426202
 * @since 2023-07-27
 */
@RunWith(PowerMockRunner.class)
public class TidbDatabaseRestoreInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private CopyRestApi copyRestApi;

    private TidbDatabaseRestoreInterceptor tidbDatabaseRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbDatabaseRestoreInterceptor = new TidbDatabaseRestoreInterceptor(tidbService,
            new TidbAgentProvider(tidbService), copyRestApi, resourceService, defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbDatabaseRestoreInterceptor.applicable(ResourceSubTypeEnum.TIDB_DATABASE.getType()));
    }

    @Test
    public void intercept_test() {
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("123123");
        taskResource.setParentUuid("89892323");
        restoreTask.setTargetObject(taskResource);

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TidbConstants.CLUSTER_INFO_LIST, "clusterlist");
        taskEnvironment.setExtendInfo(extendInfo);
        taskResource.setExtendInfo(extendInfo);
        restoreTask.setTargetEnv(taskEnvironment);

        // setRestoreTaskEndpoint
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(extendInfo);
        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(protectedResource);

        List<Endpoint> taskEndpoint = new ArrayList<>();
        Endpoint endpoint = new Endpoint("2we", "192.1.1.1", 22);
        taskEndpoint.add(endpoint);
        PowerMockito.when(tidbService.getTaskEndpoint(any())).thenReturn(taskEndpoint);
        tidbDatabaseRestoreInterceptor.setRestoreTaskEndpoint(restoreTask);
    }

}
