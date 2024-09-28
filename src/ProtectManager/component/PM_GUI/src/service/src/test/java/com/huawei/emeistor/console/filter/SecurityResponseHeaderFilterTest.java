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

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.util.NormalizerUtil;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockFilterChain;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;

import javax.servlet.ServletException;
import javax.servlet.http.Cookie;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

/**
 * 回复添加统一的header 测试
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SecurityResponseHeaderFilter.class})
public class SecurityResponseHeaderFilterTest {
    private URI uri;

    private static final  String SESSION_VALUSE = "userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private final String url = "https://w3.huawei.com";

    @Autowired
    private MockHttpServletRequest request;

    @Autowired
    private MockHttpServletResponse response;

    @MockBean
    private MockFilterChain filterChain;

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
     * 用例场景：回复添加统一的header
     * 前置条件：mock
     * 检查点：回复添加统一的header
     */
    @Test
    public void should_do_filter_successful() throws ServletException, IOException {
        SecurityResponseHeaderFilter filter = new SecurityResponseHeaderFilter();
        filter.doFilterInternal(request, response, filterChain);

        Assert.assertEquals("no-cache, no-store, must-revalidate", response.getHeader("Cache-Control"));
        Assert.assertEquals("no-cache", response.getHeader("Pragma"));
        Assert.assertEquals("Thu, 01 Jan 1970 00:00:00 GMT", response.getHeader("Expires"));
        Assert.assertEquals("connect-src 'self';object-src 'self';frame-src 'self';media-src 'self';font-src 'self';frame-ancestors 'self';script-src 'self'",
                response.getHeader("Content-Security-Policy"));
        Assert.assertEquals("no-referrer", response.getHeader("Referrer-Policy"));
    }
}
