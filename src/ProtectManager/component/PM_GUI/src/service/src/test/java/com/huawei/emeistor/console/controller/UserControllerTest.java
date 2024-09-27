/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.controller.response.LoginResponse;
import com.huawei.emeistor.console.controller.response.PageListResponse;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RBucket;
import org.redisson.api.RKeys;
import org.redisson.api.RLock;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Stream;

import javax.servlet.http.Cookie;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    UserController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class, RestTemplate.class,
    RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class
})
public class UserControllerTest {
    private static final String SEC_POLICY = "SEC_POLICY_JSON";

    private static final String USER_CACHE = "USER_LOCK_CACHE";

    private static final String SESSION_VALUSE
        = "userId=88a94c476f12a21e016f12a246e50009-loginTime=16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c";

    private static final String USER_ID = "88a94c476f12a21e016f12a246e50009";

    private final String url = "https://w3.huawei.com";

    private URI uri;

    @MockBean
    private RedissonClient redissonClient;

    @Autowired
    private UserController userController;

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

    private static final String LAST_LOGIN_TIME_KEY = "lastLoginTime";

    private static final String LAST_LOGIN_ZONE_KEY = "lastLoginZone";

    private static final String LAST_LOGIN_IP_KEY = "lastLoginIp";

    @Before
    public void setup() {
        Token token = new Token();
        token.setToken("sadfgasfasf");
        try {
            uri = new URL(NormalizerUtil.normalizeForString(url)).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        Cookie[] cookies = new Cookie[2];
        cookies[0] = new Cookie(ConfigConstant.SESSION, SESSION_VALUSE);
        cookies[1] = new Cookie(ConfigConstant.HEADER_NAME, "lili");
        request.setCookies(cookies);
    }

    @Test
    public void loginSuccess() {
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode("ABC");
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        Token token = new Token();
        token.setToken("sadfgasfasf");
        RMap<Object, Object> rMap = PowerMockito.mock(RMap.class);
        PowerMockito.when(rMap.get(LAST_LOGIN_IP_KEY)).thenReturn("0.0.0.0");
        PowerMockito.when(rMap.get(LAST_LOGIN_TIME_KEY)).thenReturn("2022-01-01 00:00:00");
        PowerMockito.when(rMap.get(LAST_LOGIN_ZONE_KEY)).thenReturn("xxx");
        PowerMockito.when(redissonClient.getMap(anyString())).thenReturn(rMap);
        ResponseEntity<Token> responseEntity = new ResponseEntity(token, HttpStatus.OK);
        ResponseEntity<SecurityPolicyBo> poEntity = getSecurityPolicyResEntity();
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), eq(Token.class))).thenReturn(responseEntity);
        PowerMockito.when(userRestTemplate.exchange(anyString(), any(), any(), eq(SecurityPolicyBo.class)))
            .thenReturn(poEntity);
        PowerMockito.when(encryptorRestClient.encrypt(any())).thenReturn(PowerMockito.mock(CiphertextVo.class));
        LoginResponse response = userController.login(authRequest);
        Assert.assertNotNull(response);
        Assert.assertEquals(response.getLastLoginIp(), "0.0.0.0");
        Assert.assertEquals(response.getLastLoginZone(), "xxx");
        Assert.assertEquals(response.getLastLoginTime(), "2022-01-01 00:00:00");
    }

    @Test
    public void loginSuccessWhenVerifyCodeExist() {
        String verifyCode = "{\"userId\": \"123\", \"userSessions\": [{\"name\": \"lili\"}]}";
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode(verifyCode);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(rBucket.isExists()).thenReturn(true);
        PowerMockito.when(rBucket.get()).thenReturn(verifyCode);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);

        // mock 安全策略
        RBucket policyBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(policyBucket.isExists()).thenReturn(true);
        PowerMockito.when(policyBucket.get())
            .thenReturn(
                "{\"minLifetime\":3,\"passComplexVal\":4,\"passCtrl\":true,\"passErrNum\":3,\"passHistoryDay\":30,\"passHistoryNum\":5,\"passLenVal\":8,\"passLockTime\":5,\"sessionTime\":30,\"usefulLife\":60}");
        PowerMockito.when(redissonClient.getBucket(SEC_POLICY)).thenReturn(policyBucket);

        // mock 无userCache
        RBucket userCache = PowerMockito.mock(RBucket.class);
        PowerMockito.when(policyBucket.isExists()).thenReturn(false);
        PowerMockito.when(redissonClient.getBucket(USER_CACHE)).thenReturn(userCache);

        PowerMockito.when(encryptorRestClient.encrypt(any())).thenReturn(PowerMockito.mock(CiphertextVo.class));
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        Token token = new Token();
        token.setToken("sadfgasfasf");
        RMap<Object, Object> rMap = PowerMockito.mock(RMap.class);
        PowerMockito.when(rMap.get(LAST_LOGIN_IP_KEY)).thenReturn("0.0.0.0");
        PowerMockito.when(rMap.get(LAST_LOGIN_TIME_KEY)).thenReturn("2022-01-01 00:00:00");
        PowerMockito.when(rMap.get(LAST_LOGIN_ZONE_KEY)).thenReturn("xxx");
        PowerMockito.when(redissonClient.getMap(anyString())).thenReturn(rMap);
        ResponseEntity<Token> responseEntity = new ResponseEntity(token, HttpStatus.OK);
        ResponseEntity<SecurityPolicyBo> poEntity = getSecurityPolicyResEntity();
        PowerMockito.when(userRestTemplate.postForEntity(anyString(), any(), eq(Token.class))).thenReturn(responseEntity);
        PowerMockito.when(userRestTemplate.exchange(anyString(), any(), any(), eq(SecurityPolicyBo.class)))
            .thenReturn(poEntity);
        PowerMockito.when(sha256Encryptor.encryptionSessionId(anyString())).thenReturn(PowerMockito.mock(String.class));
        LoginResponse response = userController.login(authRequest);
        Assert.assertNotNull(response);
        Assert.assertEquals(response.getLastLoginIp(), "0.0.0.0");
        Assert.assertEquals(response.getLastLoginZone(), "xxx");
        Assert.assertEquals(response.getLastLoginTime(), "2022-01-01 00:00:00");
    }

    @Test(expected = LegoCheckedException.class)
    public void loginRaiseExceptionWhenVerifyCodeWrong() {
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode("{\"userId\": \"88a94c476f12a21e016f12a246e50009\"}");
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(rBucket.isExists()).thenReturn(true);
        PowerMockito.when(rBucket.get()).thenReturn("{\"userId\": \"88a94c476f12a21e016f12a246e50001\"}");
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        userController.login(authRequest);
    }

    @Test(expected = RestClientException.class)
    public void loginRaiseExceptionWhenUserLock() {
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode("{\"userId\": \"88a94c476f12a21e016f12a246e50009\"}");
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        PowerMockito.when(redissonClient.getLock(any())).thenThrow(new RestClientException("\"1677929493\""));
        userController.login(authRequest);
    }

    @Test(expected = RestClientException.class)
    public void loginRaiseExceptionWhenVerifyCodeNotExist() {
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode("{\"userId\": \"88a94c476f12a21e016f12a246e50009\"}");
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        PowerMockito.when(redissonClient.getLock(any())).thenThrow(new RestClientException(""));
        userController.login(authRequest);
    }

    @Test(expected = RestClientException.class)
    public void loginRaiseExceptionWhenVerifyCodeExist() {
        AuthRequest authRequest = new AuthRequest();
        authRequest.setUserName("sysadmin");
        authRequest.setPassword("Huawei@123");
        authRequest.setVerifyCode("{\"userId\": \"88a94c476f12a21e016f12a246e50009\"}");
        RBucket stringRBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(stringRBucket.isExists()).thenReturn(true);
        PowerMockito.when(stringRBucket.get()).thenReturn("{\"userId\": \"88a94c476f12a21e016f12a246e50009\"}");
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(stringRBucket);
        RBucket intRBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(intRBucket.isExists()).thenReturn(true);
        PowerMockito.when(intRBucket.get()).thenReturn(123);
        PowerMockito.when(redissonClient.getBucket("FAILED_TIMES127.0.0.1")).thenReturn(intRBucket);
        PowerMockito.when(redissonClient.getLock(any())).thenThrow(new RestClientException(""));
        userController.login(authRequest);
    }

    @Test
    public void putSecuritySuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        RequestEntity<SecurityPolicyBo> poEntity = getSecurityPolicyRequestEntity();
        userController.putSecurity(poEntity);
    }

    @Test
    public void logoutSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        userController.logout("MANUAL");
    }

    @Test
    public void logoutSuccessWhenSessionIdHasWhip() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        Cookie[] cookies = new Cookie[2];
        cookies[0] = new Cookie(ConfigConstant.SESSION, SESSION_VALUSE);
        cookies[1] = new Cookie(ConfigConstant.HEADER_NAME, "lili");
        request.setCookies(cookies);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        userController.logout("MANUAL");
    }

    @Test
    public void logoutSuccessWhenSessionIdHasEqualSign() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(rBucket.isExists()).thenReturn(false);
        PowerMockito.when(rBucket.get()).thenReturn("{\"userId\": \"88a94c476f12a21e016f12a246e50001\"}");
        Cookie[] cookies = new Cookie[2];
        cookies[0] = new Cookie(ConfigConstant.SESSION, SESSION_VALUSE);
        cookies[1] = new Cookie(ConfigConstant.HEADER_NAME, "lili");
        request.setCookies(cookies);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        userController.logout("MANUAL");
    }

    @Test
    public void lockSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(rBucket.get()).thenReturn("123");
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(rBucket);
        RBucket sessionRBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(sessionRBucket.isExists()).thenReturn(true);
        PowerMockito.when(sessionRBucket.get())
            .thenReturn("{\"userId\": \"123\", \"userSessions\": [{\"name\": \"lili\"}]}");
        PowerMockito.when(redissonClient.getBucket("USER_LOCK_CACHE123")).thenReturn(sessionRBucket);
        PowerMockito.when(redissonClient.getBucket(anyString())).thenReturn(sessionRBucket);
        PowerMockito.when(sha256Encryptor.encryptionSessionId(anyString())).thenReturn(PowerMockito.mock(String.class));
        userController.lock("123");
    }

    @Test
    public void unlockSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        userController.unlock(USER_ID, "Huawei@123");
    }

    @Test
    public void getAllUserSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        RKeys rKey = PowerMockito.mock(RKeys.class);
        Stream<String> keys = Collections.singletonList("userId=123-456").stream();
        PowerMockito.when(redissonClient.getKeys()).thenReturn(rKey);
        PowerMockito.when(rKey.getKeysStreamByPattern(any())).thenReturn(keys);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        ResponseEntity<PageListResponse> pageListResEntity = getPageListResEntity();
        PowerMockito.when(
            restTemplate.exchange(anyString(), any(), any(), eq(PageListResponse.class), any(), any(), any(), any(),
                any())).thenReturn(pageListResEntity);
        RequestEntity<Object> poEntity = getSecurityPolicyRequestEntity();
        PageListResponse response = userController.getAllUser(poEntity);
        Assert.assertNotNull(response);
    }

    @Test
    public void deleteUserSuccess() {
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        // userController.deleteUser(USER_ID, false);
    }

    private ResponseEntity<SecurityPolicyBo> getSecurityPolicyResEntity() {
        SecurityPolicyBo policyBo = getSecurityPolicyBo();
        ResponseEntity<SecurityPolicyBo> poEntity = new ResponseEntity(policyBo, HttpStatus.OK);
        return poEntity;
    }

    private ResponseEntity<PageListResponse> getPageListResEntity() {
        Map<String, Object> map = new HashMap();
        map.put("userId", USER_ID);
        List<Map<String, Object>> user = new ArrayList();
        user.add(map);
        PageListResponse pageListResponse = new PageListResponse();
        pageListResponse.setCurrentCount(10L);
        pageListResponse.setPageSize(10);
        pageListResponse.setStartIndex(0);
        pageListResponse.setTotal(100);
        pageListResponse.setUserList(user);
        ResponseEntity<PageListResponse> responseEntity = new ResponseEntity(pageListResponse, HttpStatus.OK);
        return responseEntity;
    }

    private SecurityPolicyBo getSecurityPolicyBo() {
        SecurityPolicyBo policyBo = new SecurityPolicyBo();
        policyBo.setMinLifetime(60);
        policyBo.setUsefulLife(60 * 60);
        policyBo.setPassComplexVal(100);
        policyBo.setPassCtrl(false);
        policyBo.setPassErrNum(1);
        policyBo.setPassLenVal(3);
        policyBo.setPassLockTime(60);
        policyBo.setSessionTime(60);
        policyBo.setPassHistoryNum(5);
        policyBo.setPassHistoryDay(30);
        return policyBo;
    }

    private <T> RequestEntity<T> getSecurityPolicyRequestEntity() {
        SecurityPolicyBo policyBo = new SecurityPolicyBo();
        RequestEntity<T> requestEntity = new RequestEntity(policyBo, HttpMethod.GET, uri);
        return requestEntity;
    }
}
