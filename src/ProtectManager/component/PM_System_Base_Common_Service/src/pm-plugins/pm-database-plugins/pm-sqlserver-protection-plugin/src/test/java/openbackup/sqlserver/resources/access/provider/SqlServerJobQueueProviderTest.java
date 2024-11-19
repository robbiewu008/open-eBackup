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
package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.model.job.Job;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({SqlServerJobQueueProvider.class})
public class SqlServerJobQueueProviderTest {

    private SqlServerJobQueueProvider sqlServerJobQueueProvider;

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    @Before
    public void before(){
        this.sqlServerJobQueueProvider = new SqlServerJobQueueProvider(this.resourceService);
    }
    @Test
    public void get_customized_schedule_policy_success() {
        Mockito.when(resourceService.getBasicResourceById(any())).thenReturn(Optional.ofNullable(mockProtectedResource()));
        sqlServerJobQueueProvider.getCustomizedSchedulePolicy(mockJob());
        Mockito.verify(resourceService, Mockito.times(1)).getBasicResourceById(any());
    }

    private Job mockJob() {
        Job job = new Job();
        job.setSourceId("test");
        job.setSourceSubType("GeneralDb");
        return job;
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
