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
 * 文件上传防护注解，适配多个类型的文件校验
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-27
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface MultiFilesCheck {
    /**
     * 多个文件校验注解，适配多个特殊格式的文件
     * 注：多个FileCheck注解时，最多只有一个FileCheck注解不指定后缀名suffix字段，即最多只有一个默认的通用校验规则属性
     *
     * @return 多个文件校验注解，适配多个特殊格式的文件
     */
    FileCheck[] value();
}
