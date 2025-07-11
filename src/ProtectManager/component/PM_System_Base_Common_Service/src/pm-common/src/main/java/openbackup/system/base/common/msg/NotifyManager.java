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
package openbackup.system.base.common.msg;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.SendResult;
import org.springframework.stereotype.Component;

/**
 * all kafka message can use this method to send
 *
 */
@Slf4j
@Component
public class NotifyManager {
    private static final int RETRY_COUNT = 3;

    private static final long TWO_MINUTES = 2 * 60 * 1000L;

    @Autowired
    private KafkaTemplate<String, String> kafkaTemplate;

    /**
     * send with kafka
     *
     * @param topic topic
     * @param jsonString jsonString
     */
    public void send(String topic, String jsonString) {
        int count = 1;
        while (true) {
            try {
                SendResult<String, String> result = kafkaTemplate.send(topic, jsonString).get();
                log.debug("Send message success, result:{}", result.toString());
                return;
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
        log.error("Send message failed after retry, topic: {}", topic);
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Send message failed.");
    }
}
