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
package openbackup.access.framework.resource.service.impl;

import openbackup.access.framework.resource.service.ResourceScanService;
import openbackup.access.framework.resource.util.ResourceConstant;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 资源扫描类
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-23
 */
@Service
public class ResourceScanServiceImpl implements ResourceScanService {
    private final JobService jobService;

    public ResourceScanServiceImpl(JobService jobService) {
        this.jobService = jobService;
    }

    @Override
    public List<JobBo> queryManualScanRunningJobByResId(String resId) {
        return queryManualScanRunningJob(resId, 0, 10);
    }

    @Override
    public List<JobBo> queryManualScanRunningPage(int page, int size) {
        return queryManualScanRunningJob(null, page, size);
    }

    private List<JobBo> queryManualScanRunningJob(String resId, int page, int size) {
        QueryJobRequest request = new QueryJobRequest();
        if (resId != null) {
            request.setSourceId(resId);
        }
        request.setTypes(
            Collections.singletonList(ResourceConstant.JOB_TYPE_PREFIX + ResourceConstant.MANUAL_SCAN_RESOURCE));
        request.setStatusList(JobStatusEnum.LONG_TIME_JOB_STOP_STATUS_LIST.stream()
            .map(JobStatusEnum::name)
            .collect(Collectors.toList()));
        PagingParamRequest pagingParamRequest = new PagingParamRequest(page, size);
        SortingParamRequest sortingParamRequest = new SortingParamRequest();
        return jobService.queryJobs(request, pagingParamRequest, sortingParamRequest).getRecords();
    }

    @Override
    public boolean jobIsFinished(String jobId) {
        JobBo jobBo = jobService.queryJob(jobId);
        if (jobBo == null) {
            return true;
        }
        return JobStatusEnum.FINISHED_STATUS_LIST.contains(JobStatusEnum.get(jobBo.getStatus()));
    }
}
