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

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.kafka.annotations.TopicMessage;

import java.util.Arrays;

/**
 * Message Process Context
 *
 * @author l00272247
 * @since 2020-11-06
 */
public class MessageProcessContext {
    private final MessageListener messageListener;

    private final JSONObject params;

    private TopicMessage[] topicMessages;

    public MessageProcessContext(MessageListener messageListener, JSONObject params, TopicMessage... topicMessages) {
        this.messageListener = messageListener;
        this.params = params;
        this.topicMessages = topicMessages != null ? topicMessages : new TopicMessage[0];
    }

    public MessageListener getMessageListener() {
        return messageListener;
    }

    public JSONObject getParams() {
        return params;
    }

    /**
     * getter of topicMessages
     *
     * @return topicMessages
     */
    public TopicMessage[] getTopicMessages() {
        return Arrays.copyOf(topicMessages, topicMessages.length);
    }

    public void setTopicMessages(TopicMessage[] topicMessages) {
        this.topicMessages = topicMessages;
    }
}
