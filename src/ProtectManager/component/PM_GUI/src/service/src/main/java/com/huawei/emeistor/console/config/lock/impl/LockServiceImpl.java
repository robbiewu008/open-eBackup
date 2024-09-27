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
package com.huawei.emeistor.console.config.lock.impl;

import com.huawei.emeistor.console.config.lock.Lock;
import com.huawei.emeistor.console.config.lock.LockService;
import com.huawei.emeistor.console.config.lock.SQLLockService;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.util.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2023-07-05
 */
@Component
@Slf4j
public class LockServiceImpl implements LockService {
    @Autowired
    private SQLLockService sqlLockService;

    @Override
    public Lock createSQLDistributeLock(String key) {
        boolean isObtained = sqlLockService.createLock(key);
        return new SQLDistributeLock(key, isObtained, sqlLockService);
    }

    /**
     * BaseLock
     *
     * @author y00413474
     * @version [BCManager 8.0.0]
     * @since 2020-06-01
     */
    private abstract static class BaseLock implements Lock {
        /**
         * 获取锁并执行run
         *
         * @param run run
         */
        @Override
        public void lockAndRun(Runnable run) {
            this.lock();
            try {
                run.run();
            } finally {
                this.unlock();
            }
        }

        /**
         * 获取锁并执行callable
         *
         * @param callable callable
         * @param <V> 返回类型
         * @return 返回值
         */
        @Override
        public <V> V lockAndCall(Callable<V> callable) {
            this.lock();
            try {
                return callable.call();
            } catch (Exception e) {
                throw LegoCheckedException.cast(e);
            } finally {
                this.unlock();
            }
        }

        /**
         * 获取锁并执行run
         *
         * @param time 超时时间
         * @param unit 超时时间类型
         * @param run run
         * @return 是否成功获取锁并运行
         */
        @Override
        public boolean tryLockAndRun(long time, TimeUnit unit, Runnable run) {
            if (this.tryLock(time, unit)) {
                try {
                    run.run();
                    return true;
                } finally {
                    this.unlock();
                }
            }
            return false;
        }

        /**
         * 获取锁并执行callable
         *
         * @param time 超时时间
         * @param unit 超时时间类型
         * @param callable callable
         * @param <V> 返回类型
         * @return 返回值
         */
        @Override
        public <V> V tryLockAndCall(long time, TimeUnit unit, Callable<V> callable) {
            if (this.tryLock(time, unit)) {
                try {
                    return callable.call();
                } catch (Exception e) {
                    throw LegoCheckedException.cast(e);
                } finally {
                    this.unlock();
                }
            } else {
                return null;
            }
        }
    }

    /**
     * SQL 实现的乐观锁
     *
     * @author w30042425
     * @since 2023-06-10
     */
    public static class SQLDistributeLock extends BaseLock implements Lock {
        private final String key;

        private boolean isObtained;

        private final SQLLockService sqlLockService;

        public SQLDistributeLock(String key, boolean isObtained, SQLLockService sqlLockService) {
            this.key = key;
            this.isObtained = isObtained;
            this.sqlLockService = sqlLockService;
        }

        /**
         * 乐观锁的方式实现
         */
        @Override
        public void lock() {
            // 提供重入的功能
            if (LockAssert.alreadyLockedBy(key)) {
                log.debug("Already locked '{}'", key);
                return;
            }
            if (isObtained) {
                LockAssert.startLock(key);
                return;
            }
            spinlock(-1, TimeUnit.MILLISECONDS);
        }

        /**
         * 自旋等待，5S获取一次
         *
         * @param time 等待时间
         * @param timeUnit 时间单位
         * @return 是否获取成功
         */
        private boolean spinlock(long time, TimeUnit timeUnit) {
            long startMill = System.currentTimeMillis();
            while (true) {
                // 自璇等待5s
                try {
                    TimeUnit.MILLISECONDS.sleep(5000);
                } catch (InterruptedException e) {
                    log.error("spinlock interrupt", ExceptionUtil.getErrorMessage(e));
                }
                this.isObtained = sqlLockService.createLock(key);
                if (this.isObtained) {
                    return true;
                }
                if (time > 0) {
                    long wait = timeUnit.toMillis(time);
                    long curMill = System.currentTimeMillis();
                    if (startMill + wait < curMill) {
                        return false;
                    }
                }
            }
        }

        /**
         * 尝试获取锁
         *
         * @param time 超时时间
         * @param unit 超时时间类型
         * @return 是否获取成功
         */
        @Override
        public boolean tryLock(long time, TimeUnit unit) {
            // 提供重入的功能
            if (LockAssert.alreadyLockedBy(key)) {
                log.debug("Already locked '{}'", key);
                return true;
            }
            if (isObtained) {
                LockAssert.startLock(key);
                return true;
            }
            if (time <= 0) {
                return false;
            }
            return spinlock(time, unit);
        }

        /**
         * 解锁
         */
        @Override
        public void unlock() {
            LockAssert.endLock();
            sqlLockService.unLock(key);
        }
    }
}
