/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author z30009433
 * @since 2020-12-31
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
