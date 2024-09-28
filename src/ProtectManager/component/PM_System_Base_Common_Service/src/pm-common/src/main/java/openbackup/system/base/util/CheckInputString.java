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
 * 输入参数校验类注解类
 *
 */
@Target({ElementType.FIELD, ElementType.METHOD, ElementType.PARAMETER})
@Retention(RetentionPolicy.RUNTIME)
@Constraint(validatedBy = InputStringChecker.class)
@Documented
public @interface CheckInputString {
    /**
     * message
     *
     * @return 字段检查，消息体
     */
    String message() default "The string contains special character!";

    /**
     * 分组
     *
     * @return groups
     */
    Class<?>[] groups() default {};

    /**
     * payload
     *
     * @return payload
     */
    Class<? extends Payload>[] payload() default {};

    /**
     * 校验字符串是否包含黑名单中的特殊字符，用此字段
     * 此字段赋值使用TypeMode类的变量
     * 如果此字段赋值，则regexp正则就不再起作用
     *
     * @return 字段类型
     */
    int type() default 0;

    /**
     * 字符串最小长度，如果字符串为NULL，则不校验长度
     *
     * @return minLen
     */
    int minLen() default 0;

    /**
     * 字符串最大长度，如果字符串为NULL，则不校验长度
     *
     * @return maxLen
     */
    int maxLen() default Integer.MAX_VALUE;

    /**
     * 能否全部为空格，如果此值为FALSE，则字符串不能为NULL
     *
     * @return 是否为blank
     */
    boolean canBlank() default true;

    /**
     * 能否为NULL
     *
     * @return canNull
     */
    boolean canNull() default true;

    /**
     * 字符串校验正则表达式，如果不用正则校验，不用设置
     *
     * @return 正则
     */
    String regexp() default "";
}
