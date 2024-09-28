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

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;

import lombok.extern.slf4j.Slf4j;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.TimeUnit;

/**
 * zk 分布式锁 切面
 *
 */
@Aspect
@Component
@Slf4j
public class ZKDistributeLockAspect {
    @Autowired
    LockService lockService;

    /**
     * zk 分布式锁注解 切面
     *
     * @param joinPoint joinPoint
     * @param zkDistributeLock zkDistributeLock
     * @return Object Object
     * @throws Throwable Throwable
     */
    @Around(
        value = "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& @annotation(zkDistributeLock)")
    public Object execTaskWithZKLock(ProceedingJoinPoint joinPoint, ZKDistributeLock zkDistributeLock)
            throws Throwable {
        Object result = new Object();
        String lockName = zkDistributeLock.lockName();
        long tryLockTime = zkDistributeLock.tryLockTime();
        TimeUnit timeUnit = zkDistributeLock.timeUnit();
        long errorCode = zkDistributeLock.errorCode();
        boolean isNeedRelease = zkDistributeLock.needRelease();
        log.info("start acquire zk distributed lock name : {}, upgrade issue.", lockName);
        Lock zkLock = lockService.createDistributeLock(lockName);
        boolean canAcquireLock = false;
        try {
            canAcquireLock = zkLock.tryLock(tryLockTime, timeUnit);
            if (!canAcquireLock) {
                log.info("zk distributed lock : {} is occupied by others.", lockName);
                if (errorCode > 0L) {
                    throw new LegoCheckedException(errorCode);
                }
                return result;
            }
            log.info("acquire zk distributed lock : {} success.", lockName);
            Object[] args = joinPoint.getArgs();
            result = joinPoint.proceed(args);
            log.info("exec task end, lock : {}.", lockName);
        } finally {
            if (canAcquireLock && isNeedRelease) {
                zkLock.unlock();
            }
        }
        return result;
    }
}
