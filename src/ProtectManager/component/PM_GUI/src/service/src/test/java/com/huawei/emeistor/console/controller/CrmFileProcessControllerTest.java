/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpMethod;
import org.springframework.http.RequestEntity;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;
import java.util.HashMap;
import java.util.Map;

import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.verify;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {CrmFileProcessController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class,
        RestTemplate.class, RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class})
public class CrmFileProcessControllerTest {
    private URI uri;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @MockBean
    private RedissonClient redissonClient;

    @Autowired
    private CrmFileProcessController crmFileProcessController;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @Test
    public void importAgentSuccess() {
        Resource resource = PowerMockito.mock(Resource.class);
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        PowerMockito.when(file.getResource()).thenReturn(resource);
        crmFileProcessController.importAgent(file);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    @Test
    public void getHostAgentFile1Success() {
        Map<String, String> map = new HashMap<>();
        map.put("id", "123");
        RequestEntity<Map<String, String>> requestEntity = new RequestEntity(map, HttpMethod.GET, uri);
        crmFileProcessController.getHostAgentFile1(requestEntity);
        verify(restTemplate).execute(anyString(), any(), any(), any());
    }
}
