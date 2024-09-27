/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.query;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Page Query Config
 *
 * @author l00272247
 * @since 2020-09-24
 */
@Target({ElementType.TYPE, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Repeatable(PageQueryConfigs.class)
public @interface PageQueryConfig {
    /**
     * condition fields
     *
     * @return condition fields
     */
    String[] conditions() default {};

    /**
     * order fields
     *
     * @return order fields
     */
    String[] orders() default {};
}
