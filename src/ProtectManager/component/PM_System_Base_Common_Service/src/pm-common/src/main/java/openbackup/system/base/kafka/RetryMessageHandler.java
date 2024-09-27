/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
