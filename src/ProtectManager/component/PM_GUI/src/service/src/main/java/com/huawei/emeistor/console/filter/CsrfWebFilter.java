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
package com.huawei.emeistor.console.filter;

import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.web.filter.OncePerRequestFilter;
import org.springframework.web.util.WebUtils;

import java.io.IOException;
import java.util.List;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * CSRF 防御Filter
 *
 */
@Slf4j
public class CsrfWebFilter extends OncePerRequestFilter {
    @Value("#{'${security.csrf.ignore.path}'.split(',')}")
    private List<String> csrfIgnorePaths;

    @Value("#{'${security.csrf.download.ignore}'.split(',')}")
    private List<String> downloadPaths;

    private List<Pattern> downloadPathsPattern;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    @Override
    protected void initFilterBean() throws ServletException {
        super.initFilterBean();
        downloadPathsPattern = downloadPaths.stream().map(Pattern::compile).collect(Collectors.toList());
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
        throws ServletException, IOException {
        // 白名单放过
        if (csrfIgnorePaths.contains(request.getRequestURI())) {
            filterChain.doFilter(request, response);
            return;
        }

        // 日志下载切换到标准的浏览器下载是无法携带自定义header的，和安全蓝军确认过，GET请求不需要强制带csrf token，因此这里配置白名单
        if (isAllowedDownloadPath(request.getRequestURI(), request.getMethod())) {
            filterChain.doFilter(request, response);
            return;
        }

        // 校验CSRF TOKEN
        String headerToken = request.getHeader(ConfigConstant.HEADER_NAME);
        String token = loadTokenFromCookie(request);
        SessionInfo sessionInfo = requestUtil.getSessionInfo();
        // csrfToken是缓存中获取，需要解密
        String decryptCsrfToken = encryptorRestClient.decrypt(sessionInfo.getCsrfToken()).getPlaintext();
        sessionInfo.setCsrfToken(decryptCsrfToken);
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

    /**
     * 判断是否下载日志/文件类的白名单请求，通过正则的方式匹配url，以及请求方法
     *
     * @param uri 请求的uri
     * @param method 请求方法
     * @return 是否属于下载的白名单
     */
    protected boolean isAllowedDownloadPath(String uri, String method) {
        if (!HttpMethod.GET.name().equals(method)) {
            return false;
        }
        return downloadPathsPattern.stream().anyMatch(pattern -> pattern.matcher(uri).matches());
    }
}
