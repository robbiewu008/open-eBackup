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
package openbackup.data.access.framework.protection.handler.v2.archive;

import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.archive.ArchiveTaskManager;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 统一归档任务完成处理器
 *
 **/
@Slf4j
@Component
public class UnifiedArchiveTaskCompleteHandler extends UnifiedTaskCompleteHandler {
    private final ArchiveTaskManager archiveTaskManager;

    public UnifiedArchiveTaskCompleteHandler(ArchiveTaskManager archiveTaskManager) {
        this.archiveTaskManager = archiveTaskManager;
    }

    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Archive task success.");
        archiveTaskManager.archiveSuccess(taskCompleteMessage.getJobRequestId(),
            DmcJobStatus.getByStatus(taskCompleteMessage.getJobStatus()), taskCompleteMessage.getExtendsInfo());
    }

    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        log.debug("Archive task failed.");
        archiveTaskManager.archiveFailed(taskCompleteMessage.getJobRequestId(),
                DmcJobStatus.getByStatus(taskCompleteMessage.getJobStatus()));
    }

    @Override
    public boolean applicable(String object) {
        String jobType = JobTypeEnum.ARCHIVE.getValue() + "-" + version();
        return jobType.equals(object);
    }
}
