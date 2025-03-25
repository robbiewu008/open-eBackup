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
package openbackup.system.base.config;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;

import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.service.ConfigMapServiceImpl;
import openbackup.system.base.util.ConfigMapUtil;
import openbackup.system.base.util.KeyToolUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.Redisson;
import org.redisson.api.RedissonClient;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.File;
import java.util.Arrays;
import java.util.List;

/**
 * RedissonClientConfigTest
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ConfigMapUtil.class, Redisson.class})
public class RedissonClientConfigTest {
    @InjectMocks
    private RedissonClientConfig redissonClientConfig;

    @Mock
    private KeyToolUtil keyToolUtil;

    @Mock
    private ConfigMapServiceImpl configMapService;

    @Before
    public void init() {
        List<String> clusterServerAddress = Arrays.asList("rediss://172.12.102.11:6369",
            "rediss://172.12.102.12:6369",
            "rediss://172.12.102.13:6369");
        ReflectionTestUtils.setField(redissonClientConfig, "clusterServerAddress",
            clusterServerAddress);
        ReflectionTestUtils.setField(redissonClientConfig, "serverAddress", "rediss://infrastructure:6369");
        ReflectionTestUtils.setField(redissonClientConfig, "keyStoreFile", "/test/keystore");
    }

    /**
     * 用例场景：获取redissionClient
     * 前置条件：系统正常运行
     * 检查点：成功
     */
    @Test
    public void test_redissonClient_success() throws Exception {
        PowerMockito.when(configMapService.getValueFromSecretByKey(any(), anyBoolean())).thenReturn("aaa");

        PowerMockito.mockStatic(ConfigMapUtil.class);
        PowerMockito.when(ConfigMapUtil.getValueInConfigMap(any(), any())).thenReturn("true");

        File file = PowerMockito.mock(File.class);
        PowerMockito.whenNew(File.class).withAnyArguments().thenReturn(file);

        PowerMockito.mockStatic(Redisson.class);

        RedissonClient redissonClient = PowerMockito.mock(RedissonClient.class);
        PowerMockito.when(Redisson.create(any())).thenReturn(redissonClient);

        RedissonClient res = redissonClientConfig.redissonClient();
        Assert.assertFalse(VerifyUtil.isEmpty(res));
    }
}