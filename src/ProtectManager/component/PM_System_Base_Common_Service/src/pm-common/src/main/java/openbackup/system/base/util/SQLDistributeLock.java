package openbackup.system.base.util;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.util.concurrent.TimeUnit;

/**
 * 基于数据库的分布式锁
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-12
 */
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface SQLDistributeLock {
    String lockName();

    long tryLockTime() default 0;

    /**
     * 是否是主节点执行
     *
     * @return 是否是主节点执行
     */
    boolean masterOnly() default true;

    TimeUnit timeUnit() default TimeUnit.MINUTES;

    long errorCode() default -1L;
}
