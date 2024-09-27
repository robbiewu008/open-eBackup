/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;

import java.text.Normalizer;
import java.util.Optional;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

/**
 * 集成公共的Controller方法
 *
 * @author w00493811
 * @since 2021-01-20
 */
public class AdvBaseController {
    private static final int DEFAULT_REQUEST_ID_MAX_SIZE = 1024;

    private static final int DEFAULT_COOKIES_MAX_SIZE = 1024;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    /**
     * 会话服务
     */
    @Autowired
    private SessionService sessionService;

    /**
     * 过滤不安全的特殊字符
     *
     * @param item item
     * @return String
     */
    protected String normalizeForString(String item) {
        if (StringUtils.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
    }

    /**
     * 转化 requestEntity 为 HttpEntity
     *
     * @param body 请求Body
     * @param request 请求体
     * @param <T> 类型
     * @return HttpEntity
     */
    @ExterAttack
    protected <T> HttpEntity<T> getHttpEntity(T body, HttpServletRequest request) {
        // 构造新的http头部
        HttpHeaders newHeader = new HttpHeaders();
        newHeader.add(ConfigConstant.CLUSTER_TYPE, request.getHeader(ConfigConstant.CLUSTER_TYPE));
        newHeader.add(ConfigConstant.CLUSTER_ID, request.getHeader(ConfigConstant.CLUSTER_ID));
        if (StringUtils.isNotBlank(request.getHeader(ConfigConstant.MEMBER_ESN))) {
            newHeader.add(ConfigConstant.MEMBER_ESN, request.getHeader(ConfigConstant.MEMBER_ESN));
        }
        // 请求ID
        String requestId = request.getHeader(ConfigConstant.REQUEST_ID);

        // 防止requestId过大
        if (requestId != null && requestId.length() < DEFAULT_REQUEST_ID_MAX_SIZE) {
            newHeader.add(ConfigConstant.REQUEST_ID, Normalizer.normalize(requestId, Normalizer.Form.NFKC));
        }

        // 会话Token
        Cookie[] cookies = request.getCookies();

        // 防止cookies过大
        if (cookies != null && cookies.length < DEFAULT_COOKIES_MAX_SIZE) {
            String session = "";
            for (Cookie cookie : cookies) {
                if (ConfigConstant.SESSION.equals(cookie.getName())) {
                    session = cookie.getValue();
                    break;
                }
            }

            // session不为空的情况下，会进行查询并设置对应的值
            if (StringUtils.isNotEmpty(session)) {
                Optional.ofNullable(sessionService.getSessionInfo(session).getToken())
                    .ifPresent(token -> newHeader.add(ConfigConstant.TOKEN,
                        encryptorRestClient.decrypt(token).getPlaintext()));
            }
        }

        // 请求IP地址，否则操作日志IP地址错误
        String ipAddress = RequestUtil.getClientIpAddress(request);
        newHeader.add(ConfigConstant.REQUEST_IP, normalizeForString(ipAddress));
        return new HttpEntity<>(body, newHeader);
    }
}
