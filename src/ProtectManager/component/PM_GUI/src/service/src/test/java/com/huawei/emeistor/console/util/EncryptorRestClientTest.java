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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.exception.LegoCheckedException;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.ResponseEntity;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    EncryptorRestClient.class, ExceptionUtil.class
})
public class EncryptorRestClientTest {
    @InjectMocks
    private EncryptorRestClient encryptorRestClient;

    @Mock
    private RestTemplate restTemplate;

    @Before
    public void set_up() {
        ReflectionTestUtils.setField(encryptorRestClient, "restTemplate", restTemplate);
    }

    @Test
    public void test_get_redis_auth_from_secret_success() {
        PowerMockito.when(restTemplate.getForObject(anyString(), eq(String.class))).thenReturn("test");
        String secret = encryptorRestClient.getRedisAuthFromSecret();
        Assert.assertNotNull(secret);
    }

    @Test
    public void test_get_redis_auth_from_secret_fail() {
        PowerMockito.doThrow(new RestClientException("Error"))
            .when(restTemplate)
            .getForObject(anyString(), eq(String.class));
        String secret = encryptorRestClient.getRedisAuthFromSecret();
        Assert.assertTrue(StringUtils.isEmpty(secret));
    }

    @Test
    public void test_encrypt_success() {
        CiphertextVo encrypt = encryptorRestClient.encrypt("");
        Assert.assertTrue(StringUtils.isEmpty(encrypt.getCiphertext()));
        encrypt.setCiphertext("test");
        ResponseEntity response = PowerMockito.mock(ResponseEntity.class);
        PowerMockito.when(response.getBody()).thenReturn(encrypt);
        PowerMockito.when(
                restTemplate.postForEntity(anyString(), any(), eq(CiphertextVo.class), any(PlaintextVo.class)))
            .thenReturn(response);
        CiphertextVo result = encryptorRestClient.encrypt("test");
        Assert.assertTrue(StringUtils.isNotBlank(result.getCiphertext()));
    }

    @Test
    public void test_encrypt_fail() {
        HttpClientErrorException errorException = PowerMockito.mock(HttpClientErrorException.class);
        PowerMockito.doThrow(errorException)
            .when(restTemplate)
            .postForEntity(anyString(), any(), eq(CiphertextVo.class), any(PlaintextVo.class));
        PowerMockito.mockStatic(ExceptionUtil.class);
        try {
            encryptorRestClient.encrypt("test");
        } catch (LegoCheckedException e) {
            Assert.assertNotNull(e.getMessage());
        }
    }

    @Test
    public void test_decrypt_success() {
        PlaintextVo decrypt = encryptorRestClient.decrypt("");
        Assert.assertTrue(StringUtils.isEmpty(decrypt.getPlaintext()));
        decrypt.setPlaintext("test");
        ResponseEntity response = PowerMockito.mock(ResponseEntity.class);
        PowerMockito.when(response.getBody()).thenReturn(decrypt);
        PowerMockito.when(
                restTemplate.postForEntity(anyString(), any(), eq(PlaintextVo.class), any(CiphertextVo.class)))
            .thenReturn(response);
        PlaintextVo result = encryptorRestClient.decrypt("test");
        Assert.assertTrue(StringUtils.isNotBlank(result.getPlaintext()));
    }

    @Test
    public void test_decrypt_fail() {
        HttpClientErrorException errorException = PowerMockito.mock(HttpClientErrorException.class);
        PowerMockito.doThrow(errorException)
            .when(restTemplate)
            .postForEntity(anyString(), any(), eq(PlaintextVo.class), any(CiphertextVo.class));
        PowerMockito.mockStatic(ExceptionUtil.class);
        try {
            encryptorRestClient.decrypt("test");
        } catch (LegoCheckedException e) {
            Assert.assertNotNull(e.getMessage());
        }
    }
}
