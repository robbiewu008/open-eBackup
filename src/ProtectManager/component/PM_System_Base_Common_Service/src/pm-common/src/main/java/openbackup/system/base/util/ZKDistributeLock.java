/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.util.concurrent.TimeUnit;

/**
 * zk 分布式锁 注解
 *
 * @author s00574739
 * @since 2021/08/05
 */
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface ZKDistributeLock {
    String lockName();

    long tryLockTime();

    TimeUnit timeUnit() default TimeUnit.MINUTES;

    long errorCode() default -1L;

    boolean needRelease() default true;
}
