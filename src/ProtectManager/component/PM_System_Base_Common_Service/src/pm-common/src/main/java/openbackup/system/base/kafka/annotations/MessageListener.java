package openbackup.system.base.kafka.annotations;

import org.springframework.core.annotation.AliasFor;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.annotation.TopicPartition;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Kafka Listener
 *
 * @author l00272247
 * @since 2020-10-19
 */
@Target({ElementType.TYPE, ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@KafkaListener
@Documented
public @interface MessageListener {
    /**
     * TOPIC MESSAGE
     */
    String TOPIC_MESSAGE = "topic,message";

    /**
     * STEP_LEVEL_STATUS_LOG
     */
    String STEP_STATUS = "job_status_{status}_label";

    /**
     * job status log
     */
    String TASK_STATUS = "job_status_{payload.job_status|status}_label";

    /**
     * RETRY FACTORY
     */
    String RETRY_FACTORY = "retryFactory";

    /**
     * error messages.
     *
     * @return error messages.
     */
    TopicMessage[] messages() default {};

    /**
     * message contexts
     *
     * @return message contexts
     */
    MessageContext[] messageContexts() default {};

    /**
     * stack
     *
     * @return stack
     */
    String[] stack() default {};

    /**
     * load data from strict
     *
     * @return flag
     */
    boolean loadStack() default true;

    /**
     * data
     *
     * @return data
     */
    String[] data() default {};

    /**
     * context field
     *
     * @return context field
     */
    String contextField() default "request_id";

    /**
     * step info
     *
     * @return step info
     */
    String[] log() default {};

    /**
     * lock
     *
     * @return lock
     */
    String[] lock() default {};

    /**
     * unlock
     *
     * @return unlock
     */
    boolean unlock() default false;

    /**
     * sensitive
     *
     * @return sensitive
     */
    String[] sensitive() default {
        "%pass%", "%pwd%", "%key%", "%crypto%", "%session%", "%token%", "%fingerprint%", "%auth%", "%enc%", "%dec%",
        "%tgt%", "%iqn%", "%initiator%", "%secret%", "%cert%", "%salt%", "%private%", "%verfiycode%", "%email%",
        "%phone%", "%rand%", "%safe%", "%user_info%", "%PKCS1%", "%base64%", "%AES128%", "%AES256%", "%RSA%", "%SHA1%",
        "%SHA256%", "%SHA384%", "%SHA512%", "%algorithm%", "%AccountNumber%", "%bank%", "%cvv%", "%checkno%", "%mima%",
        "%CardPinNumber%", "%IDNumber%", "ak", "iv", "mk"
    };

    /**
     * terminated message
     *
     * @return terminated message
     */
    boolean terminatedMessage() default false;

    /**
     * failures
     *
     * @return failures
     */
    String[] failures() default {};

    /**
     * retryable
     *
     * @return retryable
     */
    boolean retryable() default false;

    /**
     * enforce stop flag
     *
     * @return enforce stop flag
     */
    boolean enforceStop() default false;

    /**
     * the id.
     *
     * @return the id.
     */
    @AliasFor(annotation = KafkaListener.class) String id() default "";

    /**
     * the container factory bean name.
     *
     * @return the container factory bean name.
     */
    @AliasFor(annotation = KafkaListener.class) String containerFactory() default "";

    /**
     * the topic names or expressions (SpEL) to listen to.
     *
     * @return the topic names or expressions (SpEL) to listen to.
     */
    @AliasFor(annotation = KafkaListener.class) String[] topics() default {};

    /**
     * the topic pattern or expression (SpEL).
     *
     * @return the topic pattern or expression (SpEL).
     */
    @AliasFor(annotation = KafkaListener.class) String topicPattern() default "";

    /**
     * the topic names or expressions (SpEL) to listen to.
     *
     * @return the topic names or expressions (SpEL) to listen to.
     */
    @AliasFor(annotation = KafkaListener.class) TopicPartition[] topicPartitions() default {};

    /**
     * the bean name for the group.
     *
     * @return the bean name for the group.
     */
    @AliasFor(annotation = KafkaListener.class) String containerGroup() default "";

    /**
     * the error handler.
     *
     * @return the error handler.
     */
    @AliasFor(annotation = KafkaListener.class) String errorHandler() default "";

    /**
     * the group id.
     *
     * @return the group id.
     */
    @AliasFor(annotation = KafkaListener.class) String groupId() default "consumerGroup";

    /**
     * false to disable.
     *
     * @return false to disable.
     */
    @AliasFor(annotation = KafkaListener.class) boolean idIsGroup() default true;

    /**
     * the client id prefix.
     *
     * @return the client id prefix.
     */
    @AliasFor(annotation = KafkaListener.class) String clientIdPrefix() default "";

    /**
     * the pseudo bean name.
     *
     * @return the pseudo bean name.
     */
    @AliasFor(annotation = KafkaListener.class) String beanRef() default "__listener";

    /**
     * the concurrency.
     *
     * @return the concurrency.
     */
    @AliasFor(annotation = KafkaListener.class) String concurrency() default "";

    /**
     * true to auto start, false to not auto start.
     *
     * @return true to auto start, false to not auto start.
     */
    @AliasFor(annotation = KafkaListener.class) String autoStartup() default "false";

    /**
     * the properties.
     *
     * @return the properties.
     */
    @AliasFor(annotation = KafkaListener.class) String[] properties() default {};
}
