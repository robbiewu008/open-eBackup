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
package openbackup.data.access.framework.protection.controller;

import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.common.util.EngineUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.converters.JobDataConverter;
import openbackup.data.access.framework.protection.controller.req.UpdateJobStatusRequest;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.listener.ITaskCompleteListener;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.job.dto.JobLogDto;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.sdk.accesspoint.model.StopPlanBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.time.Duration;
import java.util.List;
import java.util.concurrent.TimeUnit;

import javax.validation.Valid;

/**
 * Live Mount Controller
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/internal/jobs")
public class JobController {
    // 任务进度在redis保存时间，12个小时
    private static final Integer JOB_PROCESS_EXPIRE_TIME = 12 * 60 * 60;

    private static final String UPDATE_JOB_STATUS_KEY = "UpdateJobStatus";
    private static final String UPDATE_JOB_PROCESS_KEY = "UpdateJobProcess";

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private JobService jobService;

    @Autowired
    private ITaskCompleteListener taskCompleteListener;

    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 停止计划 接口：/v1/internal/jobs/{jobId}/action/stop 方法：Put
     *
     * @param jobId plan id
     * @param stopPlanBo job stop bo
     */
    @ExterAttack
    @PutMapping("/{jobId}/action/stop")
    public void stopTask(@PathVariable("jobId") String jobId, @RequestBody StopPlanBo stopPlanBo) {
        // 根据资源类型调用不同的provider处理
        String engineTaskTypeKey = EngineUtil.getEngineTaskTypeKey(stopPlanBo.getSourceSubType(), stopPlanBo.getType());
        JobProvider jobProvider = providerManager.findProvider(JobProvider.class, engineTaskTypeKey, null);
        String stopJobId = stopPlanBo.getAssociativeId();

        // 本地文件系统云备份使用的是归档备份，防勒索部署和安全一体机形态(非智能侦测任务)则使用默认provider
        if (jobProvider == null || deployTypeService.isHyperDetectDeployType() || isCyberEngineUnifiedJob(
            engineTaskTypeKey)) {
            jobProvider = unifiedJobProvider;
            stopJobId = jobId;
        }
        String jobClassName = jobProvider.getClass().getSimpleName();
        log.info("Send command of aborting job to {}. AssociativeId: {}", jobClassName, stopJobId);
        jobProvider.stopJob(stopJobId);
    }

    private boolean isCyberEngineUnifiedJob(String engineTaskTypeKey) {
        return deployTypeService.isCyberEngine() && !StringUtils.endsWith(engineTaskTypeKey,
            JobTypeEnum.ANTI_RANSOMWARE.getValue());
    }

    /**
     * 更新任务状态，如果任务状态为完结状态，则调用任务完成监听器，只更新日志；否则，同时更新状态和日志
     *
     * @param jobId 任务ID
     * @param request 任务
     */
    @ExterAttack
    @PutMapping("/{jobId}/action/update-status")
    public void updateJobStatus(@PathVariable String jobId, @RequestBody @Valid UpdateJobStatusRequest request) {
        log.info("Update job, jobId: {}, taskId: {}, status: {}, speed: {}, progress: {}", jobId,
            request.getTaskId(), request.getStatus(), request.getSpeed(), request.getProgress());
        if (!jobService.isJobPresent(jobId)) {
            log.warn("Job is not found, jobId: {}", jobId);
            return;
        }
        JobBo job = jobService.queryJob(jobId);
        if (JobStatusEnum.get(job.getStatus()).finishedStatus()) {
            log.info("Job was already finished. jobId: {}, pm job status: {}, ubc status: {}", jobId, job.getStatus(),
                request.getStatus());
            return;
        }
        if (isAlreadyUpdate(jobId, request)) {
            log.info("Job is alreadyUpdate,jobId={},speed={},jobStatus={}", request.getJobRequestId(),
                request.getSpeed(), request.getStatus());
            return;
        }
        try {
            List<JobLogDto> jobLogsDto = JobDataConverter.getJobLogsFromStatusRequest(jobId, request);
            jobService.updateJobLogs(jobId, jobLogsDto);
            handleUpdateStatus(jobId, request);
            handleCompletedTask(job, request);
        } catch (Exception e) {
            // 保留异常场景下，ubc重复上报的能力
            RBucket<Integer> progressBucket = redissonClient.getBucket(
                UPDATE_JOB_PROCESS_KEY + jobId + request.getTaskId());
            progressBucket.delete();
            RBucket<Integer> statusBucket = redissonClient.getBucket(
                UPDATE_JOB_STATUS_KEY + jobId + request.getTaskId());
            statusBucket.delete();
            throw e;
        }
    }

    /**
     * 是否需要更新任务，条件：进度不小于上次上报的进度，上次上报的状态不是结束状态
     *
     * @param jobId jobId
     * @param request request
     * @return boolean
     */
    private boolean isAlreadyUpdate(String jobId, UpdateJobStatusRequest request) {
        Integer progress = request.getProgress();
        Integer status = request.getStatus();
        if (progress != null && status != null) {
            RBucket<Integer> progressBucket = redissonClient.getBucket(
                UPDATE_JOB_PROCESS_KEY + jobId + request.getTaskId());
            RBucket<Integer> statusBucket = redissonClient.getBucket(
                UPDATE_JOB_STATUS_KEY + jobId + request.getTaskId());
            Integer redisValue = progressBucket.get();
            if (isAlreadyUpdateProcess(progress, statusBucket, redisValue)) {
                log.info("isAlreadyUpdate jobId:{}, statusBucket: {}, redisValue: {}", jobId, statusBucket.get(),
                    redisValue);
                return true;
            }
            while (!progressBucket.compareAndSet(redisValue, progress)) {
                redisValue = progressBucket.get();
                log.info("Job progressRedis is already updated. jobId: {}, redis process: {}, process: {}", jobId,
                    redisValue, progress);
                if (isAlreadyUpdateProcess(progress, statusBucket, redisValue)) {
                    return true;
                }
            }
            statusBucket.set(request.getStatus(), JOB_PROCESS_EXPIRE_TIME, TimeUnit.SECONDS);
            progressBucket.expire(Duration.ofSeconds(JOB_PROCESS_EXPIRE_TIME));
        }
        return false;
    }

    private boolean isAlreadyUpdateProcess(Integer progress, RBucket<Integer> statusBucket,
        Integer redisValue) {
        if (redisValue == null) {
            return false;
        }
        if (progress.compareTo(redisValue) < LegoNumberConstant.ZERO) {
            // 进度小于等于当前值
            return true;
        }
        // 与上次上报进度相同，上次是结束状态，则忽略该消息
        if (progress.compareTo(redisValue) == LegoNumberConstant.ZERO && statusBucket.get() != null
            && DmeJobStatusEnum.FINISHED_STATUS_LIST.contains(DmeJobStatusEnum.fromStatus(statusBucket.get()))) {
            return true;
        }
        return false;
    }

    private void handleUpdateStatus(String jobId, UpdateJobStatusRequest jobStatusRequest) {
        if (DmeJobStatusEnum.FINISHED_STATUS_LIST.contains(DmeJobStatusEnum.fromStatus(jobStatusRequest.getStatus()))) {
            return;
        }
        UpdateJobRequest jobRequest = JobDataConverter.convertStatusRequest2UpdateJobRequest(jobStatusRequest);
        jobService.updateJob(jobId, jobRequest);
    }

    private void handleCompletedTask(JobBo job, UpdateJobStatusRequest jobStatusRequest) {
        if (!DmeJobStatusEnum.FINISHED_STATUS_LIST.contains(
                DmeJobStatusEnum.fromStatus(jobStatusRequest.getStatus()))) {
            return;
        }
        log.info("pm begin to deal job: id={}, taskId={}, progress={}",
                job.getJobId(), jobStatusRequest.getTaskId(), jobStatusRequest.getProgress());
        updateProgressWhenDmeDealSuccess(job.getJobId(), jobStatusRequest);

        TaskCompleteMessageDto taskCompleteMessageDto = JobDataConverter.convertJobStatusRequestToTaskCompleteDto(
            job, jobStatusRequest);
        taskCompleteListener.taskComplete(taskCompleteMessageDto);
    }

    private void updateProgressWhenDmeDealSuccess(String jobId, UpdateJobStatusRequest jobStatusRequest) {
        if (DmeJobStatusEnum.FAILED_STATUS_LIST.contains(DmeJobStatusEnum.fromStatus(jobStatusRequest.getStatus()))) {
            log.info("job {} failed, do not update pm progress.", jobStatusRequest.getTaskId());
            return;
        }
        UpdateJobRequest updateJobRequest = JobUpdateUtil.getReportReq();
        updateJobRequest.setSpeed(JobSpeedConverter.convertJobSpeed(String.valueOf(jobStatusRequest.getSpeed())));
        // 其他组件任务成功完成，准备开始pm框架后置业务。此时更新任务进度值，表明任务达到pm框架处理阶段。
        jobService.updateJob(jobId, updateJobRequest);
    }
}
