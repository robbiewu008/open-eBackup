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
package com.huawei.oceanprotect.system.base.health;

import static org.mockito.ArgumentMatchers.anyString;
import openbackup.system.base.util.SystemUtil;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisTimeoutException;
import org.springframework.boot.SpringApplication;
import org.springframework.context.ApplicationContext;

import java.lang.reflect.Field;

/**
 * The RedissonHealthCheckerTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({SpringApplication.class, SystemUtil.class})
public class RedissonHealthCheckerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ApplicationContext applicationContext;

    @InjectMocks
    private RedissonHealthChecker redissonHealthChecker;

    /**
     * 用例名称：redisson客户端状态检查成功
     * 前置条件：redisson与redis连接成功，并且无异常
     * check点：计数器计数为0
     */
    @Test
    public void check_redisson_status_success() throws Exception {
        RBucket mockBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(mockBucket);
        PowerMockito.doNothing().when(mockBucket).set(anyString());

        redissonHealthChecker.check();

        Field countField = redissonHealthChecker.getClass().getDeclaredField("count");
        countField.setAccessible(true);
        int count = (int) countField.get(redissonHealthChecker);
        Assert.assertEquals(0, count);
    }

    /**
     * 用例名称：redisson客户端状态检查失败
     * 前置条件：redisson与redis连接成功，调用时抛出RedisTimeoutException
     * check点：计数器计数为1
     */
    @Test
    public void check_redisson_status_failed_when_redisson_connect_abnormal() throws Exception {
        PowerMockito.when(redissonClient.getBucket(anyString())).thenThrow(new RedisTimeoutException());

        redissonHealthChecker.check();

        Field countField = redissonHealthChecker.getClass().getDeclaredField("count");
        countField.setAccessible(true);
        int count = (int) countField.get(redissonHealthChecker);
        Assert.assertEquals(1, count);
    }

    /**
     * 用例名称：redisson客户端状态检查失败次数超过阈值，导致系统被迫停止
     * 前置条件：1、redisson与redis连接成功，调用时抛出RedisTimeoutException；2、失败次数超过了失败阈值
     * check点：jvm调用停止
     */
    @Test
    public void system_shout_down_when_redisson_status_abnormal_too_many() throws Exception {
        PowerMockito.when(redissonClient.getBucket(anyString())).thenThrow(new RedisTimeoutException());

        PowerMockito.mockStatic(SystemUtil.class);

        for (int i = 0; i < RedissonHealthChecker.FAILURE_THRESHOLD; i++) {
            redissonHealthChecker.check();
        }

        PowerMockito.verifyStatic(SystemUtil.class);
        SystemUtil.stopApplication(applicationContext);
    }

    /**
     * 用例名称：redisson客户端状态检查连续失败，突然redisson恢复，导致系统被迫停止
     * 前置条件：1、redisson与redis连接成功，前两次调用时抛出RedisTimeoutException，第三次调用成功
     * check点：jvm调用停止
     */
    @Test
    public void system_shout_down_when_redisson_status_abnormal_and_sudden_recovery() {
        PowerMockito.mockStatic(SystemUtil.class);

        RBucket mockBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString()))
            .thenThrow(new RedisTimeoutException())
            .thenThrow(new RedisTimeoutException())
            .thenReturn(mockBucket);
        PowerMockito.doNothing().when(mockBucket).set(anyString());

        for (int i = 0; i < 3; i++) {
            redissonHealthChecker.check();
        }

        PowerMockito.verifyStatic(SystemUtil.class);
        SystemUtil.stopApplication(applicationContext);
    }
}
