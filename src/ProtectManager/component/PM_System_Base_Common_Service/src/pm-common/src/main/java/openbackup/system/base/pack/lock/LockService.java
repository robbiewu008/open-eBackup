/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.pack.lock;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-01
 */
public interface LockService {
    /**
     * 创建分布式锁
     *
     * @param key 锁的标识
     * @return 分布式锁对象
     */
    Lock createDistributeLock(String key);

    /**
     * 创建内存锁，只针对当前jvm有效
     *
     * @param key 锁的标识
     * @return 内存锁对象
     */
    Lock createMemorylock(String key);

    /**
     * 创建SQL型分布式锁。如果不释放锁，默认持有锁一个小时
     *
     * @param key 锁的标识
     * @return 数据库锁对象
     */
    Lock createSQLDistributeLock(String key);
}
