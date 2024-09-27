/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.util.*;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import javax.servlet.http.Cookie;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {FlrFileExportController.class, RestTemplate.class, SessionServiceImpl.class,
    SecurityPolicyServiceImpl.class, RequestUtil.class, DownloadUtil.class, TimeoutUtils.class})
public class FlrFileExportControllerTest {
    private URI uri;

    private static final  String SESSION_VALUSE ="userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private final String url = "https://w3.huawei.com";

    @MockBean
    private RedissonClient redissonClient;

    @Autowired
    private FlrFileExportController fileExportController;

    @Autowired
    private MockHttpServletRequest request;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

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
        Cookie[] cookies = new Cookie[] {new Cookie(ConfigConstant.SESSION, SESSION_VALUSE)};
        request.setCookies(cookies);
    }

    @Test
    public void exportLogsSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        fileExportController.exportLogs();
    }
}
