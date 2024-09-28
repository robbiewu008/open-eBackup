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
package openbackup.system.base.controller.validator;

import org.apache.commons.collections.CollectionUtils;
import org.hibernate.validator.HibernateValidator;
import org.springframework.validation.Errors;

import java.util.Set;

import javax.validation.ConstraintViolation;
import javax.validation.Validation;
import javax.validation.Validator;

/**
 * Controller中自定义参数的基类校验器
 *
 **/
public abstract class BaseParamValidator<T> implements org.springframework.validation.Validator {
    /**
     * 定义Hibernate Validator进行基础业务校验
     */
    private final Validator validator =
            Validation.byProvider(HibernateValidator.class)
                    .configure()
                    .failFast(Boolean.TRUE)
                    .buildValidatorFactory()
                    .getValidator();

    @Override
    public void validate(Object obj, Errors errors) {
        // 先执行基本的参数校验，使用HibernateValidator校验相关的注解类
        final Set<ConstraintViolation<Object>> validateResult = validator.validate(obj);
        if (!CollectionUtils.isEmpty(validateResult)) {
            for (ConstraintViolation<Object> objectConstraintViolation : validateResult) {
                errors.rejectValue(
                        objectConstraintViolation.getPropertyPath().toString(),
                        objectConstraintViolation.getMessageTemplate(),
                        new Object[] {objectConstraintViolation.getInvalidValue()},
                        objectConstraintViolation.getMessage());
            }
            return;
        }
        // 在执行自定义的业务逻辑校验，由各个业务子类自行实现
        this.customValidate((T) obj, errors);
    }

    /**
     * 自定义参数校验
     *
     * @param obj 泛型参数，需要校验的对象
     * @param errors 静态校验的错误信息
     */
    protected abstract void customValidate(T obj, Errors errors);
}
