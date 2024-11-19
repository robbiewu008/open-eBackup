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

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ResourceScanService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.SpringBeanUtils;

import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * 资源扫描Schedule
 *
 */
@Slf4j
@Component
public class ResourceScanSchedule {
    /**
     * 最大更新的扫描任务的时间 2h
     */
    private static final long MAX_UPDATE_SCAN_JOB_TIME = 2 * 60 * 60 * 1000L;

    private final ResourceScanService resourceScanService;

    private final JobService jobService;

    public ResourceScanSchedule(ResourceScanService resourceScanService, JobService jobService) {
        this.resourceScanService = resourceScanService;
        this.jobService = jobService;
    }

    /**
     * 定时刷新扫描任务状态，防止30分钟超时
     * <p>
     * 刷新时间为任务开始时间的MAX_UPDATE_SCAN_JOB_TIME小时内，刷新隔间15分钟
     */
    @Scheduled(cron = "${system.schedule.resource.refreshScanJob}")
    public void refreshManualScanJob() {
        int page = 0;
        int size = 1000;
        List<JobBo> jobBos;
        do {
            jobBos = resourceScanService.queryManualScanRunningPage(page, size);
            page++;
            DeployTypeService deployTypeService = SpringBeanUtils.getBean(DeployTypeService.class);
            for (JobBo jobBo : jobBos) {
                long deltaTime = System.currentTimeMillis() - jobBo.getStartTime();
                long maxUpdateTime = MAX_UPDATE_SCAN_JOB_TIME;
                if (deltaTime > maxUpdateTime || deployTypeService.isHyperDetectDeployType()) {
                    continue;
                }
                UpdateJobRequest updateJobRequest = new UpdateJobRequest();
                updateJobRequest.setProgress(Optional.ofNullable(jobBo.getProgress()).orElse(0));
                jobService.updateJob(jobBo.getJobId(), updateJobRequest);
            }
        } while (jobBos.size() == size);
    }
}
