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

import openbackup.system.base.kafka.annotations.MessageContext;

import java.lang.annotation.Annotation;
import java.util.Arrays;

/**
 * Message Context Info
 *
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
