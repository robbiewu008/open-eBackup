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

import openbackup.system.base.kafka.annotations.TopicMessage;

import java.lang.annotation.Annotation;
import java.util.Arrays;

/**
 * Topic Message Info
 *
 */
public class TopicMessageInfo implements TopicMessage {
    private final String topic;

    private final String[] messages;

    private final MessagePhase[] phases;

    /**
     * constructor
     *
     * @param topic    topic
     * @param messages messages
     * @param phases   phases
     */
    public TopicMessageInfo(String topic, String[] messages, MessagePhase[] phases) {
        this.topic = topic;
        this.messages = messages;
        this.phases = phases;
    }

    @Override
    public String topic() {
        return topic;
    }

    @Override
    public String[] messages() {
        return Arrays.copyOfRange(messages, 0, messages.length);
    }

    @Override
    public MessagePhase[] phases() {
        return Arrays.copyOfRange(phases, 0, phases.length);
    }

    @Override
    public Class<? extends Annotation> annotationType() {
        return null;
    }
}
