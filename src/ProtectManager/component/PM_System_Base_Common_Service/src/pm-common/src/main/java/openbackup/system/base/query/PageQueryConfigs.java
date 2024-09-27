package openbackup.system.base.query;

import static java.lang.annotation.RetentionPolicy.RUNTIME;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

/**
 * Page Query Configs
 *
 * @author l00272247
 * @since 2020-10-10
 */
@Documented
@Target({ElementType.TYPE, ElementType.ANNOTATION_TYPE})
@Retention(RUNTIME)
public @interface PageQueryConfigs {
    /**
     * value
     *
     * @return value
     */
    PageQueryConfig[] value();
}
