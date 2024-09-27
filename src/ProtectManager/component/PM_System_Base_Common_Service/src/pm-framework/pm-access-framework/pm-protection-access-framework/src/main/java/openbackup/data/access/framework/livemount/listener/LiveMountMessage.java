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
package openbackup.data.access.framework.livemount.listener;

import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.system.base.kafka.MessagePhase;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.kafka.annotations.TopicMessage;

import org.springframework.core.annotation.AliasFor;
import org.springframework.kafka.annotation.TopicPartition;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Live Mount Message
 *
 * @author l00272247
 * @since 2020-10-23
 */
@Target({ElementType.TYPE, ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@MessageListener(
    messages = {@TopicMessage(topic = TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS, phases = MessagePhase.FAILURE)})
public @interface LiveMountMessage {
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
    String[] step() default {};

    /**
     * sensitive
     *
     * @return sensitive
     */
    String[] sensitive() default {};

    /**
     * terminated message
     *
     * @return terminated message
     */
    boolean terminatedMessage() default false;

    /**
     * the id.
     *
     * @return the id.
     */
    @AliasFor(annotation = MessageListener.class) String id() default "";

    /**
     * the container factory bean name.
     *
     * @return the container factory bean name.
     */
    @AliasFor(annotation = MessageListener.class) String containerFactory() default "";

    /**
     * the topic names or expressions (SpEL) to listen to.
     *
     * @return the topic names or expressions (SpEL) to listen to.
     */
    @AliasFor(annotation = MessageListener.class) String[] topics() default {};

    /**
     * the topic pattern or expression (SpEL).
     *
     * @return the topic pattern or expression (SpEL).
     */
    @AliasFor(annotation = MessageListener.class) String topicPattern() default "";

    /**
     * the topic names or expressions (SpEL) to listen to.
     *
     * @return the topic names or expressions (SpEL) to listen to.
     */
    @AliasFor(annotation = MessageListener.class) TopicPartition[] topicPartitions() default {};

    /**
     * the bean name for the group.
     *
     * @return the bean name for the group.
     */
    @AliasFor(annotation = MessageListener.class) String containerGroup() default "";

    /**
     * the error handler.
     *
     * @return the error handler.
     */
    @AliasFor(annotation = MessageListener.class) String errorHandler() default "";

    /**
     * the group id.
     *
     * @return the group id.
     */
    @AliasFor(annotation = MessageListener.class) String groupId() default "consumerGroup";

    /**
     * false to disable.
     *
     * @return false to disable.
     */
    @AliasFor(annotation = MessageListener.class) boolean idIsGroup() default true;

    /**
     * the client id prefix.
     *
     * @return the client id prefix.
     */
    @AliasFor(annotation = MessageListener.class) String clientIdPrefix() default "";

    /**
     * the pseudo bean name.
     *
     * @return the pseudo bean name.
     */
    @AliasFor(annotation = MessageListener.class) String beanRef() default "__listener";

    /**
     * the concurrency.
     *
     * @return the concurrency.
     */
    @AliasFor(annotation = MessageListener.class) String concurrency() default "";

    /**
     * true to auto start, false to not auto start.
     *
     * @return true to auto start, false to not auto start.
     */
    @AliasFor(annotation = MessageListener.class) String autoStartup() default "";

    /**
     * the properties.
     *
     * @return the properties.
     */
    @AliasFor(annotation = MessageListener.class) String[] properties() default {};
}
