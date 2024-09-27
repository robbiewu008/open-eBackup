/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/14
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
