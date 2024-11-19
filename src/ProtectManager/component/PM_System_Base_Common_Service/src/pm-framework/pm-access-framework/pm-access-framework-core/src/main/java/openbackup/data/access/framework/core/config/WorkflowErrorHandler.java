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
package openbackup.data.access.framework.core.config;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.base.RestClient;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.MessageRetryException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.MessageErrorHandler;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobLogBoUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Stream;

/**
 * Workflow Error Handler
 *
 */
@Slf4j
@Component
public class WorkflowErrorHandler implements MessageErrorHandler {
    private static final int TASK_FAIL_STATUS = 6;

    private static final int TASK_FAIL_PROGRESS = 100;

    @Autowired
    private KafkaTemplate kafkaTemplate;

    @RestClient
    private JobCenterRestApi jobCenter;

    @Autowired
    private JobService jobService;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * retryable exceptions
     *
     * @return retryable exceptions
     */
    @Override
    public Map<Class<? extends Throwable>, Boolean> retryableExceptions() {
        Map<Class<? extends Throwable>, Boolean> retryableExceptions = new HashMap<>(
            MessageErrorHandler.super.retryableExceptions());
        Stream.of(LegoCheckedException.class, DataMoverCheckedException.class)
            .forEach(throwable -> retryableExceptions.put(throwable, false));
        return retryableExceptions;
    }

    @Override
    public void handle(String topic, String message, Throwable throwable) {
        log.info("Close Workflow started, topic: {}, message: {}", topic, message);
        TaskCompleteMessageBo taskComplete = new TaskCompleteMessageBo();
        JSONObject properties = JSONObject.fromObject(message);
        String jobRequestId = Optional.ofNullable(properties.getString("job_request_id"))
            .orElse(properties.getString("request_id"));
        try {
            // 先尝试正常结束，正常情况下任务流程中kafka消息重试5次之后不成功，会调用到此方法
            this.completeWorkflow(throwable, taskComplete, jobRequestId);
        } catch (Exception ex) {
            // 无法正常结束的情况下，强制把任务结束，清理上下文和资源锁
            this.forceStop(jobRequestId);
            throw ex;
        }
    }

    private void forceStop(String jobRequestId) {
        log.warn("workflow handler, job ({}) normal complete failed.", jobRequestId);
        try {
            jobService.forceStopJob(jobRequestId, false);
        } catch (Exception ex) {
            // 最后托底处理，捕获所有异常
            log.error("workflow handler, job ({}) force stop failed, long time job monitor will be force forcibly"
                + " terminated when job is not updated within 30 minutes", jobRequestId, ex);
        }
    }

    @ExterAttack
    private void completeWorkflow(Throwable throwable, TaskCompleteMessageBo taskComplete, String jobRequestId) {
        RMap<String, String> rMap = redissonClient.getMap(jobRequestId, StringCodec.INSTANCE);
        taskComplete.setJobRequestId(jobRequestId);
        taskComplete.setJobStatus(TASK_FAIL_STATUS);
        taskComplete.setJobProgress(TASK_FAIL_PROGRESS);
        taskComplete.setJobId("");
        String jobId = rMap.get("job_id");
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setJobId(jobId);
        jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
        jobLogBo.setLogInfo("task_running_failed_label");
        jobLogBo.setLogDetail(String.valueOf(CommonErrorCode.SYSTEM_ERROR));
        JobLogBoUtil.initJobLogDetail(jobLogBo, throwable);
        List<JobLogBo> jobLogBoList = new ArrayList<>();
        jobLogBoList.add(jobLogBo);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setJobLogs(jobLogBoList);
        log.info("Close Workflow, update job [{}] failed log", jobId);
        jobCenter.updateJob(jobId, updateJobRequest);
        String mes = JSONObject.fromObject(taskComplete).toString();
        log.info("Close Workflow, send job complete message");
        kafkaTemplate.send(TopicConstants.JOB_COMPLETE_TOPIC, mes);
    }

    /**
     * test throwable applicable
     *
     * @param throwable throwable
     * @return check result
     */
    @Override
    public boolean applicable(Throwable throwable) {
        return ExceptionUtil.lookFor(throwable, MessageRetryException.class) == null;
    }
}
