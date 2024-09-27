/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v1;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.RedissonService;
import openbackup.system.base.util.ProviderRegistry;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * Task Complete Listener
 *
 * @author l00272247
 * @since 2020-12-18
 */
@Component
@Slf4j
public class TaskCompleteListener {
    @Autowired
    private RedissonService redissonService;

    /**
     * 服务提供者注册
     */
    @Autowired
    private ProviderRegistry registry;
    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    /**
     * 接收任务完成消息并进行对应处理
     *
     * @param message kafka message
     * @param acknowledgment kafka ack
     */
    @ExterAttack
    @MessageListener(topics = {TopicConstants.TASK_COMPLETE_TOPIC, TopicConstants.JOB_COMPLETE_TOPIC},
        groupId = "taskCompleteGroup", contextField = "job_request_id",
        containerFactory = "retryFactory", loadStack = false, enforceStop = true)
    public void taskComplete(String message, Acknowledgment acknowledgment) {
        TaskCompleteMessageDto completeMessage = JSONObject.fromObject(message).toBean(TaskCompleteMessageDto.class);
        String requestId = completeMessage.getJobRequestId();
        String jobId = completeMessage.getJobId();
        jobCenterRestApi.enforceStop(requestId, true);
        log.info("request_id:{}, job_id:{}, progress:{}", requestId, jobId, completeMessage.getJobProgress());

        RMap<String, String> context = redissonService.getMap(requestId);
        String jobType = context.get("job_type");
        TaskCompleteHandler handler = registry.findProvider(TaskCompleteHandler.class, jobType, null);
        log.info("request_id:{}, task complete get handler", requestId);
        if (VerifyUtil.isEmpty(handler)) {
            log.info("request_id:{}, can't find {} provider", requestId, jobType);
            return;
        }

        // 其他组件任务成功完成，准备开始pm框架后置业务。此时更新任务进度值，表明任务达到pm框架处理阶段。
        updateProgressWhenOtherDealSuccess(completeMessage, requestId);

        log.info("request_id:{}, task complete execute", requestId);
        processComplete(completeMessage, context, handler);
        log.info("request_id:{}, task complete commit message", requestId);
    }

    private void updateProgressWhenOtherDealSuccess(TaskCompleteMessageDto completeMessage, String requestId) {
        if (DmeJobStatusEnum.FAILED_STATUS_LIST
                .contains(DmeJobStatusEnum.fromStatus(completeMessage.getJobStatus()))) {
            log.info("job {} failed, do not update pm progress.", completeMessage.getTaskId());
            return ;
        }
        jobCenterRestApi.updateJob(requestId, JobUpdateUtil.getReportReq());
    }

    private void processComplete(TaskCompleteMessageDto completeMessage, RMap<String, String> context,
        TaskCompleteHandler handler) {
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        BeanUtils.copyProperties(completeMessage, messageBo);
        int status = completeMessage.getJobStatus();
        context.put(ContextConstants.JOB_STATUS, DmeJobStatusEnum.fromStatus(status).name());
        // 防止出现异常，导致死循环
        log.info("request_id:{}, task complete, get status", completeMessage.getJobRequestId());
        int label = completeMessage.getJobStatus();
        DmcJobStatus jobStatus = DmcJobStatus.getByStatus(label);
        if (jobStatus == null) {
            log.error("processComplete jobStatus is null");
            return;
        }
        log.info("request_id:{}, task complete processing", completeMessage.getJobRequestId());
        if (!jobStatus.isSuccess()) {
            log.warn("request_id:{}, task failed", completeMessage.getJobRequestId());
            handler.onTaskCompleteFailed(messageBo);
        } else {
            log.info("request_id:{}, task success", completeMessage.getJobRequestId());
            handler.onTaskCompleteSuccess(messageBo);
        }
    }
}
