/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.kafka.annotations;

import openbackup.system.base.kafka.MessagePhase;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Terminate Message
 *
 * @author l00272247
 * @since 2020-10-19
 */
@Target({ElementType.TYPE, ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface TopicMessage {
    /**
     * topic
     *
     * @return topic
     */
    String topic();

    /**
     * messages
     *
     * @return messages
     */
    String[] messages() default {};

    /**
     * phases. If this parameter is empty, it will be considered a success phase.
     *
     * @return phases. If this parameter is empty, it will be considered a success phase.
     */
    MessagePhase[] phases() default {};
}
