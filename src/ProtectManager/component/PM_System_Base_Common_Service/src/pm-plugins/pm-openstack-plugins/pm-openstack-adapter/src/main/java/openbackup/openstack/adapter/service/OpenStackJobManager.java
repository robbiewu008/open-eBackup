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

import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 任务相关操作管理器
 *
 */
@Slf4j
@Component
public class OpenStackJobManager {
    private static final String START_TIME = "start_time";
    private static final String DESC_ORDER_TYPE = "desc";
    private static final long MAX_WAIT_TIME = 5 * 60 * 1000L; // 5分钟毫秒数

    private final JobService jobService;

    public OpenStackJobManager(JobService jobService) {
        this.jobService = jobService;
    }

    /**
     * 根据资源id和类型查询最新一条任务
     *
     * @param resourceId 资源id
     * @param jobType {@link JobTypeEnum} 任务类型
     * @return job
     */
    public Optional<JobBo> queryLatestJob(String resourceId, String jobType) {
        // 过滤条件
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setSourceId(resourceId);
        conditions.setTypes(Collections.singletonList(jobType));
        // 排序条件
        SortingParamRequest sortParam = new SortingParamRequest();
        sortParam.setOrderBy(START_TIME);
        sortParam.setOrderType(DESC_ORDER_TYPE);
        // 分页大小
        PagingParamRequest pageParam = new PagingParamRequest();
        pageParam.setPageSize(1);
        List<JobBo> jobs = jobService.queryJobs(conditions, pageParam, sortParam, null).getRecords();
        return jobs.stream().findFirst();
    }

    /**
     * 查询某资源所有jobType类型的任务
     *
     * @param resourceId 资源id
     * @param jobType 任务类型
     * @return 任务列表
     */
    public List<JobBo> queryAllJobs(String resourceId, String jobType) {
        // 过滤条件
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setSourceId(resourceId);
        conditions.setTypes(Collections.singletonList(jobType));

        List<JobBo> jobs = new ArrayList<>();
        int pageNo = 0;
        int pageSize = 20;
        PageListResponse<JobBo> response;
        PagingParamRequest pageParam = new PagingParamRequest(pageNo, pageSize);
        do {
            pageParam.setStartPage(pageNo);
            response = jobService.queryJobs(conditions, pageParam);
            jobs.addAll(response.getRecords());
            pageNo++;
        } while (response.getRecords().size() == pageSize);

        return jobs;
    }

    /**
     * 轮询检查任务是否已成功
     *
     * @param jobId 任务id
     * @return 任务是否成功
     */
    public boolean isJobSuccess(String jobId) {
        long sleepTime = 1000L;
        long totalWaitTime = 0L;
        JobBo job = jobService.queryJob(jobId);
        while (totalWaitTime <= MAX_WAIT_TIME) {
            if (Boolean.TRUE.equals(JobStatusEnum.get(job.getStatus()).finishedStatus())) {
                break;
            }
            CommonUtil.sleep(sleepTime);
            totalWaitTime += sleepTime;
            job = jobService.queryJob(jobId);
            sleepTime *= 2;
        }
        log.info("Monitor job: {} after: {}ms, status: {}.", jobId, totalWaitTime, job.getStatus());
        return JobStatusEnum.get(job.getStatus()).checkSuccess();
    }

    /**
     * 强制停止卡住的任务
     *
     * @param jobId 任务id
     */
    public void forceStopJob(String jobId) {
        jobService.forceStopJob(jobId, false);
    }

    /**
     * 查询任务
     *
     * @param jobId 任务id
     * @return {@link JobBo} 任务信息
     */
    public JobBo queryJob(String jobId) {
        return jobService.queryJob(jobId);
    }
}
