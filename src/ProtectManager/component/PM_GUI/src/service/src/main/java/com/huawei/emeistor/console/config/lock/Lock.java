/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.config.lock;

import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

/**
 * 锁接口
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-03
 */
public interface Lock {
    /**
     * 同步获取锁，一直等待直到获取锁，如果不想无限等待，应该使用tryLock方法设置超时时间
     */
    void lock();

    /**
     * 获取锁并执行run
     *
     * @param run run
     */
    void lockAndRun(Runnable run);

    /**
     * 获取锁并执行callable
     *
     * @param callable callable
     * @param <V>      返回类型
     * @return 返回值
     */
    <V> V lockAndCall(Callable<V> callable);

    /**
     * 一定时间内尝试获取锁，返回是否成功获取锁，如果想立即返回time可以传0
     *
     * @param time 超时时间
     * @param unit 超时时间类型
     * @return boolean
     */
    boolean tryLock(long time, TimeUnit unit);

    /**
     * 获取锁并执行run
     *
     * @param time 超时时间
     * @param unit 超时时间类型
     * @param run  run
     * @return 是否成功获取锁并运行
     */
    boolean tryLockAndRun(long time, TimeUnit unit, Runnable run);

    /**
     * 获取锁并执行callable
     *
     * @param time     超时时间
     * @param unit     超时时间类型
     * @param callable callable
     * @param <V>      返回类型
     * @return 返回值
     */
    <V> V tryLockAndCall(long time, TimeUnit unit, Callable<V> callable);

    /**
     * 释放锁
     */
    void unlock();
}
