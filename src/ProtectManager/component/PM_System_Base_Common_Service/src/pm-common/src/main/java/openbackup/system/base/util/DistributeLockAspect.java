/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.TimeUnit;

/**
 * 分布式锁切面逻辑
 *
 * @author g00500588
 * @since 2021/4/16
 */
@Aspect
@Component
@Slf4j
public class DistributeLockAspect {
    @Autowired
    private RedissonClient redissonClient;

    /**
     * 方法执行前后加/解分布式锁
     *
     * @param joinPoint joinPoint
     * @param distributeLock distributeLock
     * @return execute result
     * @throws Throwable exception
     * */
    @Around(
        value = "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& @annotation(distributeLock)")
    public Object processDistributeTaskWithLock(ProceedingJoinPoint joinPoint,
        DistributeLock distributeLock) throws Throwable {
        String lockKey = distributeLock.lockKey();
        long tryLockTime = distributeLock.tryLockTime();
        long lockTime = distributeLock.lockTime();
        TimeUnit lockTimeUnit = distributeLock.lockTimeUnit();
        log.info("lockKey:{}, tryLockTime:{}, lockTime:{}, lockTimeUnit:{}", lockKey,
                tryLockTime, lockTime, lockTimeUnit);
        Object result = new Object();
        RLock lock = redissonClient.getLock(lockKey);
        if (!lock.tryLock(tryLockTime, lockTime, lockTimeUnit)) {
            log.error("failed to acquire distributed lock, lock key: {}", lockKey);
            long errorCode = distributeLock.errorCode();
            if (errorCode > 0L) {
                log.error("aop distributed lock throw error code : {}.", errorCode);
                throw new LegoCheckedException(errorCode);
            }
            return result;
        }
        try {
            log.info("acquired distributed lock success, lock key: {}", lockKey);
            Object[] args = joinPoint.getArgs();
            result = joinPoint.proceed(args);
        } finally {
            if (distributeLock.releaseLock()) {
                lock.unlock();
            }
            log.info("release distributed lock success, lock key: {}", lockKey);
        }
        return result;
    }
}
