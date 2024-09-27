/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.config.lock;

/**
 * 锁服务
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-03
 */
public interface LockService {
    /**
     * 创建SQL型分布式锁。如果不释放锁，默认持有锁一个小时
     *
     * @param key 锁的标识
     * @return 数据库锁对象
     */
    Lock createSQLDistributeLock(String key);
}
