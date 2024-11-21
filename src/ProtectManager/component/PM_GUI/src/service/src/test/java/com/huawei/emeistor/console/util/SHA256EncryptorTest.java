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
package com.huawei.emeistor.console.util;

import static org.mockito.ArgumentMatchers.anyString;

import org.apache.commons.lang3.StringUtils;
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
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {SHA256Encryptor.class})
public class SHA256EncryptorTest {
    @Mock
    private RedissonClient redissonClient;

    @InjectMocks
    private SHA256Encryptor sha256Encryptor;

    @Test
    public void test_get_salt_success() {
        String salt = sha256Encryptor.getSalt();
        Assert.assertNotNull(salt);
    }

    @Test
    public void test_init_session_salt_success() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        PowerMockito.when(rBucket.isExists()).thenReturn(false);
        sha256Encryptor.initSessionSalt();
    }

    @Test
    public void test_init_session_salt_fail() {
        sha256Encryptor.initSessionSalt();
    }

    @Test
    public void test_encryption_session_id_success() {
        String encryptionSessionId = sha256Encryptor.encryptionSessionId("");
        Assert.assertTrue(StringUtils.isEmpty(encryptionSessionId));
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        PowerMockito.when(rBucket.isExists()).thenReturn(false);
        String encryptionSession = sha256Encryptor.encryptionSessionId(
            "userId=88a94c476f12a21e016f12a246e50009-loginTime=1694679031185957b6c2e4f114dce6b9d374f909d70dced4524bcb9b2a1594af4b0a931efb7a0");
        Assert.assertTrue(StringUtils.isNotBlank(encryptionSession));
        PowerMockito.when(rBucket.isExists()).thenReturn(true);
        encryptionSession = sha256Encryptor.encryptionSessionId(
            "userId=88a94c476f12a21e016f12a246e50009-loginTime=1694679031185957b6c2e4f114dce6b9d374f909d70dced4524bcb9b2a1594af4b0a931efb7a0");
        Assert.assertTrue(StringUtils.isNotBlank(encryptionSession));
    }
}