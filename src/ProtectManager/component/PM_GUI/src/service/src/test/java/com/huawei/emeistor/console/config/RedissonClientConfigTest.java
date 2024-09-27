/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.config.datasource.DataSourceConfigService;
import com.huawei.emeistor.console.config.datasource.DataSourceRes;
import com.huawei.emeistor.console.util.EncryptorRestClient;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.Redisson;
import org.redisson.api.RedissonClient;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.File;
import java.net.URI;

/**
 * {@link RedissonClientConfig} 测试类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {File.class, Redisson.class})
public class RedissonClientConfigTest {
    /**
     * 用例名称：生成RedissonClient成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createRedissonClient() throws Exception {
        RedissonClientConfig config = PowerMockito.spy(new RedissonClientConfig());
        EncryptorRestClient client = PowerMockito.mock(EncryptorRestClient.class);
        DataSourceConfigService dataSourceConfigService = PowerMockito.mock(DataSourceConfigService.class);
        PowerMockito.when(dataSourceConfigService.getRedisClusterInfo())
            .thenReturn(PowerMockito.mock(DataSourceRes.class));
        PowerMockito.when(client.getRedisAuthFromSecret()).thenReturn("123");
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext("123");
        PowerMockito.when(client.decrypt(any())).thenReturn(plaintextVo);
        ReflectionTestUtils.setField(config, "encryptorRestClient", client);
        ReflectionTestUtils.setField(config, "dataSourceConfigService", dataSourceConfigService);
        ReflectionTestUtils.setField(config, "keyStoreFile", "file.txt");
        ReflectionTestUtils.setField(config, "serverAddress", "redis://127.0.0.1:6369");
        ReflectionTestUtils.setField(config, "keyStorePwdFile", "internal_cert.txt");

        File file = PowerMockito.mock(File.class);
        PowerMockito.when(file.toURI()).thenReturn(URI.create("/test"));
        PowerMockito.whenNew(File.class).withArguments("file.txt").thenReturn(file);
        RedissonClient mock = PowerMockito.mock(RedissonClient.class);
        PowerMockito.mockStatic(Redisson.class);
        PowerMockito.when(Redisson.create(any())).thenReturn(mock);
        config.redissonClient();
    }

    /**
     * 用例场景：域名底座正常转为IP底座
     * 前置条件：域名底座
     * 检查点：转换成功
     */
    @Test
    public void testConvertServerAddress() {
        String serverAddress = "rediss://localhost:6369";
        RedissonClientConfig redissionClientConfig = new RedissonClientConfig();
        Object convertedServerAddress = ReflectionTestUtils.invokeMethod(redissionClientConfig, "convertServerAddress",
            serverAddress);
        Assert.assertEquals("rediss://127.0.0.1:6369", convertedServerAddress);
    }
}
