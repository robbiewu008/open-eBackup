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
package openbackup.data.access.framework.protection.handler.v1.archive;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * Archive Import Task Complete Listener
 *
 */
@Component
@Slf4j
public class ArchiveImportTaskCompleteHandler implements TaskCompleteHandler {
    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private RedissonClient redissonClient;

    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        RMap<String, String> context = redissonClient
                .getMap(taskCompleteMessage.getJobRequestId(), StringCodec.INSTANCE);
        String repositoryType = context.get("repositoryType");
        String storageId = context.get("storageId");

        ArchiveImportProvider provider = providerManager.findProvider(ArchiveImportProvider.class, repositoryType);
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        BeanUtils.copyProperties(taskCompleteMessage, messageBo);
        provider.archiveImportTaskComplete(messageBo, storageId);
    }

    @ExterAttack
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        RMap<String, String> context = redissonClient
                .getMap(taskCompleteMessage.getJobRequestId(), StringCodec.INSTANCE);
        String repositoryType = context.get("repositoryType");
        String jobId = context.get(ContextConstants.JOB_ID);
        taskCompleteMessage.setJobId(jobId);

        ArchiveImportProvider provider = providerManager.findProvider(ArchiveImportProvider.class, repositoryType);
        provider.archiveImportTaskFailed(taskCompleteMessage);
    }

    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.ARCHIVE_IMPORT.getValue().equals(object);
    }
}
