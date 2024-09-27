/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener;

import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;

/**
 * 任务完成监听器接口定义
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-08
 */
public interface ITaskCompleteListener {
    /**
     * 任务完成
     *
     * @param message 任务完成消息
     */
    void taskComplete(TaskCompleteMessageDto message);
}
