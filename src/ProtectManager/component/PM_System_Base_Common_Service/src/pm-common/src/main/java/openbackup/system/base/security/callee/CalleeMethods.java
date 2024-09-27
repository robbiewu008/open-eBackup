package openbackup.system.base.security.callee;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Callee Methods
 *
 * @author l00272247
 * @since 2021-12-14
 */
@Target({ElementType.TYPE, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface CalleeMethods {
    /**
     * name
     *
     * @return name
     */
    String name() default "";

    /**
     * methods
     *
     * @return methods
     */
    CalleeMethod[] value() default {};
}
