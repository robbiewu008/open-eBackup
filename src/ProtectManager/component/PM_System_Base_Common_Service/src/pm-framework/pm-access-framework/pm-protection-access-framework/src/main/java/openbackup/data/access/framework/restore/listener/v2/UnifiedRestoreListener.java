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
package openbackup.data.access.framework.restore.listener.v2;

import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * 统一框架恢复任务开始监听器
 *
 **/
@Slf4j
@Component
public class UnifiedRestoreListener {
    private final RestoreTaskManager taskManager;

    public UnifiedRestoreListener(RestoreTaskManager taskManager) {
        this.taskManager = taskManager;
    }

    /**
     * 开始恢复任务V2版本异步接口
     *
     * @param message 恢复任务信息
     * @param acknowledgment ack信息
     */
    @ExterAttack
    @KafkaListener(
            topics = TopicConstants.RESTORE_EXECUTE_V2,
            groupId = TopicConstants.RESTORE_GROUP_V2,
            containerFactory = "batchFactory")
    public void restoreStart(String message, Acknowledgment acknowledgment) {
        final RestoreTask restoreTask = JSONObject.toBean(message, RestoreTask.class);
        taskManager.start(restoreTask);
        acknowledgment.acknowledge();
    }
}
