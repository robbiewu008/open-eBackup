package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
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
import java.util.Collections;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    RequestUtil.class, CsrfWebFilter.class
})
public class CsrfWebFilterTest {
    @InjectMocks
    private CsrfWebFilter csrfWebFilter;

    @Mock
    private RequestUtil requestUtil;

    @Mock
    private HttpServletRequest request;

    @Mock
    private HttpServletResponse response;

    @Before
    public void set_up() {
        ReflectionTestUtils.setField(csrfWebFilter, "requestUtil", requestUtil);
        ReflectionTestUtils.setField(csrfWebFilter, "csrfIgnorePaths", Collections.singletonList("/v1/ignored"));
    }

    @Test
    public void test_contains_ignored() throws ServletException, IOException {
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/ignored");
        csrfWebFilter.doFilterInternal(request, response, filterChain);
    }

    @Test
    public void test_not_contains_ignored() throws ServletException, IOException {
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/not_ignored");
        PowerMockito.when(request.getHeader(ConfigConstant.HEADER_NAME)).thenReturn("testToken");
        PowerMockito.when(request.getCookies()).thenReturn(new Cookie[] {new Cookie("_OP_TOKEN_", "testToken")});
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("testToken");
        sessionInfo.setCsrfToken("testToken");
        PowerMockito.when(requestUtil.getSessionInfo()).thenReturn(sessionInfo);
        csrfWebFilter.doFilterInternal(request, response, filterChain);
    }

    @Test
    public void test_not_contains_ignored_when_session_info_is_emtpy() throws ServletException, IOException {
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/not_ignored");
        PowerMockito.when(request.getHeader(ConfigConstant.HEADER_NAME)).thenReturn("testToken");
        PowerMockito.when(request.getCookies()).thenReturn(new Cookie[] {new Cookie("_OP_TOKEN_", "testToken")});
        csrfWebFilter.doFilterInternal(request, response, filterChain);
    }

    @Test
    public void test_not_contains_ignored_when_cookie_token_is_emtpy() throws ServletException, IOException {
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/not_ignored");
        PowerMockito.when(request.getHeader(ConfigConstant.HEADER_NAME)).thenReturn("testToken");
        PowerMockito.when(request.getCookies()).thenReturn(new Cookie[] {});
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("testToken");
        sessionInfo.setCsrfToken("testToken");
        PowerMockito.when(requestUtil.getSessionInfo()).thenReturn(sessionInfo);
        csrfWebFilter.doFilterInternal(request, response, filterChain);
    }

    @Test
    public void test_not_contains_ignored_when_token_not_equals() throws ServletException, IOException {
        FilterChain filterChain = PowerMockito.mock(FilterChain.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/not_ignored");
        PowerMockito.when(request.getHeader(ConfigConstant.HEADER_NAME)).thenReturn("testToken");
        PowerMockito.when(request.getCookies()).thenReturn(new Cookie[] {new Cookie("_OP_TOKEN_", "testToken")});
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setToken("testNotEqualsToken");
        sessionInfo.setCsrfToken("testNotEqualsToken");
        PowerMockito.when(requestUtil.getSessionInfo()).thenReturn(sessionInfo);
        csrfWebFilter.doFilterInternal(request, response, filterChain);
    }
}
