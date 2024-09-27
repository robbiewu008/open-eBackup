/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v2;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.listener.ITaskCompleteListener;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.RedissonService;
import openbackup.system.base.util.ProviderRegistry;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * 统一框架任务完成监听器
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-08
 */
@Component
@Slf4j
public class UnifiedTaskCompleteListener implements ITaskCompleteListener {
    @Autowired
    private RedissonService redissonService;

    /**
     * 服务提供者注册
     */
    @Autowired
    private ProviderRegistry registry;

    @ExterAttack
    @Override
    public void taskComplete(TaskCompleteMessageDto message) {
        String requestId = message.getJobRequestId();
        Integer jobStatus = message.getJobStatus();
        log.info("Received job complete msg, jobId: {}, job type:{}, request id:{}, jobStatus:{}", message.getJobId(),
            message.getJobType(), requestId, jobStatus);
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        BeanUtils.copyProperties(message, messageBo);
        messageBo.setExtendsInfo(message.getExtendsInfo());
        TaskCompleteHandler handler = registry.findProvider(TaskCompleteHandler.class, message.getJobType(), null);
        DmcJobStatus dmcJobStatus = DmcJobStatus.getByStatus(messageBo.getJobStatus());
        Map<String, String> context = redissonService.getMap(requestId);
        messageBo.setContext(context);
        context.put(ContextConstants.JOB_STATUS, DmeJobStatusEnum.fromStatus(jobStatus).name());
        log.debug("Dme job status name: {} and value: {}", dmcJobStatus.name(), dmcJobStatus.getStatus());
        if (dmcJobStatus.isSuccess()) {
            handler.onTaskCompleteSuccess(messageBo);
        } else {
            handler.onTaskCompleteFailed(messageBo);
        }
    }
}
