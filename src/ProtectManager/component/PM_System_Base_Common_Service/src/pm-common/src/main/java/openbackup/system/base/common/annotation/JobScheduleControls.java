package openbackup.system.base.common.annotation;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * JobScheduleControls
 *
 * @author l00272247
 * @since 2021-03-20
 */
@Target({ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
public @interface JobScheduleControls {
    /**
     * configs
     *
     * @return configs
     */
    JobScheduleControl[] value();
}
