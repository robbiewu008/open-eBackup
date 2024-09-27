package openbackup.system.base.kafka.annotations;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Message Context
 *
 * @author l00272247
 * @since 2020-10-19
 */
@Target({ElementType.TYPE, ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface MessageContext {
    /**
     * STACK
     */
    String STACK = "stack";

    /**
     * PAYLOAD
     */
    String PAYLOAD = ":payload";

    /**
     * message contexts
     *
     * @return message contexts
     */
    String[] messages() default {};

    /**
     * chain
     *
     * @return chain
     */
    String chain() default "";

    /**
     * topic, available when chain config
     *
     * @return topic
     */
    String topic() default "";
}
