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

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.data.protection.access.provider.sdk.retry.JobRetry;
import openbackup.data.protection.access.provider.sdk.retry.JobRetryEnum;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * v2接口的任务恢复重试
 *
 */
@Component
public class RestoreV2JobRetry implements JobRetry {
    @Autowired
    private RestoreTaskManager restoreTaskManager;

    @Override
    public String retryJob(String taskGuiRequest) {
        CreateRestoreTaskRequest createRestoreTaskRequest = JsonUtil.read(taskGuiRequest,
                CreateRestoreTaskRequest.class);
        return restoreTaskManager.init(createRestoreTaskRequest);
    }

    @Override
    public boolean applicable(String object) {
        return Objects.equals(object, JobRetryEnum.JobRetryTaskTypeEnum.RESTORE_V2.getType());
    }
}
