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
package openbackup.data.access.framework.copy.verify.handler;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * 统一副本校验任务完成处理器
 *
 **/
@Slf4j
@Component
public class UnifiedCopyVerifyTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final CopyVerifyTaskManager taskManager;
    private final CopyVerifyService copyVerifyService;

    /**
     * 统一副本校验完成处理器构造函数
     *
     * @param taskManager 副本校验任务管理器
     * @param copyVerifyService 副本校验服务
     */
    public UnifiedCopyVerifyTaskCompleteHandler(CopyVerifyTaskManager taskManager,
        CopyVerifyService copyVerifyService) {
        this.taskManager = taskManager;
        this.copyVerifyService = copyVerifyService;
    }

    @Override
    public boolean applicable(String object) {
        return StringUtils.equals(JobTypeEnum.COPY_VERIFY.getValue() + "-" + version(), object);
    }

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Copy verify task successful");
        CopyVerifyTask verifyTask = copyVerifyService.getCopyCheckTaskFromJob(taskCompleteMessage.getJobRequestId());
        taskManager.complete(taskCompleteMessage, verifyTask);
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Copy verify task failed");
        CopyVerifyTask verifyTask = copyVerifyService.getCopyCheckTaskFromJob(taskCompleteMessage.getJobRequestId());
        taskManager.complete(taskCompleteMessage, verifyTask);
    }
}
