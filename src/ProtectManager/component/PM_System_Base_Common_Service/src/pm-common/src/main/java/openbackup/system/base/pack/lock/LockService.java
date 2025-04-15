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
package openbackup.system.base.pack.lock;

import java.util.List;

/**
 * 功能描述
 *
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

    /**
     * 释放锁列表
     *
     * @param keyList 资源锁Id列表
     */
    void batchUnlockSqlLock(List<String> keyList);
}
