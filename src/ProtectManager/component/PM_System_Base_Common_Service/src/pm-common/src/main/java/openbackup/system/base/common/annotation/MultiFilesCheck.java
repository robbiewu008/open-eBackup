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
