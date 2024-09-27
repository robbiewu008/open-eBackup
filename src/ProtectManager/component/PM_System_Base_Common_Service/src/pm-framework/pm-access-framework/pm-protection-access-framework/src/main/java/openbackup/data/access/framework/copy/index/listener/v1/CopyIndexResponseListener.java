/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

/**
 * Copy index response msg listener, this lister will consume the copy index msg
 *
 * @author zwx1010134
 * @since 2021-05-27
 */
@Component
@Slf4j
public class CopyIndexResponseListener extends GenCopyIndex {
    @Autowired
    private CopyRestApi copyRestApi;

    /**
     * Consume copy index topic message
     *
     * @param consumerString copy index msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @Retryable(exclude = {LegoCheckedException.class}, maxAttempts = 5, backoff = @Backoff(delay = 120000))
    @KafkaListener(groupId = TopicConstants.KAFKA_CONSUMER_GROUP, topics = TopicConstants.INDEX_RESPONSE,
        autoStartup = "false")
    public void copyIndexResponse(String consumerString, Acknowledgment acknowledgment) {
        if (StringUtils.isBlank(consumerString)) {
            log.info("IndexResponse msg is invalid");
            return;
        }

        JSONObject jsonObject = JSONObject.fromObject(consumerString);
        String copyId = jsonObject.getString(ContextConstants.COPY_ID);
        log.info("Start consumer index response copyId: {} ", copyId);
        if (StringUtils.isBlank(copyId)) {
            log.info("Copy id is invalid, do nothing");
            acknowledgment.acknowledge();
            return;
        }

        // 更新状态
        String status = jsonObject.getString(CopyIndexConstants.STATUS);
        String indexStatus = CopyIndexStatus.INDEXED.getIndexStaus();
        String errorCode = jsonObject.getString(ContextConstants.ERROR_CODE);
        if (!CopyIndexConstants.SUCCESS.equals(status)) {
            indexStatus = CopyIndexStatus.INDEX_FAIL.getIndexStaus();
            errorCode = StringUtils.isBlank(errorCode)
                    ? CopyIndexStatus.INDEX_RESPONSE_ERROR_LABEL.getIndexStaus()
                    : errorCode;
            log.info("index error code as {}", errorCode);
        }
        copyRestApi.updateCopyIndexStatus(copyId, indexStatus, errorCode);
        log.info("update copy status as {} of copy {}", indexStatus, copyId);
        acknowledgment.acknowledge();
    }

    /**
     * recover scan response topic message
     *
     * @param consumerString san response delete msg
     * @param acknowledgment Acknowledgment
     * @param exception              Exception
     */
    @Recover
    public void recover(Exception exception, String consumerString, Acknowledgment acknowledgment) {
        String errorCode = CopyIndexStatus.INDEX_RESPONSE_ERROR_LABEL.getIndexStaus();
        genIndexRecover(consumerString, acknowledgment, errorCode);
    }
}

