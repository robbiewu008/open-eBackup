/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.*;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.net.URI;

import static org.mockito.ArgumentMatchers.*;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {JobController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class,
        RestTemplate.class, RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class})
public class JobControllerTest {
    private URI uri;

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
    public JobController jobController;

    @Test
    public void exportAlarmSuccess() {
        byte[] bytes = {(byte)0xB8};
        RequestEntity<byte[]> requestEntity = new RequestEntity(bytes, HttpMethod.GET, uri);
        MultiValueMap<String, String> headers =  new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        ResponseEntity<byte[]> responseEntity = new ResponseEntity(bytes, headers, HttpStatus.OK);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class))).
                thenReturn(responseEntity);
        byte[] res = jobController.exportAlarm(requestEntity);
        Assert.assertNotNull(res);
    }
}
