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
package openbackup.system.base.kafka;

import openbackup.system.base.common.exception.MessageRetryException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.util.MessageTemplate;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.Map;

/**
 * Retry Message Handler
 *
 * @author l00272247
 * @since 2021-04-22
 */
@Component
public class RetryMessageHandler implements MessageErrorHandler {
    @Autowired
    private MessageTemplate<String> messageTemplate;

    /**
     * retryable exceptions
     *
     * @return retryable exceptions
     */
    @Override
    public Map<Class<? extends Throwable>, Boolean> retryableExceptions() {
        return Collections.singletonMap(MessageRetryException.class, Boolean.TRUE);
    }

    /**
     * handle error message
     *
     * @param topic topic
     * @param message message
     * @param throwable throwable
     */
    @Override
    public void handle(String topic, String message, Throwable throwable) {
        JSONObject data = JSONObject.fromObject(message);
        data.put("message.retry.failed", true);
        messageTemplate.send(topic, data.toString());
    }

    /**
     * test throwable applicable
     *
     * @param throwable throwable
     * @return check result
     */
    @Override
    public boolean applicable(Throwable throwable) {
        return ExceptionUtil.lookFor(throwable, MessageRetryException.class) != null;
    }
}
