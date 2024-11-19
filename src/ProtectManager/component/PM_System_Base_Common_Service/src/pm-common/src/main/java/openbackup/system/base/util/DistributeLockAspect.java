/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoCheckedException;

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
     */
    @Around(value = "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& @annotation(distributeLock)")
    public Object processDistributeTaskWithLock(ProceedingJoinPoint joinPoint, DistributeLock distributeLock)
            throws Throwable {
        String lockKey = distributeLock.lockKey();
        long tryLockTime = distributeLock.tryLockTime();
        long lockTime = distributeLock.lockTime();
        TimeUnit lockTimeUnit = distributeLock.lockTimeUnit();
        log.info("lockKey:{}, tryLockTime:{}, lockTime:{}, lockTimeUnit:{}", lockKey, tryLockTime, lockTime,
                lockTimeUnit);
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
