/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.kafka;

import openbackup.system.base.kafka.annotations.MessageContext;

import java.lang.annotation.Annotation;
import java.util.Arrays;

/**
 * Message Context Info
 *
 * @author l00272247
 * @since 2020-10-25
 */
public class MessageContextInfo implements MessageContext {
    private final String[] messages;

    private final String chain;

    private final String topic;

    /**
     * constructor
     *
     * @param messages messages
     * @param chain    chain
     * @param topic    topic
     */
    public MessageContextInfo(String[] messages, String chain, String topic) {
        this.messages = messages;
        this.chain = chain;
        this.topic = topic;
    }

    @Override
    public String[] messages() {
        return Arrays.copyOfRange(messages, 0, messages.length);
    }

    @Override
    public String chain() {
        return chain;
    }

    @Override
    public String topic() {
        return topic;
    }

    @Override
    public Class<? extends Annotation> annotationType() {
        return null;
    }
}
