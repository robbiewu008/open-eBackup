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
package openbackup.data.access.framework.protection.handler.v2.live.mount;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.springframework.stereotype.Component;

/**
 * 统一卸载任务完成处理器
 *
 */
@Slf4j
@Component
public class UnifiedUnmountTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final UnifiedLiveMountTaskCompleteHandler liveMountTaskCompleteHandler;

    public UnifiedUnmountTaskCompleteHandler(UnifiedLiveMountTaskCompleteHandler liveMountTaskCompleteHandler) {
        this.liveMountTaskCompleteHandler = liveMountTaskCompleteHandler;
    }

    /**
     * 处理卸载任务失败完成
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskMessage) {
        log.debug("unmount task complete message, do unmount success, request id: {}, status: {}",
            taskMessage.getJobRequestId(), taskMessage.getJobStatus());
        liveMountTaskCompleteHandler.sendLiveMountDoneMessage(taskMessage);
    }

    /**
     * 处理卸载任务失败完成
     *
     * @param taskMessage taskMessage
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskMessage) {
        log.error("unmount task complete message, do unmount failed. requestId is {}, jobId is {}, task status: {}.",
            taskMessage.getJobRequestId(), taskMessage.getProperty(ContextConstants.JOB_ID),
            taskMessage.getJobStatus());
        liveMountTaskCompleteHandler.sendLiveMountDoneMessage(taskMessage);
    }

    /**
     * 适配器
     *
     * @param subType subType
     * @return true or false
     */
    @Override
    public boolean applicable(String subType) {
        String jobType = JobTypeEnum.UNMOUNT.getValue() + JobStatusLabelConstant.HYPHEN_STRING_DELIMITER + version();
        return jobType.equals(subType);
    }
}
