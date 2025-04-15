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
package openbackup.data.access.framework.protection.listener.v2;

import lombok.extern.slf4j.Slf4j;
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

import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * 统一框架任务完成监听器
 *
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
