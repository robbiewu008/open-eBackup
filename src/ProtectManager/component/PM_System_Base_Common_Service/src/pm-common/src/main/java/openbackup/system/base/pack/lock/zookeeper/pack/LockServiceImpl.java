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
package openbackup.system.base.pack.lock.zookeeper.pack;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.CommonUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.pack.lock.SQLLockService;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.ZookeeperService;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.curator.framework.recipes.locks.InterProcessMutex;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

/**
 * 功能描述
 *
 */
@Component
@Slf4j
public class LockServiceImpl implements LockService {
    private final Map<String, MemoryLock> lockMap = new HashMap<>();

    // lock时，超过该时间没拿到锁，则打印日志，单位ms
    private final long waitLockTimeToLog = 60000L;

    @Autowired
    private ZookeeperService zookeeperService;

    @Autowired
    private SQLLockService sqlLockService;

    /**
     * 创建分布式锁
     *
     * @param key 锁的标识
     * @return 分布式锁对象
     */
    @ExterAttack
    @Override
    public Lock createDistributeLock(String key) {
        InterProcessMutex interProcessMutex = zookeeperService.createLock(key);

        return new DistributeLock(interProcessMutex);
    }

    /**
     * 创建内存锁，只针对当前jvm有效
     *
     * @param key 锁的标识
     * @return 内存锁对象
     */
    @Override
    public synchronized Lock createMemorylock(String key) {
        MemoryLock memoryLock = lockMap.get(key);
        if (memoryLock == null) {
            ReentrantLock reentrantLock = new ReentrantLock();
            memoryLock = new MemoryLock(reentrantLock);
            lockMap.put(key, memoryLock);
        }
        return memoryLock;
    }

    @Override
    public Lock createSQLDistributeLock(String key) {
        boolean isObtained = sqlLockService.createLock(key);
        return new SQLDistributeLock(key, isObtained, sqlLockService);
    }

    @Override
    public void batchUnlockSqlLock(List<String> keyList) {
        sqlLockService.batchUnlockSqlLock(keyList);
    }

    /**
     * BaseLock
     *
     */
    private abstract class BaseLock implements Lock {
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
     * DistributeLock
     *
     */
    public class DistributeLock extends BaseLock implements Lock {
        private final InterProcessMutex interProcessMutex;

        DistributeLock(InterProcessMutex interProcessMutex) {
            this.interProcessMutex = interProcessMutex;
        }

        /**
         * 同步获取锁，一直等待直到获取锁，如果不想无限等待，应该使用tryLock方法设置超时时间
         */
        @Override
        public void lock() {
            try {
                interProcessMutex.acquire();
            } catch (Exception e) {
                throw LegoCheckedException.cast(e);
            }
        }

        /**
         * 一定时间内尝试获取锁，返回是否成功获取锁，如果想立即返回time可以传0
         *
         * @param time 超时时间
         * @param unit 超时时间类型
         * @return boolean
         */
        @Override
        public boolean tryLock(long time, TimeUnit unit) {
            try {
                return interProcessMutex.acquire(time, unit);
            } catch (Exception e) {
                throw LegoCheckedException.cast(e);
            }
        }

        /**
         * 释放锁
         */
        @Override
        public void unlock() {
            try {
                interProcessMutex.release();
            } catch (Exception e) {
                throw LegoCheckedException.cast(e);
            }
        }
    }

    /**
     * MemoryLock
     *
     */
    private class MemoryLock extends BaseLock implements Lock {
        private final ReentrantLock reentrantLock;

        MemoryLock(ReentrantLock reentrantLock) {
            this.reentrantLock = reentrantLock;
        }

        /**
         * 同步获取锁，一直等待直到获取锁，如果不想无限等待，应该使用tryLock方法设置超时时间
         */
        @Override
        public void lock() {
            reentrantLock.lock();
        }

        /**
         * 一定时间内尝试获取锁，返回是否成功获取锁，如果想立即返回time可以传0
         *
         * @param time 超时时间
         * @param unit 超时时间类型
         * @return boolean
         */
        @Override
        public boolean tryLock(long time, TimeUnit unit) {
            try {
                return reentrantLock.tryLock(time, unit);
            } catch (InterruptedException e) {
                throw LegoCheckedException.cast(e);
            }
        }

        /**
         * 释放锁
         */
        @Override
        public void unlock() {
            reentrantLock.unlock();
        }
    }

    /**
     * SQL 实现的乐观锁
     *
     */
    public class SQLDistributeLock extends BaseLock implements Lock {
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
            long tempMill = startMill;
            while (true) {
                // 自璇等待5s
                CommonUtil.sleep(5, TimeUnit.SECONDS);
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
                } else {
                    // 如果1分钟没有加锁成功，打印日志
                    long currentTimeMillis = System.currentTimeMillis();
                    if (currentTimeMillis - tempMill > waitLockTimeToLog) {
                        log.warn("spin lock wait time: {}", currentTimeMillis - startMill);
                        tempMill += waitLockTimeToLog;
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
