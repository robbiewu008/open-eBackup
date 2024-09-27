/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.verify.listener;

import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.restore.service.RestoreTaskHelper;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * 副本校验任务监听器
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/2
 **/
@Slf4j
@Component
public class CopyVerifyListener {
    private final CopyVerifyTaskManager copyVerifyTaskManager;
    private final JobService jobService;

    /**
     * 副本校验任务监听器构造器
     *
     * @param copyVerifyTaskManager 副本校验任务管理
     * @param jobService 任务
     */
    public CopyVerifyListener(CopyVerifyTaskManager copyVerifyTaskManager, JobService jobService) {
        this.copyVerifyTaskManager = copyVerifyTaskManager;
        this.jobService = jobService;
    }

    /**
     * 开始副本校验任务监听器
     *
     * @param message 恢复任务信息
     * @param acknowledgment ack信息
     */
    @KafkaListener(
        topics = TopicConstants.COPY_VERIFY_EXECUTE,
        groupId = TopicConstants.COPY_VERIFY_GROUP,
        containerFactory = "batchFactory")
    public void restoreStart(String message, Acknowledgment acknowledgment) {
        final RestoreTask restoreTask = JSONObject.toBean(message, RestoreTask.class);
        CopyVerifyTask copyVerifyTask = RestoreTaskHelper.covertToCopyCheckTask(restoreTask);
        if (!jobService.isJobPresent(copyVerifyTask.getRequestId())) {
            return;
        }
        copyVerifyTaskManager.execute(copyVerifyTask);
        acknowledgment.acknowledge();
    }
}
