package openbackup.system.base.common.condition;

import openbackup.system.base.common.enums.DeployTypeEnum;

import org.springframework.context.annotation.Conditional;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 在SpringBean上标注说明该类支持的部署类型, 不标注表示全部支持
 *
 * @since 2023-01-03
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE, ElementType.METHOD})
@Documented
@Conditional(DeployTypeCondition.class)
public @interface ConditionalOnDeployType {
    /**
     * 支持的部署类型
     *
     * @return 部署类型
     */
    DeployTypeEnum[] value();
}
