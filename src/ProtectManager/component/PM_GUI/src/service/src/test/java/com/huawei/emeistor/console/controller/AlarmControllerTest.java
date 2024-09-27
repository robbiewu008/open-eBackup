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

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.*;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.*;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import javax.servlet.http.Cookie;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import static org.mockito.ArgumentMatchers.*;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {AlarmController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class,
        RestTemplate.class, RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class})
public class AlarmControllerTest {
    private URI uri;

    private static final  String SESSION_VALUSE ="userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private final String url = "https://v1/alarms/dump/files/test.txt";

    @MockBean
    private RedissonClient redissonClient;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @Autowired
    private AlarmController alarmController;

    @Autowired
    private MockHttpServletRequest request;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @Before
    public void setup() {
        Token token = new Token();
        token.setToken("sadfgasfasf");
        try {
            uri = new URL(NormalizerUtil.normalizeForString(url)).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        Cookie[] cookies = new Cookie[]{new Cookie(ConfigConstant.SESSION, SESSION_VALUSE)};
        request.setCookies(cookies);
    }

    @Test
    public void exportEventsSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        byte[] res = alarmController.exportEvents(requestEntity);
        Assert.assertNotNull(res);
    }

    @Test
    public void exportDumpFileSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        ResponseEntity<byte[]> responseEntity = getByteResponseEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        byte[] res = alarmController.exportDumpFile("test.txt", requestEntity);
        Assert.assertNotNull(res);
    }

    private ResponseEntity<byte[]> getByteResponseEntity() {
        byte[] bytes = {(byte)0xB8};
        MultiValueMap<String, String> headers =  new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        ResponseEntity<byte[]> responseEntity = new ResponseEntity(bytes, headers, HttpStatus.OK);
        return responseEntity;
    }
}
