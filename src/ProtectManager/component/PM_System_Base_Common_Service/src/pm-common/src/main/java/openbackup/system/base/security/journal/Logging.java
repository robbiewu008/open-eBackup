/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.security.journal;

import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.security.context.Context;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Logging
 *
 * @author l00272247
 * @since 2021-12-11
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface Logging {
    /**
     * operation name
     *
     * @return operation name
     */
    String name() default "";

    /**
     * operation target
     *
     * @return operation target
     */
    String target() default "";

    /**
     * batch arguments
     *
     * @return batch arguments
     */
    String[] batch() default {};

    /**
     * operation detail
     *
     * @return operation detail
     */
    String[] details() default {};

    /**
     * operation rank
     *
     * @return operation rank
     */
    FaultEnum.AlarmSeverity rank() default FaultEnum.AlarmSeverity.INFO;

    /**
     * manual option
     *
     * @return manual option
     */
    boolean manual() default false;

    /**
     * context config
     *
     * @return context
     */
    Context[] context() default {};

    /**
     * required data
     *
     * @return required data
     */
    String[] requires() default {};

    DeployTypeEnum[] deployType() default {};

    /**
     * 用于判断是否需要执行记录事件
     *
     * @return 是否需要记录
     */
    boolean needCheckLogging() default false;
}
