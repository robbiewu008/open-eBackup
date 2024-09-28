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

import openbackup.system.base.common.cluster.BackupClusterConfigUtil;
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
 * 切面
 *
 */
@Aspect
@Component
@Slf4j
public class SQLDistributeLockAspect {
    @Autowired
    private LockService lockService;

    /**
     * sql 分布式锁注解 切面
     *
     * @param joinPoint joinPoint
     * @param sqlDistributeLock 锁信息
     * @return 返回值
     * @throws Throwable 异常
     */
    @Around(
        value = "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& @annotation(sqlDistributeLock)")
    public Object execTaskWithSQLLock(ProceedingJoinPoint joinPoint, SQLDistributeLock sqlDistributeLock)
        throws Throwable {
        Object result = new Object();
        // 增加功能，如果当前任务必须是由主节点执行
        if (sqlDistributeLock.masterOnly() && !BackupClusterConfigUtil.isMasterCluster()) {
            log.info("current cluster is not primary cluster node");
            return result;
        }
        String lockName = sqlDistributeLock.lockName();
        long tryLockTime = sqlDistributeLock.tryLockTime();
        TimeUnit timeUnit = sqlDistributeLock.timeUnit();
        long errorCode = sqlDistributeLock.errorCode();
        log.info("start sql distributed lock name : {}, upgrade issue.", lockName);
        Lock lock = lockService.createSQLDistributeLock(lockName);
        boolean canAcquireLock = false;
        try {
            canAcquireLock = lock.tryLock(tryLockTime, timeUnit);
            if (!canAcquireLock) {
                log.info("sql distributed lock : {} is occupied by others.", lockName);
                if (errorCode > 0L) {
                    throw new LegoCheckedException(errorCode);
                }
                return result;
            }
            log.info("acquire sql distributed lock : {} success.", lockName);
            result = joinPoint.proceed(joinPoint.getArgs());
            log.info("exec task end, lock : {}.", lockName);
        } finally {
            if (canAcquireLock) {
                lock.unlock();
            }
        }
        return result;
    }
}
