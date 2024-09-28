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
package openbackup.system.base.common.constants;

import lombok.AccessLevel;
import lombok.RequiredArgsConstructor;

/**
 * CacheConstant
 *
 */
@RequiredArgsConstructor(access = AccessLevel.PRIVATE)
public class CacheConstant {
    /**
     * 启动后，1分钟执行刷新
     */
    public static final long INITIAL_DELAY = 60 * 1000L;

    /**
     * 一分钟刷新一次
     */
    public static final long PERIOD = 60 * 1000L;

    /**
     * redis map key
     */
    public static final String CACHE_REDIS_KEY = "redisCache";

    /**
     * redis 更新zookeeper锁key
     */
    public static final String CACHE_REDIS_UPDATE = "/redisCacheUpdate";

    /**
     * protect engine缓存key值
     */
    public static final String PROTECT_ENGINE_POD_INFO_CACHE = "protectEnginePodInfoCache";
}
