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
package com.huawei.emeistor.console.health;

import com.huawei.emeistor.console.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RedissonClient;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * The RedissonHealthChecker
 *
 * @author g30003063
 * @since 2022/1/27
 */
@Slf4j
@Component
@EnableScheduling
public class RedissonHealthChecker {
    /**
     * 失败的最大次数
     */
    public static final int FAILURE_THRESHOLD = 10;

    private static final SimpleDateFormat SIMPLE_DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    // 启动后，200秒执行
    private static final long INITIAL_DELAY = 200 * 1000L;

    private static final long PERIOD = 25 * 1000L;

    private static final String KEY = "gui_redis_health_check_key";

    private final RedissonClient redissonClient;

    /**
     * 全参数构造
     *
     * @param redissonClient redissonClient
     */
    public RedissonHealthChecker(RedissonClient redissonClient) {
        this.redissonClient = redissonClient;
    }

    /**
     * 定时检查redisson使用正常
     */
    @ExterAttack
    @Scheduled(initialDelay = INITIAL_DELAY, fixedRate = PERIOD)
    public void check() {
        try {
            redissonClient.getBucket(KEY).set(SIMPLE_DATE_FORMAT.format(new Date()));
        } catch (Exception e) {
            log.error("Fail to check redisson status.", e);
        }
    }
}
