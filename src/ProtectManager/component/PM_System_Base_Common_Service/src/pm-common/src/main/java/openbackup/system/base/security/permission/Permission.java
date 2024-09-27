/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.security.permission;

import openbackup.system.base.common.constants.AuthOperationEnum;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Access
 *
 * @author l00272247
 * @since 2021-12-11
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface Permission {
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

    /**
     * 资源类型
     */
    ResourceSetTypeEnum resourceSetType() default ResourceSetTypeEnum.RESOURCE;

    /**
     * 操作类型
     */
    OperationTypeEnum operation() default OperationTypeEnum.QUERY;

    /**
     * 操作所需权限名称
     */
    AuthOperationEnum[] authOperations() default {};

    /**
     * 资源uuid数组表达式
     */
    String target() default "";

    /**
     * check role permission
     *
     * @return if check role permission
     */
    boolean checkRolePermission() default false;

    /**
     * enable check auth
     *
     * @return enable result
     */
    boolean enableCheckAuth() default true;
}
