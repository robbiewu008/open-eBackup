/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import openbackup.system.base.common.log.OperationContextLoader;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * current user
 *
 * @author l00272247
 * @since 2019-11-04
 */
@Target({ElementType.METHOD, ElementType.PARAMETER})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Repeatable(OperationContexts.class)
public @interface OperationContext {
    /**
     * value expression, supports:<br/>
     * <li>field: a.c.b</li>
     * <li>nullable: a?.c?.b</li>
     * <li>collection: a.0.b</li>
     * <li>each: a.*.b (support array/collection/map)</li>
     * <li>size: a.size() (support array/collection/map)</li>
     * <li>stringify: a.string()</li>
     * <li>jsonify: not support</li>
     *
     * @return data load value
     */
    String value();

    /**
     * value name in context
     *
     * @return name
     */
    String name() default "";

    /**
     * data type
     *
     * @return date type
     */
    Class<?> type() default OperationContext.class;

    /**
     * data loader
     *
     * @return data loader
     */
    Class<? extends OperationContextLoader> loader() default OperationContextLoader.class;
}

