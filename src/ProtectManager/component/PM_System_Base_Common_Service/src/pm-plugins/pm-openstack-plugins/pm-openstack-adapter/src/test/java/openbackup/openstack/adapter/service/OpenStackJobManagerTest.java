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
package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.openstack.adapter.service.OpenStackJobManager;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

/**
 * {@link OpenStackJobManager} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-13
 */
public class OpenStackJobManagerTest {
    private final JobService jobService = Mockito.mock(JobService.class);

    private final OpenStackJobManager manager = new OpenStackJobManager(jobService);

    /**
     * 用例场景：监控任务是否成功时，如果任务为结束状态，则退出循环
     * 前置条件：任务已结束
     * 检查点： 能够成功退出循环
     */
    @Test
    public void should_callQueryJobOneTime_when_monitorIsJobSuccess_given_finishedStatusJob() {
        JobBo job = new JobBo();
        job.setStatus(JobStatusEnum.SUCCESS.name());
        String jobId = UUIDGenerator.getUUID();
        Mockito.when(jobService.queryJob(jobId)).thenReturn(job);
        manager.isJobSuccess(jobId);
        Mockito.verify(jobService, Mockito.times(1)).queryJob(jobId);
    }

    /**
     * 用例场景：查询所有任务时，能够退出循环，并返回正确任务个数
     * 前置条件：无
     * 检查点： 能够成功退出循环，且返回正确任务数
     */
    @Test
    public void should_returnAllJobsSuccess_when_queryAllJobs() {
        String resourceId = UUIDGenerator.getUUID();
        String jobType = JobTypeEnum.RESTORE.getValue();

        List<JobBo> firstResult = new ArrayList<>();
        IntStream.range(0, 20).forEach(i -> firstResult.add(new JobBo()));
        PageListResponse<JobBo> firstResp = new PageListResponse<>(21, firstResult);
        PageListResponse<JobBo> secondResp = new PageListResponse<>(21, Collections.singletonList(new JobBo()));

        Mockito.when(jobService.queryJobs(any(), any(PagingParamRequest.class)))
            .thenReturn(firstResp)
            .thenReturn(secondResp);

        List<JobBo> jobs = manager.queryAllJobs(resourceId, jobType);
        assertThat(jobs).hasSize(21);
    }
}
