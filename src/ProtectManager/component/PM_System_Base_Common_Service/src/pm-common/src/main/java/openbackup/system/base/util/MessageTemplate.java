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
package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.SendResult;
import org.springframework.stereotype.Component;
import org.springframework.util.concurrent.ListenableFuture;

import java.util.UUID;

/**
 * Message Template
 *
 * @param <K> template K
 */
@Component
@Slf4j
public class MessageTemplate<K> {
    /**
     * REQUEST ID
     */
    public static final String REQUEST_ID = "request_id";
    private static final int RETRY_COUNT = 3;

    private static final long TWO_MINUTES = 2 * 60 * 1000L;

    @Autowired
    private KafkaTemplate<K, String> kafkaTemplate;

    @Autowired
    private MessageCallback<K, String> callback;

    /**
     * send method
     *
     * @param topic topic
     * @param data data
     * @return future
     */
    public ListenableFuture<SendResult<K, String>> send(String topic, JSONObject data) {
        if (!data.containsKey(REQUEST_ID)) {
            data.set(REQUEST_ID, UUID.randomUUID().toString());
        }
        return send(topic, data.toString(), 1);
    }

    /**
     * send method
     *
     * @param topic topic
     * @param data data
     * @return future
     */
    public ListenableFuture<SendResult<K, String>> send(String topic, String data) {
        return send(topic, data, 0);
    }

    private ListenableFuture<SendResult<K, String>> send(String topic, String data, int stack) {
        JSONObject json = JSONObject.fromObject(data);
        String requestId = json.getString(REQUEST_ID);
        String caller = ExceptionUtil.getCaller(stack + IsmNumberConstant.TWO);
        log.info("Start send message. topic: {}, requestId: {}, caller: {}", topic, requestId, caller);
        kafkaTemplate.setProducerListener(callback);
        int count = 1;
        while (true) {
            try {
                return kafkaTemplate.send(topic, data);
            } catch (Exception e) {
                log.error("Send message failed, failedCount: {}.", count, ExceptionUtil.getErrorMessage(e));
            }
            count++;
            if (count > RETRY_COUNT) {
                break;
            }
            try {
                Thread.sleep(TWO_MINUTES);
            } catch (InterruptedException e) {
                log.error("Thread sleep failed.", ExceptionUtil.getErrorMessage(e));
            }
        }
        log.error("Send message failed after retry, topic: {}, requestId: {}", topic, requestId);
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Send kafka message failed.");
    }
}
