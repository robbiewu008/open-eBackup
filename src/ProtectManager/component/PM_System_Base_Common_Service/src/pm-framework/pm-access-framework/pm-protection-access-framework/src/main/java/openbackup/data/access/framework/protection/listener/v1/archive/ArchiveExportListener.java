/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v1.archive;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportObject;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveImportProvider;
import openbackup.system.base.common.model.repository.RepositoryType;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * 归档副本导入消息监听器
 *
 * @author z30009433
 * @since 2021-01-08
 */
@Component
@Slf4j
public class ArchiveExportListener {
    @Autowired
    private ProviderManager providerManager;

    /**
     * Consume restore topic message
     *
     * @param msg archive import msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.ARCHIVE_COPIES_TO_S3_TOPIC, containerFactory = "retryFactory")
    public void archiveImport(String msg, Acknowledgment acknowledgment) {
        if (VerifyUtil.isEmpty(msg)) {
            log.info("topic = archive.copies.to.repository, msg is empty");
            return;
        }
        JSONObject msgJson = JSONObject.parseObject(msg);
        ArchiveImportProvider archiveProvider = providerManager.findProvider(
                ArchiveImportProvider.class, String.valueOf(RepositoryType.S3.getType()));
        archiveProvider.archiveUpdateSnap(createArchiveObject(msgJson));
    }

    private ArchiveImportObject createArchiveObject(JSONObject msgJson) {
        // 从message获取storageId，jobid
        String requestId = msgJson.get(ContextConstants.REQUEST_ID).toString();
        String storageId = msgJson.get("storage_Id").toString();
        String jobId = msgJson.get(ContextConstants.JOB_ID).toString();
        String archiveCopyId = msgJson.get(ContextConstants.ARCHIVE_COPY_ID).toString();
        String backupCopyId = msgJson.get(ContextConstants.COPY_ID).toString();
        int autoRetryTimes = Integer.parseInt(msgJson.get(ContextConstants.AUTO_RETRY_TIMES).toString());
        return ArchiveImportObject.builder()
                .requestId(requestId)
                .jobId(jobId)
                .storageId(storageId)
                .archiveCopyId(archiveCopyId)
                .backupCopyId(backupCopyId)
                .autoRetryTimes(autoRetryTimes)
                .build();
    }
}
