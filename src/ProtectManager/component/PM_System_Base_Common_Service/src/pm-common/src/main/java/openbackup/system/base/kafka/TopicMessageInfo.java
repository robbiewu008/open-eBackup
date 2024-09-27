package openbackup.system.base.kafka;

import openbackup.system.base.kafka.annotations.TopicMessage;

import java.lang.annotation.Annotation;
import java.util.Arrays;

/**
 * Topic Message Info
 *
 * @author l00272247
 * @since 2020-11-02
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
