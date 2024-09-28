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
