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
