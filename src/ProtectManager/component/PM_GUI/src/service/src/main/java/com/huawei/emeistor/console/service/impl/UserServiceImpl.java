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

import com.huawei.emeistor.console.bean.ADFSForwardDto;
import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.controller.request.SendDynamicCodeRequest;
import com.huawei.emeistor.console.controller.response.LoginResponse;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.CaptchaService;
import com.huawei.emeistor.console.service.SecurityPolicyService;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.service.UserService;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseCookie;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.text.Normalizer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Enumeration;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 登录相关service
 *
 * @author t00482481
 * @since 2020-9-06
 */
@Component
@Slf4j
public class UserServiceImpl implements UserService {
    private static final String LOGIN_URL = "/v1/auth/token";

    private static final String SAML_LOGIN_URL = "/v1/auth/saml/samlToken";

    private static final String ADFS_LOGIN_URL = "/v1/auth/adfsToken";

    private static final String ADFS_FORWARD_INFO_URL = "/v1/internal/adfs/config/forward/info";

    private static final String HCS_LOGIN_URL = "/v1/auth/hcs-token";

    private static final String DME_LOGIN_URL = "/v1/auth/dme-token";

    private static final String SEND_DYNAMIC_CODE_URL = "/v1/auth/dynamic-code/send";

    private static final String IS_GUI = "isGui";

    private static final String AUTH_TOKEN = "X-Auth-Token";

    private static final String USER_ALREADY_LOCKED = "\"1677929493\"";

    private static final String NEED_EMAIL_DYNAMIC_PWD_ERROR = "\"1677752064\"";

    private static final String LAST_LOGIN_TIME_KEY = "lastLoginTime";

    private static final String LAST_LOGIN_ZONE_KEY = "lastLoginZone";

    private static final String LAST_LOGIN_IP_KEY = "lastLoginIp";

    private static final String LAST_LOGIN_INFO = "Last_Login_Info";

    private static final String DATE_TIME = "yyyy-MM-dd HH:mm:ss";

    private static final String USER_TYPE = "userType";


    private static final Pattern PATTERN = Pattern.compile("<errorCode>(\\d*?)</errorCode>");

    @Autowired
    @Qualifier("userRestTemplate")
    private RestTemplate userRestTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    @Value("${api.gateway.endpoint}")
    private String userApi;

    @Autowired
    private SecurityPolicyService securityPolicyService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private CaptchaService captchaService;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 登录
     *
     * @param authRequest 请求参数
     * @return 登录响应
     */
    @Override
    @ExterAttack
    public LoginResponse login(AuthRequest authRequest) {
        AuthRequest localAuthRequest = NormalizerUtil.normalizeForBean(authRequest, AuthRequest.class);
        captchaService.checkVerifyCode(localAuthRequest);
        HttpHeaders header = new HttpHeaders();
        header.add(IS_GUI, "true");
        header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
        String requestIp = RequestUtil.getClientIpAddress(request);
        header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
        HttpEntity<Object> httpEntity = new HttpEntity<>(localAuthRequest, header);
        Token token;
        ResponseEntity<Token> responseEntity;
        LoginResponse res = new LoginResponse();
        try {
            setLoginResoponse(localAuthRequest, header, requestIp, httpEntity, res);
        } catch (RestClientException e) {
            String message = e.getMessage();
            if (message != null && (message.contains(USER_ALREADY_LOCKED) || message.contains(
                NEED_EMAIL_DYNAMIC_PWD_ERROR))) {
                sessionService.needVerificationCode(requestIp, false);
                captchaService.cleanVerificationCodeTag(requestIp);
            } else if (sessionService.needVerificationCode(requestIp, true)) {
                captchaService.addVerificationCodeTag();
            } else {
                log.error("error message: ", e);
            }
            throw e;
        } finally {
            captchaService.invalidVerificationCode();
            sessionService.unlock(localAuthRequest.getUserName());
        }
        return res;
    }

    private void setLoginResoponse(AuthRequest localAuthRequest, HttpHeaders header, String requestIp,
        HttpEntity<Object> httpEntity, LoginResponse res) {
        log.info("The user: {}, starts to acquire distributed locks ", localAuthRequest.getUserName());
        sessionService.lock(localAuthRequest.getUserName());

        log.info("The distributed lock was successfully obtained.");
        ResponseEntity<Token> responseEntity = userRestTemplate.postForEntity(
            NormalizerUtil.normalizeForString(userApi + LOGIN_URL), httpEntity, Token.class);
        Token token = Objects.requireNonNull(responseEntity).getBody();
        header.add(AUTH_TOKEN, Objects.requireNonNull(token).getToken());

        log.info("Obtaining the conversation mark succeeded.");
        SecurityPolicyBo secBo = securityPolicyService.getSecurityPolicy(new HttpEntity<>(header));
        SessionInfo session = sessionService.genSession(
            sessionService.getUserCache(Objects.requireNonNull(token).getUserId()), secBo, token);
        sessionService.needVerificationCode(requestIp, false);
        captchaService.cleanVerificationCodeTag(requestIp);
        addSessionToCookie(session);
        addCsrfTokenToCookie(session.getCsrfToken());
        res.setSessionId(session.getSessionId());
        res.setModifyPassword(token.isModifyPassword());
        res.setUserId(token.getUserId());
        res.setExpireDay(token.getExpireDay());
        res.setServiceProduct(token.getServiceProduct());

        // 取最后登录时间，最后登录IP
        setLastLoginInfo(requestIp, res, token);
        log.info("The user logs in successfully and returns to the GUI.");
    }


    private void setLastLoginInfo(String requestIp, LoginResponse res, Token token) {
        RMap<String, String> lastLoginInfo = redissonClient.getMap(token.getUserId() + LAST_LOGIN_INFO);

        // 非首次登录，不为空
        if (!lastLoginInfo.isEmpty()) {
            res.setLastLoginTime(lastLoginInfo.get(LAST_LOGIN_TIME_KEY));
            res.setLastLoginIp(lastLoginInfo.get(LAST_LOGIN_IP_KEY));
            res.setLastLoginZone(lastLoginInfo.get(LAST_LOGIN_ZONE_KEY));
        }

        // 塞入本次登录时间，本次登录IP
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_TIME);
        Date date = new Date();
        lastLoginInfo.put(LAST_LOGIN_TIME_KEY, dateFormat.format(date));
        lastLoginInfo.put(LAST_LOGIN_ZONE_KEY, token.getTimeZone());
        lastLoginInfo.put(LAST_LOGIN_IP_KEY, requestIp);
    }

    @Override
    @ExterAttack
    public void sendDynamicCode(SendDynamicCodeRequest sendDynamicCodeRequest) {
        captchaService.checkVerifyCode(sendDynamicCodeRequest.getVerifyCode());
        HttpHeaders header = new HttpHeaders();
        header.add(IS_GUI, "true");
        header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
        String requestIp = RequestUtil.getClientIpAddress(request);
        header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
        HttpEntity<Object> httpEntity = new HttpEntity<>(sendDynamicCodeRequest, header);
        try {
            log.info("The user: {}, starts to acquire distributed locks ", sendDynamicCodeRequest.getUserName());
            sessionService.lock(sendDynamicCodeRequest.getUserName());

            log.info("The distributed lock was successfully obtained.");
            userRestTemplate.postForEntity(NormalizerUtil.normalizeForString(userApi + SEND_DYNAMIC_CODE_URL),
                httpEntity, Object.class);
            log.info("send dynamic code successfully .");

            sessionService.needVerificationCode(requestIp, false);
            captchaService.cleanVerificationCodeTag(requestIp);
            log.info("send dynamic code successfully and returns to the GUI.");
        } catch (RestClientException e) {
            String message = e.getMessage();
            if (message != null && message.contains(USER_ALREADY_LOCKED)) {
                sessionService.needVerificationCode(requestIp, false);
                captchaService.cleanVerificationCodeTag(requestIp);
            } else {
                log.error("send dynamic code error: ", e);
            }
            throw e;
        } finally {
            captchaService.invalidVerificationCode();
            sessionService.unlock(sendDynamicCodeRequest.getUserName());
        }
    }

    @Override
    public void samlLogin(HttpServletRequest request, HttpServletResponse response) throws IOException {
        HttpHeaders header = buildForwardHeader(request);
        try {
            header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
            String requestIp = RequestUtil.getClientIpAddress(request);
            header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
            header.getAccept().add(MediaType.APPLICATION_JSON);
            HttpEntity<Object> httpEntity = new HttpEntity<>(header);
            ResponseEntity<Token> responseEntity = userRestTemplate.postForEntity(
                NormalizerUtil.normalizeForString(userApi + SAML_LOGIN_URL), httpEntity, Token.class);
            Token token = Objects.requireNonNull(responseEntity).getBody();
            header.add(AUTH_TOKEN, Objects.requireNonNull(token).getToken());
            header.getAccept().add(MediaType.APPLICATION_JSON);
            log.info("Obtaining the session token succeeded.");
            SecurityPolicyBo secBo = securityPolicyService.getSecurityPolicy(new HttpEntity<>(header));
            SessionInfo session = sessionService.genSession(
                sessionService.getUserCache(Objects.requireNonNull(token).getUserId()), secBo, token);
            setSamlLoginCookie(session, response);
            response.sendRedirect(response.encodeRedirectURL("/console/#/home"));
        } catch (RestClientException e) {
            log.error("saml login error", e);
            response.sendRedirect(response.encodeRedirectURL("/console/#/error-page?type=LoginLimiter"));
        }
    }

    @Override
    public void adfsLogin(HttpServletRequest request, HttpServletResponse response) throws IOException {
        HttpHeaders header = buildForwardHeader(request);
        try {
            header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
            String requestIp = RequestUtil.getClientIpAddress(request);
            header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
            header.getAccept().add(MediaType.APPLICATION_JSON);
            String code = request.getParameter("code");
            MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
            map.add("code", NormalizerUtil.normalizeForString(code));
            HttpEntity<Object> httpEntity = new HttpEntity<>(map, header);
            ResponseEntity<Token> responseEntity = userRestTemplate.exchange(
                NormalizerUtil.normalizeForString(userApi + ADFS_LOGIN_URL), HttpMethod.POST, httpEntity, Token.class);
            Token token = Objects.requireNonNull(responseEntity).getBody();
            header.add(AUTH_TOKEN, Objects.requireNonNull(token).getToken());
            header.getAccept().add(MediaType.APPLICATION_JSON);
            log.info("Obtaining the adfs session token succeeded.");
            SecurityPolicyBo secBo = securityPolicyService.getSecurityPolicy(new HttpEntity<>(header));
            SessionInfo session = sessionService.genSession(
                sessionService.getUserCache(Objects.requireNonNull(token).getUserId()), secBo, token);
            setSamlLoginCookie(session, response);
            LoginResponse res = new LoginResponse();
            setLastLoginInfo(requestIp, res, token);
            String redirectURL = StringUtils.join("/console/#/home?lastLoginTime=", res.getLastLoginTime(),
                "&lastLoginIp=", res.getLastLoginIp(), "&lastLoginZone=", res.getLastLoginZone());
            log.info("adfs redirect url:{}", redirectURL);
            response.sendRedirect(response.encodeRedirectURL(redirectURL));
        } catch (LegoCheckedException e) {
            log.error("adfs login error", ExceptionUtil.getErrorMessage(e));
            response.sendRedirect(response.encodeRedirectURL("/console/#/login?errorCode=" + e.getErrorCode()));
        } catch (RestClientException e) {
            log.error("adfs login error", ExceptionUtil.getErrorMessage(e));
            Optional<String> errorCodeOp = extractErrorCode(e.getMessage());
            if (errorCodeOp.isPresent()) {
                response.sendRedirect(response.encodeRedirectURL("/console/#/login?errorCode=" + errorCodeOp.get()));
            } else {
                response.sendRedirect(response.encodeRedirectURL("/console/#/error-page?type=LoginLimiter"));
            }
        }
    }

    private Optional<String> extractErrorCode(String errorMessage) {
        Matcher matcher = PATTERN.matcher(errorMessage);
        while (matcher.find()) {
            String group = matcher.group();
            String errorCode = group.replaceAll("\\D", "");
            if (StringUtils.isNotEmpty(errorCode)) {
                return Optional.of(errorCode);
            }
        }
        return Optional.empty();
    }

    @Override
    public ADFSForwardDto adfsRedirect(HttpServletRequest request, HttpServletResponse response) throws IOException {
        HttpHeaders header = buildForwardHeader(request);
        header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
        String requestIp = RequestUtil.getClientIpAddress(request);
        header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
        header.getAccept().add(MediaType.APPLICATION_JSON);
        HttpEntity<Object> httpEntity = new HttpEntity<>(header);
        ResponseEntity<ADFSForwardDto> responseEntity = userRestTemplate.exchange(
            NormalizerUtil.normalizeForString(userApi + ADFS_FORWARD_INFO_URL), HttpMethod.GET, httpEntity,
            ADFSForwardDto.class);
        ADFSForwardDto dto = responseEntity.getBody();
        String redirectUrl = buildRedirectUrl(Objects.requireNonNull(dto).getProviderUrl(), dto.getClientId(),
            dto.getCallbackUrl());
        dto.setForwardUrl(redirectUrl);
        String logoutUrl = buildLogoutRedirectUrl(dto.getProviderUrl(), dto.getCallbackUrl());
        dto.setLogoutForwardUrl(logoutUrl);
        return dto;
    }

    private String buildRedirectUrl(String url, String clientId, String redirectUrl) throws MalformedURLException {
        String hostUrl = getHostUrl(url);
        return hostUrl + "/adfs/oauth2/authorize/?response_type=code&client_id=" + clientId + "&redirect_uri="
            + redirectUrl;
    }

    private String buildLogoutRedirectUrl(String url, String redirectUrl) throws MalformedURLException {
        String adfsHostUrl = getHostUrl(url);
        String opHostUrl = getHostUrl(redirectUrl);
        return adfsHostUrl + "/adfs/oauth2/logout?post_logout_redirect_uri=" + opHostUrl + "/console/#/login";
    }

    private static String getHostUrl(String url) throws MalformedURLException {
        URL urls = new URL(url);
        return "https://" + urls.getHost();
    }

    /**
     * hcs 用户登录处理
     */
    @Override
    public void hcsLogin() {
        try {
            HttpHeaders header = new HttpHeaders();
            header.add(AUTH_TOKEN, request.getHeader(AUTH_TOKEN));
            header.add(ConfigConstant.HCS_FLAG, request.getHeader(ConfigConstant.HCS_FLAG));
            header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
            String requestIp = RequestUtil.getClientIpAddress(request);
            header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
            HttpEntity<Object> httpEntity = new HttpEntity<>(header);
            ResponseEntity<Token> responseEntity = userRestTemplate.postForEntity(
                NormalizerUtil.normalizeForString(userApi + HCS_LOGIN_URL), httpEntity, Token.class);
            Token token = Objects.requireNonNull(responseEntity).getBody();
            header.set(AUTH_TOKEN, Objects.requireNonNull(token).getToken());
            SecurityPolicyBo secBo = securityPolicyService.getSecurityPolicy(new HttpEntity<>(header));
            SessionInfo session = sessionService.genSession(
                sessionService.getUserCache(Objects.requireNonNull(token).getUserId()), secBo, token);
            setHcsLoginCookie(session);
            log.info("Obtaining the hcs user Token succeeded.");
        } catch (RestClientException e) {
            log.error("Hcs login error", ExceptionUtil.getErrorMessage(e));
            throw e;
        }
    }

    private void setHcsLoginCookie(SessionInfo sessionInfo) {
        ResponseCookie sessionCookie = ResponseCookie.from(ConfigConstant.SESSION, sessionInfo.getSessionId())
            .httpOnly(true)
            .secure(true)
            .path(ConfigConstant.SEPARATE)
            .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
            .build();
        response.setHeader(HttpHeaders.SET_COOKIE, sessionCookie.toString());
        Cookie csrfCookie = new Cookie(ConfigConstant.CSRF_COOKIE_NAME, sessionInfo.getCsrfToken());
        csrfCookie.setPath(ConfigConstant.SEPARATE);
        csrfCookie.setHttpOnly(false);
        csrfCookie.setSecure(true);
        response.addCookie(csrfCookie);
        Cookie userCookie = new Cookie(ConfigConstant.USER_ID, sessionInfo.getUserId());
        userCookie.setPath(ConfigConstant.SEPARATE);
        userCookie.setHttpOnly(false);
        userCookie.setSecure(true);
        response.addCookie(userCookie);
        Cookie userTypeCookie = new Cookie(USER_TYPE, "HCS");
        userTypeCookie.setPath(ConfigConstant.SEPARATE);
        userTypeCookie.setHttpOnly(false);
        userTypeCookie.setSecure(true);
        response.addCookie(userTypeCookie);
    }

    /**
     * dme 用户登录处理
     */
    @Override
    public void dmeLogin() {
        try {
            HttpHeaders header = new HttpHeaders();
            header.add(AUTH_TOKEN, request.getHeader(AUTH_TOKEN));
            header.add(ConfigConstant.DME_FLAG, request.getHeader(ConfigConstant.DME_FLAG));
            header.add(ConfigConstant.CLUSTER_TYPE, ConfigConstant.LOCAL_CLUSTER_TYPE);
            String requestIp = RequestUtil.getClientIpAddress(request);
            header.add(ConfigConstant.REQUEST_IP, Normalizer.normalize(requestIp, Normalizer.Form.NFKC));
            HttpEntity<Object> httpEntity = new HttpEntity<>(header);
            ResponseEntity<Token> responseEntity = userRestTemplate.postForEntity(
                    NormalizerUtil.normalizeForString(userApi + DME_LOGIN_URL), httpEntity, Token.class);
            Token token = Objects.requireNonNull(responseEntity).getBody();
            header.set(AUTH_TOKEN, Objects.requireNonNull(token).getToken());
            SecurityPolicyBo secBo = securityPolicyService.getSecurityPolicy(new HttpEntity<>(header));
            SessionInfo session = sessionService.genSession(
                    sessionService.getUserCache(Objects.requireNonNull(token).getUserId()), secBo, token);
            setDmeLoginCookie(session);
            log.info("Obtaining the dme user Token succeeded.");
        } catch (RestClientException e) {
            log.error("DME login error", ExceptionUtil.getErrorMessage(e));
            throw e;
        }
    }

    private void setDmeLoginCookie(SessionInfo sessionInfo) {
        ResponseCookie sessionCookie = ResponseCookie.from(ConfigConstant.SESSION, sessionInfo.getSessionId())
                .httpOnly(true)
                .secure(true)
                .path(ConfigConstant.SEPARATE)
                .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
                .build();
        response.setHeader(HttpHeaders.SET_COOKIE, sessionCookie.toString());
        Cookie csrfCookie = new Cookie(ConfigConstant.CSRF_COOKIE_NAME, sessionInfo.getCsrfToken());
        csrfCookie.setPath(ConfigConstant.SEPARATE);
        csrfCookie.setHttpOnly(false);
        csrfCookie.setSecure(true);
        response.addCookie(csrfCookie);
        Cookie userCookie = new Cookie(ConfigConstant.USER_ID, sessionInfo.getUserId());
        userCookie.setPath(ConfigConstant.SEPARATE);
        userCookie.setHttpOnly(false);
        userCookie.setSecure(true);
        response.addCookie(userCookie);
        Cookie userTypeCookie = new Cookie(USER_TYPE, "DME");
        userTypeCookie.setPath(ConfigConstant.SEPARATE);
        userTypeCookie.setHttpOnly(false);
        userTypeCookie.setSecure(true);
        response.addCookie(userTypeCookie);
    }

    private void setSamlLoginCookie(SessionInfo session, HttpServletResponse response) {
        ResponseCookie responseCookie = ResponseCookie.from(ConfigConstant.SESSION, session.getSessionId())
            .httpOnly(true)
            .secure(true)
            .path(ConfigConstant.COOKIE_PATH)
            .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
            .build();
        response.addHeader(HttpHeaders.SET_COOKIE, responseCookie.toString());
        ResponseCookie userIdCookie = ResponseCookie.from("userId", session.getUserId())
            .httpOnly(false)
            .secure(true)
            .path(ConfigConstant.COOKIE_PATH)
            .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
            .build();
        response.addHeader(HttpHeaders.SET_COOKIE, userIdCookie.toString());
        ResponseCookie csrfCookie = ResponseCookie.from(ConfigConstant.CSRF_COOKIE_NAME, session.getCsrfToken())
            .httpOnly(false)
            .secure(true)
            .path(ConfigConstant.COOKIE_PATH)
            .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
            .build();
        response.addHeader(HttpHeaders.SET_COOKIE, csrfCookie.toString());
    }

    private HttpHeaders buildForwardHeader(HttpServletRequest httpServletRequest) {
        HttpHeaders headers = new HttpHeaders();
        Enumeration<String> headerNameEnumeration = httpServletRequest.getHeaderNames();
        while (headerNameEnumeration.hasMoreElements()) {
            String headerName = headerNameEnumeration.nextElement();
            Enumeration<String> headerValueEnumeration = httpServletRequest.getHeaders(headerName);
            while (headerValueEnumeration.hasMoreElements()) {
                headers.add(headerName, headerValueEnumeration.nextElement());
            }
        }
        return headers;
    }

    private void addSessionToCookie(SessionInfo sessionInfo) {
        ResponseCookie responseCookie = ResponseCookie.from(ConfigConstant.SESSION, sessionInfo.getSessionId())
            .httpOnly(true)
            .secure(true)
            .path(ConfigConstant.COOKIE_PATH)
            .sameSite(ConfigConstant.SAME_SITE_STRATEGY)
            .build();
        response.setHeader(HttpHeaders.SET_COOKIE, responseCookie.toString());
    }

    /**
     * 将csrf token保存到cookie中
     *
     * @param token csrf token
     */
    private void addCsrfTokenToCookie(String token) {
        Cookie cookie = new Cookie(ConfigConstant.CSRF_COOKIE_NAME, token);
        cookie.setPath(ConfigConstant.COOKIE_PATH);
        cookie.setHttpOnly(false);
        cookie.setSecure(true);
        response.addCookie(cookie);
    }
}