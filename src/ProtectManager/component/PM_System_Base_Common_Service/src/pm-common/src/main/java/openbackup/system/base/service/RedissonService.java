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
package openbackup.system.base.service;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.File;

/**
 * Redisson Service
 *
 * @author l00272247
 * @since 2021-02-27
 */
@Component
public class RedissonService {
    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private LockService lockService;

    /**
     * get map
     *
     * @param name name
     * @return map object
     */
    @ExterAttack
    public RMap<String, String> getMap(String name) {
        return redissonClient.getMap(name, StringCodec.INSTANCE);
    }

    /**
     * once
     *
     * @param flag task flag
     * @param name redis context name
     * @param runnable runnable
     */
    public void once(String flag, String name, Runnable runnable) {
        once(flag, name, runnable, null);
    }

    /**
     * once
     *
     * @param flag task flag
     * @param name name
     * @param runnable runnable
     * @param isRetryable retryable
     */
    public void once(String flag, String name, Runnable runnable, Boolean isRetryable) {
        RMap<String, String> map = getMap(name);
        once(flag, map, runnable, isRetryable);
    }

    /**
     * once
     *
     * @param flag task flag
     * @param map map
     * @param runnable runnable
     */
    public void once(String flag, RMap<String, String> map, Runnable runnable) {
        once(flag, map, runnable, null);
    }

    /**
     * once
     *
     * @param flag task flag
     * @param map map
     * @param runnable runnable
     * @param isRetryable retryable
     */
    public void once(String flag, RMap<String, String> map, Runnable runnable, Boolean isRetryable) {
        String path = flag.startsWith("/") ? flag : "/" + flag;
        String key = File.pathSeparator + "once" + path;
        lockService
                .createDistributeLock(key)
                .lockAndRun(
                        () -> {
                            if (map.containsKey(key)) {
                                return;
                            }
                            try {
                                map.put(key, Boolean.TRUE.toString());
                                runnable.run();
                                if (Boolean.TRUE.equals(isRetryable)) {
                                    map.remove(key);
                                }
                            } catch (RuntimeException e) {
                                if (Boolean.FALSE.equals(isRetryable)) {
                                    map.put(key, Boolean.FALSE.toString());
                                } else {
                                    map.remove(key);
                                }
                                throw new LegoCheckedException(e.getMessage(), e.getCause());
                            }
                        });
    }
}
