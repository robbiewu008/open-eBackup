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

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {ExchangeMailboxJobQueueProvider.class})
public class ExchangeMailboxJobQueueProviderTest {

    private ExchangeMailboxJobQueueProvider exchangeMailboxJobQueueProvider;

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    @Before
    public void before(){
        this.exchangeMailboxJobQueueProvider = new ExchangeMailboxJobQueueProvider(this.resourceService);
    }

    /**
     * 用例场景：测试获取自定义策略
     * 前置条件：无
     * 检查点：结果符合预期
     */
    @Test
    public void get_customized_schedule_policy_success() {
        Mockito.when(resourceService.getBasicResourceById(any()))
            .thenReturn(Optional.ofNullable(mockProtectedResource()));
        List<JobSchedulePolicy> policies = exchangeMailboxJobQueueProvider.getCustomizedSchedulePolicy(mockJob());
        Mockito.verify(resourceService, Mockito.times(2)).getBasicResourceById(any());
        Assert.assertEquals(1, policies.size());
        Assert.assertEquals(-10, policies.get(0).getScopeJobLimit());
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
        extendInfo.put(ExchangeConstant.MAX_CONCURRENT_JOB_NUMBER, "10");
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
