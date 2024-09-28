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
package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.IOException;
import java.util.Arrays;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AuthFilter.class, RequestUtil.class})
public class AuthFilterTest {
    @InjectMocks
    private AuthFilter authFilter;

    @Mock
    private RequestUtil requestUtil;

    @Mock
    private HttpServletRequest request;

    @Mock
    private HttpServletResponse response;

    @Before
    public void set_up() {
        ReflectionTestUtils.setField(authFilter, "authWhiteListUri", Arrays.asList("/v1/white1", "/v1/white2"));
        ReflectionTestUtils.setField(authFilter, "authWhiteListStatic",
            Arrays.asList("/v1/white/static1", "/v1/white/static2"));
        ReflectionTestUtils.setField(authFilter, "authWhiteListStaticRegex",
            Arrays.asList("/console/[0-9]+.[0-9a-zA-Z]+.js", "/console/common.[0-9a-zA-Z]+.js"));
        ReflectionTestUtils.setField(authFilter, "requestUtil", requestUtil);
    }

    @Test
    public void test_filter_success() throws ServletException, IOException {
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("testToken");
        PowerMockito.when(requestUtil.getSessionInfo()).thenReturn(sessionInfo);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/not_contains");
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        authFilter.doFilterInternal(request, response, filterChain);
    }

    @Test
    public void test_filter_fail() throws ServletException, IOException {
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("");
        PowerMockito.when(requestUtil.getSessionInfo()).thenReturn(sessionInfo);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/black");
        ServletOutputStream outputStream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(response.getOutputStream()).thenReturn(outputStream);
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        authFilter.doFilterInternal(request, response, filterChain);
    }
}
