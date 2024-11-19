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
package openbackup.ndmp.protection.access.provider;

import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * ndmp自定义任务排队规则
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class NdmpJobQueueProviderTest {
    @InjectMocks
    private NdmpJobQueueProvider provider;

    @Mock
    private ResourceService resourceService;

    /**
     * 用例场景：ndmp任务类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        Job job = initJob();
        boolean result = provider.applicable(job);
        Assert.assertTrue(result);
    }

    private Job initJob() {
        Job job = new Job();
        job.setType(JobTypeEnum.BACKUP.getValue());
        job.setSourceSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        job.setSourceId("f9ecfd86-096d-3486-ba6e-67b3357331ca");
        return job;
    }

    /**
     * 用例场景：ndmp自定义排队策略组装成功 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回值正确
     */
    @Test
    public void test_get_job_customized_schedule_policy_success() {
        Job job = initJob();
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("f9ecfd86-096d-3486-ba6e-67b3357331ca");
        HashMap<String, String> extendInfo = new HashMap<>();
        resource.setExtendInfo(extendInfo);
        PowerMockito.when(resourceService.getBasicResourceById(eq(job.getSourceId())))
            .thenReturn(Optional.of(resource));
        List<JobSchedulePolicy> schedulePolicy = provider.getCustomizedSchedulePolicy(job);
        Assert.assertEquals(schedulePolicy.size(), 1);
        Assert.assertEquals(schedulePolicy.get(0).getScopeJobLimit(), -8);
    }
}
