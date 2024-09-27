/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import openbackup.system.base.common.constants.FaultEnum;

import openbackup.system.base.security.journal.Logging;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * operation logging
 *
 * @see Logging
 * @author l00272247
 * @since 2019-11-09
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Deprecated
public @interface OperationLogging {
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
     * operation detail
     *
     * @return operation detail
     */
    String[] detail() default {};

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
     * sync send option
     *
     * @return sync send option
     */
    boolean isSync() default true;
}
