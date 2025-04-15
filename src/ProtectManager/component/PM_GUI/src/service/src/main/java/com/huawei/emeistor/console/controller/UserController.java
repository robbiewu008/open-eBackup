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
package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.bean.SecurityPolicyBo;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.controller.request.SendDynamicCodeRequest;
import com.huawei.emeistor.console.controller.response.LoginResponse;
import com.huawei.emeistor.console.controller.response.PageListResponse;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.RsaService;
import com.huawei.emeistor.console.service.SecurityPolicyService;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.service.UserService;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpMethod;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.HttpStatusCodeException;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;
import javax.validation.Valid;

/**
 * 用户的接口转发
 *
 * @see [相关类/方法]
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
@Slf4j
public class UserController extends AdvBaseController {
    private static final String SESSION_PREFIX = "userId=";

    private static final String USER_LOCK = "/v1/users/{userID}/action/lock";

    private static final String USER_LOGOUT = "/v1/users/{logoutType}/action/logout";

    private static final String USER_UNLOCK = "/v1/users/{userID}/action/unlock";

    private static final String USER_DELETE = "/v1/users/{userId}";

    private static final String LIST_USERS = "/v1/users?filter={filter}&startIndex={startIndex}&"
        + "pageSize={pageSize}&orderType={orderType}&orderBy={orderBy}";

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletRequest request;

    @Value("${api.gateway.endpoint}")
    private String userApi;

    @Autowired
    private SecurityPolicyService securityPolicyService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private UserService userService;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private RsaService rsaService;

    /**
     * 登录
     *
     * @param authRequest AuthRequest
     * @return LoginResponse
     */
    @ExterAttack
    @PostMapping("/v1/auth/action/login")
    public LoginResponse login(@RequestBody @Valid AuthRequest authRequest) {
        log.info("login start");
        authRequest.setUserName(rsaService.decrypt(authRequest.getUserName()));
        authRequest.setPassword(rsaService.decrypt(authRequest.getPassword()));
        return userService.login(NormalizerUtil.normalizeForBean(authRequest, AuthRequest.class));
    }

    /**
     * 发送动态口令
     *
     * @param sendDynamicCodeRequest sendDynamicCodeRequest
     */
    @ExterAttack
    @PostMapping("/v1/auth/dynamic-code/action/send")
    public void sendDynamicCode(@RequestBody @Valid SendDynamicCodeRequest sendDynamicCodeRequest) {
        log.info("Send Dynamic code start");
        userService.sendDynamicCode(
            NormalizerUtil.normalizeForBean(sendDynamicCodeRequest, SendDynamicCodeRequest.class));
    }

    /**
     * 修改安全策略
     *
     * @param requestEntity 安全策略
     */
    @ExterAttack
    @PutMapping("/v1/security")
    public void putSecurity(RequestEntity<SecurityPolicyBo> requestEntity) {
        HttpEntity<SecurityPolicyBo> httpEntity = super.getHttpEntity(requestEntity.getBody(), request);
        securityPolicyService.updateSecurityPolicy(httpEntity);
    }

    /**
     * 登出
     *
     * @param logoutType 登出类型（MANUAL or TIMEOUT）
     */
    @ExterAttack
    @PostMapping("/v1/auth/action/logout")
    public void logout(@RequestParam("logoutType") String logoutType) {
        log.info("The system starts to exit, and logs are recorded logoutType: {} ", logoutType);
        HttpEntity<Object> httpEntity = new HttpEntity<>(requestUtil.getForwardHeaderAndValidCsrf());
        restTemplate.exchange(NormalizerUtil.normalizeForString(userApi + USER_LOGOUT), HttpMethod.GET, httpEntity,
            String.class, NormalizerUtil.normalizeForString(logoutType));
        log.info("Logging procedure succeeds and start deleting a session.");
        sessionService.deleteSessionFromReq(request, true);
    }

    /**
     * 锁定用户
     *
     * @param userId 被锁定用户的信息
     */
    @ExterAttack
    @PutMapping("/v1/users/{userId}/action/lock")
    public void lock(@PathVariable("userId") String userId) {
        HttpEntity<Object> httpEntity = new HttpEntity<>(NormalizerUtil.normalizeForString(userId),
            requestUtil.getForwardHeaderAndValidCsrf());
        restTemplate.put(NormalizerUtil.normalizeForString(userApi + USER_LOCK), httpEntity,
            NormalizerUtil.normalizeForString(userId));
        sessionService.logoutAllSession(userId);
    }

    /**
     * 解锁用户
     *
     * @param userId 用户Id
     * @param password 管理员密码
     */
    @ExterAttack
    @PutMapping("/v1/users/{userId}/action/unlock")
    public void unlock(@PathVariable("userId") String userId, @RequestBody String password) {
        try {
            HttpEntity<Object> httpEntity = new HttpEntity<>(NormalizerUtil.normalizeForString(password),
                requestUtil.getForwardHeaderAndValidCsrf());

            restTemplate.put(NormalizerUtil.normalizeForString(userApi + USER_UNLOCK), httpEntity,
                NormalizerUtil.normalizeForString(userId));
        } catch (HttpStatusCodeException e) {
            if (e.getResponseBodyAsString().contains(String.valueOf(CommonErrorCode.REACH_MAX_RESET_LIMIT))) {
                log.error("failed to unlock user, delete session cookie, userId: {}.", userId);
                sessionService.deleteSessionFromReq(request, true);
            }
            throw e;
        }
    }

    /**
     * 获取用户列表
     *
     * @param requestEntity 请求体
     * @return 用户列表
     */
    @ExterAttack
    @GetMapping("/v1/users")
    public PageListResponse getAllUser(RequestEntity<Object> requestEntity) {
        HttpEntity<Object> httpEntity = new HttpEntity<>(requestEntity.getBody(),
            requestUtil.getForwardHeaderAndValidCsrf());
        String filter = NormalizerUtil.normalizeForString(request.getParameter("filter"));
        String orderType = NormalizerUtil.normalizeForString(request.getParameter("orderType"));
        String orderBy = NormalizerUtil.normalizeForString(request.getParameter("orderBy"));
        String startIndex = NormalizerUtil.normalizeForString(request.getParameter("startIndex"));
        String pageSize = NormalizerUtil.normalizeForString(request.getParameter("pageSize"));
        ResponseEntity<PageListResponse> res = restTemplate.exchange(
            NormalizerUtil.normalizeForString(userApi + LIST_USERS), HttpMethod.GET, httpEntity, PageListResponse.class,
            filter, startIndex, pageSize, orderType, orderBy);
        List<Map<String, Object>> userList = Objects.requireNonNull(res.getBody()).getUserList();
        List<String> onlineUserList = sessionService.getOnlineUserList();
        for (Map<String, Object> user : userList) {
            if (onlineUserList.contains(SESSION_PREFIX + user.get("userId"))) {
                user.put("login", true);
            }
        }
        return res.getBody();
    }

    /**
     * hcs 用户登录
     */
    @ExterAttack
    @PostMapping("/v1/auth/hcs/login")
    @ResponseBody
    public void hcsLogin() {
        log.info("hcs login start");
        userService.hcsLogin();
    }

    /**
     * dme 用户登录
     */
    @ExterAttack
    @PostMapping("/v1/auth/dme/login")
    @ResponseBody
    public void dmeLogin() {
        log.info("dme login start");
        userService.dmeLogin();
    }

    /**
     * 删除用户
     *
     * @param userId 用户id
     * @param requestEntity 请求体
     */
    @ExterAttack
    @DeleteMapping("/v1/users/{userId}")
    public void deleteUser(@PathVariable("userId") String userId, RequestEntity<Object> requestEntity) {
        String query = requestEntity.getUrl().getQuery();
        String url = userApi + request.getRequestURI().replaceFirst(ConfigConstant.CONSOLE, "");
        if (query != null) {
            url = url + "?" + requestEntity.getUrl().getRawQuery();
        }
        URI uri;
        try {
            uri = new URL(NormalizerUtil.normalizeForString(url)).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        HttpEntity<Object> httpEntity = new HttpEntity<>(requestEntity.getBody(),
            requestUtil.getForwardHeaderAndValidCsrf());
        restTemplate.exchange(uri, Objects.requireNonNull(requestEntity.getMethod()), httpEntity, Object.class);
        sessionService.logoutAllSession(userId);
    }
}
