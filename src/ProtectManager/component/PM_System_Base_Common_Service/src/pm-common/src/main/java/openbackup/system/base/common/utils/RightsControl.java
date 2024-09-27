/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.security.permission.Permission;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * operation permission
 *
 * @see Permission
 * @author p00264414
 * @since 2020-01-09
 */
@Target({ElementType.METHOD, ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Deprecated
public @interface RightsControl {
    /**
     * roles
     *
     * @return roles
     */
    String[] roles() default {};

    /**
     * resources
     *
     * @return resource
     */
    String[] resources() default {};
}
