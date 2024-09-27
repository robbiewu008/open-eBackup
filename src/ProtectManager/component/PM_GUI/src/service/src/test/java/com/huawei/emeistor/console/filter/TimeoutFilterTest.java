/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import com.sun.org.apache.xerces.internal.dom.AbortException;
import org.apache.hc.core5.http.HttpStatus;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockFilterChain;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import javax.servlet.ServletException;
import javax.servlet.http.Cookie;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import static org.mockito.ArgumentMatchers.any;

/**
 * 配置Filter 单元测试
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {TimeoutFilter.class, TimeoutUtils.class, SessionService.class, SHA256Encryptor.class})
public class TimeoutFilterTest {
    private URI uri;

    private static final  String SESSION_VALUSE = "userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private final String url = "https://w3.huawei.com";

    @Autowired
    private MockHttpServletRequest request;

    @Autowired
    private MockHttpServletResponse response;

    @MockBean
    private MockFilterChain filterChain;

    @MockBean
    private SessionService sessionService;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @MockBean
    private TimeoutUtils timeoutUtils;

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
        request.setServletPath("servletPath + ");
        request.setPathInfo("pathInfo");
    }

    /**
     * 用例场景：配置Filter成功
     * 前置条件：mock
     * 检查点：配置Filter成功
     */
    @Test
    public void should_do_internal_filter_successful() throws ServletException, IOException {
        // init
        TimeoutFilter filter = initFileter();
        // mock
        PowerMockito.when(sha256Encryptor.encryptionSessionId(any())).thenReturn("123");
        PowerMockito.when(sessionService.checkTimeout(any())).thenReturn(true);
        filter.doFilterInternal(request, response, filterChain);
        // then
        Assert.assertTrue(response.getStatus() == HttpStatus.SC_MOVED_TEMPORARILY);
    }

    /**
     * 用例场景：配置Filter异常
     * 前置条件：mock
     * 检查点：配置Filter异常
     */
    @Test
    public void should_do_internal_filter_throw_error() throws ServletException, IOException {
        // init
        TimeoutFilter filter = initFileter();
        // mock
        PowerMockito.when(sha256Encryptor.encryptionSessionId(any())).thenThrow(new LegoCheckedException("error"));
        PowerMockito.doNothing().when(sessionService).deleteSessionFromReq(any());
        filter.doFilterInternal(request, response, filterChain);
        // then
        Assert.assertTrue(response.getStatus() == HttpStatus.SC_SERVER_ERROR);
    }

    /**
     * 用例场景：配置Filter异常
     * 前置条件：mock
     * 检查点：配置Filter异常
     */
    @Test
    public void should_do_internal_filter_throw_exception_error() throws ServletException, IOException {
        // init
        TimeoutFilter filter = initFileter();
        // mock
        PowerMockito.when(sha256Encryptor.encryptionSessionId(any())).thenThrow(new AbortException());
        PowerMockito.doNothing().when(sessionService).deleteSessionFromReq(any());
        filter.doFilterInternal(request, response, filterChain);
        // then
        Assert.assertTrue(response.getStatus() == HttpStatus.SC_INTERNAL_SERVER_ERROR);
    }

    private TimeoutFilter initFileter() {
        TimeoutFilter filter = new TimeoutFilter();
        ReflectionTestUtils.setField(filter, "sessionService", sessionService);
        ReflectionTestUtils.setField(filter, "timeoutUtils", timeoutUtils);
        ReflectionTestUtils.setField(filter, "sha256Encryptor", sha256Encryptor);
        return filter;
    }
}