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
package openbackup.data.access.framework.restore.handler.v2;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.protection.common.converters.JobDataConverter;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.access.framework.restore.service.RestoreTaskService;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;

/**
 * 统一恢复任务完成处理器
 *
 */
@Slf4j
@Component
public class UnifiedRestoreTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final RestoreTaskManager restoreTaskManager;
    private final RestoreTaskService restoreTaskService;
    private final CopyVerifyTaskManager copyVerifyTaskManager;

    /**
     * 统一恢复任务完成处理器
     *
     * @param restoreTaskManager 恢复任务管理器
     * @param restoreTaskService 恢复任务服务
     * @param copyVerifyTaskManager 副本校验任务管理器
     */
    public UnifiedRestoreTaskCompleteHandler(RestoreTaskManager restoreTaskManager,
            RestoreTaskService restoreTaskService, CopyVerifyTaskManager copyVerifyTaskManager) {
        this.restoreTaskManager = restoreTaskManager;
        this.restoreTaskService = restoreTaskService;
        this.copyVerifyTaskManager = copyVerifyTaskManager;
    }

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        String taskId = taskCompleteMessage.getTaskId();
        final RestoreTask restoreTask = restoreTaskService.getRestoreTaskFromJob(requestId);
        if (isMainTaskComplete(taskId, restoreTask)) {
            log.info("Restore main task success, requestId={}, taskId={}.", requestId, taskId);
            restoreTaskManager.complete(restoreTask,
                    JobDataConverter.convertToProviderJobStatus(taskCompleteMessage.getJobStatus()));
        } else {
            log.info("Restore sub verify task success, requestId={}, taskId={}.", requestId, taskId);
            copyVerifyTaskManager.complete(taskCompleteMessage, covertToVerifyTask(taskId, restoreTask));
        }
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        String taskId = taskCompleteMessage.getTaskId();
        final RestoreTask restoreTask = restoreTaskService.getRestoreTaskFromJob(requestId);
        if (isMainTaskComplete(taskId, restoreTask)) {
            log.info("Restore main task failed, requestId={}, taskId={}.", requestId, taskId);
            restoreTaskManager.complete(restoreTask,
                JobDataConverter.convertToProviderJobStatus(taskCompleteMessage.getJobStatus()));
        } else {
            log.info("Restore sub verify task failed, requestId={}, taskId={}.", requestId, taskId);
            copyVerifyTaskManager.complete(taskCompleteMessage, covertToVerifyTask(taskId, restoreTask));
        }
    }

    private static CopyVerifyTask covertToVerifyTask(String taskId, RestoreTask restoreTask) {
        CopyVerifyTask verifyTask = new CopyVerifyTask();
        BeanUtils.copyProperties(restoreTask, verifyTask);
        verifyTask.setTaskId(taskId);
        return verifyTask;
    }

    private static boolean isMainTaskComplete(String taskId, RestoreTask restoreTask) {
        return StringUtils.equals(taskId, restoreTask.getTaskId());
    }

    @Override
    public boolean applicable(String jobType) {
        return Arrays.asList(
                        JobTypeEnum.RESTORE.getValue() + "-" + version(),
                        JobTypeEnum.INSTANT_RESTORE.getValue() + "-" + version())
                .contains(jobType);
    }
}
