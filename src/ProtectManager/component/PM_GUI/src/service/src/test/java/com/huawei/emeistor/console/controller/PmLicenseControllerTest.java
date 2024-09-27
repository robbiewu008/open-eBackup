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
package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

/**
 * 转发license导入接口测试类
 *
 * @author w00574036
 * @since 2023-03-13
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
        PmLicenseController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class, RestTemplate.class,
        RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class
})
public class PmLicenseControllerTest {
    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @Autowired
    private PmLicenseController pmLicenseController;

    @Test
    public void importLicenseTest() {
        MultipartFile basicLicenseFile = PowerMockito.mock(MultipartFile.class);
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(null, HttpStatus.OK);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
                .thenReturn(responseObjectEntity);
        pmLicenseController.importLicense(basicLicenseFile);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }
}