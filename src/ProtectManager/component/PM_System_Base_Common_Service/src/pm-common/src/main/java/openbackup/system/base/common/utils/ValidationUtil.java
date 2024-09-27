/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.hibernate.validator.HibernateValidator;

import java.util.Set;

import javax.validation.ConstraintViolation;
import javax.validation.Validation;
import javax.validation.Validator;

/**
 * 实现任何地方校验有@size等注解的bean的工具类
 *
 * @author y30046482
 * @since 2023-10-19
 */
public class ValidationUtil {
    /**
     * 开启快速结束模式 failFast (true)
     */
    private static Validator failFastValidator = Validation.byProvider(HibernateValidator.class)
        .configure()
        .failFast(true)
        .buildValidatorFactory().getValidator();

    /**
     * 全部校验
     */
    private static Validator validator = Validation.buildDefaultValidatorFactory().getValidator();

    private ValidationUtil() {
    }

    /**
     * 注解验证参数(快速失败模式)
     *
     * @param obj 待检查bean
     * @param msg 错误消息
     */
    public static synchronized <T> void fastFailValidate(T obj, String msg) {
        Set<ConstraintViolation<T>> constraintViolations = failFastValidator.validate(obj);
        if (constraintViolations.size() > 0) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, msg);
        }
    }

    /**
     * 注解验证参数(全部校验)
     *
     * @param obj 待检查bean
     * @param msg 错误消息
     */
    public static synchronized <T> void allCheckValidate(T obj, String msg) {
        Set<ConstraintViolation<T>> constraintViolations = validator.validate(obj);
        if (constraintViolations.size() > 0) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, msg);
        }
    }
}
