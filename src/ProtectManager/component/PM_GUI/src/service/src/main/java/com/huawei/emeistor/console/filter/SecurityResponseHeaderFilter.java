/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.filter;

import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 回复添加统一的header
 *
 * @author t00482481
 * @since 2020-9-06
 */
public class SecurityResponseHeaderFilter extends OncePerRequestFilter {
    /**
     * Content-Security-Policy
     */
    private static final String CONTENT_SECURITY_POLICY =
        "connect-src 'self';object-src 'self';frame-src 'self';media-src 'self';font-src 'self';frame-ancestors 'self';"
            + "script-src 'self'";

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
        throws ServletException, IOException {
        // 设置缓存(禁止带有敏感数据的Web页面缓存)
        response.setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response.setHeader("Pragma", "no-cache");
        response.setDateHeader("Expires", 0);
        response.setHeader("Content-Security-Policy", CONTENT_SECURITY_POLICY);
        response.setHeader("Referrer-Policy", "no-referrer");

        // 安全响应头设置
        response.setHeader("X-Frame-Options", "SAMEORIGIN");
        response.setHeader("X-Content-Type-Options", "nosniff");
        response.setHeader("X-XSS-Protection", "1; mode=block");
        response.setHeader("Strict-Transport-Security", "max-age=31536000;includeSubDomains");
        filterChain.doFilter(request, response);
    }
}