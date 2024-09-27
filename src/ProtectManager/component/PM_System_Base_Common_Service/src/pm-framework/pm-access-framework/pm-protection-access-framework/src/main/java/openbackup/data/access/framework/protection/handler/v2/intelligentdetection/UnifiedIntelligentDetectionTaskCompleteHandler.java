/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.handler.v2.intelligentdetection;

import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * 统一智能侦测任务完成处理器
 *
 * @author: x30028756
 * @version: [CyberEngine 1.0.0]
 * @since: 2023年3月2日23:40:36
 **/
@Slf4j
@Component
public class UnifiedIntelligentDetectionTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final JobService jobService;

    /**
     * 统一智能侦测任务完成处理器构造函数
     *
     * @param jobService job服务
     */
    public UnifiedIntelligentDetectionTaskCompleteHandler(JobService jobService) {
        this.jobService = jobService;
    }

    @Override
    public boolean applicable(String object) {
        return StringUtils.equals(JobTypeEnum.ANTI_RANSOMWARE.getValue() + "-" + version(), object);
    }

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        log.info("Intelligent detectionTask taskId:{} successful.", requestId);
        UpdateJobRequest jobRequest = new UpdateJobRequest();
        jobRequest.setStatus(JobStatusEnum.SUCCESS);
        jobRequest.setProgress(LegoNumberConstant.HUNDRED);
        jobService.updateJob(requestId, jobRequest);
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        log.info("Intelligent detectionTask taskId:{} task failed.", requestId);
        UpdateJobRequest jobRequest = new UpdateJobRequest();
        jobRequest.setStatus(JobStatusEnum.FAIL);
        jobRequest.setProgress(LegoNumberConstant.HUNDRED);
        jobService.updateJob(requestId, jobRequest);
    }
}