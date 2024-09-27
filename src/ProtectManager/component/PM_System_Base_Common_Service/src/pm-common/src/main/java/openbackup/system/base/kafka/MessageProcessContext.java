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
