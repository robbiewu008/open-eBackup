/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

/**
 * Copy index msg listener, this lister will consume the copy index msg
 *
 * @author zwx1010134
 * @since 2021-05-27
 */
@Component
@Slf4j
public class CopyIndexDeleteListener {
    /**
     * Consume copy delete topic message
     *
     * @param consumerString copy index delete msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @Retryable(exclude = {LegoCheckedException.class}, maxAttempts = 5, backoff = @Backoff(delay = 120000))
    @KafkaListener(groupId = TopicConstants.KAFKA_CONSUMER_GROUP, topics = TopicConstants.DELETE_INDEX_RESPONSE,
        autoStartup = "false")
    public void copyIndexDeleteResponse(String consumerString, Acknowledgment acknowledgment) {
        if (StringUtils.isBlank(consumerString)) {
            log.info("IndexResponse msg is invalid");
            return;
        }

        JSONObject jsonObject = JSONObject.fromObject(consumerString);
        String status = jsonObject.getString(CopyIndexConstants.STATUS);
        log.info("Start consumer copy delete status: {} ", status);
        if (!CopyIndexConstants.SUCCESS.equals(status)) {
            log.error("Failed to delete copy index");
            acknowledgment.acknowledge();
            return;
        }
        log.info("Finish to delete copy index");
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
        acknowledgment.acknowledge();
        log.error("Consumer index delete message error.");
    }
}
