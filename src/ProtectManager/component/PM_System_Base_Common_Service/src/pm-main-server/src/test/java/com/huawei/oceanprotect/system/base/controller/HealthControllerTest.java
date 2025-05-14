/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import static org.mockito.ArgumentMatchers.anyString;

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

/**
 * 测试类
 *
 * @author w00504341
 * @since 2023-08-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    HealthController.class
})
public class HealthControllerTest {
    @InjectMocks
    private HealthController healthController;

    @Mock
    private RedissonClient redissonClient;

    /**
     * 用例场景：健康度检查
     * 前置条件：NA
     * 检查点：健康
     */
    @Test
    public void test_health_success() {
        RBucket<Object> bucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket("health_check_key")).thenReturn(bucket);
        PowerMockito.doNothing().when(bucket).set(anyString());
        Assert.assertNotNull(healthController.health());
    }
}
