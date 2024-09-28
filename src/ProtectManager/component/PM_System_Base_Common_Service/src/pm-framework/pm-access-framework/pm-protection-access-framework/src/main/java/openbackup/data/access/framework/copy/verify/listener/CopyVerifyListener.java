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
