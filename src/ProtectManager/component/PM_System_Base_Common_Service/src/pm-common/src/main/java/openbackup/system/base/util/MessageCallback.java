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

import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.service.SensitiveDataEliminateService;

import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.ProducerListener;
import org.springframework.stereotype.Component;

import java.util.Locale;

/**
 * 爱数保护计划消息回调类
 *
 * @author l00557046
 * @version [BCManager 8.0.0]
 * @since 2020-08-15
 */
@Component
public class MessageCallback<K, V> implements ProducerListener<K, V> {
    private static final Logger LOGGER = LoggerFactory.getLogger(MessageCallback.class);
    @Autowired
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    /**
     * success handler
     *
     * @param producerRecord producerRecord
     * @param recordMetadata recordMetadata
     */
    @Override
    public void onSuccess(ProducerRecord<K, V> producerRecord, RecordMetadata recordMetadata) {
        producerRecord.value();
        LOGGER.debug("Send message success : {}", stringify(producerRecord));
    }

    /**
     * error handler
     *
     * @param producerRecord producerRecord
     * @param recordMetadata recordMetadata
     * @param exception exception
     */
    @Override
    public void onError(ProducerRecord<K, V> producerRecord, RecordMetadata recordMetadata, Exception exception) {
        LOGGER.error("Send message error : {}", stringify(producerRecord), ExceptionUtil.getErrorMessage(exception));
    }

    private String stringify(ProducerRecord<K, V> producerRecord) {
        String topic = producerRecord.topic();
        Integer partition = producerRecord.partition();
        String headers = String.valueOf(producerRecord.headers());
        String key = String.valueOf(producerRecord.key());
        Object value = sensitiveDataEliminateService.eliminate(producerRecord.value());
        String timestamp = String.valueOf(producerRecord.timestamp());
        return String.format(Locale.ENGLISH,
            "ProducerRecord(topic=%s, partition=%s, headers=%s, indicator=%s, value=%s, timestamp=%s)", topic,
            partition, headers, key, value, timestamp);
    }
}
