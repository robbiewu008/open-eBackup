package openbackup.system.base.security.callee;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Callee Method
 *
 * @author l00272247
 * @since 2021-12-14
 */
@Target({ElementType.METHOD, ElementType.TYPE, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Repeatable(CalleeMethods.class)
public @interface CalleeMethod {
    /**
     * name
     *
     * @return name
     */
    String name() default "";

    /**
     * args
     *
     * @return args
     */
    Class<?>[] args() default {CalleeMethod.class};
}
