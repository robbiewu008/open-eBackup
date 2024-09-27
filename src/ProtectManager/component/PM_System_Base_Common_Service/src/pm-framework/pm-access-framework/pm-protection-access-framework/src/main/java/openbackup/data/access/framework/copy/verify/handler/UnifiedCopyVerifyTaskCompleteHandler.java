/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.verify.handler;

import openbackup.data.access.framework.copy.verify.service.CopyVerifyService;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * 统一副本校验任务完成处理器
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/3
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
