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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.RequestUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import javax.servlet.http.HttpServletRequest;

/**
 * RequestUtil Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RequestContextHolder.class})
public class RequestUtilTest {
    @InjectMocks
    private RequestUtil requestUtil;

    @Mock
    private TokenVerificationService tokenVerificationService;

    /**
     * 用例场景：x-forwarded-for请求头为null，客户端IP为getRemoteAddr方法获取的结果
     * 前置条件：x-forwarded-for请求头为null
     * 检查点：客户端IP为getRemoteAddr方法获取的结果
     */
    @Test
    public void should_return_remote_addr_if_x_forwarded_for_header_is_null_ips_when_getClientIpAddress() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getRemoteAddr()).thenReturn(MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
        PowerMockito.when(request.getHeader("client-ip")).thenReturn(MockHttpServletRequest.DEFAULT_REMOTE_ADDR);
        String clientIpAddress = RequestUtil.getClientIpAddress(request);
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
        String clientIpAddress = RequestUtil.getClientIpAddress(request);
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
        String clientIp = RequestUtil.getClientIpAddress(request);
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
        String clientIp = RequestUtil.getClientIpAddress(request);
        Assert.assertEquals(clientIp, "10.10.10.10");
    }

    /*
     * 测试用例：获取ip
     * 前置条件：无
     * CHECK点：获取ip成功
     */
    @Test
    public void test_getIpAddress() {
        PowerMockito.mockStatic(RequestContextHolder.class);
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        ServletRequestAttributes requestAttributes = new ServletRequestAttributes(request);
        PowerMockito.when(RequestContextHolder.getRequestAttributes())
            .thenReturn(requestAttributes);
        PowerMockito.when(request.getHeader("x-forwarded-for")).thenReturn("10.10.10.10");
        String clientIp = requestUtil.getIpAddress();
        Assert.assertEquals(clientIp, "10.10.10.10");
    }

    /*
     * 测试用例：获取用户名
     * 前置条件：无
     * CHECK点：获取用户名成功
     */
    @Test
    public void test_getUserName() {
        TokenBo token = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("sysadmin_test");
        token.setUser(userBo);
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(token);
        String userName = requestUtil.getUserName();
        Assert.assertEquals(userName, "sysadmin_test");
    }
}