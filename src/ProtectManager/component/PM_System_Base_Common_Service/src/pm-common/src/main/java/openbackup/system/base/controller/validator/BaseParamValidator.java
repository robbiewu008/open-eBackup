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
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/3/5
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
