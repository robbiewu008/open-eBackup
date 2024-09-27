/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.web.filter.OncePerRequestFilter;
import org.springframework.web.util.WebUtils;

import java.io.IOException;
import java.util.List;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * CSRF 防御Filter
 *
 * @author l00422407
 * @since 2021-03-30
 */
@Slf4j
public class CsrfWebFilter extends OncePerRequestFilter {
    @Value("#{'${security.csrf.ignore.path}'.split(',')}")
    private List<String> csrfIgnorePaths;

    @Autowired
    private RequestUtil requestUtil;

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
        throws ServletException, IOException {
        // 白名单放过
        if (csrfIgnorePaths.contains(request.getRequestURI())) {
            filterChain.doFilter(request, response);
            return;
        }

        // 校验CSRF TOKEN
        String headerToken = request.getHeader(ConfigConstant.HEADER_NAME);
        String token = loadTokenFromCookie(request);
        SessionInfo sessionInfo = requestUtil.getSessionInfo();
        if (!isaMatchCsrfToken(headerToken, token, sessionInfo)) {
            log.error("Csrf token check failed! url: {}", request.getRequestURI());
            response.setStatus(HttpStatus.FOUND.value());
            return;
        }
        filterChain.doFilter(request, response);
    }

    private boolean isaMatchCsrfToken(String headerToken, String token, SessionInfo sessionInfo) {
        if (sessionInfo == null) {
            log.error("sessionInfo is null");
            return false;
        }
        if (StringUtils.isEmpty(headerToken) || StringUtils.isEmpty(token)) {
            log.error("headerToken or token is null");
            return false;
        }
        if (!StringUtils.equals(headerToken, token) || !StringUtils.equals(headerToken, sessionInfo.getCsrfToken())) {
            log.error("headerToken token csrfToken is not equal.");
            return false;
        }
        return true;
    }

    /**
     * 从cookie中加载csrf token
     *
     * @param request HttpServletRequest
     * @return String
     */
    private String loadTokenFromCookie(HttpServletRequest request) {
        Cookie cookie = WebUtils.getCookie(request, ConfigConstant.CSRF_COOKIE_NAME);
        if (cookie == null || StringUtils.isEmpty(cookie.getValue())) {
            log.error("cookie is null");
            return StringUtils.EMPTY;
        }

        return cookie.getValue();
    }
}
