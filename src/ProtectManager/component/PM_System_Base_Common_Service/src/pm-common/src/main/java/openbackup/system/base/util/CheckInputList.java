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
package openbackup.system.base.util;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import javax.validation.Constraint;
import javax.validation.Payload;

/**
 * 输入参数list注解类
 *
 */
@Target({ElementType.FIELD, ElementType.METHOD, ElementType.PARAMETER})
@Retention(RetentionPolicy.RUNTIME)
@Constraint(validatedBy = ListStringChecker.class)
@Documented
public @interface CheckInputList {
    /**
     * 错误信息
     *
     * @return 错误信息
     */
    String message() default "parameter check error.";

    /**
     * 集合最大长度
     *
     * @return 集合最大长度
     */
    int maxSize() default Integer.MAX_VALUE;

    /**
     * 集合最小长度
     *
     * @return 集合最小长度
     */
    int minSize() default 0;

    /**
     * 集合中每个字符串的最大长度
     *
     * @return 集合中每个字符串的最大长度
     */
    int strMaxLength() default 0;

    /**
     * 集合中每个字符串的最小长度
     *
     * @return 集合中每个字符串的最小长度
     */
    int strMinLength() default 0;

    /**
     * 集合中字符串的总长度
     *
     * @return 集合中字符串的总长度
     */
    int totalLength() default Integer.MAX_VALUE;

    /**
     * 是否进行特殊字符校验，默认为true
     *
     * @return 是否进行特殊字符校验，默认为true
     */
    boolean specialCharCheck() default true;

    /**
     * 特殊字符的key，不设置的话取common对应的特殊字符
     *
     * @return 特殊字符的key，不设置的话取common对应的特殊字符
     */
    int specialKey() default 0;

    /**
     * 特殊字符组成的字符串
     *
     * @return 特殊字符组成的字符串
     */
    String specialChars() default "";

    /**
     * 正则表达式，如果设置，集合中每个字符串都要符合正则表达式.
     *
     * @return 正则表达式，如果设置，集合中每个字符串都要符合正则表达式.
     */
    String strRegexp() default "";

    /**
     * groups
     *
     * @return groups
     */
    Class<?>[] groups() default {};

    /**
     * <? extends Payload>
     *
     * @return <? extends Payload>
     */
    Class<? extends Payload>[] payload() default {};
}

