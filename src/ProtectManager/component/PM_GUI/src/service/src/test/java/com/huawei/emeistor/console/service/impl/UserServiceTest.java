package com.huawei.emeistor.console.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.bean.UserCache;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.controller.request.SendDynamicCodeRequest;
import com.huawei.emeistor.console.controller.response.LoginResponse;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.CaptchaService;
import com.huawei.emeistor.console.service.SecurityPolicyService;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import jodd.util.collection.ArrayEnumeration;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.io.IOException;
import java.util.Enumeration;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 用户serviceTest
 *
 * @author w30042425
 * @since 2023-02-14
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {UserServiceImpl.class, RestTemplate.class, SecurityPolicyService.class, RequestUtil.class})
@PrepareForTest(RequestUtil.class)
public class UserServiceTest {
    @Mock
    private HttpServletResponse response;

    @Mock
    private HttpServletRequest httpServletRequest;

    @Autowired
    private UserServiceImpl userService;

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @MockBean
    private SecurityPolicyService securityPolicyService;

    @MockBean
    private SessionService sessionService;

    @MockBean
    private CaptchaService captchaService;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private RedissonClient redissonClient;

    private static final String LAST_LOGIN_TIME_KEY = "lastLoginTime";

    private static final String LAST_LOGIN_ZONE_KEY = "lastLoginZone";

    private static final String LAST_LOGIN_IP_KEY = "lastLoginIp";

    @Test
    public void test_saml_login() throws IOException {
        Enumeration<String> enumeration = new ArrayEnumeration<>(new String[] {"Accept", "Accept/xml"});
        PowerMockito.when(httpServletRequest.getHeaderNames()).thenReturn(enumeration);
        PowerMockito.when(httpServletRequest.getHeaders(anyString())).thenReturn(enumeration);
        PowerMockito.when(httpServletRequest.getHeader(anyString())).thenReturn("Accept");
        Token token = new Token();
        token.setToken("sadfgasfasf");
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(token, HttpStatus.OK);
        ReflectionTestUtils.setField(userService, "userRestTemplate", userRestTemplate);
        ReflectionTestUtils.setField(userService, "securityPolicyService", securityPolicyService);
        ReflectionTestUtils.setField(userService, "sessionService", sessionService);
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), any())).thenReturn(responseEntity);
        SecurityPolicyBo securityPolicyBo = new SecurityPolicyBo();
        PowerMockito.when(securityPolicyService.getSecurityPolicy(any())).thenReturn(securityPolicyBo);
        UserCache userCache = new UserCache();
        PowerMockito.when(sessionService.getUserCache(anyString())).thenReturn(userCache);
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setSessionId("testSessionId");
        PowerMockito.when(sessionService.genSession(any(), any(), any())).thenReturn(sessionInfo);
        userService.samlLogin(httpServletRequest, response);
    }

    @Test
    public void test_send_dynamic_code_fail() {
        SendDynamicCodeRequest sendDynamicCodeRequest = new SendDynamicCodeRequest();
        sendDynamicCodeRequest.setVerifyCode("45455");
        sendDynamicCodeRequest.setPassword("test102");
        sendDynamicCodeRequest.setUserName("TEST102");
        sendDynamicCodeRequest.setUserType("COMMON");
        ReflectionTestUtils.setField(userService, "userRestTemplate", userRestTemplate);
        PowerMockito.doThrow(new LegoCheckedException(1677752064L, "Error"))
            .when(userRestTemplate)
            .postForEntity(anyString(), any(), any());
        try {
            userService.sendDynamicCode(sendDynamicCodeRequest);
        } catch (LegoCheckedException e) {
            Assert.assertEquals(e.getErrorCode(), 1677752064L);
        }
    }

    @Test
    public void test_last_login() {
        AuthRequest localAuthRequest = new AuthRequest();
        localAuthRequest.setUserName("111");
        localAuthRequest.setPassword("aaa");
        PowerMockito.doNothing().when(sessionService).lock(any());
        Token token = new Token();
        token.setToken("sadfgasfasf");
        token.setUserId("000");
        token.setServiceProduct("HCS");
        RMap<Object, Object> rMap = PowerMockito.mock(RMap.class);
        PowerMockito.when(rMap.get(LAST_LOGIN_IP_KEY)).thenReturn("0.0.0.0");
        PowerMockito.when(rMap.get(LAST_LOGIN_TIME_KEY)).thenReturn("2022-01-01 00:00:00");
        PowerMockito.when(rMap.get(LAST_LOGIN_ZONE_KEY)).thenReturn("xxx");
        PowerMockito.when(redissonClient.getMap(anyString())).thenReturn(rMap);
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(token, HttpStatus.OK);
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), any())).thenReturn(responseEntity);
        SecurityPolicyBo securityPolicyBo = new SecurityPolicyBo();
        PowerMockito.when(securityPolicyService.getSecurityPolicy(any())).thenReturn(securityPolicyBo);
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setSessionId("testSessionId");
        PowerMockito.when(sessionService.genSession(any(), any(), any())).thenReturn(sessionInfo);
        LoginResponse loginResponse = userService.login(localAuthRequest);
        Assert.assertNotNull(loginResponse);
        Assert.assertEquals(loginResponse.getLastLoginIp(), "0.0.0.0");
        Assert.assertEquals(loginResponse.getLastLoginZone(), "xxx");
        Assert.assertEquals(loginResponse.getLastLoginTime(), "2022-01-01 00:00:00");
        Assert.assertEquals(loginResponse.getServiceProduct(), "HCS");
    }

    @Test
    public void test_hcs_login() throws IOException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.addHeader("X-Auth-Token", "hcsToken");
        request.addHeader(ConfigConstant.HCS_FLAG, "1.0");
        ReflectionTestUtils.setField(userService, "request", request);
        Token token = new Token();
        token.setToken("sadfgasfasf");
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(token, HttpStatus.OK);
        ReflectionTestUtils.setField(userService, "userRestTemplate", userRestTemplate);
        ReflectionTestUtils.setField(userService, "securityPolicyService", securityPolicyService);
        ReflectionTestUtils.setField(userService, "sessionService", sessionService);
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), any())).thenReturn(responseEntity);
        SecurityPolicyBo securityPolicyBo = new SecurityPolicyBo();
        PowerMockito.when(securityPolicyService.getSecurityPolicy(any())).thenReturn(securityPolicyBo);
        UserCache userCache = new UserCache();
        PowerMockito.when(sessionService.getUserCache(anyString())).thenReturn(userCache);
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setSessionId("testSessionId");
        PowerMockito.when(sessionService.genSession(any(), any(), any())).thenReturn(sessionInfo);
        userService.hcsLogin();
        Assert.assertTrue(true);
    }

    @Test
    public void test_dme_login() throws IOException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.addHeader("X-Auth-Token", "dmeToken");
        request.addHeader(ConfigConstant.DME_FLAG, "1.0");
        ReflectionTestUtils.setField(userService, "request", request);
        Token token = new Token();
        token.setToken("sadfgasfasf");
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(token, HttpStatus.OK);
        ReflectionTestUtils.setField(userService, "userRestTemplate", userRestTemplate);
        ReflectionTestUtils.setField(userService, "securityPolicyService", securityPolicyService);
        ReflectionTestUtils.setField(userService, "sessionService", sessionService);
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), any())).thenReturn(responseEntity);
        SecurityPolicyBo securityPolicyBo = new SecurityPolicyBo();
        PowerMockito.when(securityPolicyService.getSecurityPolicy(any())).thenReturn(securityPolicyBo);
        UserCache userCache = new UserCache();
        PowerMockito.when(sessionService.getUserCache(anyString())).thenReturn(userCache);
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setSessionId("testSessionId");
        PowerMockito.when(sessionService.genSession(any(), any(), any())).thenReturn(sessionInfo);
        userService.dmeLogin();
        Assert.assertTrue(true);
    }
}
