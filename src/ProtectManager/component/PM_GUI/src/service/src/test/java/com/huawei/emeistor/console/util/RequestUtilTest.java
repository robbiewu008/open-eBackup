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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.service.SessionService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpHeaders;
import org.springframework.mock.web.MockHttpServletRequest;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

/**
 * RequestUtil工具测试类
 *
 * @author xwx1016404
 * @since 2021-07-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RequestUtil.class})
public class RequestUtilTest {
    @InjectMocks
    private RequestUtil requestUtil;

    @Mock
    private SessionService sessionService;

    @Mock
    private HttpServletRequest request;

    @Mock
    private EncryptorRestClient encryptorRestClient;

    /**
     * 用例场景：x-forwarded-for请求头为null，客户端IP为getRemoteAddr方法获取的结果
     * 前置条件：x-forwarded-for请求头为null
     * 检查点：客户端IP为getRemoteAddr方法获取的结果
     */
    @Test
    public void should_return_remote_addr_if_x_forwarded_for_header_is_null_ips_when_getClientIpAddress() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getRemoteAddr()).thenReturn(MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
        String clientIpAddress = requestUtil.getClientIpAddress(request);
        Assert.assertEquals(clientIpAddress, MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
    }

    /**
     * 用例场景：x-forwarded-for请求头为“UNKNOWN”，客户端IP为getRemoteAddr方法获取的结果
     * 前置条件：x-forwarded-for请求头为“UNKNOWN”
     * 检查点：客户端IP为getRemoteAddr方法获取的结果
     */
    @Test
    public void should_return_remote_addr_if_x_forwarded_for_header_is_unknown_ips_when_getClientIpAddress() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getHeader("x-forwarded-for")).thenReturn("UNKNOWN");
        PowerMockito.when(request.getRemoteAddr()).thenReturn(MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
        String clientIpAddress = requestUtil.getClientIpAddress(request);
        Assert.assertEquals(clientIpAddress, MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
    }

    /**
     * 用例场景：x-forwarded-for请求头包含多个IP，客户端IP为x-forwarded-for请求头中第一个IP
     * 前置条件：x-forwarded-for请求头包含多个IP
     * 检查点：客户端IP为x-forwarded-for请求头中第一个IP
     */
    @Test
    public void should_return_first_ip_if_x_forwarded_for_header_contains_multi_ips_when_getClientIpAddress() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getHeader("x-forwarded-for")).thenReturn("10.10.10.10, 192.168.1.1");
        String clientIp = requestUtil.getClientIpAddress(request);
        Assert.assertEquals(clientIp, "10.10.10.10");
    }

    /**
     * 用例场景：x-forwarded-for请求头只有一个IP，客户端IP为x-forwarded-for请求头中的这个IP
     * 前置条件：x-forwarded-for请求头只有一个IP
     * 检查点：客户端IP为x-forwarded-for请求头中的这个IP
     */
    @Test
    public void should_return_first_ip_if_x_forwarded_for_header_is_ip_when_getClientIpAddress() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getHeader("x-forwarded-for")).thenReturn("10.10.10.10");
        String clientIp = requestUtil.getClientIpAddress(request);
        Assert.assertEquals(clientIp, "10.10.10.10");
    }

    /**
     * 用例场景: 测试在有效的 Cookies 和请求头情况下，getForwardHeaderAndValidCsrf 方法的行为。
     * 前置条件:
     * - 请求中包含
     * HCS_FLAG、TOKEN、MEMBER_ESN、DME_AUTH_TOKEN、DME_AZ、REQUEST_ID、CLUSTER_TYPE、CLUSTER_ID
     * 和 HOST 这些头信息。
     * - 请求中包含有效的 SESSION Cookie。
     * 检查点:
     * - 检查返回的 HttpHeaders 是否包含正确的头信息。
     */
    @Test
    public void should_return_header_when_get_forward_header_and_valid_csrf_with_valid_cookies_and_headers() {
        // 设置请求头
        PowerMockito.when(request.getHeader(ConfigConstant.HCS_FLAG)).thenReturn("true");
        PowerMockito.when(request.getHeader(ConfigConstant.TOKEN)).thenReturn("test-token");
        PowerMockito.when(request.getHeader(ConfigConstant.MEMBER_ESN)).thenReturn("test-esn");
        PowerMockito.when(request.getHeader(ConfigConstant.DME_AUTH_TOKEN)).thenReturn("dme-auth-token");
        PowerMockito.when(request.getHeader(ConfigConstant.DME_AZ)).thenReturn("dme-az");
        PowerMockito.when(request.getHeader(ConfigConstant.REQUEST_ID)).thenReturn("request-id");
        PowerMockito.when(request.getHeader(ConfigConstant.CLUSTER_TYPE)).thenReturn("test-cluster-type");
        PowerMockito.when(request.getHeader(ConfigConstant.CLUSTER_ID)).thenReturn("cluster-id");
        PowerMockito.when(request.getHeader(ConfigConstant.HOST)).thenReturn("manage-ip");

        // 设置 Cookie
        Cookie cookie = new Cookie(ConfigConstant.SESSION, "session-value");
        PowerMockito.when(request.getCookies()).thenReturn(new Cookie[]{cookie});

        // 设置 SessionService 的返回值
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("encrypted-token");
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext("decrypted-token");
        PowerMockito.when(sessionService.getSessionInfo("session-value")).thenReturn(sessionInfo);
        PowerMockito.when(encryptorRestClient.decrypt("encrypted-token")).thenReturn(plaintextVo);
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();

        // 验证
        Assert.assertNotNull(headers); // 确保返回的 HttpHeaders 不为空
        Assert.assertEquals("test-token", headers.getFirst(ConfigConstant.HCS_AUTH_TOKEN));
        Assert.assertEquals("test-esn", headers.getFirst(ConfigConstant.MEMBER_ESN));
        Assert.assertEquals("dme-auth-token", headers.getFirst(ConfigConstant.DME_AUTH_TOKEN));
        Assert.assertEquals("dme-az", headers.getFirst(ConfigConstant.DME_AZ));
        Assert.assertEquals("request-id", headers.getFirst(ConfigConstant.REQUEST_ID));
        Assert.assertEquals("test-cluster-type", headers.getFirst(ConfigConstant.CLUSTER_TYPE));
        Assert.assertEquals("cluster-id", headers.getFirst(ConfigConstant.CLUSTER_ID));
        Assert.assertEquals("manage-ip", headers.getFirst(ConfigConstant.MANAGE_IP));
        Assert.assertEquals("decrypted-token", headers.getFirst(ConfigConstant.TOKEN));
    }

    /**
     * 用例场景: 测试在没有 Cookies 的情况下，getForwardHeaderAndValidCsrf 方法的行为。
     * 前置条件:
     * - 请求中包含 HCS_FLAG 和 TOKEN 头信息。
     * - 请求中没有 SESSION Cookie。
     * 检查点:
     * - 检查返回的 HttpHeaders 是否包含 HCS_AUTH_TOKEN，但不应包含 TOKEN。
     */
    @Test
    public void should_return_header_with_no_token_when_get_forward_header_and_valid_csrf_with_no_cookies() {
        // 设置请求头
        PowerMockito.when(request.getHeader(ConfigConstant.HCS_FLAG)).thenReturn("true");
        PowerMockito.when(request.getHeader(ConfigConstant.TOKEN)).thenReturn("test-token");

        // 不设置 Cookies
        PowerMockito.when(request.getCookies()).thenReturn(null);
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();

        // 验证
        Assert.assertNotNull(headers);
        Assert.assertEquals("test-token", headers.getFirst(ConfigConstant.HCS_AUTH_TOKEN));
        Assert.assertNull(headers.get(ConfigConstant.TOKEN));
    }
}
