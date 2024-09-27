/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * role type config
 *
 * @author l00272247
 * @since 2019-11-26
 */
@Target({ElementType.FIELD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface RoleTypeConfig {
    /**
     * 该类型的内建角色
     *
     * @return builtin roles
     */
    String builtinRoles() default "";

    /**
     * 该类型角色是否支持创建
     *
     * @return creatable
     */
    boolean creatable() default false;

    /**
     * 该类型角色是否支持修改
     *
     * @return modifiable
     */
    boolean modifiable() default false;

    /**
     * 该类型角色是否在界面可见
     *
     * @return visible
     */
    boolean visible() default true;

    /**
     * 该类型角色是否可用于创建用户
     *
     * @return result
     */
    boolean available() default false;
}
