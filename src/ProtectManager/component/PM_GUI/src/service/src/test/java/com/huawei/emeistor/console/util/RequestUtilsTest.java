package com.huawei.emeistor.console.util;

import static org.mockito.ArgumentMatchers.anyString;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpHeaders;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpSession;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RequestUtils.class})
public class RequestUtilsTest {
    @Test
    public void test_get_forward_Header_and_valid_csrf_success() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getHeader(anyString())).thenReturn("test");
        HttpSession httpSession = PowerMockito.mock(HttpSession.class);
        PowerMockito.when(httpSession.getAttribute(anyString())).thenReturn("test");
        PowerMockito.when(request.getSession()).thenReturn(httpSession);
        HttpHeaders forwardHeaderAndValidCsrf = RequestUtils.getForwardHeaderAndValidCsrf(request);
        Assert.assertNotNull(forwardHeaderAndValidCsrf);
    }
}
