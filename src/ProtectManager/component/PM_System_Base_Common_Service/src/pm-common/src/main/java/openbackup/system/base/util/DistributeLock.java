package openbackup.system.base.util;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.util.concurrent.TimeUnit;

/**
 * @author g00500588
 * @since 2021/4/15
 */
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface DistributeLock {
    String lockKey();

    long tryLockTime();

    long lockTime();

    TimeUnit lockTimeUnit() default TimeUnit.SECONDS;

    boolean releaseLock() default true;

    long errorCode() default -1L;
}
