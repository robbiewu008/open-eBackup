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
package openbackup.system.base.common.utils;

import org.redisson.api.RBucket;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.Serializable;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.TimeUnit;
import java.util.function.Supplier;

/**
 * CacheUtils
 *
 */
@Component
public class CacheUtils {
    @Autowired
    private RedissonClient redissonClient;

    /**
     * 通用的缓存获取方法
     *
     * @param key 缓存键
     * @param loadDataFunction 数据加载函数（从数据源加载数据）
     * @param cacheMinutes 缓存有效期（分钟）
     * @param nullCacheMinutes 空值缓存有效期（分钟）
     * @return 缓存数据，如果不存在则返回null
     */
    public <T> Optional<T> getDataFromCache(String key, Supplier<T> loadDataFunction, int cacheMinutes,
        int nullCacheMinutes) {
        // 第一次尝试从缓存获取数据
        RBucket<T> bucket = redissonClient.getBucket(key);
        T value = bucket.get();
        if (value != null) {
            return isNullValue(value) ? Optional.empty() : Optional.of(value); // 如果缓存的是空值标记，返回null
        }

        // 缓存未命中，准备加锁
        RLock lock = redissonClient.getLock("LOCK:" + key);
        try {
            // 获取分布式锁，使用看门狗自动续期
            lock.lock();

            // 双重检查，再次从缓存获取
            value = bucket.get();
            if (value != null) {
                return isNullValue(value) ? Optional.empty() : Optional.of(value);
            }

            // 从数据源加载数据
            value = loadDataFunction.get();

            // 设置缓存并处理空值
            if (value != null && !isEmptyValue(value)) {
                bucket.set(value, cacheMinutes, TimeUnit.MINUTES); // 缓存有效期为指定分钟
            } else {
                // 防止缓存穿透，设置空值标记，有效期较短
                bucket.set(createNullValue(), nullCacheMinutes, TimeUnit.MINUTES);
            }

            return Optional.of(value);
        } finally {
            lock.unlock();
        }
    }

    /**
     * 判断是否是空值（例如空列表、空字符串等）
     *
     * @param value value
     * @return 如果对象为空或空集合/字符串，则返回true；否则返回false
     */
    private <T> boolean isNullValue(T value) {
        return value instanceof NullValue;
    }

    /**
     * 判断是否是空值（例如空列表、空字符串等）
     *
     * @param value value
     * @return 如果对象为空或空集合/字符串，则返回true；否则返回false
     */
    private <T> boolean isEmptyValue(T value) {
        if (value instanceof List) {
            return ((List<?>) value).isEmpty();
        }
        // 可以根据需要扩展其他类型的空值判断
        return value == null;
    }

    /**
     * 创建空值标记
     *
     * @return 空值
     */
    private <T> T createNullValue() {
        return (T) new NullValue();
    }

    /**
     * 空值标记类，用于区分缓存中的null和未缓存
     */
    private static class NullValue implements Serializable {
        private static final long serialVersionUID = -1L;
    }
}
