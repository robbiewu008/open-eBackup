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

import openbackup.data.access.client.sdk.api.base.RestClient;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.apache.kafka.clients.consumer.Consumer;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.listener.ConsumerAwareListenerErrorHandler;
import org.springframework.kafka.listener.ListenerExecutionFailedException;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author p00511147
 * @since 2020-10-10
 */
@Slf4j
@Component("consumerAwareErrorHandler")
public class KafkaErrorHandlerConfig implements ConsumerAwareListenerErrorHandler {
    private static final int TASK_FAIL_STATUS = 6;

    private static final int TASK_FAIL_PROGRESS = 100;

    @Autowired
    private KafkaTemplate kafkaTemplate;

    @Autowired
    private RedissonClient redissonClient;

    @RestClient
    private JobCenterRestApi jobCenter;

    /**
     * kafka处理异常
     *
     * @param message   message
     * @param exception exception
     * @param consumer  consumer
     * @return result
     */
    @ExterAttack
    @Override
    public Object handleError(Message<?> message, ListenerExecutionFailedException exception, Consumer<?, ?> consumer) {
        log.error("kafka message:{}", message);
        log.error("kafka Exception:", exception);

        TaskCompleteMessageBo taskComplete = new TaskCompleteMessageBo();
        JSONObject properties = JSONObject.fromObject(message.getPayload());
        String jobRequestId = Optional.ofNullable(properties.getString("job_request_id"))
            .orElse(properties.getString("request_id"));
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
        List<JobLogBo> jobLogBoList = new ArrayList<>();
        jobLogBoList.add(jobLogBo);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setJobLogs(jobLogBoList);
        jobCenter.updateJob(jobId, updateJobRequest);
        String mes = JSONObject.fromObject(taskComplete).toString();
        kafkaTemplate.send(TopicConstants.JOB_COMPLETE_TOPIC, mes);
        return null;
    }
}
