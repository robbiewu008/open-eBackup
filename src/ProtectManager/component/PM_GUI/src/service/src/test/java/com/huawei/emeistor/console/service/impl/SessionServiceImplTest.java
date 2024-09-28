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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.TimeoutUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RBucket;
import org.redisson.api.RKeys;
import org.redisson.api.RedissonClient;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpMethod;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Stream;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.when;

/**
 * SessionServiceImpl Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({SessionServiceImpl.class})
public class SessionServiceImplTest {
    @Mock
    private HttpServletResponse response;

    @Mock
    private HttpServletRequest httpServletRequest;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private TimeoutUtils timeoutUtils;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @InjectMocks
    private SessionServiceImpl sessionServiceImpl;

    @Test
    public void check_delete_cookies_from_req() {
        MockHttpServletRequest request = new MockHttpServletRequest(
                HttpMethod.GET.name(), "/rest/v1/auth/action/login");
        MockHttpServletResponse mockResp = new MockHttpServletResponse();
        String userLockCacheVal = "USER_LOCK_CACHEc386a6ab7a3344029eac31b512d504e8";
        Cookie cookie1 = new Cookie(ConfigConstant.SESSION, userLockCacheVal);
        Cookie[] cookies = new Cookie[]{cookie1};
        request.setCookies(cookies);
        // mock timeoutUtils.getWhiteList()的结果
        when(timeoutUtils.getWhiteList()).thenReturn(
                Arrays.asList("/rest/v1/auth/action/login", "/login", "/rest/v1/captcha"));
        response = mockResp;
        sessionServiceImpl.deleteSessionFromReq(request);
        verify(timeoutUtils).getWhiteList();
    }

    @Test
    public void not_should_delete_if_cookie_name_equal_locale_when_check_timeout() {
        MockHttpServletRequest request = new MockHttpServletRequest(
                HttpMethod.GET.name(), "/rest/v1/auth/action/login");
        MockHttpServletResponse mockResp = new MockHttpServletResponse();
        Cookie localeCookie = new Cookie("locale", "zh-cn");
        Cookie[] cookies = new Cookie[]{localeCookie};
        request.setCookies(cookies);
        when(timeoutUtils.getWhiteList()).thenReturn(
                Arrays.asList("/rest/v1/auth/action/login", "/login", "/rest/v1/captcha"));
        response = mockResp;
        sessionServiceImpl.deleteSessionFromReq(request);
        Assert.assertEquals(1, request.getCookies().length);
    }

    @Test
    public void should_return_false_if_request_ip_equals_session_client_ip_when_check_timeout() {
        MockHttpServletRequest request = new MockHttpServletRequest(
                HttpMethod.GET.name(), "/rest/v1/system/initConfig");
        String sessionId =
                "userId=88a94c476f12a21e016f12a246e50009-loginTime=1627457002912850e4b4ebd2231ae0266ac2831686" +
                        "c729ce6e98d28fd428888e9d85f7900b0fd";
        Cookie cookie1 = new Cookie(ConfigConstant.SESSION, sessionId);
        Cookie[] cookies = new Cookie[]{cookie1};
        request.setCookies(cookies);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        Mockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        Mockito.when(rBucket.isExists()).thenReturn(true);

        SessionInfo sessionInfo = getSessionInfo(sessionId);
        Mockito.when(rBucket.get()).thenReturn(sessionInfo);
        Mockito.when(httpServletRequest.getRemoteAddr()).thenReturn("60.148.231.224");
        boolean checkTimeout = sessionServiceImpl.checkTimeout(sessionId);
        Assert.assertTrue(checkTimeout);
    }

    @Test
    public void getAllOnlineSession() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        RKeys rKey = PowerMockito.mock(RKeys.class);
        Stream<String> keys = Arrays.asList("userId=123-456","userId=456-789").stream();
        PowerMockito.when(redissonClient.getKeys()).thenReturn(rKey);
        PowerMockito.when(rKey.getKeysStreamByPattern(any())).thenReturn(keys);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        List<String> onlineSessionList =  sessionServiceImpl.getOnlineSessionIdList();
        Assert.assertNotNull(onlineSessionList);

    }

    private SessionInfo getSessionInfo(String sessionId) {
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setClientSessionIp("60.148.231.223");
        sessionInfo.setSessionId(sessionId);
        return sessionInfo;
    }
}
