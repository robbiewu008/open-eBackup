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
package openbackup.system.base.redis;

import org.redisson.api.RSet;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Set;

/**
 * redisSet实现类
 *
 */
@Service
public class RedisSetService {
    @Autowired
    private RedissonClient redissonClient;

    /**
     * 向 Redis Set 中添加元素
     *
     * @param key redisKey
     * @param values 要添加的元素
     */
    public void addToSet(String key, String... values) {
        // 获取 Redis 中的 Set
        RSet<String> set = redissonClient.getSet(key);
        // 添加元素
        set.addAll(Arrays.asList(values));
    }

    /**
     * 获取 Redis Set 中的所有元素
     *
     * @param key redisKey
     * @return Redis Set
     */
    public Set<String> getAllFromSet(String key) {
        // 获取 Redis 中的 Set
        RSet<String> set = redissonClient.getSet(key);
        // 返回 Set 中的所有元素
        return set.readAll();
    }

    /**
     * 从 Redis Set 中移除元素
     *
     * @param key redisKey
     * @param values 要移除的元素
     */
    public void removeFromSet(String key, String... values) {
        // 获取 Redis 中的 Set
        RSet<String> set = redissonClient.getSet(key);
        // 删除指定的元素
        set.removeAll(Arrays.asList(values));  // 使用 removeAll 方法批量移除
    }

    /**
     * 清空 Redis Set 中所有元素
     *
     * @param key redisKey
     */
    public void clearSet(String key) {
        // 获取 Redis 中的 Set
        RSet<String> set = redissonClient.getSet(key);
        // 清空 Set
        set.clear();
    }

    /**
     * 向 Redis Set 中合并元素
     *
     * @param key redisKey
     * @param inputSet 要添加的元素
     */

    public void mergeSets(String key, Set<String> inputSet) {
        // 获取 Redis 中的 Set
        RSet<String> redisSet = redissonClient.getSet(key);

        // 合并 Java Set 和 Redis Set
        redisSet.addAll(inputSet);  // 将 inputSet 中的元素添加到 redisSet
    }
}
