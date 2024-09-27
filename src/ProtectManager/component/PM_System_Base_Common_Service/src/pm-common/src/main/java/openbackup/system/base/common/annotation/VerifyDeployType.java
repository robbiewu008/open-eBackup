package openbackup.system.base.common.annotation;

import openbackup.system.base.common.enums.DeployTypeEnum;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 校验部署方式注解
 *
 * @author c30035089
 * @since 2023-01-30
 */
@Target({ElementType.METHOD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface VerifyDeployType {
    DeployTypeEnum[] notSupportedDeployTypes() default {};
}
