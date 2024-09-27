/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
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
import org.springframework.http.MediaType;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.RequestEntity;
import org.springframework.http.HttpMethod;
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
@SpringBootTest(classes = {LogExportController.class, RestTemplate.class, SessionServiceImpl.class,
    SecurityPolicyServiceImpl.class, RequestUtil.class, DownloadUtil.class, TimeoutUtils.class})
public class LogExportControllerTest {
    private URI uri;

    private final String url = "https://w3.huawei.com";

    @MockBean
    private RedissonClient redissonClient;

    @Autowired
    private LogExportController logExportController;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

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
    }

    @Test
    public void exportLogsSuccess() {
        byte[] bytes = {(byte) 0xB8};
        MultiValueMap<String, String> headers = new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        Cookie[] cookies = getCookie();
        request.setCookies(cookies);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        ResponseEntity<byte[]> responseEntity = new ResponseEntity(bytes, headers, HttpStatus.OK);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(byte[].class)))
            .thenReturn(responseEntity);
        logExportController.exportLogs();
    }

    private Cookie[] getCookie() {
        return new Cookie[]{new Cookie("name", "sysadmin")};
    }
}
