package openbackup.system.base.security.callee;

import org.springframework.context.annotation.Import;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Callee Method Scan
 *
 * @author l00272247
 * @since 2021-12-14
 */
@Target({ElementType.TYPE, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Import(CalleeMethodRegister.class)
public @interface CalleeMethodScan {
    String[] value() default {};
}
