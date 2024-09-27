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
