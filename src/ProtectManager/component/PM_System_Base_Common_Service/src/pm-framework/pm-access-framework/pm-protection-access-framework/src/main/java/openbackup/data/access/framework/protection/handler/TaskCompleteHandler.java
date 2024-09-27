/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.handler;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.util.Applicable;

/**
 * Task Complete Handler
 *
 * @author l00272247
 * @since 2020-12-18
 */
public interface TaskCompleteHandler extends Applicable<String> {
    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage);


    /**
     * 判断任务状态是否为成功
     *
     * @param taskCompleteMessage taskCompleteMessage
     */
    default void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
    }
}
