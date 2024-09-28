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
package openbackup.access.framework.resource.schedule;

import openbackup.access.framework.resource.schedule.ResourceScanSchedule;
import openbackup.access.framework.resource.service.ResourceScanService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.List;

/**
 * ResourceScanScheduleTest
 *
 */
public class ResourceScanScheduleTest {
    private ResourceScanSchedule resourceScanSchedule;

    private ResourceScanService resourceScanService;

    private JobService jobService;

    @Before
    public void init() {
        resourceScanService = Mockito.mock(ResourceScanService.class);
        jobService = Mockito.mock(JobService.class);
        resourceScanSchedule = new ResourceScanSchedule(resourceScanService, jobService);
    }

    /**
     * 用例名称：定时更新扫描任务。
     * 前置条件：无。
     * check点：触发任务更新。
     */
    @Test
    public void refresh_manual_scan_job_success() {
        List<JobBo> jobBos = new ArrayList<>();
        JobBo jobBo1 = new JobBo();
        jobBo1.setJobId("jobId");
        jobBo1.setStartTime(Long.MAX_VALUE);
        jobBos.add(jobBo1);
        jobBos.add(jobBo1);
        Mockito.when(resourceScanService.queryManualScanRunningPage(0, 1000)).thenReturn(jobBos);
        resourceScanSchedule.refreshManualScanJob();
        Mockito.verify(jobService, Mockito.times(2)).updateJob(Mockito.anyString(), Mockito.any());
    }
}
