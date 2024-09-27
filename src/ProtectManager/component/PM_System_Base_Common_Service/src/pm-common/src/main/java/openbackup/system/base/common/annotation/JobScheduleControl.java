package openbackup.system.base.common.annotation;

import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Inherited;
import java.lang.annotation.Repeatable;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Job Schedule Control
 *
 * @author l00272247
 * @since 2021-03-17
 */
@Target({ElementType.METHOD, ElementType.TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Repeatable(JobScheduleControls.class)
public @interface JobScheduleControl {
    /**
     * job type
     *
     * @return job type
     */
    JobTypeEnum jobType();

    /**
     * scope
     *
     * @return scope
     */
    String scope() default "";

    /**
     * globalJobLimit
     *
     * @return globalJobLimit
     */
    String globalJobLimit() default "";

    /**
     * scopeJobLimit
     *
     * @return scopeJobLimit
     */
    int scopeJobLimit() default 0;

    /**
     * strict scope
     *
     * @return strict scope
     */
    boolean strictScope() default true;

    /**
     * majorPriority
     *
     * @return majorPriority
     */
    int majorPriority() default 0;

    /**
     * minorPriorities
     *
     * @return minorPriorities
     */
    String[] minorPriorities() default {};

    /**
     * resumeStatus
     *
     * @return resumeStatus
     */
    JobStatusEnum resumeStatus() default JobStatusEnum.READY;
}